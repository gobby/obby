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

#include <net6/packet.hpp>
#include <net6/user.hpp>
#include <net6/address.hpp>
#include <net6/non_copyable.hpp>
#include <string>
#include "serialise/object.hpp"
#include "colour.hpp"

namespace obby
{

class user_table;

/** User in a obby session.
 */
	
class user : private net6::non_copyable
{
public:
	/** Flags that belong to a user.
	 */
	class flags
	{
	public:
		static const flags NONE;
		static const flags CONNECTED;

	        flags operator|(flags other) const { return flags(m_value | other.m_value); }
        	flags operator&(flags other) const { return flags(m_value & other.m_value); }
	        flags operator^(flags other) const { return flags(m_value ^ other.m_value); }
        	flags& operator|=(flags other) { m_value |= other.m_value; return *this; }
	        flags& operator&=(flags other) { m_value &= other.m_value; return *this; }
        	flags& operator^=(flags other) { m_value ^= other.m_value; return *this; }
	        flags operator~() const { return flags(~m_value); }

		operator bool() const { return m_value != NONE.m_value; }
		bool operator!() const { return m_value == NONE.m_value; }
	        bool operator==(flags other) const { return m_value == other.m_value; }
        	bool operator!=(flags other) const { return m_value != other.m_value; }

	        unsigned int get_value() const { return m_value; }

	protected:
        	explicit flags(unsigned int value) : m_value(value) { }

	        unsigned int m_value;
	};

	/** User-wide privileges.
	 */
	class privileges
	{
	public:
		static const privileges NONE;
		static const privileges CREATE_DOCUMENT;

	        privileges operator|(privileges other) const { return privileges(m_value | other.m_value); }
        	privileges operator&(privileges other) const { return privileges(m_value & other.m_value); }
	        privileges operator^(privileges other) const { return privileges(m_value ^ other.m_value); }
        	privileges& operator|=(privileges other) { m_value |= other.m_value; return *this; }
	        privileges& operator&=(privileges other) { m_value &= other.m_value; return *this; }
        	privileges& operator^=(privileges other) { m_value ^= other.m_value; return *this; }
	        privileges operator~() const { return privileges(~m_value); }

		operator bool() const { return m_value != NONE.m_value; }
		bool operator!() const { return m_value == NONE.m_value; }
        	bool operator==(privileges other) const { return m_value == other.m_value; }
	        bool operator!=(privileges other) const { return m_value != other.m_value; }

	        unsigned int get_value() const { return m_value; }

	protected:
        	explicit privileges(unsigned int value) : m_value(value) { }

	        unsigned int m_value;
	};

	/** Creates a new user from an existing net6::user.
	 * @param id Unique obby ID for this user.
	 * @param user6 Underlaying net6::user object.
	 * @param colour User colour.
	 */
	user(unsigned int id,
	     const net6::user& user6,
	     const colour& colour);

	/** Creates a new user that represents a client that has already left
	 * the obby session.
	 * @param id Unique obby ID for this user.
	 * @param name Name of the user.
	 * @param colour User colour.
	 */
	user(unsigned int id,
	     const std::string& name,
	     const colour& colour);

	/** Creates a user from a serialised user object.
	 */
	user(const serialise::object& obj);

	/** Serialises a user to a serialisation object.
	 */
	void serialise(serialise::object& obj) const;

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
	void assign_net6(const net6::user& user6,
	                 const colour& colour);

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

	/** Returns the user colour.
	 */
	const colour& get_colour() const;

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
	void set_colour(const colour& colour);

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
	colour m_colour;

	std::string m_token;
	std::string m_password;

	flags m_flags;
	privileges m_privs;
};

} // namespace obby

namespace serialise
{

template<>
class context<obby::user*>
{
public:
	context();
	context(const obby::user_table& user_table);

	virtual std::string to_string(const obby::user* from) const;
	virtual const obby::user* from_string(const std::string& string) const;

protected:
	virtual void on_stream_setup(std::stringstream& stream) const;

	const obby::user_table* m_user_table;
};

template<>
class context<const obby::user*>: public context<obby::user*>
{
public:
	context():
		context<obby::user*>() {}
	context(const obby::user_table& user_table):
		context<obby::user*>(user_table) {}
protected:
	virtual void on_stream_setup(std::stringstream& stream) const;
};

template<>
class hex_context<obby::user*>: public context<obby::user*>
{
public:
	hex_context():
		context<obby::user*>() {}
	hex_context(const obby::user_table& user_table):
		context<obby::user*>(user_table) {}
protected:
	virtual void on_stream_setup(std::stringstream& stream) const;
};

template<>
class hex_context<const obby::user*>: public context<const obby::user*>
{
public:
	hex_context():
		context<const obby::user*>() {}
	hex_context(const obby::user_table& user_table):
		context<const obby::user*>(user_table) {}
protected:
	virtual void on_stream_setup(std::stringstream& stream) const;
};

} // namespace serialise

#endif // _OBBY_USER_HPP_



