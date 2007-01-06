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

#include <cassert>
#include "client_user_table.hpp"
#include "client_buffer.hpp"

obby::client_user_table::client_user_table(net6::client& client,
                                           const client_buffer& buffer)
 : user_table(), m_client(client), m_buffer(buffer)
{
}

obby::client_user_table::~client_user_table()
{
}

void obby::client_user_table::on_net_sync_init(const net6::packet& pack)
{
	if(pack.get_param_count() < 1) return;
	if(pack.get_param(0).get_type() != net6::packet::param::INT) return;

	m_id_counter = pack.get_param(0).as_int();
	m_users.clear();
}

void obby::client_user_table::on_net_sync_record(const net6::packet& pack)
{
	// on_net_sync_record may be called with 2 or 5 parameters.
	if(pack.get_param_count() < 3)
	{
		// This is an existing user. We get the ID for the user table
		// and the ID of the existing user.
		if(pack.get_param_count() < 2) return;
		if(pack.get_param(0).get_type() != net6::packet::param::INT)
			return;
		if(pack.get_param(1).get_type() != net6::packet::param::INT)
			return;

		unsigned int id = pack.get_param(0).as_int();
		unsigned int user_id = pack.get_param(1).as_int();

		obby::user* user = m_buffer.find_user(user_id);
		assert(user != NULL);

		m_users.push_back(user_table::user(id, *user) );
	}
	else
	{
		// This is a user that is not anymore participating in the
		// obby session. We get its ID for the user table and its name
		// and color to markup text written by him.
		if(pack.get_param_count() < 5) return;
		if(pack.get_param(0).get_type() != net6::packet::param::INT)
			return;
		if(pack.get_param(1).get_type() != net6::packet::param::STRING)
			return;
		if(pack.get_param(2).get_type() != net6::packet::param::INT)
			return;
		if(pack.get_param(3).get_type() != net6::packet::param::INT)
			return;
		if(pack.get_param(4).get_type() != net6::packet::param::INT)
			return;

		unsigned int id = pack.get_param(0).as_int();
		const std::string& name = pack.get_param(1).as_string();
		unsigned int red = pack.get_param(2).as_int();
		unsigned int green = pack.get_param(3).as_int();
		unsigned int blue = pack.get_param(4).as_int();

		m_users.push_back(user(id, name, red, green, blue) );
	}
}

void obby::client_user_table::on_net_sync_final(const net6::packet& pack)
{
	// Nothing to do.
}

