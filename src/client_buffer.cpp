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

obby::client_buffer::client_buffer(const std::string& hostname, unsigned int port)
 : buffer(), m_unsynced(), m_connection(hostname, port, false)
{
}

obby::client_buffer::~client_buffer()
{
	std::list<record*>::iterator iter;
	for(iter = m_unsynced.begin(); iter != m_unsynced.end(); ++ iter)
		delete *iter;
}

obby::client_buffer::select()
{
	m_connection.select();
}

obby::client_buffer::select(unsigned int timeout)
{
	m_connection.select(timeout);
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

