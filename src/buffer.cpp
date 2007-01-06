/* libobby - Network text editing library
 * Copyright (C) 2005 0x539 dev group
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this program; if not, write to the Free
 * Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include <cassert>
#include "buffer.hpp"

obby::buffer::document_iterator::document_iterator()
 : base_iterator()
{
}

obby::buffer::document_iterator::document_iterator(const base_iterator& iter)
 : base_iterator(iter)
{
}

obby::buffer::document_iterator&
obby::buffer::document_iterator::operator=(const base_iterator& iter)
{
	return static_cast<document_iterator&>(base_iterator::operator=(iter) );
}

obby::document& obby::buffer::document_iterator::operator*()
{
	return *base_iterator::operator*();
}

const obby::document& obby::buffer::document_iterator::operator*() const
{
	return *base_iterator::operator*();
}

obby::document* obby::buffer::document_iterator::operator->()
{
	return *base_iterator::operator->();
}

const obby::document* obby::buffer::document_iterator::operator->() const
{
	return *base_iterator::operator->();
}

obby::buffer::buffer()
 : m_usertable(NULL)
{
}

obby::buffer::~buffer()
{
	std::list<document*>::iterator doc_i;
	for(doc_i = m_doclist.begin(); doc_i != m_doclist.end(); ++ doc_i)
		delete *doc_i;

	std::list<user*>::iterator user_i;
	for(user_i = m_userlist.begin(); user_i != m_userlist.end(); ++ user_i)
		delete *user_i;
}

obby::document* obby::buffer::find_document(unsigned int id) const
{
	std::list<document*>::const_iterator iter;
	for(iter = m_doclist.begin(); iter != m_doclist.end(); ++ iter)
		if( (*iter)->get_id() == id)
			return *iter;
	return NULL;
}

obby::user* obby::buffer::find_user(unsigned int id) const
{
	std::list<user*>::const_iterator iter;
	for(iter = m_userlist.begin(); iter != m_userlist.end(); ++ iter)
		if( (*iter)->get_id() == id)
			return *iter;
	return NULL;
}

obby::user* obby::buffer::find_user(const std::string& name) const
{
	std::list<user*>::const_iterator iter;
	for(iter = m_userlist.begin(); iter != m_userlist.end(); ++ iter)
		if( (*iter)->get_name() == name)
			return *iter;
	return NULL;
}

obby::buffer::document_iterator obby::buffer::document_begin() const
{
	return static_cast<document_iterator>(m_doclist.begin() );
}

obby::buffer::document_iterator obby::buffer::document_end() const
{
	return static_cast<document_iterator>(m_doclist.end() );
}

obby::buffer::signal_user_join_type obby::buffer::user_join_event() const
{
	return m_signal_user_join;
}

obby::buffer::signal_user_part_type obby::buffer::user_part_event() const
{
	return m_signal_user_part;
}

obby::buffer::signal_insert_document_type
obby::buffer::insert_document_event() const
{
	return m_signal_insert_document;
}

obby::buffer::signal_remove_document_type
obby::buffer::remove_document_event() const
{
	return m_signal_remove_document;
}

obby::buffer::signal_message_type
obby::buffer::message_event() const
{
	return m_signal_message;
}

obby::user* obby::buffer::add_user(net6::peer& peer, int red, int green,
                                   int blue)
{
	user* new_user = new user(peer, red, green, blue);
	m_userlist.push_back(new_user);
	m_usertable->insert_user(*new_user);
	return new_user;
}

void obby::buffer::remove_user(user* user_to_remove)
{
	m_usertable->delete_user(*user_to_remove);
	m_userlist.erase(
		std::remove(
			m_userlist.begin(),
			m_userlist.end(),
			user_to_remove
		),
		m_userlist.end()
	);

	delete user_to_remove;
}

