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

#ifndef _OBBY_USER_HPP_
#define _OBBY_USER_HPP_

#include <string>
#include <net6/address.hpp>
#include <net6/peer.hpp>

namespace obby
{

/** User in a obby session
 */
	
class user
{
public: 
	/** Creates a new user with underlaying net6 peer object and a color
	 * whose components should be between 0 and 255.
	 */
	user(const net6::peer& peer, int red, int green, int blue);
	~user();

	/** Returns the user name of this user.
	 */
	const std::string& get_name() const;

	/** Returns the address of this user.
	 */
	const net6::address& get_address() const;

	/** Returns a unique ID for this user.
	 */
	unsigned int get_id() const;

	/** Returns the red component of the user color.
	 */
	int get_red() const;

	/** Returns the green component of the user color.
	 */
	int get_green() const;

	/** Returns the blue component of the user color.
	 */
	int get_blue() const;
protected:
	const net6::peer& m_peer;

	int m_red;
	int m_green;
	int m_blue;
};
	
}

#endif // _OBBY_USER_HPP_

