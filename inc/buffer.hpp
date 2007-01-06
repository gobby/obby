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
#include "document_info.hpp"
#include "user_table.hpp"

namespace obby
{

/** Abstract base class for obby buffers. A buffer contains multiple documents
 * that are synchronised through many users and a user list.
 */

class buffer : private net6::non_copyable
{
public:
	// Document iterator typedef
	typedef std::list<document_info*>::size_type document_size_type;
	typedef ptr_iterator<
		document_info,
		std::list<document_info*>,
		std::list<document_info*>::const_iterator
	> document_iterator;

	// Signal types
	typedef sigc::signal<void, user&> signal_user_join_type;
	typedef sigc::signal<void, user&> signal_user_part_type;
	typedef sigc::signal<void, document_info&> signal_document_insert_type;
	typedef sigc::signal<void, document_info&> signal_document_rename_type;
	typedef sigc::signal<void, document_info&> signal_document_remove_type;
	typedef sigc::signal<void, obby::user&, const std::string&>
		signal_message_type;
	typedef sigc::signal<void, const std::string&>
		signal_server_message_type;

	buffer();
	virtual ~buffer();

	/** Returns the user table associated with the buffer.
	 */
	const user_table& get_user_table() const;

	/** Waits indefinitely for incoming events.
	 */
	virtual void select() = 0;

	/** Waits for incoming events or until <em>timeout</em> expires.
	 */
	virtual void select(unsigned int timeout) = 0;

	/* Creates a new document with predefined content.
	 * signal_document_insert will be emitted if it has been created.
	 */
	virtual void create_document(const std::string& title,
	                             const std::string& content = "") = 0;

	/** Removes an existing document. signal_document_remove will be
	 * emitted if the document has been removed.
	 */
	virtual void remove_document(document_info& doc) = 0;
	
	/** Looks for a document with the given ID which belongs to the user
	 * with the given owner ID. Note that we do not take a real user object
	 * here because the ID is enough and one might not have a user object
	 * to the corresponding ID. So a time-consuming lookup is obsolete.
	 */
	document_info* find_document(unsigned int owner_id,
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

	/** Signal which will be emitted if a new user has joined the obby
	 * session.
	 */
	signal_user_join_type user_join_event() const;

	/** Signal which will be emitted if a user has quit.
	 */
	signal_user_part_type user_part_event() const;
	
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
        /** Adds a new document with the given title to the buffer.
	 */
	virtual document_info& add_document_info(const user* owner,
	                                         unsigned int id,
	                                         const std::string& title) = 0;

	/** Translates a user parameter back to a net6 packet parameter.
	 */
	net6::basic_parameter* translate_user(const std::string& str) const;

	/** Translates a document parameter back to a net6 packet parameter.
	 */
	net6::basic_parameter* translate_document(const std::string& str) const;

	/** net6 main object to keep net6 initialised during the obby session.
	 */
	net6::main m_netkit;

	/** GMP random number generator.
	 */
	gmp_randclass m_rclass;

	/** User table which stores all the users in the session.
	 */
	user_table m_usertable;

	/** List of documents.
	 */
	std::list<document_info*> m_doclist;

	/** Counter for document IDs.
	 */
	unsigned int m_doc_counter;

	signal_user_join_type m_signal_user_join;
	signal_user_part_type m_signal_user_part;

	signal_document_insert_type m_signal_document_insert;
	signal_document_rename_type m_signal_document_rename;
	signal_document_remove_type m_signal_document_remove;

	signal_message_type m_signal_message;
	signal_server_message_type m_signal_server_message;
};

}

#endif // _OBBY_BUFFER_HPP_
