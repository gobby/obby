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

#include "gettext.hpp"
#include "serialise/error.hpp"
#include "user_table.hpp"

obby::user_table::user_table()
{
}

obby::user_table::~user_table()
{
	clear();
}

void obby::user_table::serialise(serialise::object& obj) const
{
	for(base_iterator i = m_user_map.begin(); i != m_user_map.end(); ++ i)
	{
		serialise::object& user = obj.add_child();
		user.set_name("user");
		i->second->serialise(user);
	}
}

void obby::user_table::deserialise(const serialise::object& obj)
{
	for(serialise::object::child_iterator iter = obj.children_begin();
	    iter != obj.children_end();
	    ++ iter)
	{
		if(iter->get_name() == "user")
		{
			// Create new user
			user* new_user = new user(*iter);

			// Check for conflicts
			if(m_user_map.find(new_user->get_id()) !=
			   m_user_map.end() || new_user->get_id() == 0)
			{
				format_string str(
					_("User ID %0% is already in use")
				);
				str << new_user->get_id();

				delete new_user;
				throw serialise::error(
					str.str(),
					iter->get_line()
				);
			}

			// Insert into user map
			m_user_map[new_user->get_id()] = new_user;
		}
		else
		{
			// TODO: unexpected_child_error
			format_string str(_("Unexpected child node: '%0%'") );
			str << iter->get_name();
			throw serialise::error(str.str(), iter->get_line() );
		}
	}
}

void obby::user_table::clear()
{
	for(base_iterator i = m_user_map.begin(); i != m_user_map.end(); ++ i)
		delete i->second;

	m_user_map.clear();
}

obby::user* obby::user_table::add_user(unsigned int id,
                                       const net6::user& user6,
                                       const colour& colour)
{
	// Find already exiting user with the given name
	user* existing_user = find_int(user6.get_name() );
	if(existing_user != NULL)
	{
		// If this user would be connected, net6 should have denied
		// the login process with the "Name is already in use" error
		if(existing_user->get_flags() & user::flags::CONNECTED)
			throw std::logic_error("obby::user_table::add_user");

		// Assign new net6::user to existing obby::user.
		existing_user->assign_net6(user6, colour);
		return existing_user;
	}
	else
	{
		// Make sure the ID is not already in use.
		if(id == 0 || m_user_map.find(id) != m_user_map.end() )
			throw std::logic_error("obby::user_table::add_user");

		// User seems to be here for his first time: Create a new user.
		user* new_user = new user(id, user6, colour);

		// Insert user into user list
		m_user_map[id] = new_user;

		return new_user;
	}
}

obby::user* obby::user_table::add_user(unsigned int id,
                                       const std::string& name,
                                       const colour& colour)
{
	// Look for an existing user with this name.
	user* existing_user = find_int(name);

	// We can not assign the new user to this one, because the user
	// we are currently adding is not connected to the obby session.
	if(existing_user != NULL)
		throw std::logic_error("obby::user_table::add_user");

	// Make sure the ID is not already in use.
	if(id == 0 || m_user_map.find(id) != m_user_map.end() )
		throw std::logic_error("obby::user_table::add_user");

	user* new_user = new user(id, name, colour);
	m_user_map[id] = new_user;

	return new_user;
}

void obby::user_table::remove_user(const user& user_to_remove)
{
	// Release underlaying net6::user object, this disables the connected
	// flag, too. Keep the user in the list to recognize him if he rejoins.
	const_cast<user&>(user_to_remove).release_net6();
}

unsigned int obby::user_table::find_free_id() const
{
	unsigned int free_id = 1;
	for(base_iterator i = m_user_map.begin(); i != m_user_map.end(); ++ i)
		if( i->second->get_id() >= free_id)
			free_id = i->second->get_id() + 1;

	return free_id;
}

obby::user_table::iterator obby::user_table::begin(user::flags inc_flags,
                                                   user::flags exc_flags) const
{
	return iterator(m_user_map, m_user_map.begin(), inc_flags, exc_flags);
}

obby::user_table::iterator obby::user_table::end(user::flags inc_flags,
                                                 user::flags exc_flags) const
{
	return iterator(m_user_map, m_user_map.end(), inc_flags, exc_flags);
}

const obby::user* obby::user_table::find(unsigned int id,
                                         user::flags inc_flags,
                                         user::flags exc_flags) const
{
	base_iterator iter = m_user_map.find(id);
	if(iter == m_user_map.end() ) return NULL;

	user::flags flags = iter->second->get_flags();
	if(!iterator::check_flags(flags, inc_flags, exc_flags) ) return NULL;
	return iter->second;
}

const obby::user* obby::user_table::find(const net6::user& user,
                                         user::flags inc_flags,
                                         user::flags exc_flags) const
{
	for(base_iterator i = m_user_map.begin(); i != m_user_map.end(); ++ i)
	{
		if(~i->second->get_flags() & user::flags::CONNECTED) continue;
		if( &i->second->get_net6() != &user) continue;

		user::flags flags = i->second->get_flags();
		if(!iterator::check_flags(flags, inc_flags, exc_flags) ) break;
		return i->second;
	}

	return NULL;
}

const obby::user* obby::user_table::find(const std::string& name,
                                         user::flags inc_flags,
                                         user::flags exc_flags) const
{
	for(base_iterator i = m_user_map.begin(); i != m_user_map.end(); ++ i)
	{
		if(i->second->get_name() != name) continue;

		user::flags flags = i->second->get_flags();
		if(!iterator::check_flags(flags, inc_flags, exc_flags) ) break;
		return i->second;
	}

	return NULL;
}

obby::user_table::size_type obby::user_table::count(user::flags inc_flags,
                                                    user::flags exc_flags) const
{
	// No criteria: Optimize by returning the cached value from the map
	if(inc_flags == user::flags::NONE && exc_flags == user::flags::NONE)
		return m_user_map.size();

	size_type c = 0;
	for(iterator iter = begin(inc_flags, exc_flags);
	    iter != end(inc_flags, exc_flags);
	    ++ iter)
	{
		++ c;
	}

	return c;
}

obby::user* obby::user_table::find_int(const std::string& name)
{
	for(base_iterator iter = m_user_map.begin();
	    iter != m_user_map.end();
	    ++ iter)
	{
		if(iter->second->get_name() == name)
			return iter->second;
	}

	return NULL;
}
