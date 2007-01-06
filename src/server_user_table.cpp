/* libobby - Network text editing library
 * Copyright (C) 2005 0x539 dev group
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this program; if not, write to the Free
 * Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include "server_user_table.hpp"

obby::server_user_table::server_user_table(net6::server& server,
                                           const server_buffer& buffer)
 : user_table(), m_server(server), m_buffer(buffer)
{
	// Add obby server user to user table for changes from server
	m_users.push_back(user(0, "obby server", 255, 255, 255) );
}

obby::server_user_table::~server_user_table()
{
}

void obby::server_user_table::synchronise(net6::server::peer& peer)
{
	// Send init packet
	net6::packet init_pack("obby_sync_usertable_init");
	init_pack << m_id_counter;
	m_server.send(init_pack, peer);

	// Send usertable record for each item in the list
	std::list<user>::iterator iter;
	for(iter = m_users.begin(); iter != m_users.end(); ++ iter)
	{
		net6::packet pack("obby_sync_usertable_record");
		if(iter->get_user() )
		{
			// Send just user id if the user still exists
			pack << iter->get_id() << iter->get_user()->get_id();
		}
		else
		{
			// Otherwise, send name and color
			pack << iter->get_id() << iter->get_name()
			     << iter->get_red() << iter->get_green()
			     << iter->get_blue();
		}
		m_server.send(pack, peer);
	}
	
	// Send final sync
	net6::packet final_pack("obby_sync_usertable_final");
	m_server.send(final_pack, peer);
}

