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

#ifndef _OBBY_SERVER_USER_TABLE_HPP_
#define _OBBY_SERVER_USER_TABLE_HPP_

#include <net6/server.hpp>
#include "user_table.hpp"

namespace obby
{

class server_buffer;
	
/** User table to store user information through multiple obby sessions or
 * to store user colourising even if a user has lost the connection.
 */

class server_user_table : public user_table
{
public:	
	server_user_table(net6::server& server, const server_buffer& buffer);
	~server_user_table();

	/** Synchronize a user table with a client.
	 */
	void synchronise(net6::server::peer& peer);
protected:
	net6::server& m_server;
	const server_buffer& m_buffer;
};

}

#endif // _OBBY_SERVER_USER_TABLE_HPP_

