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

#include <iostream>
#include "user_table.hpp"

obby::user_table::user_table()
{
}

obby::user_table::~user_table()
{
	for(std::list<user*>::iterator iter = m_userlist.begin();
	    iter != m_userlist.end(); ++ iter)
		delete *iter;
}

obby::user* obby::user_table::add_user(net6::peer& peer, int red, int green,
                                       int blue)
{
	// Find already exiting user with the given name
	user* existing_user = find_user(peer.get_name() );
	if(existing_user)
	{
		// If this user would be connected, net6 should have denied
		// the login process with the "Name is already in use" error
		if(existing_user->get_flags() & user::CONNECTED)
		{
			std::cerr << "obby::user_table::add_user: " 
			          << "User " << peer.get_id() << " exists "
				  << "already " << std::endl;
			return NULL;
		}

		// Assign new peer to existing user.
		existing_user->assign_peer(peer, red, green, blue);
		return existing_user;
	}
	else
	{
		// User seems to be here for his first time: Create a new user.
		user* new_user = new user(peer, red, green, blue);

		// Insert user into user list
		m_userlist.push_back(new_user);

		return new_user;
	}
}

obby::user* obby::user_table::add_user(unsigned int id, const std::string& name,
                                       int red, int green, int blue)
{
	// Look for an existing user with this name.
	user* existing_user = find_user(name);
	if(existing_user)
	{
		// We can not assign the new user to this one, because the user
		// we are currently adding is not connected to the obby session.
		std::cerr << "obby::user_table::add_user: "
		          << "User " << id << " exists already!" << std::endl;
		return NULL;
	}

	user* new_user = new user(id, name, red, green, blue);
	m_userlist.push_back(new_user);

	return new_user;
}

void obby::user_table::remove_user(user* user_to_remove)
{
	// Release underlaying peer object, this disables the connected flag,
	// too. Keep the user in the list to recognize him if he rejoins.
	user_to_remove->release_peer();
}
