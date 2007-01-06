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

#include "buffer.hpp"

#if 0
obby::buffer::buffer()
 : m_netkit(), m_rclass(GMP_RAND_ALG_LC, 16), m_doc_counter(0)
{
	// Initialize gettext
	init_gettext();

	// Register user type
	net6::packet::register_type(
		net6::parameter<user*>::TYPE_ID,
		sigc::mem_fun(*this, &buffer::translate_user)
	);

	// Register document type
	net6::packet::register_type(
		net6::parameter<document_info*>::TYPE_ID,
		sigc::mem_fun(*this, &buffer::translate_document)
	);

	// Seed random number generator with system time
	m_rclass.seed(std::time(NULL) );
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

obby::document_info* obby::buffer::find_document(unsigned int owner_id,
                                                 unsigned int id) const
{
	std::list<document_info*>::const_iterator iter;
	for(iter = m_doclist.begin(); iter != m_doclist.end(); ++ iter)
	{
		// Check document ID
		if( (*iter)->get_id() == id)
		{
			// Get owner
			const user* owner = (*iter)->get_owner();

			// Does the document have an owner?
			if(!owner)
			{
				// No. Check if we requested such a document
				if(owner_id == 0)
				{
					// Ok.
					return *iter;
				}
			}
			else
			{
				// Compare owners
				if(owner->get_id() == owner_id)
				{
					// Ok.
					return *iter;
				}
			}
		}
	}

	/* No such document */
	return NULL;
}

bool obby::buffer::check_colour(int red, int green, int blue,
                                const user* ignore) const
{
	// Check for existing colours
	// TODO: Check for colours that non-connected users occupy?
	for(user_table::user_iterator<user::CONNECTED> iter =
		m_usertable.user_begin<user::CONNECTED>();
	    iter != m_usertable.user_end<user::CONNECTED>();
	    ++ iter)
	{
		if(&(*iter) == ignore)
			continue;

		if((abs(red   - iter->get_red()) +
		    abs(green - iter->get_green()) +
		    abs(blue  - iter->get_blue())) < 32)
		{
			return false;
		}
	}
	return true;
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

obby::buffer::signal_user_colour_type obby::buffer::user_colour_event() const
{
	return m_signal_user_colour;
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

net6::basic_parameter*
obby::buffer::translate_user(const std::string& str) const
{
	// Read user ID
	std::stringstream stream(str);
	int user_id;
	stream >> std::hex >> user_id;

	// Read with success?
	if(stream.bad() )
		throw net6::basic_parameter::bad_format(
			"User ID is not a hexadecimal integer"
		);

	// Check for no user
	if(user_id == 0) return new net6::parameter<user*>(NULL);
	// Find corresponding user
	obby::user* found_user = m_usertable.find_user<user::NONE>(user_id);

	if(found_user == NULL)
	{
		// No such user
		format_string str("User ID %0% does not exist");
		str << user_id;
		throw net6::basic_parameter::bad_format(str.str() );
	}

	// Done
	return new net6::parameter<user*>(found_user);
}

net6::basic_parameter*
obby::buffer::translate_document(const std::string& str) const
{
	// Read document and owner IDs
	std::stringstream stream(str);
	int owner_id, document_id;
	stream >> std::hex >> owner_id >> document_id;

	// Check for valid integers
	if(stream.bad() )
		throw net6::basic_parameter::bad_format(
			"Document ID has to be two hexadecimal integers"
		);

	// Lookup document
	document_info* info = find_document(owner_id, document_id);
	if(!info)
	{
		// No such document
		format_string str(
			"Document ID %0% from User %1% does not exist");
		str << document_id << owner_id;
		throw net6::basic_parameter::bad_format(str.str() );
	}

	return new net6::parameter<obby::document_info*>(info);
}
#endif
