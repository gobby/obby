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

#ifndef _OBBY_BUFFER_HPP_
#define _OBBY_BUFFER_HPP_

#include <string>
#include <list>
#include <sigc++/signal.h>
#include <net6/non_copyable.hpp>
#include "user.hpp"
#include "document.hpp"
#include "user_table.hpp"

namespace obby
{

/** Abstract base class for obby buffers. A buffer contains multiple documents
 * that are synchronized through many users and a user list.
 */

class buffer : private net6::non_copyable
{
public:
	/** Iterator to iterate through the list of documents.
	 */
	class document_iterator : public std::list<document*>::const_iterator
	{
	public:
		typedef std::list<document*>::const_iterator base_iterator;

		document_iterator();
		document_iterator(const base_iterator& iter);

		document_iterator& operator=(const base_iterator& iter);

		document& operator*();
		const document& operator*() const;

		document* operator->();
		const document* operator->() const;
	};

	typedef sigc::signal<void, user&> signal_user_join_type;
	typedef sigc::signal<void, user&> signal_user_part_type;
	typedef sigc::signal<void, document&> signal_insert_document_type;
	typedef sigc::signal<void, document&> signal_remove_document_type;

	buffer();
	virtual ~buffer();

	/** Looks for a document with the given ID.
	 */
	document* find_document(unsigned int id) const;

	/** Looks for a user with the given ID.
	 */
	user* find_user(unsigned int id) const;

	/** Looks for a user with the given user name.
	 */
	user* find_user(const std::string& name) const;

	/** Returns the begin of the document list.
	 */
	document_iterator document_begin() const;

	/** Returns the end of the document list.
	 */
	document_iterator document_end() const;
	
	/** Signal which will be emitted if a new user has joined the obby
	 * session.
	 */
	signal_user_join_type user_join_event() const;

	/** Signal which will be emitted if a user has quit.
	 */
	signal_user_part_type user_part_event() const;
	
	/** Signal which will be emitted when another participiant in the
	 * obby session has created a new document.
	 */
	signal_insert_document_type insert_document_event() const;

	/** Signal which will be emitted when another participiant in the
	 * obby session has removed an existing document.
	 */
	signal_remove_document_type remove_document_event() const;
protected:
	/** Internal function to add a new user to the user list.
	 */
	virtual user* add_user(net6::peer& peer, int red, int green, int blue);

	/** Internal function to remove an existing user from the user list.
	 */
	virtual void remove_user(user* user_to_remove);

	/** Adds a new document with the given ID to the buffer. The internal
	 * ID counter is set to the new given document ID.
	 */
	virtual document& add_document(unsigned int id) = 0;

	/** User table to identify users through multiple obby sessions.
	 */
	user_table* m_usertable;

	/** List of documents.
	 */
	std::list<document*> m_doclist;

	/** User list.
	 */
	std::list<user*> m_userlist;

	signal_user_join_type m_signal_user_join;
	signal_user_part_type m_signal_user_part;

	signal_insert_document_type m_signal_insert_document;
	signal_remove_document_type m_signal_remove_document;
};

}

#endif // _OBBY_BUFFER_HPP_
