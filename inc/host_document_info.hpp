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

#ifndef _OBBY_HOST_DOCUMENT_INFO_HPP_
#define _OBBY_HOST_DOCUMENT_INFO_HPP_

#include <net6/host.hpp>
#include "local_document_info.hpp"
#include "server_document_info.hpp"

namespace obby
{

template<typename Document, typename Selector>
class basic_host_buffer;

/** Information about a document that is provided without being subscribed to
 * a document.
 */
template<typename Document, typename Selector>
class basic_host_document_info:
	virtual public basic_local_document_info<Document, Selector>,
	virtual public basic_server_document_info<Document, Selector>
{
public:
	typedef basic_document_info<Document, Selector> base_type;
	typedef basic_local_document_info<Document, Selector> base_local_type;
	typedef basic_server_document_info<Document, Selector> base_server_type;

	typedef typename base_server_type::document_type document_type;

	typedef basic_host_buffer<Document, Selector> buffer_type;
	typedef typename buffer_type::net_type net_type;

	typedef typename base_local_type::subscription_state subscription_state;

	basic_host_document_info(const buffer_type& buffer,
	                         net_type& net,
	                         const user* owner,
	                         unsigned int id,
	                         const std::string& title,
	                         const std::string& content);

	basic_host_document_info(const buffer_type& buffer,
	                         net_type& net,
	                         const serialise::object& obj);

	/** Inserts the given text at the given position into the document.
	 */
	virtual void insert(position pos, const std::string& text);

	/** Erases the given range from the document.
	 */
	virtual void erase(position pos, position len);

	/** Renames the current document.
	 */
	virtual void rename(const std::string& new_title);

	/** Sends a subscribe request for the local user. If the subscribe
	 * request succeeded, the subscribe_event will be emitted.
	 */
	virtual void subscribe();

	/** Unsubscribes the local user from this document. signal_unsubscribe
	 * will be emitted if the request has been accepted.
	 */
	virtual void unsubscribe();

	/** @brief Returns the state of the local user's subscription to
	 * this document.
	 */
	virtual subscription_state get_subscription_state() const;

protected:
	/** Internally subscribes a user to this document.
	 */
	virtual void user_subscribe(const user& user);

	/** Internally unsubscribes a user from this document.
	 */
	virtual void user_unsubscribe(const user& user);

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
basic_host_document_info<Document, Selector>::
	basic_host_document_info(const buffer_type& buffer,
	                         net_type& net,
	                         const user* owner,
	                         unsigned int id,
	                         const std::string& title,
	                         const std::string& content):
	base_type(buffer, net, owner, id, title),
	base_local_type(buffer, net, owner, id, title),
 	base_server_type(buffer, net, owner, id, title, content)
{
	// Server adds owner automagically to jupiter algo. If the local user
	// is the owner, it does not need to be added to jupiter.
	// TODO: Find a better solution here, maybe another server_document_info
	// ctor
	if(&buffer.get_self() == owner)
		base_server_type::m_jupiter->client_remove(*owner);
}

template<typename Document, typename Selector>
basic_host_document_info<Document, Selector>::
	basic_host_document_info(const buffer_type& buffer,
	                         net_type& net,
	                         const serialise::object& obj):
	base_type(buffer, net, obj),
	base_local_type(buffer, net, obj),
	base_server_type(buffer, net, obj)
{
}

template<typename Document, typename Selector>
void basic_host_document_info<Document, Selector>::
	insert(position pos,
	       const std::string& text)
{
	const user* self = &get_buffer().get_self();

	base_server_type::insert_impl(pos, text, self);
}

template<typename Document, typename Selector>
void basic_host_document_info<Document, Selector>::
	erase(position pos,
	      position len)
{
	base_server_type::erase_impl(pos, len, &get_buffer().get_self() );
}

template<typename Document, typename Selector>
void basic_host_document_info<Document, Selector>::
	rename(const std::string& new_title)
{
	base_server_type::rename_impl(new_title, &get_buffer().get_self() );
}

template<typename Document, typename Selector>
void basic_host_document_info<Document, Selector>::subscribe()
{
	// TODO: Call a base-class method that does not try to sync the document
	// contents to the user, it is not senseful in our case.
	base_server_type::subscribe_user(get_buffer().get_self() );
}

template<typename Document, typename Selector>
void basic_host_document_info<Document, Selector>::unsubscribe()
{
	// TODO: Same here
	base_server_type::unsubscribe_user(get_buffer().get_self() );
}

template<typename Document, typename Selector>
typename basic_host_document_info<Document, Selector>::subscription_state
basic_host_document_info<Document, Selector>::get_subscription_state() const
{
	// SUBSCRIBING and UNSUBSCRIBING do not exist in our case
	if(base_local_type::is_subscribed() )
		return base_local_type::SUBSCRIBED;
	else
		return base_local_type::UNSUBSCRIBED;
}

template<typename Document, typename Selector>
void basic_host_document_info<Document, Selector>::
	user_subscribe(const user& user)
{
	// TODO: Clean this up by a function like add_client_to_jupiter /
	// remove_client_from_jupiter that the host may overload to prevent
	// from adding the local client to jupiter

	// Do not call server function because it will add the client to
	// jupiter in any case.
	base_type::user_subscribe(user);

	// Add client to jupiter if it is not the local client
	if(&user != &get_buffer().get_self() )
		base_server_type::m_jupiter->client_add(user);
}

template<typename Document, typename Selector>
void basic_host_document_info<Document, Selector>::
	user_unsubscribe(const user& user)
{
	// Remove client from jupiter if is is not the local client
	if(&user != &get_buffer().get_self() )
		base_server_type::m_jupiter->client_remove(user);

	// Call base function
	base_type::user_unsubscribe(user);
}

template<typename Document, typename Selector>
const typename basic_host_document_info<Document, Selector>::buffer_type&
basic_host_document_info<Document, Selector>::get_buffer() const
{
	return dynamic_cast<const buffer_type&>(base_server_type::get_buffer());
}

template<typename Document, typename Selector>
typename basic_host_document_info<Document, Selector>::net_type&
basic_host_document_info<Document, Selector>::get_net6()
{
	return dynamic_cast<net_type&>(base_server_type::get_net6());
}

template<typename Document, typename Selector>
const typename basic_host_document_info<Document, Selector>::net_type&
basic_host_document_info<Document, Selector>::get_net6() const
{
	return dynamic_cast<const net_type&>(base_server_type::get_net6());
}

} // namespace obby

#endif // _OBBY_HOST_DOCUMENT_INFO_HPP_
