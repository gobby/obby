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
#include "document_packet.hpp"

namespace obby
{

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

	document_info(const user* owner, unsigned int id,
	              const std::string& title);

	/** Returns the owner of this document. It may return NULL if the
	 * document has no owner (indicating that the server created the
	 * document).
	 */
	const user* get_owner() const;

	/** Returns a unique ID for this document.
	 */
	unsigned int get_id() const;

	/** Returns the title set for this document.
	 */
	const std::string& get_title() const;

	/** Returns the content of the document, if available.
	 */
	const document& get_content() const;

	/** Renames the document or requests a rename operation.
	 * signal_rename will be emitted if the document has been renamed.
	 */
	virtual void rename(const std::string& new_title) = 0;

	/** Checks if the given user is subscribed to this document.
	 */
	bool is_subscribed(const user& user) const;

	/** Returns the begin of the list of subscribed users.
	 */
	user_iterator user_begin() const;

	/** Returns the end of the list of subscrbed users.
	 */
	user_iterator user_end() const;

	/** Returns the amount of subscribed users.
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

	/** Called by the buffer when a network packet concerning this document
	 * was received.
	 */
	virtual void on_net_packet(const document_packet& pack) = 0;

	/** Called by the buffer when a user has joined.
	 * TODO: Replace by a signal connection to the buffer.
	 */
	virtual void obby_user_join(const user& user);

	/** Called by the buffer when a user has left.
	 * TODO: Replace by a signal connection to the buffer.
	 */
	virtual void obby_user_part(const user& user);

protected:
	/** Subscribes a user to this document.
	 */
	virtual void user_subscribe(const user& user);

	/** Unsubscribes a user from this document.
	 */
	virtual void user_unsubscribe(const user& user);

	/** Internally renames the document.
	 */
	void rename_impl(const std::string& title);

	const user* m_owner;
	unsigned int m_id;
	std::string m_title;

	std::auto_ptr<document> m_document;
	std::list<const user*> m_users;

	signal_rename_type m_signal_rename;
	signal_subscribe_type m_signal_subscribe;
	signal_unsubscribe_type m_signal_unsubscribe;
};

} // namespace obby

namespace net6
{

/** obby document packet type
 */
template<>
class parameter<obby::document_info*> : public basic_parameter {
public:
	parameter(obby::document_info* document)
	 : basic_parameter(TYPE_ID, document) { }

	virtual basic_parameter* clone() const {
		return new parameter<obby::document_info*>(
			as<obby::document_info*>()
		);
	}

	virtual std::string to_string() const {
		obby::document_info* document = as<obby::document_info*>();

		int owner_id = 0;
		if(document->get_owner() != NULL)
			owner_id = document->get_owner()->get_id();

		std::stringstream stream;
		stream << std::hex << owner_id << " " << document->get_id();
		return stream.str();
	}

	static const identification_type TYPE_ID = 'd';
};

template<>
class parameter<obby::document_info> : public parameter<obby::document_info*> {
public:
	parameter(const obby::document_info& document)
	 : parameter<obby::document_info*>(&const_cast<obby::document_info&>(document) ) { }
};

} // namespace net6

#endif // _OBBY_DOCUMENT_INFO_HPP_
