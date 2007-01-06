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
#include <net6/packet.hpp>
#include "ptr_iterator.hpp"

namespace obby
{

class document_info;

/** User in a obby session.
 */
	
class user
{
public:
	enum flags {
		NONE = 0x00,
		CONNECTED = 0x01
	};

	/*typedef ptr_iterator<
		document_info,
		std::list<document_info*>,
		std::list<document_info*>::const_iterator
	> document_iterator;*/
	
	/** Creates a new user from an existing peer.
	 * @param peer Underlaying net6 peer object.
	 * @param red Red colour component of the user colour (0-255)
	 * @param green Green colour component of the user colour (0-255)
	 * @param blue Blue colour component of the user colour (0-255)
	 */
	user(net6::peer& peer, int red, int green, int blue);

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
	void assign_peer(net6::peer& peer, int red, int green, int blue);

	/** Returns the underlaying peer object.
	 */
	net6::peer* get_peer() const;

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

	/** Returns the password for this user (only available with server
	 * or host buffers).
	 */
	const std::string& get_password() const;

	/** Returns the flags that are currently set for this user.
	 */
	flags get_flags() const;

	/** Changes the password for this user
	 */
	void set_password(const std::string& password);

	/** Adds the given flags to this user's flags.
	 */
	void add_flags(flags new_flags);

	/** Removes the given flags from this user's flags.
	 */
	void remove_flags(flags old_flags);

	// WTF? Users can't be identical because the IDs are unique..?
	//  - armin
#if 0
	/** Checks if both objects are identical by comparing their id.
	 */
	bool operator==(const user& other) { return m_id == other.m_id; }

	/** Checks if both objects differ by comparing their id.
	 */
	bool operator!=(const user& other) { return m_id != other.m_id; }
#endif

#if 0
	/** Adds this document to the user's list of subscribed documents.
	 * Note that this function does not add the user to the subscribe
	 * list of the document. Use local_document_info::subscribe to
	 * subscribe the local user to a document.
	 */
	void subscribe(document_info& document);

	/** Removes the given document from the user's list of subscribed
	 * documents. Note that this function does not remove the user
	 * from the subscribe list of the document. Use
	 * local_document_info::unsubscribe to unsubscribe the local user
	 * from a document.
	 */
	void unsubscribe(document_info& document);

	/** Looks for a given document ID in the list of subscribed documents.
	 */
	document_info* find_document(unsigned int id) const;

	/** Looks if the user is subscribed to the given document.
	 */
	bool is_subscribed(const document_info& document) const;

	/** Returns the begin of the document list the user is subscribed to.
	 */
	document_iterator document_begin() const;

	/** Returns the end of the document list the user is subscribed to.
	 */
	document_iterator document_end() const;
#endif
protected:
	net6::peer* m_peer;

	unsigned int m_id;
	std::string m_name;
	int m_red;
	int m_green;
	int m_blue;

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

