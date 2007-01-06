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

#ifndef _OBBY_BUFFER_HPP_
#define _OBBY_BUFFER_HPP_

#include <list>
#include <gmpxx.h>
#include <net6/main.hpp>
#include <net6/object.hpp>
#include "common.hpp"
#include "format_string.hpp"
#include "document_info.hpp"
#include "user_table.hpp"

namespace obby
{

/** Abstract base class for obby buffers. A buffer contains multiple documents
 * that are synchronised through many users and a user list.
 */
template<typename selector_type>
class basic_buffer : private net6::non_copyable, public sigc::trackable
{
public:
	typedef basic_document_info<selector_type> base_document_info;
	typedef basic_document_info<selector_type> document_info;

	// Document iterator typedef
	typedef typename std::list<document_info*>::size_type
		document_size_type;

	typedef ptr_iterator<
		document_info,
		std::list<document_info*>,
		typename std::list<document_info*>::const_iterator
	> document_iterator;

	// Signal types
	typedef sigc::signal<void, const user&>
		signal_user_join_type;
	typedef sigc::signal<void, const user&>
		signal_user_part_type;
	typedef sigc::signal<void, const user&>
		signal_user_colour_type;
	typedef sigc::signal<void, document_info&>
		signal_document_insert_type;
	typedef sigc::signal<void, document_info&>
		signal_document_rename_type;
	typedef sigc::signal<void, document_info&>
		signal_document_remove_type;
	typedef sigc::signal<void, const user&, const std::string&>
		signal_message_type;
	typedef sigc::signal<void, const std::string&>
		signal_server_message_type;

	basic_buffer();
	virtual ~basic_buffer();

	/** Returns the user table associated with the buffer.
	 */
	const user_table& get_user_table() const;

	/** Returns the selector of the underlaying net6 network object.
	 */
	selector_type& get_selector();

	/** Returns the selector of the underlaying net6 network object.
	 */
	const selector_type& get_selector() const;

	/* Creates a new document with predefined content.
	 * signal_document_insert will be emitted if it has been created.
	 */
	virtual void document_create(const std::string& title,
	                             const std::string& content) = 0;

	/** Removes an existing document. signal_document_remove will be
	 * emitted if the document has been removed.
	 */
	virtual void document_remove(base_document_info& doc) = 0;
	
	/** Looks for a document with the given ID which belongs to the user
	 * with the given owner ID. Note that we do not take a real user object
	 * here because the ID is enough and one might not have a user object
	 * to the corresponding ID. So a time-consuming lookup is obsolete.
	 */
	document_info* document_find(unsigned int owner_id,
	                             unsigned int id) const;

	/** Returns the begin of the document list.
	 */
	document_iterator document_begin() const;

	/** Returns the end of the document list.
	 */
	document_iterator document_end() const;

	/** Returns the size of the document list.
	 */
	document_size_type document_count() const;

	/** Sends a global chat message to all users.
	 */
	virtual void send_message(const std::string& message) = 0;

	/** Checks if given colour components match an other
	 * already present one too closely.
	 * TODO: Move this function to user table?
	 */
	bool check_colour(int red, int green, int blue,
	                  const user* ignore = NULL) const;

	/** Signal which will be emitted if a new user has joined the obby
	 * session.
	 */
	signal_user_join_type user_join_event() const;

	/** Signal which will be emitted if a user has quit.
	 */
	signal_user_part_type user_part_event() const;

	/** Signal which will be emitted if a user changes his colour.
	 */
	signal_user_colour_type user_colour_event() const;

	/** Signal which will be emitted when another participant in the
	 * obby session has created a new document.
	 */
	signal_document_insert_type document_insert_event() const;

	/** Signal which will be emitted when another participant in the
	 * obby session renames one document.
	 */
	signal_document_rename_type document_rename_event() const;

	/** Signal which will be emitted when another participant in the
	 * obby session has removed an existing document.
	 */
	signal_document_remove_type document_remove_event() const;

	/** Signal which will be emitted when another participant in the
	 * obby session sends a chat packet.
	 */
	signal_message_type message_event() const;

	/** Signal which will be emitted when the server of the
	 * obby session sends a chat packet.
	 */
	signal_server_message_type server_message_event() const;

	/** Current obby protocol version.
	 */
	static const unsigned long PROTOCOL_VERSION;
protected:
        /** Internal function to add a document to the buffer.
	 */
	void document_add(document_info& document);

	/** Internal function to delete a document from the buffer.
	 */
	void document_delete(document_info& document);

	/** Internal function to clear the whole document list.
	 */
	void document_clear();

	/** Translates a user parameter back to a net6 packet parameter.
	 */
	net6::basic_parameter* translate_user(const std::string& str) const;

	/** Translates a document parameter back to a net6 packet parameter.
	 */
	net6::basic_parameter* translate_document(const std::string& str) const;

	/** net6 main object to keep net6 initialised during the obby session.
	 */
	net6::main m_netkit;

	/** Net object.
	 */
	std::auto_ptr<net6::basic_object<selector_type> > m_net;

	/** GMP random number generator.
	 */
	gmp_randclass m_rclass;

	/** User table which stores all the users in the session.
	 */
	user_table m_user_table;

	/** List of documents.
	 */
	std::list<document_info*> m_docs;

	/** Counter for document IDs.
	 */
	unsigned int m_doc_counter;

	signal_user_join_type m_signal_user_join;
	signal_user_part_type m_signal_user_part;
	signal_user_colour_type m_signal_user_colour;

	signal_document_insert_type m_signal_document_insert;
	signal_document_rename_type m_signal_document_rename;
	signal_document_remove_type m_signal_document_remove;

	signal_message_type m_signal_message;
	signal_server_message_type m_signal_server_message;
};

typedef basic_buffer<net6::selector> buffer;

template<typename selector_type>
const unsigned long basic_buffer<selector_type>::PROTOCOL_VERSION = 2;

template<typename selector_type>
basic_buffer<selector_type>::basic_buffer()
 : m_rclass(GMP_RAND_ALG_LC, 16), m_doc_counter(0)
{
	// Initialize gettext
	init_gettext();

	// Register user type
	net6::packet::register_type(
		net6::parameter<user*>::TYPE_ID,
		sigc::mem_fun(*this, &basic_buffer::translate_user)
	);

	// Register document type
	net6::packet::register_type(
		net6::parameter<basic_document_info<selector_type>*>::TYPE_ID,
		sigc::mem_fun(*this, &basic_buffer::translate_document)
	);

	// Seed random number generator with system time
	m_rclass.seed(std::time(NULL) );
}

template<typename selector_type>
basic_buffer<selector_type>::~basic_buffer()
{
	document_clear();
}

template<typename selector_type>
const user_table& basic_buffer<selector_type>::get_user_table() const
{
	return m_user_table;
}

template<typename selector_type>
selector_type& basic_buffer<selector_type>::get_selector()
{
	if(!m_net) throw std::logic_error("obby::basic_buffer::get_selector");
	return m_net->get_selector();
}

template<typename selector_type>
const selector_type& basic_buffer<selector_type>::get_selector() const
{
	if(!m_net) throw std::logic_error("obby::basic_buffer::get_selector");
	return m_net->get_selector();
}

template<typename selector_type>
typename basic_buffer<selector_type>::document_info*
basic_buffer<selector_type>::document_find(unsigned int owner_id,
                                           unsigned int id) const
{
	document_iterator iter;
	for(iter = m_docs.begin(); iter != m_docs.end(); ++ iter)
	{
		// Check document ID
		if(iter->get_id() != id) continue;
		// Check owner ID
		if(iter->get_owner_id() != owner_id) continue;
		// Found requested document
		return &(*iter);
	}

	return NULL;
}

template<typename selector_type>
bool basic_buffer<selector_type>::check_colour(int red, int green, int blue,
                                               const user* ignore) const
{
	for(user_table::iterator iter =
		m_user_table.begin(user::flags::CONNECTED);
	    iter != m_user_table.end(user::flags::CONNECTED);
	    ++ iter)
	{
		// Ignore given user to ignore
		if(&(*iter) == ignore) continue;

		// TODO: obby::colour class to perform this check
		if( abs(red - iter->get_red()) +
		    abs(green - iter->get_green()) +
		    abs(blue - iter->get_blue()) < 32)
		{
			// Conflict
			return false;
		}
	}

	return true;
}

template<typename selector_type>
typename basic_buffer<selector_type>::document_iterator
basic_buffer<selector_type>::document_begin() const
{
	return static_cast<document_iterator>(m_docs.begin() );
}

template<typename selector_type>
typename basic_buffer<selector_type>::document_iterator
basic_buffer<selector_type>::document_end() const
{
	return static_cast<document_iterator>(m_docs.end() );
}

template<typename selector_type>
typename basic_buffer<selector_type>::document_size_type
basic_buffer<selector_type>::document_count() const
{
	return m_docs.size();
}

template<typename selector_type>
typename basic_buffer<selector_type>::signal_user_join_type
basic_buffer<selector_type>::user_join_event() const
{
	return m_signal_user_join;
}

template<typename selector_type>
typename basic_buffer<selector_type>::signal_user_part_type
basic_buffer<selector_type>::user_part_event() const
{
	return m_signal_user_part;
}

template<typename selector_type>
typename basic_buffer<selector_type>::signal_user_colour_type
basic_buffer<selector_type>::user_colour_event() const
{
	return m_signal_user_colour;
}

template<typename selector_type>
typename basic_buffer<selector_type>::signal_document_insert_type
basic_buffer<selector_type>::document_insert_event() const
{
	return m_signal_document_insert;
}

template<typename selector_type>
typename basic_buffer<selector_type>::signal_document_rename_type
basic_buffer<selector_type>::document_rename_event() const
{
	return m_signal_document_rename;
}

template<typename selector_type>
typename basic_buffer<selector_type>::signal_document_remove_type
basic_buffer<selector_type>::document_remove_event() const
{
	return m_signal_document_remove;
}

template<typename selector_type>
typename basic_buffer<selector_type>::signal_message_type
basic_buffer<selector_type>::message_event() const
{
	return m_signal_message;
}

template<typename selector_type>
typename basic_buffer<selector_type>::signal_server_message_type
basic_buffer<selector_type>::server_message_event() const
{
	return m_signal_server_message;
}

template<typename selector_type>
net6::basic_parameter*
basic_buffer<selector_type>::translate_user(const std::string& str) const
{
	// Read user ID
	unsigned int user_id;
	std::stringstream stream(str);
	stream >> std::hex >> user_id;

	// Check for success
	if(stream.bad() )
	{
		throw net6::basic_parameter::bad_format(
			"User ID ought to be a hexadecimal integer"
		);
	}

	// No user
	if(user_id == 0) return new net6::parameter<user*>(NULL);

	// Find corresponding user in user table
	const user* found_user = m_user_table.find(user_id);
	if(found_user == NULL)
	{
		// No such user
		format_string str("User ID %0% does not exist");
		str << user_id;
		throw net6::basic_parameter::bad_format(str.str() );
	}

	// Done
	// TODO: Type should be net6::parameter<const user*>
	return new net6::parameter<user*>(const_cast<user*>(found_user) );
}

template<typename selector_type>
net6::basic_parameter*
basic_buffer<selector_type>::translate_document(const std::string& str) const
{
	// Read document and owner ID
	unsigned int owner_id, document_id;
	std::stringstream stream(str);
	stream >> std::hex >> owner_id >> document_id;

	// Check for success
	if(stream.bad() )
	{
		throw net6::basic_parameter::bad_format(
			"Document ID ought to be two hexadecimal integers"
		);
	}

	// Lookup document
	basic_document_info<selector_type>* info =
		document_find(owner_id, document_id);

	if(info == NULL)
	{
		// No such document
		format_string str("Document ID %0%/%1% does not exist");
		str << owner_id << document_id;
		throw net6::basic_parameter::bad_format(str.str() );
	}

	return new net6::parameter<basic_document_info<selector_type>*>(info);
}

template<typename selector_type>
void basic_buffer<selector_type>::document_add(document_info& document)
{
	// Add new document into list
	m_docs.push_back(&document);
	// Emit document_insert signal
	m_signal_document_insert.emit(document);
	// Emit user_subscribe signal for each user that is initially
	// subscribed to the document.
	for(typename document_info::user_iterator iter = document.user_begin();
	    iter != document.user_end();
	    ++ iter)
		document.subscribe_event().emit(*iter);
}

template<typename selector_type>
void basic_buffer<selector_type>::document_delete(document_info& info)
{
	// TODO: Emit user_unsubscribe signal for each user that was subscribed?
	// Emit document_remove signal
	m_signal_document_remove.emit(info);
	// Delete from list
	m_docs.erase(
		std::remove(m_docs.begin(), m_docs.end(), &info),
		m_docs.end()
	);

	// Delete document
	delete &info;
}

template<typename selector_type>
void basic_buffer<selector_type>::document_clear()
{
	// TODO: Emit document_remove signal for each document?
	typename std::list<document_info*>::iterator iter;
	for(iter = m_docs.begin(); iter != m_docs.end(); ++ iter)
		delete *iter;

	m_docs.clear();
}

} // namespace obby

#endif // _OBBY_BUFFER_HPP_
