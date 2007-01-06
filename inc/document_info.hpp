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
#include <net6/object.hpp>
#include "ptr_iterator.hpp"
#include "user.hpp"
#include "user_table.hpp"
#include "document.hpp"
#include "document_packet.hpp"

namespace obby
{

template<typename selector_type>
class basic_buffer;

/** Information about a document that is provided without being subscribed to
 * a document.
 */
template<typename selector_type>
class basic_document_info : private net6::non_copyable, public sigc::trackable
{
public:
	enum privileges {
		PRIV_NONE      = 0x00,
		PRIV_SUBSCRIBE = 0x01, // User may subscribe to document
		PRIV_MODIFY    = 0x02, // User may modify the document
		PRIV_CLOSE     = 0x04, // User may close the document (completly)
		PRIV_RENAME    = 0x08  // TODO: Implement rename in Gobby :)
	};

	class privileges_table;

	typedef sigc::signal<void, const std::string&> signal_rename_type;
	typedef sigc::signal<void, const user&> signal_subscribe_type;
	typedef sigc::signal<void, const user&> signal_unsubscribe_type;

	typedef std::list<user*>::size_type user_size_type;
	typedef ptr_iterator<
		const user,
		std::list<const user*>,
		std::list<const user*>::const_iterator
	> user_iterator;

	basic_document_info(const basic_buffer<selector_type>& buffer,
	                    net6::basic_object<selector_type>& net,
	                    const user* owner, unsigned int id,
	                    const std::string& title);

	/** Returns the owner of this document. It may return NULL if the
	 * document has no owner (indicating that the server created the
	 * document).
	 */
	const user* get_owner() const;

	/** Returns the ID of the owner or 0 if the document has no owner.
	 */
	unsigned int get_owner_id() const;

	/** Returns a unique ID for this document.
	 */
	unsigned int get_id() const;

	/** Returns the title set for this document.
	 */
	const std::string& get_title() const;

	/** Returns the content of the document, if available.
	 */
	const document& get_content() const;

	/** Inserts the given text at the given position into the document.
	 */
	virtual void insert(position pos, const std::string& text) = 0;

	/** Erases the given range from the document.
	 */
	virtual void erase(position pos, position len) = 0;

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
//	virtual void on_net_packet(const document_packet& pack) = 0;

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
	void document_rename(const std::string& title);

	/** Internal function to create the underlaying document.
	 */
	void assign_document();

	/** Internal function to release the underlaying document.
	 */
	void release_document();

	const basic_buffer<selector_type>& m_buffer;
	net6::basic_object<selector_type>& m_net;

	const user* m_owner;
	unsigned int m_id;
	std::string m_title;

	std::auto_ptr<privileges_table> m_priv_table;
	std::auto_ptr<document> m_document;
	std::list<const user*> m_users;

	signal_rename_type m_signal_rename;
	signal_subscribe_type m_signal_subscribe;
	signal_unsubscribe_type m_signal_unsubscribe;

public:
	/** Returns the buffer to which this document_info belongs.
	 */
	const basic_buffer<selector_type>& get_buffer() const;

private:
	/** Returns the underlaying net6 object through which requests are
	 * transmitted.
	 */
	net6::basic_object<selector_type>& get_net6();

	/** Returns the underlaying net6 object through which requests are
	 * transmitted.
	 */
	const net6::basic_object<selector_type>& get_net6() const;
};

// typename basic_document_info<selector_type>::privileges combination operators
template<typename selector_type>
inline typename basic_document_info<selector_type>::privileges operator|(typename basic_document_info<selector_type>::privileges lhs, typename basic_document_info<selector_type>::privileges rhs) {
	return static_cast<typename basic_document_info<selector_type>::privileges>(
		static_cast<int>(lhs) | static_cast<int>(rhs)
	);
}

template<typename selector_type>
inline typename basic_document_info<selector_type>::privileges operator&(typename basic_document_info<selector_type>::privileges lhs, typename basic_document_info<selector_type>::privileges rhs) {
	return static_cast<typename basic_document_info<selector_type>::privileges>(
		static_cast<int>(lhs) & static_cast<int>(rhs)
	);
}

template<typename selector_type>
inline typename basic_document_info<selector_type>::privileges operator^(typename basic_document_info<selector_type>::privileges lhs, typename basic_document_info<selector_type>::privileges rhs) {
	return static_cast<typename basic_document_info<selector_type>::privileges>(
		static_cast<int>(lhs) ^ static_cast<int>(rhs)
	);
}

template<typename selector_type>
inline typename basic_document_info<selector_type>::privileges& operator|=(typename basic_document_info<selector_type>::privileges& lhs, typename basic_document_info<selector_type>::privileges rhs) {
	return lhs = (lhs | rhs);
}

template<typename selector_type>
inline typename basic_document_info<selector_type>::privileges& operator&=(typename basic_document_info<selector_type>::privileges& lhs, typename basic_document_info<selector_type>::privileges rhs) {
	return lhs = (lhs & rhs);
}

template<typename selector_type>
inline typename basic_document_info<selector_type>::privileges& operator^=(typename basic_document_info<selector_type>::privileges& lhs, typename basic_document_info<selector_type>::privileges rhs) {
	return lhs = (lhs ^ rhs);
}

template<typename selector_type>
inline typename basic_document_info<selector_type>::privileges operator~(typename basic_document_info<selector_type>::privileges rhs) {
	return static_cast<typename basic_document_info<selector_type>::privileges>(static_cast<int>(rhs) );
}

/** Table that stores the privileges for multiple users.
 */
template<typename selector_type>
class basic_document_info<selector_type>::privileges_table
{
public:
	/** Adds a new user with the user's default privileges.
	 */
	void user_add(const user& user);

	/** Queries the privileges for the given user.
	 */
	privileges user_privileges(const user& user) const;
protected:
	typedef std::map<const user*, privileges> priv_map;
	priv_map m_privs;
};

} // namespace obby

namespace net6
{

/** obby document packet type
 */
template<typename selector_type>
class parameter<obby::basic_document_info<selector_type>*>
 : public basic_parameter {
public:
	parameter(obby::basic_document_info<selector_type>* document)
	 : basic_parameter(TYPE_ID, document) { }

	virtual basic_parameter* clone() const {
		return new parameter<obby::basic_document_info<selector_type>*>(
			as<obby::basic_document_info<selector_type>*>()
		);
	}

	virtual std::string to_string() const {
		obby::basic_document_info<selector_type>* document =
			as<obby::basic_document_info<selector_type>*>();

		int owner_id = 0;
		if(document->get_owner() != NULL)
			owner_id = document->get_owner()->get_id();

		std::stringstream stream;
		stream << std::hex << owner_id << " " << document->get_id();
		return stream.str();
	}

	static const identification_type TYPE_ID = 'd';
};

template<typename selector_type>
class parameter<obby::basic_document_info<selector_type> > :
 public parameter<obby::basic_document_info<selector_type>*> {
public:
	parameter(const obby::basic_document_info<selector_type>& document)
	 : parameter<obby::basic_document_info<selector_type>*>(
		&const_cast<obby::basic_document_info<selector_type>&>(document)
	   )
	{
	}
};

} // namespace net6

namespace obby
{

typedef basic_document_info<net6::selector> document_info;

template<typename selector_type>
basic_document_info<selector_type>::
	basic_document_info(const basic_buffer<selector_type>& buffer,
	                    net6::basic_object<selector_type>& net,
	                    const user* owner, unsigned int id,
	                    const std::string& title)
 : m_buffer(buffer), m_net(net), m_owner(owner), m_id(id), m_title(title),
   m_priv_table(new privileges_table)
{
	// Add all initial users to the table, with default values
	const user_table& user_table = buffer.get_user_table();
	for(user_table::user_iterator<user::NONE> iter =
		user_table.user_begin<user::NONE>();
	    iter != user_table.user_end<user::NONE>();
	    ++ iter)
	{
		m_priv_table->user_add(*iter);
	}
}

template<typename selector_type>
const user* basic_document_info<selector_type>::get_owner() const
{
	return m_owner;
}

template<typename selector_type>
unsigned int basic_document_info<selector_type>::get_owner_id() const
{
	if(m_owner == NULL)
		return 0;

	return m_owner->get_id();
}

template<typename selector_type>
unsigned int basic_document_info<selector_type>::get_id() const
{
	return m_id;
}

template<typename selector_type>
const std::string& basic_document_info<selector_type>::get_title() const
{
	return m_title;
}

template<typename selector_type>
const document& basic_document_info<selector_type>::get_content() const
{
	if(m_document.get() == NULL)
		throw std::logic_error("obby::document_info::get_content");

	return *m_document;
}

template<typename selector_type>
bool basic_document_info<selector_type>::is_subscribed(const user& user) const
{
	return std::find(
		m_users.begin(),
		m_users.end(),
		&user
	) != m_users.end();
}

template<typename selector_type>
typename basic_document_info<selector_type>::user_iterator
basic_document_info<selector_type>::user_begin() const
{
	return m_users.begin();
}

template<typename selector_type>
typename basic_document_info<selector_type>::user_iterator
basic_document_info<selector_type>::user_end() const
{
	return m_users.end();
}

template<typename selector_type>
typename basic_document_info<selector_type>::user_size_type
basic_document_info<selector_type>::user_count() const
{
	return m_users.size();
}

template<typename selector_type>
typename basic_document_info<selector_type>::signal_rename_type
basic_document_info<selector_type>::rename_event() const
{
	return m_signal_rename;
}

template<typename selector_type>
typename basic_document_info<selector_type>::signal_subscribe_type
basic_document_info<selector_type>::subscribe_event() const
{
	return m_signal_subscribe;
}

template<typename selector_type>
typename basic_document_info<selector_type>::signal_unsubscribe_type
basic_document_info<selector_type>::unsubscribe_event() const
{
	return m_signal_unsubscribe;
}

template<typename selector_type>
void basic_document_info<selector_type>::obby_user_join(const user& user)
{
	// Add new user into privileges table
	m_priv_table->user_add(user);
}

template<typename selector_type>
void basic_document_info<selector_type>::obby_user_part(const user& user)
{
	// User left the session: Unsubscribe from document
	user_unsubscribe(user);
}

template<typename selector_type>
void basic_document_info<selector_type>::user_subscribe(const user& user)
{
	if(is_subscribed(user) )
		throw std::logic_error("basic_document_info::user_subscribe");

	m_users.push_back(&user);
	m_signal_subscribe.emit(user);
}

template<typename selector_type>
void basic_document_info<selector_type>::user_unsubscribe(const user& user)
{
	if(!is_subscribed(user) )
		throw std::logic_error("basic_document_info::user_unsubscribe");

	m_users.erase(
		std::remove(m_users.begin(), m_users.end(), &user),
		m_users.end()
	);

	m_signal_unsubscribe.emit(user);
}

template<typename selector_type>
void basic_document_info<selector_type>::
	document_rename(const std::string& title)
{
	m_title = title;
	m_signal_rename.emit(title);
}

template<typename selector_type>
void basic_document_info<selector_type>::assign_document()
{
	m_document.reset(new document);
}

template<typename selector_type>
void basic_document_info<selector_type>::release_document()
{
	m_document.reset(NULL);
}

template<typename selector_type>
const basic_buffer<selector_type>&
basic_document_info<selector_type>::get_buffer() const
{
	return m_buffer;
}

template<typename selector_type>
net6::basic_object<selector_type>&
basic_document_info<selector_type>::get_net6()
{
	return m_net;
}

template<typename selector_type>
const net6::basic_object<selector_type>&
basic_document_info<selector_type>::get_net6() const
{
	return m_net;
}

// privileges_table
template<typename selector_type>
void basic_document_info<selector_type>::privileges_table::
	user_add(const user& user)
{
	// TODO: Initial privileges as parameter
	m_privs[&user] = basic_document_info<selector_type>::PRIV_NONE;
}

template<typename selector_type>
typename basic_document_info<selector_type>::privileges
basic_document_info<selector_type>::privileges_table::
	user_privileges(const user& user) const
{
	typename priv_map::const_iterator iter = m_privs.find(&user);
	if(iter == m_privs.end() )
	{
		throw std::logic_error(
			"obby::basic_document_info::privileges_table::"
			"user_privileges"
		);
	}

	return iter->second;
}

} // namespace obby

#endif // _OBBY_DOCUMENT_INFO_HPP_

