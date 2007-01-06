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

#include "server_buffer.hpp"

obby::server_buffer::server_buffer(unsigned int port)
 : buffer(), m_server(port, false)
{
	m_server.join_event().connect(
		sigc::mem_fun(*this, &server_buffer::on_join) );
	m_server.login_event().connect(
		sigc::mem_fun(*this, &server_buffer::on_login) );
	m_server.part_event().connect(
		sigc::mem_fun(*this, &server_buffer::on_part) );
	m_server.data_event().connect(
		sigc::mem_fun(*this, &server_buffer::on_data) );
}

obby::server_buffer::~server_buffer()
{
}

void obby::server_buffer::select()
{
	m_server.select();
}

void obby::server_buffer::select(unsigned int timeout)
{
	m_server.select(timeout);
}

void obby::server_buffer::insert(position pos, const std::string& text)
{
	// TODO: Insert to buffer
	// TODO: Add to history
	// TODO: Tell clients new revision
}

void obby::server_buffer::erase(position from, position to)
{
	// TODO: Delete from buffer
	// TODO: Add to history
	// TODO: Tell clients new revision
}

obby::server_buffer::signal_join_type obby::server_buffer::join_event() const
{
	return m_signal_join;
}

obby::server_buffer::signal_login_type obby::server_buffer::login_event() const
{
	return m_signal_login;
}

obby::server_buffer::signal_part_type obby::server_buffer::part_event() const
{
	return m_signal_part;
}

void obby::server_buffer::on_join(net6::server::peer& peer)
{
	m_signal_join.emit(peer);
}

void obby::server_buffer::on_login(net6::server::peer& peer)
{
	// Client logged in. Synchronise the complete buffer, but
	// seperate it into multiple packets to not block other high-priority
	// network packets like chat packets.
	net6::packet init_sync("obby_sync_init");
	init_sync << static_cast<int>(m_revision);
	m_server.send(init_sync, peer);

	std::string::size_type pos = 0, prev = 0;
	while( (pos = m_buffer.find('\n')) != std::string::npos)
	{
		net6::packet line_sync("obby_sync_line");
		line_sync << m_buffer.substr(pos, prev - pos);
		m_server.send(line_sync, peer);
		prev = ++ pos;
	}

	net6::packet line_sync("obby_sync_line");
	line_sync << m_buffer.substr(prev);
	m_server.send(line_sync, peer);

	net6::packet final_sync("obby_sync_final");
	m_server.send(final_sync, peer);

	m_signal_login.emit(peer);
}

void obby::server_buffer::on_part(net6::server::peer& peer)
{
	m_signal_part.emit(peer);
}

void obby::server_buffer::on_data(const net6::packet& pack,
                                  net6::server::peer& peer)
{
	if(pack.get_command() == "obby_record")
	{
		// Create record from packet
		record* rec = record::from_packet(pack);
		if(!rec) return;

		// Look for wished revision
		std::list<record*>::iterator iter;
		for(iter = m_history.begin(); iter != m_history.end(); ++ iter)
			if( (*iter)->get_revision() == rec->get_revision() )
				break;

		// Wished Revision does not exist
		if(iter == m_history.end() && rec->get_revision() != 0)
			return;

		// Apply newer Revision on the new record
		while(iter != m_history.begin() )
		{
			-- iter;

			if( (*iter)->get_from() != rec->get_from() )
				(*iter)->apply(*rec);
		}

		// Ignore record if it got invalid
		if(!rec->is_valid() )
		{
			delete rec;
			return;
		}

		// Apply record on buffer
		rec->apply(*this);

		// Increment revision
		rec->set_revision(++ m_revision);

		// Set correct sender
		rec->set_from(peer.get_id() );

		// Add change to history
		m_history.push_front(rec);

		// Tell clients
		m_server.send(rec->to_packet() );

		// Emit changed signal
		rec->emit_buffer_signal(*this);
	}
}

