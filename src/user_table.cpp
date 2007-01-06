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

#include <cassert>
#include <net6/server.hpp>
#include "user_table.hpp"

obby::user_table::user::user(unsigned int id, obby::user& obj)
 : m_id(id), m_obj(&obj), m_name(obj.get_name()),
   m_red(obj.get_red()), m_green(obj.get_green()), m_blue(obj.get_blue())
{
}

obby::user_table::user::user(unsigned int id, const std::string& name,
                             unsigned int red, unsigned int green,
                             unsigned int blue)
 : m_id(id), m_obj(NULL), m_name(name), m_red(red), m_green(green), m_blue(blue)
{
}

obby::user_table::user::user(const user& other)
 : m_id(other.m_id), m_obj(other.m_obj), m_name(other.m_name),
   m_red(other.m_red), m_green(other.m_green), m_blue(other.m_blue)
{
}

obby::user_table::user::~user()
{
}

unsigned int obby::user_table::user::get_id() const
{
	return m_id;
}

obby::user* obby::user_table::user::get_user() const
{
	return m_obj;
}

const std::string& obby::user_table::user::get_name() const
{
	return m_name;
}

unsigned int obby::user_table::user::get_red() const
{
	return m_red;
}

unsigned int obby::user_table::user::get_green() const
{
	return m_green;
}

unsigned int obby::user_table::user::get_blue() const
{
	return m_blue;
}

obby::user_table::user_table()
 : m_id_counter(0)
{
}

obby::user_table::~user_table()
{
}

void obby::user_table::insert_user(obby::user& new_user)
{
	// Look for an existing user
	std::list<user>::iterator iter;
	for(iter = m_users.begin(); iter != m_users.end(); ++ iter)
	{
		if(iter->m_name == new_user.get_name() )
		{
			// Found existing user
			assert(iter->m_obj == NULL);
			
			// Update color and user object
			iter->m_obj = &new_user;
			iter->m_red = new_user.get_red();
			iter->m_green = new_user.get_green();
			iter->m_blue = new_user.get_blue();

			return;
		}
	}

	// No one found: Create new user
	m_users.push_back(user(++ m_id_counter, new_user) );
}

void obby::user_table::delete_user(obby::user& new_user)
{
	// Search for the user in the table
	std::list<user>::iterator iter;
	for(iter = m_users.begin(); iter != m_users.end(); ++ iter)
	{
		// Is it this one?
		if(iter->m_obj == &new_user)
		{
			// Reset user object: The user has quit.
			iter->m_obj = NULL;
			return;
		}
	}

	assert(iter != m_users.end() );
}

const obby::user_table::user* obby::user_table::find(unsigned int id) const
{
	std::list<user>::const_iterator iter;
	for(iter = m_users.begin(); iter != m_users.end(); ++ iter)
		if(id == iter->m_id)
			return &(*iter);
	return NULL;
}

const obby::user_table::user*
obby::user_table::find_from_user_id(unsigned int id) const
{
	std::list<user>::const_iterator iter;
	for(iter = m_users.begin(); iter != m_users.end(); ++ iter)
	{
		// Server user
		if(id == 0)
		{
			if(iter->m_id == 0)
			{
				return &(*iter);
			}
		}

		// Normal user
		else if(iter->m_obj)
		{
			if(iter->m_obj->get_id() == id)
			{
				return &(*iter);
			}
		}
	}

	return NULL;
}

obby::user_table::user_iterator obby::user_table::user_begin() const
{
	return m_users.begin();
}

obby::user_table::user_iterator obby::user_table::user_end() const
{
	return m_users.end();
}

