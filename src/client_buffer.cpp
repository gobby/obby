/* libobby - Network text editing library
 * Copyright (C) 2005 0x539 dev group
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this program; if not, write to the Free
 * Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include <cassert>
#include "client_buffer.hpp"

obby::client_buffer::client_buffer(const std::string& hostname,
                                   unsigned int port)
 : buffer(), m_unsynced(),
   m_connection(net6::ipv4_address::create_from_hostname(hostname, port) )
{
	m_connection.join_event().connect(
		sigc::mem_fun(*this, &client_buffer::on_join) );
	m_connection.part_event().connect(
		sigc::mem_fun(*this, &client_buffer::on_part) );
	m_connection.close_event().connect(
		sigc::mem_fun(*this, &client_buffer::on_close) );
	m_connection.data_event().connect(
		sigc::mem_fun(*this, &client_buffer::on_data) );
	m_connection.login_failed_event().connect(
		sigc::mem_fun(*this, &client_buffer::on_login_failed) );
}

obby::client_buffer::~client_buffer()
{
	std::list<record*>::iterator iter;
	for(iter = m_unsynced.begin(); iter != m_unsynced.end(); ++ iter)
		delete *iter;
}

void obby::client_buffer::login(const std::string& name)
{
	m_connection.login(name);
}

net6::client::peer* obby::client_buffer::get_self() const
{
	return m_connection.get_self();
}

void obby::client_buffer::select()
{
	m_connection.select();
}

void obby::client_buffer::select(unsigned int timeout)
{
	m_connection.select(timeout);
}

void obby::client_buffer::insert(position pos, const std::string& text)
{
	// Insert into buffer
	insert_nosync(pos, text);
	// Add to unsynced changes
	record* rec = new insert_record(pos, text, m_revision,
	                                m_connection.get_self()->get_id() );
	// Apply new change to other unsynced changes
	std::list<record*>::iterator iter;
	for(iter = m_unsynced.begin(); iter != m_unsynced.end(); )
	{
		rec->apply(**iter);

		// Remove this change if it gets invalid
		if(!(*iter)->is_valid() )
		{
			delete *iter;
			iter = m_unsynced.erase(iter);
		}
		else
			++ iter;
	}
	// Put new change to unsynced changes
	m_unsynced.push_front(rec);
	// Send sync request to server
	m_connection.send(rec->to_packet() );
}

void obby::client_buffer::erase(position from, position to)
{
	// Get text that will be erased
	std::string text = m_buffer.substr(from, to - from);
	// Delete from buffer
	erase_nosync(from, to);
	// Add to unsynced changes
	record* rec = new delete_record(from, text, m_revision,
	                               m_connection.get_self()->get_id() );
	// Apply new change to other unsynced changes
	std::list<record*>::iterator iter;
	for(iter = m_unsynced.begin(); iter != m_unsynced.end(); )
	{
		rec->apply(**iter);

		// Remove this change if it gets invalid
		if(!(*iter)->is_valid() )
		{
			delete *iter;
			iter = m_unsynced.erase(iter);
		}
		else
			++ iter;
	}
	// Put the new change into the list of unsynced changes
	m_unsynced.push_front(rec);
	// Send sync request to server
	m_connection.send(rec->to_packet() );
}

obby::client_buffer::signal_join_type obby::client_buffer::join_event() const
{
	return m_signal_join;
}

obby::client_buffer::signal_sync_type obby::client_buffer::sync_event() const
{
	return m_signal_sync;
}

obby::client_buffer::signal_part_type obby::client_buffer::part_event() const
{
	return m_signal_part;
}

obby::client_buffer::signal_close_type obby::client_buffer::close_event() const
{
	return m_signal_close;
}

obby::client_buffer::signal_login_failed_type
obby::client_buffer::login_failed_event() const
{
	return m_signal_login_failed;
}

void obby::client_buffer::on_join(net6::client::peer& peer)
{
	m_signal_join.emit(peer);
}

void obby::client_buffer::on_part(net6::client::peer& peer)
{
	m_signal_part.emit(peer);
}

void obby::client_buffer::on_close()
{
	m_signal_close.emit();
}

void obby::client_buffer::on_data(const net6::packet& pack)
{
	if(pack.get_command() == "obby_record")
		on_net_record(pack);
	if(pack.get_command() == "obby_sync_init")
		on_net_sync_init(pack);
	if(pack.get_command() == "obby_sync_line")
		on_net_sync_line(pack);
	if(pack.get_command() == "obby_sync_final")
		on_net_sync_final(pack);
}

void obby::client_buffer::on_login_failed(const std::string& reason)
{
	m_signal_login_failed.emit(reason);
}

void obby::client_buffer::on_net_record(const net6::packet& pack)
{
	// Convert network packet to record
	record* rec = record::from_packet(pack);
	if(!rec) return;

	// Apply the record to the history
	m_history.push_front(rec->clone() );

	// Apply unsynced changes on new record
	std::list<record*>::iterator iter;
	for(iter = m_unsynced.begin(); iter != m_unsynced.end(); ++ iter)
		(*iter)->apply(*rec);

	// Is the record still valid?
	if(!rec->is_valid() )
		return;

	// Record in unsynced changes that is synced with this packet
	record* sync_record = NULL;

	// Apply record on unsynced changes
	while(iter != m_unsynced.begin() )
	{
		-- iter;
		rec->apply(**iter);

		if( (*iter)->get_from() == rec->get_from() )
			if( (*iter)->get_id() == rec->get_id() )
				sync_record = *iter;

		// Did the new change invalidate an unsynced one?
		if(!(*iter)->is_valid() )
		{
			assert( (*iter) != sync_record);

			// Remove it from the list of unsynced changes
			delete *iter;
			iter = m_unsynced.erase(iter);
		}
	}

	// Remove unsynced change in order to be synced now
	if(sync_record)
	{
		// Reverse the sync_record to undo it.
		record* rev_sync = sync_record->reverse(*this);

		// Apply the undo operations on other unsynced changes
		std::list<record*>::iterator iter;
		for(iter = m_unsynced.begin(); iter != m_unsynced.end(); )
		{
			if(*iter == sync_record)
				iter = m_unsynced.erase(iter);
			else
			{
				rev_sync->apply(**iter);
				++iter;
			}
		}

		// The unsynced change itself is no longer needed
		delete sync_record;

		// Apply undo of the unsynced change
		rev_sync->apply(*rec);
		rev_sync->apply(*this);

		rev_sync->emit_buffer_signal(*this);
		delete rev_sync;
	}

	// Apply record
	rec->apply(*this);

	// Update revision number
	m_revision = rec->get_revision();

	// Emit changed signal
	rec->emit_buffer_signal(*this);

	// rec is no longer needed, a copy has been put into the history
	delete rec;
}

void obby::client_buffer::on_net_sync_init(const net6::packet& pack)
{
	if(pack.get_param_count() < 1) return;
	if(pack.get_param(0).get_type() != net6::packet::param::INT) return;

	m_revision = pack.get_param(0).as_int();
	m_buffer = "";
}

void obby::client_buffer::on_net_sync_line(const net6::packet& pack)
{
	if(pack.get_param_count() < 1) return;
	if(pack.get_param(0).get_type() != net6::packet::param::STRING) return;

	if(!m_buffer.empty() )
	{
		m_lines.push_back(m_buffer.length() );
		m_buffer += "\n";
	}

	m_buffer += pack.get_param(0).as_string();
}

void obby::client_buffer::on_net_sync_final(const net6::packet& pack)
{
	m_signal_sync.emit();
}

