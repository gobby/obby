/* libobby - Network text editing library
 * Copyright (C) 2005, 2006 0x539 dev group
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
#include "position.hpp"
#include "ptr_iterator.hpp"
#include "format_string.hpp"
#include "user.hpp"
#include "document_packet.hpp"
#include "serialise/object.hpp"

namespace obby
{

template<typename Document, typename Selector>
class basic_buffer;

/** Information about a document that is provided without being subscribed to
 * a document.
 */
template<typename Document, typename Selector>
class basic_document_info: private net6::non_copyable, public sigc::trackable
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

        	privileges operator|(privileges other) const
		{
			return privileges(m_value | other.m_value);
		}

	        privileges operator&(privileges other) const
		{
			return privileges(m_value & other.m_value);
		}

        	privileges operator^(privileges other) const
		{
			return privileges(m_value ^ other.m_value);
		}

	        privileges& operator|=(privileges other)
		{
			m_value |= other.m_value; return *this;
		}

        	privileges& operator&=(privileges other)
		{
			m_value &= other.m_value; return *this;
		}

	        privileges& operator^=(privileges other)
		{
			m_value ^= other.m_value; return *this;
		}

        	privileges operator~() const
		{
			return privileges(~m_value);
		}

		operator bool() const { return m_value != NONE.m_value; }
		bool operator!() const { return m_value == NONE.m_value; }

	        bool operator==(privileges other) const
		{
			return m_value == other.m_value;
		}

        	bool operator!=(privileges other) const
		{
			return m_value != other.m_value;
		}

	        unsigned int get_value() const { return m_value; }

	protected:
        	explicit privileges(unsigned int value) : m_value(value) { }

	        unsigned int m_value;
	};

	typedef Document document_type;
	typedef Selector selector_type;

	class privileges_table;

	typedef sigc::signal<void, const std::string&> signal_rename_type;
	typedef sigc::signal<void, const user&> signal_subscribe_type;
	typedef sigc::signal<void, const user&> signal_unsubscribe_type;

	// TODO: Outsource subscribed users into an own class
	typedef std::list<user*>::size_type user_size_type;
	typedef ptr_iterator<
		const user,
		std::list<const user*>,
		std::list<const user*>::const_iterator
	> user_iterator;

	typedef basic_buffer<document_type, selector_type> buffer_type;
	typedef typename buffer_type::net_type net_type;

	basic_document_info(const buffer_type& buffer,
	                    net_type& net,
	                    const user* owner, unsigned int id,
	                    const std::string& title);

	basic_document_info(const buffer_type& buffer,
	                    net_type& net,
	                    const serialise::object& obj);

	/** Serialises the document into the given serialisation object.
	 */
	void serialise(serialise::object& obj) const;

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
	const Document& get_content() const;

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
	virtual bool is_subscribed(const user& user) const;

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

	const buffer_type& m_buffer;
	net_type& m_net;

	const user* m_owner;
	unsigned int m_id;
	std::string m_title;

	std::auto_ptr<privileges_table> m_priv_table;
	std::auto_ptr<document_type> m_document;
	std::list<const user*> m_users;

	signal_rename_type m_signal_rename;
	signal_subscribe_type m_signal_subscribe;
	signal_unsubscribe_type m_signal_unsubscribe;

public:
	/** Returns the buffer to which this document_info belongs.
	 */
	const buffer_type& get_buffer() const;

protected:
	/** Returns the underlaying net6 object through which requests are
	 * transmitted.
	 */
	net_type& get_net6();

	/** Returns the underlaying net6 object through which requests are
	 * transmitted.
	 */
	const net_type& get_net6() const;
};

template<typename Document, typename Selector>
const typename basic_document_info<Document, Selector>::privileges
basic_document_info<Document, Selector>::privileges::NONE =
typename basic_document_info<Document, Selector>::privileges(0x00000000);

template<typename Document, typename Selector>
const typename basic_document_info<Document, Selector>::privileges
basic_document_info<Document, Selector>::privileges::SUBSCRIBE =
typename basic_document_info<Document, Selector>::privileges(0x00000001);

template<typename Document, typename Selector>
const typename basic_document_info<Document, Selector>::privileges
basic_document_info<Document, Selector>::privileges::MODIFY =
typename basic_document_info<Document, Selector>::privileges(0x00000002);

template<typename Document, typename Selector>
const typename basic_document_info<Document, Selector>::privileges
basic_document_info<Document, Selector>::privileges::CLOSE =
typename basic_document_info<Document, Selector>::privileges(0x00000004);

template<typename Document, typename Selector>
const typename basic_document_info<Document, Selector>::privileges
basic_document_info<Document, Selector>::privileges::RENAME =
typename basic_document_info<Document, Selector>::privileges(0x00000008);

template<typename Document, typename Selector>
const typename basic_document_info<Document, Selector>::privileges
basic_document_info<Document, Selector>::privileges::ADMIN =
typename basic_document_info<Document, Selector>::privileges(0x00000010);

template<typename Document, typename Selector>
const typename basic_document_info<Document, Selector>::privileges
basic_document_info<Document, Selector>::privileges::ALL =
typename basic_document_info<Document, Selector>::privileges(0xffffffff);

/** Table that stores the privileges for multiple users.
 */
template<typename Document, typename Selector>
class basic_document_info<Document, Selector>::privileges_table
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

template<typename DocumentInfo>
class document_info_context
{
public:
	typedef DocumentInfo document_info_type;
	typedef typename document_info_type::buffer_type buffer_type;

	document_info_context();
	document_info_context(const buffer_type& buffer);

	virtual std::string to_string(document_info_type* from) const;

	virtual document_info_type*
	from_string(const std::string& string) const;
protected:
	virtual void on_stream_setup(std::stringstream& stream) const = 0;

	const buffer_type* m_buffer;
};

template<typename Document, typename Selector>
class context<obby::basic_document_info<Document, Selector>*>:
	public document_info_context<
		obby::basic_document_info<Document, Selector>
	>
{
public:
	typedef typename document_info_context<
		obby::basic_document_info<Document, Selector>
	>::document_info_type document_info_type;

	typedef typename document_info_context<
		obby::basic_document_info<Document, Selector>
	>::buffer_type buffer_type;

	context():
		document_info_context<document_info_type>() {}
	context(const buffer_type& buffer):
		document_info_context<document_info_type>(buffer) {}

protected:
	virtual void on_stream_setup(std::stringstream& stream) const;
};

template<typename Document, typename Selector>
class context<const obby::basic_document_info<Document, Selector>*>:
	public document_info_context<
		const obby::basic_document_info<Document, Selector>
	>
{
public:
	typedef typename document_info_context<
		const obby::basic_document_info<Document, Selector>
	>::document_info_type document_info_type;

	typedef typename document_info_context<
		const obby::basic_document_info<Document, Selector>
	>::buffer_type buffer_type;

	context():
		document_info_context<document_info_type>() {}
	context(const buffer_type& buffer):
		document_info_context<document_info_type>(buffer) {}

protected:
	virtual void on_stream_setup(std::stringstream& stream) const;
};

template<typename Document, typename Selector>
class hex_context<obby::basic_document_info<Document, Selector>*>:
	public context<obby::basic_document_info<Document, Selector>*>
{
public:
	typedef typename context<
		obby::basic_document_info<Document, Selector>*
	>::document_info_type document_info_type;

	typedef typename context<
		obby::basic_document_info<Document, Selector>*
	>::buffer_type buffer_type;

	hex_context():
		context<document_info_type*>() {}
	hex_context(const buffer_type& buffer):
		context<document_info_type*>(buffer) {}
protected:
	virtual void on_stream_setup(std::stringstream& stream) const;
};

template<typename Document, typename Selector>
class hex_context<const obby::basic_document_info<Document, Selector>*>:
	public context<const obby::basic_document_info<Document, Selector>*>
{
public:
	typedef typename context<
		const obby::basic_document_info<Document, Selector>*
	>::document_info_type document_info_type;

	typedef typename context<
		const obby::basic_document_info<Document, Selector>*
	>::buffer_type buffer_type;

	hex_context():
		context<const document_info_type*>() {}
	hex_context(const buffer_type& buffer):
		context<const document_info_type*>(buffer) {}
protected:
	virtual void on_stream_setup(std::stringstream& stream) const;
};

template<typename DocumentInfo>
document_info_context<DocumentInfo>::document_info_context():
	m_buffer(NULL)
{
}

template<typename DocumentInfo>
document_info_context<DocumentInfo>::
	document_info_context(const buffer_type& buffer):
	m_buffer(&buffer)
{
}

template<typename DocumentInfo>
std::string document_info_context<DocumentInfo>::
	to_string(document_info_type* from) const
{
	std::stringstream stream;
	on_stream_setup(stream);
	stream << from->get_owner_id() << ' ' << from->get_id();
	return stream.str();
}

template<typename DocumentInfo>
typename document_info_context<DocumentInfo>::document_info_type*
document_info_context<DocumentInfo>::
	from_string(const std::string& string) const
{
	// We need a buffer to lookup the document
	if(m_buffer == NULL)
		throw conversion_error("Buffer object required");

	// Read document and owner id
	unsigned int owner_id, document_id;
	std::stringstream stream(string);
	on_stream_setup(stream);
	stream >> owner_id >> document_id;

	// Successful conversion?
	if(stream.bad() )
		throw conversion_error("Document ID ought to be two integers");

	// Lookup document
	document_info_type* info =
		m_buffer->document_find(owner_id, document_id);

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

template<typename Document, typename Selector>
void context<obby::basic_document_info<Document, Selector>*>::
	on_stream_setup(std::stringstream& stream) const
{
}

template<typename Document, typename Selector>
void context<const obby::basic_document_info<Document, Selector>*>::
	on_stream_setup(std::stringstream& stream) const
{
}

template<typename Document, typename Selector>
void hex_context<obby::basic_document_info<Document, Selector>*>::
	on_stream_setup(std::stringstream& stream) const
{
	stream >> std::hex;
}

template<typename Document, typename Selector>
void hex_context<const obby::basic_document_info<Document, Selector>*>::
	on_stream_setup(std::stringstream& stream) const
{
	stream >> std::hex;
}

} // namespace serialise

namespace obby
{


template<typename Document, typename Selector>
basic_document_info<Document, Selector>::
	basic_document_info(const buffer_type& buffer,
	                    net_type& net,
	                    const user* owner, unsigned int id,
	                    const std::string& title):
	m_buffer(buffer), m_net(net), m_owner(owner), m_id(id), m_title(title),
	m_priv_table(
		new privileges_table(privileges::SUBSCRIBE | privileges::MODIFY)
	)
{
}

template<typename Document, typename Selector>
basic_document_info<Document, Selector>::
	basic_document_info(const buffer_type& buffer,
	                    net_type& net,
	                    const serialise::object& obj):
	m_buffer(buffer), m_net(net),
	m_owner(
		obj.get_required_attribute("owner").
			obby::serialise::attribute::as<const user*>(
				::serialise::context<const user*>(
					buffer.get_user_table()
				)
			)
	),
	m_id(
		obj.get_required_attribute("id").
			obby::serialise::attribute::as<unsigned int>()
	),
	m_title(
		obj.get_required_attribute("title").
			obby::serialise::attribute::as<std::string>()
	),
	m_priv_table(
		new privileges_table(privileges::SUBSCRIBE | privileges::MODIFY)
	)
{
}

template<typename Document, typename Selector>
void basic_document_info<Document, Selector>::
	serialise(serialise::object& obj) const
{
	/* Cannot serialise an object whose content we do not have */
	if(m_document.get() == NULL)
	{
		throw std::logic_error(
			"obby::basic_document_info::serialise:\n"
			"No content available, probably not subscribed"
		);
	}

	obj.add_attribute("owner").set_value(m_owner);
	obj.add_attribute("id").set_value(m_id);
	obj.add_attribute("title").set_value(m_title);

	for(typename document_type::chunk_iterator chunk_it =
		m_document->chunk_begin();
	    chunk_it != m_document->chunk_end();
	    ++ chunk_it)
	{
		serialise::object& chunk = obj.add_child();
		chunk.set_name("chunk");

		chunk.add_attribute("content").set_value(chunk_it.get_text() );
		chunk.add_attribute("author").set_value(chunk_it.get_author() );
	}
}

template<typename Document, typename Selector>
const user* basic_document_info<Document, Selector>::get_owner() const
{
	return m_owner;
}

template<typename Document, typename Selector>
unsigned int basic_document_info<Document, Selector>::get_owner_id() const
{
	if(m_owner == NULL)
		return 0;

	return m_owner->get_id();
}

template<typename Document, typename Selector>
unsigned int basic_document_info<Document, Selector>::get_id() const
{
	return m_id;
}

template<typename Document, typename Selector>
const std::string& basic_document_info<Document, Selector>::get_title() const
{
	return m_title;
}

template<typename Document, typename Selector>
const Document& basic_document_info<Document, Selector>::get_content() const
{
	if(m_document.get() == NULL)
	{
		throw std::logic_error(
			"obby::basic_document_info::get_content:\n"
			"No content available, probably not subscribed"
		);
	}

	return *m_document;
}

template<typename Document, typename Selector>
const typename basic_document_info<Document, Selector>::privileges_table&
basic_document_info<Document, Selector>::get_privileges_table() const
{
	return *m_priv_table;
}

template<typename Document, typename Selector>
bool basic_document_info<Document, Selector>::
	is_subscribed(const user& user) const
{
	// TODO: Use std::set
	return std::find(
		m_users.begin(),
		m_users.end(),
		&user
	) != m_users.end();
}

template<typename Document, typename Selector>
typename basic_document_info<Document, Selector>::user_iterator
basic_document_info<Document, Selector>::user_begin() const
{
	return m_users.begin();
}

template<typename Document, typename Selector>
typename basic_document_info<Document, Selector>::user_iterator
basic_document_info<Document, Selector>::user_end() const
{
	return m_users.end();
}

template<typename Document, typename Selector>
typename basic_document_info<Document, Selector>::user_size_type
basic_document_info<Document, Selector>::user_count() const
{
	return m_users.size();
}

template<typename Document, typename Selector>
typename basic_document_info<Document, Selector>::signal_rename_type
basic_document_info<Document, Selector>::rename_event() const
{
	return m_signal_rename;
}

template<typename Document, typename Selector>
typename basic_document_info<Document, Selector>::signal_subscribe_type
basic_document_info<Document, Selector>::subscribe_event() const
{
	return m_signal_subscribe;
}

template<typename Document, typename Selector>
typename basic_document_info<Document, Selector>::signal_unsubscribe_type
basic_document_info<Document, Selector>::unsubscribe_event() const
{
	return m_signal_unsubscribe;
}

template<typename Document, typename Selector>
void basic_document_info<Document, Selector>::obby_user_join(const user& user)
{
}

template<typename Document, typename Selector>
void basic_document_info<Document, Selector>::obby_user_part(const user& user)
{
	// User left the session: Unsubscribe from document
	if(is_subscribed(user) )
		user_unsubscribe(user);
}

template<typename Document, typename Selector>
void basic_document_info<Document, Selector>::user_subscribe(const user& user)
{
	if(is_subscribed(user) )
	{
		throw std::logic_error(
			"obby::basic_document_info::user_subscribe:\n"
			"User is already subscribed"
		);
	}

	m_users.push_back(&user);
	m_signal_subscribe.emit(user);
}

template<typename Document, typename Selector>
void basic_document_info<Document, Selector>::user_unsubscribe(const user& user)
{
	if(!is_subscribed(user) )
	{
		throw std::logic_error(
			"obby::basic_document_info::user_unsubscribe:\n"
			"User is not subscribed"
		);
	}

	m_users.erase(
		std::remove(m_users.begin(), m_users.end(), &user),
		m_users.end()
	);

	m_signal_unsubscribe.emit(user);
}

template<typename Document, typename Selector>
void basic_document_info<Document, Selector>::
	document_rename(const std::string& title)
{
	m_title = title;
	m_signal_rename.emit(title);
}

template<typename Document, typename Selector>
void basic_document_info<Document, Selector>::assign_document()
{
	// TODO: Document template given to buffer that may be copied here.
	// TODO: Give only user table to document
	m_document.reset(new Document(get_buffer().get_document_template() ));
}

template<typename Document, typename Selector>
void basic_document_info<Document, Selector>::release_document()
{
	m_document.reset(NULL);
}

template<typename Document, typename Selector>
const typename basic_document_info<Document, Selector>::buffer_type&
basic_document_info<Document, Selector>::get_buffer() const
{
	return m_buffer;
}

template<typename Document, typename Selector>
typename basic_document_info<Document, Selector>::net_type&
basic_document_info<Document, Selector>::get_net6()
{
	return m_net;
}

template<typename Document, typename Selector>
const typename basic_document_info<Document, Selector>::net_type&
basic_document_info<Document, Selector>::get_net6() const
{
	return m_net;
}

// privileges_table
template<typename Document, typename Selector>
basic_document_info<Document, Selector>::privileges_table::
	privileges_table(privileges default_privileges):
	m_default_privs(default_privileges)
{
}

template<typename Document, typename Selector>
typename basic_document_info<Document, Selector>::privileges
basic_document_info<Document, Selector>::privileges_table::
	get_default_privileges() const
{
	return m_default_privs;
}

template<typename Document, typename Selector>
typename basic_document_info<Document, Selector>::privileges
basic_document_info<Document, Selector>::privileges_table::
	privileges_query(const user& user, privileges privs) const
{
	typename priv_map::const_iterator iter = m_privs.find(&user);
	if(iter == m_privs.end() ) return m_default_privs & privs;
	return iter->second & privs;
}

template<typename Document, typename Selector>
void basic_document_info<Document, Selector>::privileges_table::
	privileges_change(const user& user, privileges privs)
{
	m_privs[&user] = privs;
	m_signal_privileges_changed.emit(user, privs);
}

template<typename Document, typename Selector>
typename basic_document_info<Document, Selector>::privileges_table::
	signal_privileges_changed_type
basic_document_info<Document, Selector>::privileges_table::
	privileges_changed_event() const
{
	return m_signal_privileges_changed;
}

} // namespace obby

#endif // _OBBY_DOCUMENT_INFO_HPP_
