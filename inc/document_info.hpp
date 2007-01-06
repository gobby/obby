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

#ifndef _OBBY_DOCUMENT_INFO_HPP_
#define _OBBY_DOCUMENT_INFO_HPP_

#include <sigc++/signal.h>
#include "ptr_iterator.hpp"
#include "user.hpp"
#include "document.hpp"

namespace obby
{

class buffer;

/** Information about a document that is provided without being subscribed to
 * a document.
 */

class document_info : private net6::non_copyable
{
public:
	typedef sigc::signal<void, const std::string&> signal_rename_type;
	typedef sigc::signal<void, const user&> signal_subscribe_type;
	typedef sigc::signal<void, const user&> signal_unsubscribe_type;

	typedef std::list<user*>::size_type user_size_type;
	typedef ptr_iterator<
		const user,
		std::list<const user*>,
		std::list<const user*>::const_iterator
	> user_iterator;

	document_info(const buffer& buf, unsigned int id,
	              const std::string& title);
	~document_info();

	/** Returns a unique ID for this document.
	 */
	unsigned int get_id() const;

	/** Returns the title set for this document.
	 */
	const std::string& get_title() const;

	/** Returns the buffer to which the document is assigned.
	 */
	const buffer& get_buffer() const;

	/** Returns the document for this info, if one is assigned.
	 */
	document* get_document();

	/** Returns the document for this info, if one is assigned.
	 */
	const document* get_document() const;

	/** Renames the document or requests a rename operation.
	 * signal_rename will be emitted if the document has been renamed.
	 */
	virtual void rename(const std::string& new_title) = 0;

	/** Looks if the user with the given ID is subscribed to this document.
	 */
	const user* find_user(unsigned int id) const;

	/** Checks if the given user is subscribed to this document.
	 */
	bool is_subscribed(const user& from) const;

	/** Returns the begin of the list of subscribed users.
	 */
	user_iterator user_begin() const;

	/** Returns the end of the list of subscrbed users.
	 */
	user_iterator user_end() const;

	/** Retruns the amount of subscribed users.
	 */
	user_size_type user_count() const;

	/** Signal which is emitted if the documents gets renamed.
	 */
	signal_rename_type rename_event() const;

	/** Signal which is emitted if a user subscribed to this document.
	 */
	signal_subscribe_type subscribe_event() const;

	/** Signal which is emitted if a user unsubscribed from this document.
	 */
	signal_unsubscribe_type unsubscribe_event() const;

	/** Called by the buffer when a user has joined the obby session.
	 */
	virtual void obby_user_join(const user& new_user);
	
	/** Called by the buffer when a user has left the obby session.
	 */
	virtual void obby_user_part(const user& existing_user);

protected:
	/** Assigns a document to the document info.
	 */
	virtual void assign_document() = 0;

	/** Releases the underlaying document from the info.
	 */
	void release_document();

	const buffer& m_buffer;
	unsigned int m_id;
	std::string m_title;
	document* m_document;

	std::list<const user*> m_userlist;

	signal_rename_type m_signal_rename;
	signal_subscribe_type m_signal_subscribe;
	signal_unsubscribe_type m_signal_unsubscribe;
};

}

#endif // _OBBY_DOCUMENT_INFO_HPP_
