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

#ifndef _OBBY_USER_HPP_
#define _OBBY_USER_HPP_

#include <string>
#include <net6/address.hpp>
#include <net6/peer.hpp>

namespace obby
{

/** User in a obby session.
 */
	
class user
{
public:
	enum flags {
		NONE = 0x00,
		CONNECTED = 0x01
	};
	
	/** Creates a new user from an existing peer.
	 * @param peer Underlaying net6 peer object.
	 * @param red Red colour component of the user colour (0-255)
	 * @param green Green colour component of the user colour (0-255)
	 * @param blue Blue colour component of the user colour (0-255)
	 */
	user(const net6::peer& peer, int red, int green, int blue);

	/** Creates a new user that represents a peer that has already left
	 * the obby session.
	 * @param id ID of the underlaying peer.
	 * @param name Name of the peer.
	 * @param red Red colour component of the user colour (0-255)
	 * @param green Green colour component of the user colour (0-255)
	 * @param blue Blue colour component of the user colour (0-255)
	 */
	user(unsigned int id, const std::string& name, int red, int green,
	     int blue);
	~user();

	/** Releases the underlaying peer object from this user. This is useful
	 * if this peer object gets deleted because the corresponding client
	 * left the obby session. The user object itself is stored furthur to
	 * identify the if he rejoins.
	 */
	void release_peer();

	/** Reassigns a new peer to this user. This happens if a user has
	 * left the obby session and rejoined (maybe with another colour, in
	 * this case the colour in all the documents gets updated).
	 */
	void assign_peer(const net6::peer& peer, int red, int green, int blue);

	/** Returns the underlaying peer object.
	 */
	const net6::peer* get_peer() const;

	/** Returns the user name of this user.
	 */
	const std::string& get_name() const;

	/** Returns the address of this user. Note that the address is not
	 * cached, so this function segfaults if the user is not connected to
	 * the session.
	 */
	const net6::address& get_address() const;

	/** Returns a unique ID for this user.
	 */
	unsigned int get_id() const;

	/** Returns the red component of the user colour.
	 */
	int get_red() const;

	/** Returns the green component of the user colour.
	 */
	int get_green() const;

	/** Returns the blue component of the user colour.
	 */
	int get_blue() const;

	/** Returns the flags that are currently set for this user.
	 */
	flags get_flags() const;

	/** Adds the given flags to this user's flags.
	 */
	void add_flags(flags new_flags);

	/** Removes the given flags from this user's flags.
	 */
	void remove_flags(flags old_flags);
protected:
	const net6::peer* m_peer;

	unsigned int m_id;
	std::string m_name;
	int m_red;
	int m_green;
	int m_blue;

	flags m_flags;
};

// Flag combination operations
inline user::flags operator|(user::flags rhs, user::flags lhs) {
	return static_cast<user::flags>(
		static_cast<int>(rhs) | static_cast<int>(lhs)
	);
}

inline user::flags operator&(user::flags rhs, user::flags lhs) {
	return static_cast<user::flags>(
		static_cast<int>(rhs) & static_cast<int>(lhs)
	);
}

inline user::flags operator^(user::flags rhs, user::flags lhs) {
	return static_cast<user::flags>(
		static_cast<int>(rhs) ^ static_cast<int>(lhs)
	);
}

inline user::flags& operator|=(user::flags& rhs, user::flags lhs) {
	return rhs = (rhs | lhs);
}

inline user::flags& operator&=(user::flags& rhs, user::flags lhs) {
	return rhs = (rhs & lhs);
}

inline user::flags& operator^=(user::flags& rhs, user::flags lhs) {
	return rhs = (rhs ^ lhs);
}

inline user::flags operator~(user::flags rhs) {
	return static_cast<user::flags>(~static_cast<int>(rhs) );
}

}

#endif // _OBBY_USER_HPP_

