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

#include "ptr_iterator.hpp"
#include <net6/packet.hpp>
#include <net6/user.hpp>
#include <net6/address.hpp>
#include <net6/non_copyable.hpp>
#include <string>

namespace obby
{

class document_info;

/** User in a obby session.
 */
	
class user : private net6::non_copyable
{
public:
	enum flags {
		NONE = 0x00,
		CONNECTED = 0x01
	};

	/** Creates a new user from an existing net6::user.
	 * @param user6 Underlaying net6::user object.
	 * @param red Red colour component of the user colour (0-255)
	 * @param green Green colour component of the user colour (0-255)
	 * @param blue Blue colour component of the user colour (0-255)
	 */
	user(const net6::user& user6, int red, int green, int blue);

	/** Creates a new user that represents a client that has already left
	 * the obby session.
	 * @param id ID of the underlaying user.
	 * @param name Name of the user.
	 * @param red Red colour component of the user colour (0-255)
	 * @param green Green colour component of the user colour (0-255)
	 * @param blue Blue colour component of the user colour (0-255)
	 */
	user(unsigned int id, const std::string& name, int red, int green,
	     int blue);

	/** Releases the underlaying net6::user object from this user.
	 * This is useful if this object gets deleted because the
	 * corresponding client left the obby session. The obby::user object
	 * itself is stored furthur to identify the client if he rejoins.
	 */
	void release_net6();

	/** Reassigns a new net6::user to this user. This happens if a user has
	 * left the obby session and rejoined (maybe with another colour, in
	 * this case the colour in all the documents gets updated).
	 */
	void assign_net6(const net6::user& user6, int red, int green, int blue);

	/** Returns the underlaying net6::user object.
	 */
	const net6::user& get_net6() const;

	/** Returns the user name of this user.
	 */
	const std::string& get_name() const;

	/** Returns the address of this user. Note that the address is not
	 * cached, so this function throws std::logic_error if the user is not
	 * connected to the session. net6::not_connected_error is thrown if the
	 * address of the user is not known, e.g. there is no direct connection
	 * to this user.
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

	/** Returns the random token set for this user.
	 */
	const std::string& get_token() const;

	/** Returns the password for this user (only available with server
	 * or host buffers).
	 */
	const std::string& get_password() const;

	/** Returns the flags that are currently set for this user.
	 */
	flags get_flags() const;

	/** Sets the three colour components of the user colour.
	 */
	void set_colour(int red, int green, int blue);

	/** Changes the token to the one the server generated for this user.
	 */
	void set_token(const std::string& token);

	/** Changes the password for this user
	 */
	void set_password(const std::string& password);

	/** Adds the given flags to this user's flags.
	 */
	void add_flags(flags new_flags);

	/** Removes the given flags from this user's flags.
	 */
	void remove_flags(flags old_flags);

protected:
	const net6::user* m_user6;

	unsigned int m_id;
	std::string m_name;
	int m_red;
	int m_green;
	int m_blue;

	std::string m_token;
	std::string m_password;

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

namespace net6
{

/** obby user packet type
 */
template<>
class parameter<obby::user*> : public basic_parameter {
public:
	parameter(obby::user* user)
	 : basic_parameter(TYPE_ID, user) { }

	virtual basic_parameter* clone() const {
		return new parameter<obby::user*>(as<obby::user*>() );
	}

	virtual std::string to_string() const {
		obby::user* user = as<obby::user*>();
		int user_id = user ? user->get_id() : 0;

		std::stringstream stream;
		stream << std::hex << user_id;
		return stream.str();
	}

	static const identification_type TYPE_ID = 'u';
};

template<>
class parameter<obby::user> : public parameter<obby::user*> {
public:
	parameter(obby::user& user)
	 : parameter<obby::user*>(&user) { }
};

template<>
class parameter<const obby::user*> : public parameter<obby::user*> {
public:
	parameter(const obby::user* user)
	: parameter<obby::user*>(const_cast<obby::user*>(user) ) { }
};

}

#endif // _OBBY_USER_HPP_


