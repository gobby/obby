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
#include <string>
#include <list>

namespace obby
{

/** Table that contains all users.
 */

class user_table : private net6::non_copyable
{
public:
	// Basic user iterator typedef 
	typedef ptr_iterator<
		user,
		std::list<user*>,
		std::list<user*>::const_iterator
	> basic_user_iterator;

	// User iterator class. It iterates through all the users whose
	// flags match (or don't match if inverse is true) the given flags.
	template<user::flags matching_flags/* = user::NONE*/, bool inverse = false>
	class user_iterator : public basic_user_iterator
	{
	public:
		typedef basic_user_iterator base_iterator;
		typedef std::list<user*> list_type;

		user_iterator(const list_type& list)
		 : base_iterator(), m_list(list) {
			// Increase to first valid value
			inc_valid();
		}

		user_iterator(const list_type& list, const base_iterator& iter)
		 : base_iterator(iter), m_list(list) {
			// Increate to first valid value
			inc_valid();
		}

		user_iterator& operator++() {
			base_iterator::operator++ ();
			inc_valid();
			return *this;
		}

		user_iterator& operator--() {
			base_iterator::operator--();
			dec_valid();
			return *this;
		}

		user_iterator operator++(int dummy) {
			user_iterator temp = *this;
			operator++();
			return temp;
		}

		user_iterator operator--(int dummy) {
			user_iterator temp = *this;
			operator--();
			return temp;
		}
	protected:
		/** If the current value is not a value according to
		 * <em>matching_flags</em>, the iterator is increased until
		 * a valid value has been found.
		 */
		void inc_valid() {
			while( (*this != m_list.end()) && (inverse ? (
				((*this)->get_flags() & matching_flags)
					!= 0
				) : ( ((*this)->get_flags() & matching_flags)
					!= matching_flags
				) )
			) {
				base_iterator::operator++();
			}
		}

		/** Same as inc_valid(), but the iterator is decreaed until
		 * such a value has been found.
		 */
		void dec_valid() {
			while( (*this != m_list.begin()) && (inverse ? (
				((*this)->get_flags() & matching_flags)
					!= 0
				) : ( ((*this)->get_flags() & matching_flags)
					!= matching_flags
				) )
			) {
				base_iterator::operator--();
			}
		}

		/** Underlaying list.
		 */
		const list_type& m_list;
	};

	user_table();
	virtual ~user_table();

	/** Adds a new user to the user list. The name and ID is read from the
	 * net6::user object. Because of the fact that a net6::user object
	 * exists, the new user will be marked as connected. If a user with the
	 * same ID exists, and is not connected, the net6::user will be
	 * assigned to this user, the colour is updated. If a user with this ID
	 * is already connected, std::logic_error is thrown.
	 */
	obby::user* add_user(const net6::user& user, int red, int green,
	                     int blue);

	/** Adds a new user to the user list. No net6::user exists, so the
	 * connected flag will not be set. If a user with this ID exists
	 * already, std::logic_error will be thrown.
	 */
	obby::user* add_user(unsigned int id, const std::string& name, int red,
	                     int green, int blue);

	/** Removes a user from the user list. This means that this user gets
	 * marked as non-connected and the reference to the underlaying peer
	 * object is dropped.
	 */
	void remove_user(user* user_to_remove);
	
	/** Returns the beginning of the user list.
	 */
	template<user::flags matching_flags, bool inverse>
	user_iterator<matching_flags, inverse> user_begin() const {
		return user_iterator<matching_flags, inverse>(
			m_userlist, m_userlist.begin()
		);
	}

	template<user::flags matching_flags>
	user_iterator<matching_flags> user_begin() const {
		return user_begin<matching_flags, false>();
	}

	user_iterator<user::NONE> user_begin() const {
		return user_begin<user::NONE, false>();
	}

	/** Returns the end of the user list.
	 */
	template<user::flags matching_flags, bool inverse>
	user_iterator<matching_flags, inverse> user_end() const {
		return user_iterator<matching_flags, inverse>(
			m_userlist, m_userlist.end()
		);
	}

	template<user::flags matching_flags>
	user_iterator<matching_flags> user_end() const {
		return user_end<matching_flags, false>();
	}

	user_iterator<user::NONE> user_end() const {
		return user_end<user::NONE, false>();
	}

	/** Looks for a user whose flags match (or don't match, if inverse
	 * is true) the given flags.
	 */
	template<user::flags matching_flags, bool inverse>
	user* find_user() const {
		user_iterator<matching_flags, inverse> iter =
			user_begin<matching_flags, inverse>();
		if(iter == user_end<matching_flags, inverse>() )
			return NULL;
		return &(*iter);
	}

	template<user::flags matching_flags>
	user* find_user() const {
		return find_user<matching_flags, false>();
	}

	user* find_user() const {
		return find_user<user::NONE, false>();
	}

	/** Finds a user whose flags match matching_flags (or don't match, if
	 * inverse is true) and if the function <em>func</em> of obby::user
	 * returns the given <em>hint</em>. You may use the other find_user
	 * functions that search a user with the given name, id or something.
	 */
	template<
		user::flags matching_flags,
		bool inverse,
		typename ret_type,
		ret_type(user::*func)() const
	> user* find_user_val(ret_type hint) const {
		for(user_iterator<matching_flags, inverse> iter =
			user_begin<matching_flags, inverse>();
		    iter != user_end<matching_flags, inverse>();
		    ++ iter) {
			user* cur_user = &(*iter);
			if( (cur_user->*func)() == hint) {
				return cur_user;
			}
		}

		return NULL;
	}

	/** Finds a user whose flags match matching_flags (or don't match, if
	 * inverse is true) and if the function <em>func</em> of obby::user
	 * returns the same address as <em>hint</em>'s address. Of course,
	 * ret_type has to be a reference for this to work. This is a helper
	 * function for user_table::find_user(const net6::user&).
	 */
	template<
		user::flags matching_flags,
		bool inverse,
		typename ret_type,
		ret_type(user::*func)() const
	> user* find_user_ref(ret_type hint) const {
		for(user_iterator<matching_flags, inverse> iter =
			user_begin<matching_flags, inverse>();
		    iter != user_end<matching_flags, inverse>();
		    ++ iter) {
			user* cur_user = &(*iter);
			if( &(cur_user->*func)() == &hint) {
				return cur_user;
			}
		}
	}

	/** Searches a user with the given flags that has the given ID.
	 */
	template<user::flags matching_flags, bool inverse>
	user* find_user(unsigned int id) const {
		return find_user_val<matching_flags, inverse, unsigned int,
			&user::get_id>(id);
	}

	template<user::flags matching_flags>
	user* find_user(unsigned int id) const {
		return find_user<matching_flags, false>(id);
	}

	user* find_user(unsigned int id) const {
		return find_user<user::NONE, false>(id);
	}

	/** Searches a user which represents the given underlaying net6::user
	 * object.
	 */
	template<user::flags matching_flags, bool inverse>
	user* find_user(const net6::user& user) const {
		return find_user_ref<matching_flags, inverse,
			const net6::user&, &user::get_net6>(user);
	}

	template<user::flags matching_flags>
	user* find_user(const net6::user& user) const {
		return find_user<matching_flags, false>(user);
	}

	user* find_user(const net6::user& user) const {
		return find_user<user::NONE, false>(user);
	}

	/** Searches for a user with the given name.
	 */
	template<user::flags matching_flags, bool inverse>
	user* find_user(const std::string& name) const {
		return find_user_val<matching_flags, inverse,
			const std::string&, &user::get_name>(name);
	}

	template<user::flags matching_flags>
	user* find_user(const std::string& name) const {
		return find_user<matching_flags, false>(name);
	}

	user* find_user(const std::string& name) const {
		return find_user<user::NONE, false>(name);
	}

	/** Counts users.
	 */
	template<user::flags matching_flags, bool inverse>
	unsigned int user_count() const {
		unsigned int c = 0;
		for(user_iterator<matching_flags, inverse> iter =
			user_begin<matching_flags, inverse>();
		    iter != user_end<matching_flags, inverse>();
		    ++ iter, ++ c) ;
		return c;
	}
protected:
	/** List holding the users.
	 */
	std::list<user*> m_userlist;
};

}

#endif // _OBBY_USER_TABLE_HPP_
