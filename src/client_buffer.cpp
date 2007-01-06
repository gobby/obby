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

void obby::client_buffer::insert(const position& pos, const std::string& text)
{
	// Insert into buffer
	insert_nosync(pos, text);
	// Add to unsynced changes
	record* rec = new insert_record(pos, text, m_revision,
	                                m_connection.get_self()->get_id() );
	m_unsynced.push_front(rec);
	// Send sync request to server
	m_connection.send(rec->to_packet() );
}

void obby::client_buffer::erase(const position& from, const position& to)
{
	// Delete from buffer
	erase_nosync(from, to);
	// Add to unsynced changes
	record* rec = new delete_record(from, to, m_revision,
	                                m_connection.get_self()->get_id() );
	m_unsynced.push_front(rec);
	// Send sync request to server
	m_connection.send(rec->to_packet() );
}

obby::client_buffer::signal_insert_type
obby::client_buffer::insert_event() const
{
	return m_signal_insert;
}

obby::client_buffer::signal_delete_type
obby::client_buffer::delete_event() const
{
	return m_signal_delete;
}

obby::client_buffer::signal_join_type obby::client_buffer::join_event() const
{
	return m_signal_join;
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
	{
		record* rec = record::from_packet(pack);
		if(!rec) return;

		// Undo all non-synced changes
		std::list<record*>::iterator iter;
		for(iter = m_unsynced.begin(); iter != m_unsynced.end(); ++iter)
		{
			record* rev_iter = (*iter)->reverse(*this);
			rev_iter->apply(*this);
			delete rev_iter;

			// TODO: Check if it behaves correct if we have
			// more than one unsynced changes
			if( (*iter)->get_from() != rec->get_from() ||
			    (*iter)->get_id() != rec->get_id() )
			{
				rec->apply(**iter);
			}
		}

		// Apply new record
		rec->apply(*this);

		// Record in unsynced changes that is synced with this packet
		record* sync_record = NULL;

		// Redo all unsynced changes except the one that just came in
		while(iter != m_unsynced.begin() )
		{
			-- iter;

			// Check if this record is the one we are syncing
			if( (*iter)->get_from() == rec->get_from() )
			{
				if( (*iter)->get_id() == rec->get_id() )
				{
					// Delete unsynced record
					sync_record = (*iter)->reverse(*this);
					delete *iter;

					// Record is synced - remove from list
					iter = m_unsynced.erase(iter);
					continue;
				}
			}

			// TODO: Maybe switch these two statements, don't know :/
			// Apply the current change to the new record
			(*iter)->apply(*rec);

			// Apply the current change to the syncing record
			if(sync_record) (sync_record)->apply(**iter);

			// Reapply revision to the buffer
			(*iter)->apply(*this);
		}

		// Put the synced record into history
		m_history.push_front(rec);

		// Update revision
		m_revision = rec->get_revision();

		// TODO: Check if sync_record record changes the document
		// or if sync_record and rec result in the same document.
		// Don't emit any signals in this case.

		// Emit changed signal
		// again, HACKHACKHACK :D
		if(pack.get_param(0).as_string() == "insert")
		{
			if(sync_record)
				m_signal_delete.emit(
					*static_cast<delete_record*>(
						sync_record
					)
				);

			m_signal_insert.emit(
				*static_cast<insert_record*>(rec) );
		}
		
		if(pack.get_param(0).as_string() == "delete")
		{
			if(sync_record)
				m_signal_insert.emit(
					*static_cast<insert_record*>(
						sync_record
					)
				);

			m_signal_delete.emit(
				*static_cast<delete_record*>(rec) );
		}

		// sync_record is not needed any longer.
		delete sync_record;
	}
}

void obby::client_buffer::on_login_failed(const std::string& reason)
{
	m_signal_login_failed.emit(reason);
}

