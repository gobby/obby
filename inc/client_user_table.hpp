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

#ifndef _OBBY_CLIENT_USER_TABLE_HPP_
#define _OBBY_CLIENT_USER_TABLE_HPP_

#include <net6/packet.hpp>
#include <net6/client.hpp>
#include "user_table.hpp"

namespace obby
{

class client_buffer;

/** User table to store user information through multiple obby sessions or
 * to store user colourising even if a user has lost the connection.
 */
	
class client_user_table : public user_table
{
public:	
	client_user_table(net6::client& client, const client_buffer& buffer);
	~client_user_table();

	/** Called by a client_buffer when a user table is synced.
	 */
	void on_net_sync_init(const net6::packet& pack);

	/** Called by a client_buffer when a user table is synced.
	 */
	void on_net_sync_record(const net6::packet& pack);

	/** Called by a client_buffer when a user table is synced.
	 */
	void on_net_sync_final(const net6::packet& pack);
protected:
	net6::client& m_client;
	const client_buffer& m_buffer;
};

}

#endif // _OBBY_CLIENT_USER_TABLE_HPP_

