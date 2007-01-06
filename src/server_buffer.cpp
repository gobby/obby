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

void obby::server_buffer::insert(const position& pos, const std::string& text)
{
	// TODO: Insert to buffer
	// TODO: Add to history
	// TODO: Tell clients new revision
}

void obby::server_buffer::erase(const position& from, const position& to)
{
	// TODO: Delete from buffer
	// TODO: Add to history
	// TODO: Tell clients new revision
}

obby::server_buffer::signal_insert_type
obby::server_buffer::insert_event() const
{
	return m_signal_insert;
}

obby::server_buffer::signal_delete_type
obby::server_buffer::delete_event() const
{
	return m_signal_delete;
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
		record* rec = record::from_packet(pack);
//		std::cout << "Got record from packet: " << rec << std::endl;
		// TODO: Ensure that rec exists

		// Undo all changes until the revision of rec has been reached
		// TODO: Ensure that the wished revision exists.
		std::list<record*>::iterator iter;
		for(iter = m_history.begin(); iter != m_history.end(); ++ iter)
		{
			// Break if wished revision has been reached
			if((*iter)->get_revision() == rec->get_revision() )
				break;
	
			record* rev_iter = (*iter)->reverse(*this);
			rev_iter->apply(*this);
			delete rev_iter;

			// Apply rec on the current record in history to put
			// it to the right direction
			rec->apply(**iter);
		}

		// Apply record.
		rec->apply(*this);

		// Redo the changes to reach the current revision
		while(iter != m_history.begin() )
		{
			-- iter;

			// Apply the current change to the new record
			(*iter)->apply(*rec);

			// Re-apply this revision to the buffer
			(*iter)->apply(*this);
		} 

		// Increment revision
		rec->set_revision(++ m_revision);

		// Set correct sender
		rec->set_from(peer.get_id() );

		// Add change to history
		m_history.push_front(rec);

		// Tell clients
		m_server.send(rec->to_packet() );

		// Emit changed signal
		// HACKHACKHACK :D
		if(pack.get_param(0).as_string() == "insert")
			m_signal_insert.emit(
				*static_cast<insert_record*>(rec) );
		if(pack.get_param(0).as_string() == "delete")
			m_signal_delete.emit(
				*static_cast<delete_record*>(rec) );
	}
}

