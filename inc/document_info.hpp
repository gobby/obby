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
#include "format_string.hpp"
#include "user.hpp"
#include "document.hpp"
#include "document_packet.hpp"
#include "serialise/object.hpp"

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
	class privileges
	{
	public:
		static const privileges NONE;
		static const privileges SUBSCRIBE;
		static const privileges MODIFY;
		static const privileges CLOSE;
		static const privileges RENAME;
		static const privileges ADMIN;
		static const privileges ALL;

        	privileges operator|(privileges other) const { return privileges(m_value | other.m_value); }
	        privileges operator&(privileges other) const { return privileges(m_value & other.m_value); }
        	privileges operator^(privileges other) const { return privileges(m_value ^ other.m_value); }
	        privileges& operator|=(privileges other) { m_value |= other.m_value; return *this; }
        	privileges& operator&=(privileges other) { m_value &= other.m_value; return *this; }
	        privileges& operator^=(privileges other) { m_value ^= other.m_value; return *this; }
        	privileges operator~() const { return privileges(~m_value); }

		operator bool() const { return m_value != NONE.m_value; }
		bool operator!() const { return m_value == NONE.m_value; }
	        bool operator==(privileges other) const { return m_value == other.m_value; }
        	bool operator!=(privileges other) const { return m_value != other.m_value; }

	        unsigned int get_value() const { return m_value; }

	protected:
        	explicit privileges(unsigned int value) : m_value(value) { }

	        unsigned int m_value;
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

	basic_document_info(const basic_buffer<selector_type>& buffer,
	                    net6::basic_object<selector_type>& net,
	                    const serialise::object& obj);

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

	/** Returns the privileges table for this document.
	 */
	const privileges_table& get_privileges_table() const;

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

template<typename selector_type>
const typename basic_document_info<selector_type>::privileges basic_document_info<selector_type>::privileges::NONE = typename basic_document_info<selector_type>::privileges(0x00000000);
template<typename selector_type>
const typename basic_document_info<selector_type>::privileges basic_document_info<selector_type>::privileges::SUBSCRIBE = typename basic_document_info<selector_type>::privileges(0x00000001);
template<typename selector_type>
const typename basic_document_info<selector_type>::privileges basic_document_info<selector_type>::privileges::MODIFY = typename basic_document_info<selector_type>::privileges(0x00000002);
template<typename selector_type>
const typename basic_document_info<selector_type>::privileges basic_document_info<selector_type>::privileges::CLOSE = typename basic_document_info<selector_type>::privileges(0x00000004);
template<typename selector_type>
const typename basic_document_info<selector_type>::privileges basic_document_info<selector_type>::privileges::RENAME = typename basic_document_info<selector_type>::privileges(0x00000008);
template<typename selector_type>
const typename basic_document_info<selector_type>::privileges basic_document_info<selector_type>::privileges::ADMIN = typename basic_document_info<selector_type>::privileges(0x00000010);
template<typename selector_type>
const typename basic_document_info<selector_type>::privileges basic_document_info<selector_type>::privileges::ALL = typename basic_document_info<selector_type>::privileges(~basic_document_info<selector_type>::privileges::NONE);

/** Table that stores the privileges for multiple users.
 */
template<typename selector_type>
class basic_document_info<selector_type>::privileges_table
{
public:
	typedef sigc::signal<void, const user&, privileges>
		signal_privileges_changed_type;

	/** Creates a new privileges_table with the given default privileges.
	 * The default privileges are used if there is no privileges entry for
	 * a user.
	 */
	privileges_table(privileges default_privileges);

	/** Returns the default privileges set for this table.
	 */
	privileges get_default_privileges() const;

	/** Queries the privileges for the given user.
	 */
	privileges privileges_query(const user& user,
	                            privileges privs = privileges::ALL) const;

	/** Changes the privileges of a user.
	 */
	void privileges_change(const user& user, privileges privs);

	/** Signal which is emitted if the privileges for a given user have
	 * changed (by a document administrator or the document owner).
	 */
	signal_privileges_changed_type privileges_changed_event() const;
protected:
	typedef std::map<const user*, privileges> priv_map;

	privileges m_default_privs;
	priv_map m_privs;

	signal_privileges_changed_type m_signal_privileges_changed;
};

} // namespace obby

namespace serialise
{

// TODO: Make a specialised version with const data_type in net6 for const
// objects.
template<typename selector_type>
class context<obby::basic_document_info<selector_type>*>
{
public:
	typedef obby::basic_buffer<selector_type> buffer;
	typedef obby::basic_document_info<selector_type> document_info;

	context();
	context(const buffer& buffer);

	virtual std::string to_string(document_info* from) const;
	virtual document_info* from_string(const std::string& string) const;
protected:
	const buffer* m_buffer;
};

template<typename selector_type>
class context<const obby::basic_document_info<selector_type>*>
{
public:
	typedef obby::basic_buffer<selector_type> buffer;
	typedef obby::basic_document_info<selector_type> document_info;

	context();
	context(const buffer& buffer);

	virtual std::string to_string(const document_info* from) const;
	virtual const document_info* from_string(const std::string& string) const;
protected:
	const buffer* m_buffer;
};

template<typename selector_type>
context<obby::basic_document_info<selector_type>*>::context():
	m_buffer(NULL)
{
}

template<typename selector_type>
context<const obby::basic_document_info<selector_type>*>::context():
	m_buffer(NULL)
{
}

template<typename selector_type>
context<obby::basic_document_info<selector_type>*>::
	context(const buffer& buffer):
	m_buffer(&buffer)
{
}

template<typename selector_type>
context<const obby::basic_document_info<selector_type>*>::
	context(const buffer& buffer):
	m_buffer(&buffer)
{
}

template<typename selector_type>
std::string context<obby::basic_document_info<selector_type>*>::
	to_string(document_info* from) const
{
	typedef context<const document_info*> const_context;

	// Use const context to convert
	if(m_buffer == NULL)
	{
		const_context ctx;
		return ctx.to_string(from);
	}
	else
	{
		const_context ctx(*m_buffer);
		return ctx.to_string(from);
	}
}

template<typename selector_type>
typename context<obby::basic_document_info<selector_type>*>::document_info*
context<obby::basic_document_info<selector_type>*>::
	from_string(const std::string& string) const
{
	// We need a buffer to lookup the document
	if(m_buffer == NULL)
		throw conversion_error("Buffer object required");

	// Read document and owner id
	unsigned int owner_id, document_id;
	std::stringstream stream(string);
	stream >> owner_id >> document_id;

	// Successful conversion?
	if(stream.bad() )
		throw conversion_error("Document ID ought to be two integers");

	// Lookup document
	document_info* info = m_buffer->document_find(owner_id, document_id);

	if(info == NULL)
	{
		// No such document
		obby::format_string str("Document ID %0%/%1% does not exist");
		str << owner_id << document_id;
		throw conversion_error(str.str() );
	}

	// Done
	return info;
}

template<typename selector_type>
std::string context<const obby::basic_document_info<selector_type>*>::
	to_string(const document_info* from) const
{
	std::stringstream stream;
	stream << from->get_owner_id() << ' ' << from->get_id();
	return stream.str();
}

template<typename selector_type>
const typename context<const obby::basic_document_info<selector_type>*>::
	document_info*
context<const obby::basic_document_info<selector_type>*>::
	from_string(const std::string& string) const
{
	typedef context<document_info*> nonconst_context;

	// Use non-const context to convert
	if(m_buffer == NULL)
	{
		nonconst_context ctx;
		return ctx.from_string(string);
	}
	else
	{
		nonconst_context ctx(*m_buffer);
		return ctx.from_string(string);
	}
}

} // namespace serialise

namespace obby
{

typedef basic_document_info<net6::selector> document_info;

template<typename selector_type>
basic_document_info<selector_type>::
	basic_document_info(const basic_buffer<selector_type>& buffer,
	                    net6::basic_object<selector_type>& net,
	                    const user* owner, unsigned int id,
	                    const std::string& title):
	m_buffer(buffer), m_net(net), m_owner(owner), m_id(id), m_title(title),
	m_priv_table(
		new privileges_table(privileges::SUBSCRIBE | privileges::MODIFY)
	)
{
}

template<typename selector_type>
basic_document_info<selector_type>::
	basic_document_info(const basic_buffer<selector_type>& buffer,
	                    net6::basic_object<selector_type>& net,
	                    const serialise::object& obj):
	m_buffer(buffer), m_net(net),
	m_owner(
		obj.get_required_attribute("owner").
			serialise::attribute::as<const user*>(
				buffer.get_user_table()
			)
	),
	m_id(
		obj.get_required_attribute("id").
			serialise::attribute::as<unsigned int>()
	),
	m_title(
		obj.get_required_attribute("title").
			serialise::attribute::as<std::string>()
	),
	m_priv_table(
		new privileges_table(privileges::SUBSCRIBE | privileges::MODIFY)
	)
{
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
const typename basic_document_info<selector_type>::privileges_table&
basic_document_info<selector_type>::get_privileges_table() const
{
	return *m_priv_table;
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
}

template<typename selector_type>
void basic_document_info<selector_type>::obby_user_part(const user& user)
{
	// User left the session: Unsubscribe from document
	if(is_subscribed(user) )
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
basic_document_info<selector_type>::privileges_table::
	privileges_table(privileges default_privileges)
 : m_default_privs(default_privileges)
{
}

template<typename selector_type>
typename basic_document_info<selector_type>::privileges
basic_document_info<selector_type>::privileges_table::
	get_default_privileges() const
{
	return m_default_privs;
}

template<typename selector_type>
typename basic_document_info<selector_type>::privileges
basic_document_info<selector_type>::privileges_table::
	privileges_query(const user& user, privileges privs) const
{
	typename priv_map::const_iterator iter = m_privs.find(&user);
	if(iter == m_privs.end() ) return m_default_privs & privs;
	return iter->second & privs;
}

template<typename selector_type>
void basic_document_info<selector_type>::privileges_table::
	privileges_change(const user& user, privileges privs)
{
	m_privs[&user] = privs;
	m_signal_privileges_changed.emit(user, privs);
}

template<typename selector_type>
typename basic_document_info<selector_type>::privileges_table::
	signal_privileges_changed_type
basic_document_info<selector_type>::privileges_table::
	privileges_changed_event() const
{
	return m_signal_privileges_changed;
}

} // namespace obby

#endif // _OBBY_DOCUMENT_INFO_HPP_


