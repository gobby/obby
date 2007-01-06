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
#include "buffer.hpp"

obby::buffer::buffer()
 : m_netkit()
{
}

obby::buffer::~buffer()
{
	std::list<document_info*>::iterator doc_i;
	for(doc_i = m_doclist.begin(); doc_i != m_doclist.end(); ++ doc_i)
		delete *doc_i;
}

const obby::user_table& obby::buffer::get_user_table() const
{
	return m_usertable;
}

obby::document_info* obby::buffer::find_document(unsigned int id) const
{
	std::list<document_info*>::const_iterator iter;
	for(iter = m_doclist.begin(); iter != m_doclist.end(); ++ iter)
		if( (*iter)->get_id() == id)
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

obby::buffer::document_size_type obby::buffer::document_count() const
{
	return m_doclist.size();
}

obby::buffer::signal_user_join_type obby::buffer::user_join_event() const
{
	return m_signal_user_join;
}

obby::buffer::signal_user_part_type obby::buffer::user_part_event() const
{
	return m_signal_user_part;
}

obby::buffer::signal_document_insert_type
obby::buffer::document_insert_event() const
{
	return m_signal_document_insert;
}

obby::buffer::signal_document_rename_type
obby::buffer::document_rename_event() const
{
	return m_signal_document_rename;
}

obby::buffer::signal_document_remove_type
obby::buffer::document_remove_event() const
{
	return m_signal_document_remove;
}

obby::buffer::signal_message_type
obby::buffer::message_event() const
{
	return m_signal_message;
}
obby::buffer::signal_server_message_type
obby::buffer::server_message_event() const
{
	return m_signal_server_message;
}

