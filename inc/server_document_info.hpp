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

#ifndef _OBBY_SERVER_DOCUMENT_INFO_HPP_
#define _OBBY_SERVER_DOCUMENT_INFO_HPP_

#include <net6/server.hpp>
#include "serialise/object.hpp"
#include "serialise/attribute.hpp"
#include "insert_operation.hpp"
#include "delete_operation.hpp"
#include "record.hpp"
#include "jupiter_server.hpp"
#include "document_packet.hpp"
#include "document_info.hpp"

namespace obby
{

template<typename selector_type>
class basic_server_buffer;

/** Information about a document that is provided without being subscribed to
 * a document.
 */
template<typename selector_type>
class basic_server_document_info
 : virtual public basic_document_info<selector_type>
{
public:
	basic_server_document_info(
		const basic_server_buffer<selector_type>& buffer,
		net6::basic_server<selector_type>& net,
		const user* owner, unsigned int id,
		const std::string& title, const std::string& content
	);

	/** Deserialises a document from a serialisation object.
	 */
	basic_server_document_info(
		const basic_server_buffer<selector_type>& buffer,
		net6::basic_server<selector_type>& net,
		const serialise::object& obj
	);

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
	void erase_impl(position pos, position len, const user* author);

	/** Renames the document. The operation is performed by 
	 * <em>from</em>.
	 */
	void rename_impl(const std::string& new_title, const user* from);

	/** Executes a network packet.
	 */
	bool execute_packet(const document_packet& pack, const user& from);

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

	/** Callback from jupiter implementation with a record of a local
	 * operation that may be sent to the given user.
	 */
	virtual void on_jupiter_local(const record& rec, const user& user,
	                              const obby::user* from);

	/** Callback from jupiter implementation with a record of a remote
	 * operation (got by a user) that may be sent to others.
	 */
	virtual void on_jupiter_remote(const record& rec, const user& user,
	                               const obby::user* from);

	std::auto_ptr<jupiter_server> m_jupiter;

public:
	/** Returns the buffer to which this document_info belongs.
	 */
	const basic_server_buffer<selector_type>& get_buffer() const;

private:
	/** Returns the underlaying net6 object.
	 */
	net6::basic_server<selector_type>& get_net6();

	/** Returns the underlaying net6 object.
	 */
	const net6::basic_server<selector_type>& get_net6() const;
};

typedef basic_server_document_info<net6::selector> server_document_info;

template<typename selector_type>
basic_server_document_info<selector_type>::basic_server_document_info(
	const basic_server_buffer<selector_type>& buffer,
	net6::basic_server<selector_type>& net,
	const user* owner, unsigned int id,
	const std::string& title, const std::string& content
) : basic_document_info<selector_type>(buffer, net, owner, id, title)
{
	// Assign document content
	basic_document_info<selector_type>::assign_document();
	// Create initial content
	basic_document_info<selector_type>::
		m_document->insert(0, content, NULL);

	// Create jupiter server implementation
	m_jupiter.reset(new jupiter_server(
		*basic_document_info<selector_type>::m_document
	) );

	// Owner is subscribed implicitely
	if(owner != NULL)
		user_subscribe(*owner);

	// Connect to signals
	m_jupiter->local_event().connect(
		sigc::mem_fun(
			*this,
			&basic_server_document_info::on_jupiter_local
		)
	);

	m_jupiter->remote_event().connect(
		sigc::mem_fun(
			*this,
			&basic_server_document_info::on_jupiter_remote
		)
	);
}

template<typename selector_type>
basic_server_document_info<selector_type>::basic_server_document_info(
	const basic_server_buffer<selector_type>& buffer,
	net6::basic_server<selector_type>& net,
	const serialise::object& obj
):
	basic_document_info<selector_type>(buffer, net, obj)
{
	// TODO: Avoid code duplication somehow
	// Assign document content
	basic_document_info<selector_type>::assign_document();
	// Deserialise document
	basic_document_info<selector_type>::
		m_document->deserialise(obj, buffer.get_user_table() );

	// Create jupiter server implementation
	m_jupiter.reset(new jupiter_server(
		*basic_document_info<selector_type>::m_document
	) );
	// Connect to signals
	m_jupiter->local_event().connect(
		sigc::mem_fun(
			*this,
			&basic_server_document_info::on_jupiter_local
		)
	);

	m_jupiter->remote_event().connect(
		sigc::mem_fun(
			*this,
			&basic_server_document_info::on_jupiter_remote
		)
	);
}

template<typename selector_type>
void basic_server_document_info<selector_type>::
	insert(position pos, const std::string& text)
{
	insert_impl(pos, text, NULL);
}

template<typename selector_type>
void basic_server_document_info<selector_type>::
	erase(position pos, position len)
{
	erase_impl(pos, len, NULL);
}

template<typename selector_type>
void basic_server_document_info<selector_type>::
	rename(const std::string& new_title)
{
	rename_impl(new_title, NULL);
}

template<typename selector_type>
void basic_server_document_info<selector_type>::
	subscribe_user(const user& user)
{
	// Subscribe given user
	user_subscribe(user);

	const net6::user& user6 = user.get_net6();
	unsigned int line_count = basic_document_info<selector_type>::
		m_document->get_line_count();

	// Synchronise initial document to user
	document_packet init_pack(*this, "sync_init");
	init_pack << line_count;
	get_net6().send(init_pack, user.get_net6() );

	// Send content
	for(unsigned int i = 0; i < line_count; ++ i)
	{
		document_packet line_pack(*this, "sync_line");
		basic_document_info<selector_type>::
			m_document->get_line(i).append_packet(line_pack);
		get_net6().send(line_pack, user.get_net6() );
	}

	// Tell clients
	document_packet pack(*this, "subscribe");
	pack << &user;
	get_net6().send(pack);
}

template<typename selector_type>
void basic_server_document_info<selector_type>::
	unsubscribe_user(const user& user)
{
	// Unsubscribe user
	user_unsubscribe(user);

	// Tell clients
	document_packet pack(*this, "unsubscribe");
	pack << &user;
	get_net6().send(pack);
}

template<typename selector_type>
void basic_server_document_info<selector_type>::
	on_net_packet(const document_packet& pack, const user& from)
{
	if(!execute_packet(pack, from) )
	{
		throw net6::bad_value(
			"Unexpected command: " + pack.get_command()
		);
	}
}

template<typename selector_type>
void basic_server_document_info<selector_type>::obby_user_join(const user& user)
{
	basic_document_info<selector_type>::obby_user_join(user);
}

template<typename selector_type>
void basic_server_document_info<selector_type>::obby_user_part(const user& user)
{
	basic_document_info<selector_type>::obby_user_part(user);
}

template<typename selector_type>
void basic_server_document_info<selector_type>::
	user_subscribe(const user& user)
{
	// Call base function
	basic_document_info<selector_type>::user_subscribe(user);
	// Add client to jupiter
	m_jupiter->client_add(user);
}

template<typename selector_type>
void basic_server_document_info<selector_type>::
	user_unsubscribe(const user& user)
{
	// Remove client from jupiter
	m_jupiter->client_remove(user);
	// Call base function
	basic_document_info<selector_type>::user_unsubscribe(user);
}

template<typename selector_type>
void basic_server_document_info<selector_type>::
	insert_impl(position pos, const std::string& text, const user* author)
{
	insert_operation op(pos, text);
	m_jupiter->local_op(op, author);
}

template<typename selector_type>
void basic_server_document_info<selector_type>::
	erase_impl(position pos, position len, const user* author)
{
	delete_operation op(pos, len);
	m_jupiter->local_op(op, author);
}

template<typename selector_type>
void basic_server_document_info<selector_type>::
	rename_impl(const std::string& new_title, const user* from)
{
	// Rename document
	basic_document_info<selector_type>::document_rename(new_title);
	// Forward to clients
	document_packet pack(*this, "rename");
	pack << from << new_title;
	get_net6().send(pack);
}

template<typename selector_type>
bool basic_server_document_info<selector_type>::
	execute_packet(const document_packet& pack, const user& from)
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

template<typename selector_type>
void basic_server_document_info<selector_type>::
	on_net_rename(const document_packet& pack, const user& from)
{
	// TODO: Authentication

	const std::string& new_title =
		pack.get_param(0).net6::parameter::as<std::string>();

	rename_impl(new_title, &from);
}

template<typename selector_type>
void basic_server_document_info<selector_type>::
	on_net_record(const document_packet& pack, const user& from)
{
	unsigned int index = 2;

	record rec(
		pack,
		index,
		basic_document_info<selector_type>::m_buffer.get_user_table()
	);

	m_jupiter->remote_op(rec, &from);
}

template<typename selector_type>
void basic_server_document_info<selector_type>::
	on_net_subscribe(const document_packet& pack, const user& from)
{
	subscribe_user(from);
}

template<typename selector_type>
void basic_server_document_info<selector_type>::
	on_net_unsubscribe(const document_packet& pack, const user& from)
{
	unsubscribe_user(from);
}

template<typename selector_type>
void basic_server_document_info<selector_type>::
	on_jupiter_local(const record& rec,
	                 const user& user,
	                 const obby::user* from)
{
	document_packet pack(*this, "record");
	pack << from;
	rec.append_packet(pack);
	get_net6().send(pack, user.get_net6() );
}

template<typename selector_type>
void basic_server_document_info<selector_type>::
	on_jupiter_remote(const record& rec, const user& user, const obby::user* from)
{
	// TODO: Make only one signal in jupiter_server
	on_jupiter_local(rec, user, from);
}

template<typename selector_type>
const basic_server_buffer<selector_type>&
basic_server_document_info<selector_type>::get_buffer() const
{
	return dynamic_cast<const basic_server_buffer<selector_type>&>(
		basic_document_info<selector_type>::m_buffer
	);
}

template<typename selector_type>
net6::basic_server<selector_type>&
basic_server_document_info<selector_type>::get_net6()
{
	return dynamic_cast<net6::basic_server<selector_type>&>(
		basic_document_info<selector_type>::m_net
	);
}

template<typename selector_type>
const net6::basic_server<selector_type>&
basic_server_document_info<selector_type>::get_net6() const
{
	return dynamic_cast<const net6::basic_server<selector_type>&>(
		basic_document_info<selector_type>::m_net
	);
}

} // namespace obby

#endif // _OBBY_SERVER_DOCUMENT_INFO_HPP_

