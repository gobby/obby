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

#ifndef _OBBY_SERVER_DOCUMENT_INFO_HPP_
#define _OBBY_SERVER_DOCUMENT_INFO_HPP_

#include <net6/server.hpp>
#include "serialise/object.hpp"
#include "serialise/attribute.hpp"
#include "no_operation.hpp"
#include "split_operation.hpp"
#include "insert_operation.hpp"
#include "delete_operation.hpp"
#include "record.hpp"
#include "jupiter_server.hpp"
#include "document_packet.hpp"
#include "document_info.hpp"

namespace obby
{

template<typename Document, typename Selector>
class basic_server_buffer;

/** Information about a document that is provided without being subscribed to
 * a document.
 */
template<typename Document, typename Selector>
class basic_server_document_info:
	virtual public basic_document_info<Document, Selector>
{
public:
	typedef basic_document_info<Document, Selector> base_type;
	typedef typename base_type::document_type document_type;

	typedef basic_server_buffer<Document, Selector> buffer_type;
	typedef typename buffer_type::net_type net_type;
	typedef jupiter_server<Document> jupiter_type;
	typedef typename jupiter_type::record_type record_type;

	basic_server_document_info(const buffer_type& buffer,
	                           net_type& net,
	                           const user* owner,
	                           unsigned int id,
	                           const std::string& title,
	                           const std::string& encoding,
	                           const std::string& content);

	/** Deserialises a document from a serialisation object.
	 */
	basic_server_document_info(const buffer_type& buffer,
	                           net_type& net,
	                           const serialise::object& obj);

	/** Inserts the given text at the given position into the document.
	 */
	virtual void insert(position pos, const std::string& text);

	/** Erases the given range from the document.
	 */
	virtual void erase(position pos, position len);

	/** Renames the given document.
	 */
	virtual void rename(const std::string& new_title);

	/** Subscribes the given user to this document.
	 */
	void subscribe_user(const user& user);

	/** Unsubscribes the given user from this document.
	 */
	void unsubscribe_user(const user& user);

	/** Called by the buffer if a network event occured that belongs to the
	 * document.
	 */
	virtual void on_net_packet(const document_packet& pack,
	                           const user& from);

	/** Called by the buffer when a user has joined.
	 */
	virtual void obby_user_join(const user& user);

	/** Called by the buffer when a user has left.
	 */
	virtual void obby_user_part(const user& user);

	/** @brief Called when the session has been closed.
	 */
	virtual void obby_session_close();

protected:
	/** Internal function that subscribes a user to this document.
	 */
	virtual void user_subscribe(const user& user);

	/** Internal function that unsubscribes a user from this document.
	 */
	virtual void user_unsubscribe(const user& user);

	/** Inserts text written by <em>author</em> into the document.
	 */
	void insert_impl(position pos,
	                 const std::string& text,
	                 const user* author);

	/** Erases text from the document. The operation is performed by
	 * <em>author</em>.
	 */
	void erase_impl(position pos,
	                position len,
	                const user* author);

	/** Renames the document. The operation is performed by 
	 * <em>from</em>.
	 */
	void rename_impl(const std::string& new_title,
	                 const user* from);

	/** Executes a network packet.
	 */
	bool execute_packet(const document_packet& pack,
	                    const user& from);

	/** Rename request.
	 */
	virtual void on_net_rename(const document_packet& pack,
	                           const user& from);
	
	/** Change in a document.
	 */
	virtual void on_net_record(const document_packet& pack,
	                           const user& from);

	/** Subscribe request.
	 */
	virtual void on_net_subscribe(const document_packet& pack,
	                              const user& from);

	/** Unsubscribe request.
	 */
	virtual void on_net_unsubscribe(const document_packet& pack,
	                                const obby::user& from);

	/** Callback from jupiter implementation with a record
	 * that may be sent to the given user.
	 */
	virtual void on_jupiter_record(const record_type& rec, const user& user,
	                               const obby::user* from);

	/** @brief Broadcasts a user subscription to the other users.
	 */
	void broadcast_subscription(const user& user);

	/** @brief Broadcasts user unsubscription to the other users.
	 */
	void broadcast_unsubscription(const user& user);

	/** @brief Implementation of the session close callback that does
	 * not call the base function.
	 */
	void session_close_impl();

	std::auto_ptr<jupiter_type> m_jupiter;

public:
	/** Returns the buffer to which this document_info belongs.
	 */
	const buffer_type& get_buffer() const;

protected:
	/** Returns the underlaying net6 object.
	 */
	net_type& get_net6();

	/** Returns the underlaying net6 object.
	 */
	const net_type& get_net6() const;
};

template<typename Document, typename Selector>
basic_server_document_info<Document, Selector>::
	basic_server_document_info(const buffer_type& buffer,
	                           net_type& net,
	                           const user* owner,
	                           unsigned int id,
	                           const std::string& title,
	                           const std::string& encoding,
	                           const std::string& content):
	base_type(buffer, net, owner, id, title, encoding)
{
	base_type::assign_document();
	base_type::m_document->insert(0, content, NULL);

	// Create jupiter server implementation
	m_jupiter.reset(new jupiter_type(
		*basic_document_info<Document, Selector>::m_document
	) );

	// Owner is subscribed implicitely
	if(owner != NULL)
		user_subscribe(*owner);

	// Connect to signals
	m_jupiter->record_event().connect(
		sigc::mem_fun(
			*this,
			&basic_server_document_info::on_jupiter_record
		)
	);
}

template<typename Document, typename Selector>
basic_server_document_info<Document, Selector>::
	basic_server_document_info(const buffer_type& buffer,
	                           net_type& net,
	                           const serialise::object& obj):
	base_type(buffer, net, obj)
{
	// TODO: Avoid code duplication somehow
	// Assign document content
	base_type::assign_document();

	// Deserialise document
	for(serialise::object::child_iterator child_it = obj.children_begin();
	    child_it != obj.children_end();
	    ++ child_it)
	{
		if(child_it->get_name() != "chunk")
			continue; // TODO: Throw unexpected child error

		const serialise::attribute& content_attr =
			child_it->get_required_attribute("content");
		const serialise::attribute& author_attr =
			child_it->get_required_attribute("author");

		base_type::m_document->append(
			content_attr.as<std::string>(),
			author_attr.as<const obby::user*>(
				buffer.get_user_table()
			)
		);
	}

	// Create jupiter server implementation
	m_jupiter.reset(new jupiter_type(
		*basic_document_info<Document, Selector>::m_document
	) );
	// Connect to signals
	m_jupiter->record_event().connect(
		sigc::mem_fun(
			*this,
			&basic_server_document_info::on_jupiter_record
		)
	);
}

template<typename Document, typename Selector>
void basic_server_document_info<Document, Selector>::
	insert(position pos,
	       const std::string& text)
{
	insert_impl(pos, text, NULL);
}

template<typename Document, typename Selector>
void basic_server_document_info<Document, Selector>::
	erase(position pos,
	      position len)
{
	erase_impl(pos, len, NULL);
}

template<typename Document, typename Selector>
void basic_server_document_info<Document, Selector>::
	rename(const std::string& new_title)
{
	rename_impl(new_title, NULL);
}

template<typename Document, typename Selector>
void basic_server_document_info<Document, Selector>::
	subscribe_user(const user& user)
{
	// Note that the host is able to subscribe the local user even if
	// no network connection is available. The host overloads this
	// function and basic_local_document_info::subscribe for that purpose
	if(base_type::m_net == NULL)
	{
		throw std::logic_error(
			"obby::basic_server_document_info::subscribe_user:\n"
			"Cannot subscribe user without having a network object "
		);
	}

	// TODO: Check userflags for connected?

	// Subscribe given user
	user_subscribe(user);

	// Synchronise initial document to user
	document_type& doc = *base_type::m_document;

	document_packet init_pack(*this, "sync_init");
	init_pack << doc.size();
	get_net6().send(init_pack, user.get_net6() );

	// Send content
	for(typename document_type::chunk_iterator iter = doc.chunk_begin();
	    iter != doc.chunk_end();
	    ++ iter)
	{
		// TODO: Do not send all at once.
		document_packet chunk_pack(*this, "sync_chunk");
		chunk_pack << iter.get_text() << iter.get_author();
		get_net6().send(chunk_pack, user.get_net6() );
	}

	// Broadcast subscription
	broadcast_subscription(user);
}

template<typename Document, typename Selector>
void basic_server_document_info<Document, Selector>::
	unsubscribe_user(const user& user)
{
	if(base_type::m_net == NULL)
	{
		throw std::logic_error(
			"obby::basic_server_document_info::unsubscribe_user:\n"
			"Cannot subscribe user without having a network object "
		);
	}

	// Unsubscribe user
	user_unsubscribe(user);
	// Broadcast unsubscription
	broadcast_unsubscription(user);
}

template<typename Document, typename Selector>
void basic_server_document_info<Document, Selector>::
	on_net_packet(const document_packet& pack,
	              const user& from)
{
	if(!execute_packet(pack, from) )
	{
		throw net6::bad_value(
			"Unexpected command: " + pack.get_command()
		);
	}
}

template<typename Document, typename Selector>
void basic_server_document_info<Document, Selector>::
	obby_user_join(const user& user)
{
	basic_document_info<Document, Selector>::obby_user_join(user);
}

template<typename Document, typename Selector>
void basic_server_document_info<Document, Selector>::
	obby_user_part(const user& user)
{
	basic_document_info<Document, Selector>::obby_user_part(user);
}

template<typename Document, typename Selector>
void basic_server_document_info<Document, Selector>::obby_session_close()
{
	session_close_impl();
	basic_document_info<Document, Selector>::session_close_impl();
}

template<typename Document, typename Selector>
void basic_server_document_info<Document, Selector>::
	user_subscribe(const user& user)
{
	// Call base function
	basic_document_info<Document, Selector>::user_subscribe(user);
	// Add client to jupiter
	m_jupiter->client_add(user);
}

template<typename Document, typename Selector>
void basic_server_document_info<Document, Selector>::
	user_unsubscribe(const user& user)
{
	// Remove client from jupiter
	m_jupiter->client_remove(user);
	// Call base function
	basic_document_info<Document, Selector>::user_unsubscribe(user);
}

template<typename Document, typename Selector>
void basic_server_document_info<Document, Selector>::
	insert_impl(position pos,
	            const std::string& text,
	            const user* author)
{
	if(m_jupiter.get() != NULL)
	{
		insert_operation<document_type> op(pos, text);
		m_jupiter->local_op(op, author);
	}
	else
	{
		base_type::m_document->insert(pos, text, author);
	}
}

template<typename Document, typename Selector>
void basic_server_document_info<Document, Selector>::
	erase_impl(position pos,
	           position len,
	           const user* author)
{
	if(m_jupiter.get() != NULL)
	{
		delete_operation<document_type> op(pos, len);
		m_jupiter->local_op(op, author);
	}
	else
	{
		base_type::m_document->erase(pos, len);
	}
}

template<typename Document, typename Selector>
void basic_server_document_info<Document, Selector>::
	rename_impl(const std::string& new_title,
	            const user* from)
{
	// Rename document
	basic_document_info<Document, Selector>::document_rename(new_title);

	if(base_type::m_net != NULL)
	{
		// Forward to clients
		document_packet pack(*this, "rename");
		pack << from << new_title;
		get_net6().send(pack);
	}
}

template<typename Document, typename Selector>
bool basic_server_document_info<Document, Selector>::
	execute_packet(const document_packet& pack,
	               const user& from)
{
	// TODO: std::map<>
	if(pack.get_command() == "rename")
		{ on_net_rename(pack, from); return true; }

	if(pack.get_command() == "record")
		{ on_net_record(pack, from); return true; }

	if(pack.get_command() == "subscribe")
		{ on_net_subscribe(pack, from); return true; }

	if(pack.get_command() == "unsubscribe")
		{ on_net_unsubscribe(pack, from); return true; }

	return false;
}

template<typename Document, typename Selector>
void basic_server_document_info<Document, Selector>::
	on_net_rename(const document_packet& pack,
	              const user& from)
{
	// TODO: Authentication

	const std::string& new_title =
		pack.get_param(0).net6::parameter::as<std::string>();

	rename_impl(new_title, &from);
}

template<typename Document, typename Selector>
void basic_server_document_info<Document, Selector>::
	on_net_record(const document_packet& pack,
	              const user& from)
{
	unsigned int index = 2;

	record_type rec(
		pack,
		index,
		base_type::m_buffer.get_user_table()
	);

	m_jupiter->remote_op(rec, &from);
}

template<typename Document, typename Selector>
void basic_server_document_info<Document, Selector>::
	on_net_subscribe(const document_packet& pack,
	                 const user& from)
{
	subscribe_user(from);
}

template<typename Document, typename Selector>
void basic_server_document_info<Document, Selector>::
	on_net_unsubscribe(const document_packet& pack,
	                   const user& from)
{
	unsubscribe_user(from);
}

template<typename Document, typename Selector>
void basic_server_document_info<Document, Selector>::
	on_jupiter_record(const record_type& rec,
	                  const user& user,
	                  const obby::user* from)
{
	document_packet pack(*this, "record");
	pack << from;
	rec.append_packet(pack);
	get_net6().send(pack, user.get_net6() );
}

template<typename Document, typename Selector>
void basic_server_document_info<Document, Selector>::
	broadcast_subscription(const user& user)
{
	document_packet pack(*this, "subscribe");
	pack << &user;
	get_net6().send(pack);
}

template<typename Document, typename Selector>
void basic_server_document_info<Document, Selector>::
	broadcast_unsubscription(const user& user)
{
	document_packet pack(*this, "unsubscribe");
	pack << &user;
	get_net6().send(pack);
}

template<typename Document, typename Selector>
void basic_server_document_info<Document, Selector>::session_close_impl()
{
	m_jupiter.reset(NULL);
}

template<typename Document, typename Selector>
const typename basic_server_document_info<Document, Selector>::buffer_type&
basic_server_document_info<Document, Selector>::get_buffer() const
{
	return dynamic_cast<const buffer_type&>(base_type::get_buffer() );
}

template<typename Document, typename Selector>
typename basic_server_document_info<Document, Selector>::net_type&
basic_server_document_info<Document, Selector>::get_net6()
{
	return dynamic_cast<net_type&>(base_type::get_net6() );
}

template<typename Document, typename Selector>
const typename basic_server_document_info<Document, Selector>::net_type&
basic_server_document_info<Document, Selector>::get_net6() const
{
	return dynamic_cast<const net_type&>(base_type::get_net6() );
}

} // namespace obby

#endif // _OBBY_SERVER_DOCUMENT_INFO_HPP_
