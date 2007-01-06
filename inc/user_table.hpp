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

#ifndef _OBBY_USER_TABLE_HPP_
#define _OBBY_USER_TABLE_HPP_

#include <list>
#include <string>
#include "user.hpp"
	
namespace obby
{

/** User table to store user information through multiple obby sessions or
 * to store user colourising even if a user has lost the connection.
 */
	
class user_table : private net6::non_copyable
{
public:
	/** User in the user table. Be sure to make a difference between
	 * obby::user and obby::user_table::user! obby::user is a user
	 * that is currently participating at the obby session,
	 * obby::user_table::user just holds some information to identify
	 * a user through multiple obby sessions.
	 */
	class user
	{
		friend class user_table;
	public:
		/** Creates a new user for the user table. Usually the user
		 * table creates its users and you do not have to use this
		 * constructor manually.
		 * @param id Unique ID for this user
		 * @param obj obby::user object associated with this user.
		 */
		user(unsigned int id, obby::user& obj);

		/** Creates a new user for the user table. Usually the user
		 * tables creates its users and you do not have to use this
		 * constructor manually.
		 * @param id Uniquie ID for this user
		 * @param name Name of this uer
		 * @param red Red color component of the user color
		 * @param green Green color component of the user color
		 * @param blue Blue color component of the user color
		 */
		user(unsigned int id, const std::string& name,
		     unsigned int red, unsigned int green, unsigned int blue);

		user(const user& other);
		~user();

		/** Returns the ID for this user.
		 */
		unsigned int get_id() const;

		/** Returns the corresponding obby::user object. It may return
		 * NULL if this user has already left the obby session.
		 */
		obby::user* get_user() const;

		/** Returns the name of this user.
		 */
		const std::string& get_name() const;

		/** Returns the red color component of this user.
		 */
		unsigned int get_red() const;

		/** Returns the green color component of this user.
		 */
		unsigned int get_green() const;

		/** Returns the blue color component of this user.
		 */
		unsigned int get_blue() const;

	protected:
		unsigned int m_id;
		obby::user* m_obj;
		std::string m_name;
		unsigned int m_red, m_green, m_blue;
	};

	typedef std::list<user>::const_iterator user_iterator;
	
	/** Creates a new user table.
	 */
	user_table();
	~user_table();

	/** Inserts a new user into the table. An old user is used if
	 * he has the same name as the new one.
	 */
	void insert_user(obby::user& new_user);

	/** Removes a user from the user table. User information such as
	 * name and color are stored further on to identify new users.
	 */
	void delete_user(obby::user& old_user);

	/** Finds a user by its user id. Note that this is the user id of the
	 * obby::user_table::user, not the one of the corresponding obby::user.
	 */
	const user* find(unsigned int id) const;

	/** Finds a user by its user id. Note that this is the user id of the
	 * corresponding obby::user, not the one of the obby::user_table::user.
	 */
	const user* find_from_user_id(unsigned int id) const;

	/** Returns the beginning of the user list.
	 */
	user_iterator user_begin() const;

	/** Returns the end of the user list.
	 */
	user_iterator user_end() const;
protected:
	/** List of users that are registered in the user table.
	 */
	std::list<user> m_users;

	/** ID counter to generate unique user IDs.
	 */
	unsigned int m_id_counter;
};

}

#endif // _OBBY_USER_TABLE_HPP_

