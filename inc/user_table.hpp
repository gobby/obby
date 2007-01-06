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

#include "user.hpp"
#include "ptr_iterator.hpp"
#include <net6/non_copyable.hpp>
#include <map>
#include <string>

namespace obby
{

/** Table that contains all users.
 */

class user_table: private net6::non_copyable
{
public:
	typedef std::map<unsigned int, user*> user_map;
	typedef user_map::size_type size_type;
	typedef user_map::const_iterator base_iterator;

	class iterator: public base_iterator
	{
	public:
		typedef user_map map_type;

		iterator(const map_type& map,
		         user::flags inc_flags,
		         user::flags exc_flags):
			base_iterator(), m_map(map), m_inc_flags(inc_flags),
			m_exc_flags(exc_flags)
		{
		}

		iterator(const map_type& map,
		         const base_iterator& base,
		         user::flags inc_flags,
		         user::flags exc_flags):
			base_iterator(base), m_map(map), m_inc_flags(inc_flags),
			m_exc_flags(exc_flags)
		{
			inc_valid();
		}

		iterator(const iterator& other):
			base_iterator(other), m_map(other.m_map),
			m_inc_flags(other.m_inc_flags),
			m_exc_flags(other.m_exc_flags)
		{
		}

		iterator& operator=(const iterator& other)
		{
			if(&m_map != &other.m_map)
			{
				throw std::logic_error(
					"obby::user_table::iterator::"
					"operator="
				);
			}

			base_iterator::operator=(other);
			return *this;
		}

		const user& operator*() const
		{
			return *base_iterator::operator->()->second;
		}

		const user* operator->() const
		{
			return base_iterator::operator->()->second;
		}

		iterator& operator++()
		{
			base_iterator::operator++();
			inc_valid();
			return *this;
		}

		iterator operator++(int)
		{
			iterator temp(*this);
			operator++();
			return temp;
		}

		static bool check_flags(obby::user::flags flags,
		                        obby::user::flags inc_flags,
		                        obby::user::flags exc_flags)
		{
			return (
				( (flags & inc_flags) == inc_flags) &&
				( (flags & exc_flags) == 0)
			);
		}

	protected:
		void inc_valid()
		{
			while(*this != m_map.end() )
			{
				obby::user::flags flags = (*this)->get_flags();
				if(check_flags(flags, m_inc_flags, m_exc_flags))
					break;

				base_iterator::operator++();
			}
		}

		const map_type& m_map;
		user::flags m_inc_flags;
		user::flags m_exc_flags;
	};

	typedef sigc::signal<void> signal_deserialised_type;

	user_table();
	virtual ~user_table();

	/** Serialises a user_table to a serialisation object.
	 */
	void serialise(serialise::object& obj) const;

	/** Deserialises a user_table from a serialisation object.
	 */
	void deserialise(const serialise::object& obj);

	/** Clears all users from the user table.
	 */
	void clear();

	/** Adds a new user to the user list. The name and ID is read from the
	 * net6::user object. Because of the fact that a net6::user object
	 * exists, the new user will be marked as connected. If a user with the
	 * same ID exists, and is not connected, the net6::user will be
	 * assigned to this user, the colour is updated. If a user with this ID
	 * is already connected, std::logic_error is thrown.
	 */
	const obby::user* add_user(unsigned int id,
	                           const net6::user& user,
	                           const colour& colour);

	/** Adds a new user to the user list. No net6::user exists, so the
	 * connected flag will not be set. If a user with this ID exists
	 * already, std::logic_error will be thrown.
	 */
	const obby::user* add_user(unsigned int id,
	                           const std::string& name,
	                           const colour& colour);

	/** Removes a user from the user list. This means that this user gets
	 * marked as non-connected and the reference to the underlaying
	 * net6::user object is dropped.
	 */
	void remove_user(const user& user_to_remove);

	/** Assigns a token to this user that is used for password hashing.
	 */
	void set_user_token(const user& user, const std::string& token);

	/** Assigns a password to this user.
	 */
	void set_user_password(const user& user, const std::string& password);

	/** Changes this user's colour.
	 */
	void set_user_colour(const user& user, const colour& colour);

	/** Looks for a new user ID that is currently not in use by another
	 * user.
	 */
	unsigned int find_free_id() const;

	/** Returns the beginning of the user list.
	 */
	iterator begin(user::flags inc_flags,
	               user::flags exc_flags) const;

	/** Returns the end of the user list.
	 *
	 * TODO: No need for flags here?
	 */
	iterator end(user::flags inc_flags,
	             user::flags exc_flags) const;

	/** Searches a user with the given flags that has the given ID.
	 */
	const user* find(unsigned int id,
	                 user::flags inc_flags,
	                 user::flags exc_flags) const;

	/** Searches a user which represents the given underlaying net6::user
	 * object.
	 */
	const user* find(const net6::user& user,
	                 user::flags inc_flags,
	                 user::flags exc_flags) const;

	/** Searches for a user with the given name.
	 */
	const user* find(const std::string& name,
	                 user::flags inc_flags,
	                 user::flags exc_flags) const;

	/** Counts users.
	 */
	size_type count(user::flags inc_flags,
	                user::flags exc_flags) const;

	/** @brief Signal that is emitted after the user table has been
	 * deserialised.
	 */
	signal_deserialised_type deserialised_event() const;
protected:
	/** Internal function to find a user with the given ID that has to be
	 * in the user table.
	 */
	user& lookup(unsigned int id);

	/** Internal function to find a non-const user with the given name.
	 */
	user* find_int(const std::string& name);

	/** List holding the users.
	 */
	user_map m_user_map;

	signal_deserialised_type m_signal_deserialised;
};

}

#endif // _OBBY_USER_TABLE_HPP_
