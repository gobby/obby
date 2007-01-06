/* libobby - Network text editing library
 * Copyright (C) 2005 0x539 dev group
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public
 * License as published by the Free Software Foundation; either
 i* version 2 of the License, or (at your option) any later version.
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

#include "document_info.hpp"
#include "buffer.hpp"

obby::document_info::document_info(const buffer& buf, const user* owner,
                                   unsigned int id, const std::string& title)
 : m_buffer(buf), m_owner(owner), m_id(id), m_title(title), m_document(NULL)
{
	// Subscribe the owner to the document
	if(owner) m_userlist.push_back(owner);
}

obby::document_info::~document_info()
{
	// Release underlaying document
	release_document();
}

unsigned int obby::document_info::get_id() const
{
	return m_id;
}

const std::string& obby::document_info::get_title() const
{
	return m_title;
}

const obby::buffer& obby::document_info::get_buffer() const
{
	return m_buffer;
}

obby::document* obby::document_info::get_document()
{
	return m_document;
}

const obby::document* obby::document_info::get_document() const
{
	return m_document;
}

const obby::user* obby::document_info::get_owner() const
{
	return m_owner;
}

const obby::user* obby::document_info::find_user(unsigned int id) const
{
	for(user_iterator iter = user_begin(); iter != user_end(); ++ iter)
		if(iter->get_id() == id)
			return &(*iter);
	return NULL;
}

bool obby::document_info::is_subscribed(const user& from) const
{
	return find_user(from.get_id() ) != NULL;
}

obby::document_info::user_iterator obby::document_info::user_begin() const
{
	return m_userlist.begin();
}

obby::document_info::user_iterator obby::document_info::user_end() const
{
	return m_userlist.end();
}

obby::document_info::user_size_type obby::document_info::user_count() const
{
	return m_userlist.size();
}

obby::document_info::signal_rename_type
obby::document_info::rename_event() const
{
	return m_signal_rename;
}

obby::document_info::signal_subscribe_type
obby::document_info::subscribe_event() const
{
	return m_signal_subscribe;
}

obby::document_info::signal_unsubscribe_type
obby::document_info::unsubscribe_event() const
{
	return m_signal_unsubscribe;
}

void obby::document_info::release_document()
{
	delete m_document;
	m_document = NULL;
}

void obby::document_info::obby_user_join(const user& new_user)
{
}

void obby::document_info::obby_user_part(const user& existing_user)
{ 
	for(std::list<const user*>::iterator iter = m_userlist.begin();
	    iter != m_userlist.end();
	    ++ iter)
	{
		// Is the user that has part the session in this document's
		// subscription list?
		if((*iter)->get_id() == existing_user.get_id() )
		{
			// Emit signal that the user unsubscribed
			m_signal_unsubscribe.emit(existing_user);
			// Remove it from the list
			m_userlist.erase(iter);
			// User has been removed, nothing to do anymore
			break;
		}
	}
}

