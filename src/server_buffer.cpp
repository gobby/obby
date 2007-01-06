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
 : buffer(), m_counter(0), m_server(port)
{
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

