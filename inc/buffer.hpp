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
#include "user.hpp"
#include "document.hpp"

namespace obby
{

/** Abstract base class for obby buffers. A buffer contains multiple documents
 * that are synchronized through many users and a user list.
 */

class buffer
{
public:
	typedef sigc::signal<void, document&> signal_insert_doc_type;
	typedef sigc::signal<void, document&> signal_remove_doc_type;

	buffer();
	virtual ~buffer();

	/** Adds a new document to the buffer and uses the internal ID counter
	 * to assign a unique ID to it.
	 */
	document& add_document();

	/** Adds a new document with the given ID to the buffer. The internal
	 * ID counter is set to the new given document ID.
	 */
	virtual document& add_document(unsigned int id) = 0;

	/** Looks for a document with the given ID.
	 */
	document* find_document(unsigned int id) const;

	/** Looks for a user with the given ID.
	 */
	user* find_user(unsigned int id) const;

	/** Looks for a user with the given user name.
	 */
	user* find_user(const std::string& name) const;

	/** Signal which will be emitted when another participiant in the
	 * obby session has created a new document.
	 */
	signal_insert_doc_type insert_doc_event() const;

	/** Signal which will be emitted when another participiant in the
	 * obby session has removed an existing document.
	 */
	signal_remove_doc_type remove_doc_event() const;
protected:
	/** Internal function to add a new user to the user list.
	 */
	virtual user* add_user(net6::peer& peer, int red, int green, int blue);

	unsigned int m_doc_counter;
	std::list<document*> m_doclist;
	std::list<user*> m_userlist;

	signal_insert_doc_type signal_insert_doc;
	signal_remove_doc_type signal_remove_doc;
};

}

#endif // _OBBY_BUFFER_HPP_
