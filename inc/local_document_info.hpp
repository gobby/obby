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

#ifndef _OBBY_LOCAL_DOCUMENT_INFO_HPP_
#define _OBBY_LOCAL_DOCUMENT_INFO_HPP_

#include <net6/local.hpp>
#include "serialise/object.hpp"
#include "document_info.hpp"

namespace obby
{

template<typename selector_type>
class basic_local_buffer;

/** Information about a document that is provided without being subscribed to
 * a document.
 */

template<typename selector_type>
class basic_local_document_info
 : virtual public basic_document_info<obby::document, selector_type>
{
public:
	basic_local_document_info(
		const basic_local_buffer<selector_type>& buffer,
		net6::basic_local<selector_type>& net,
		const user* owner, unsigned int id,
		const std::string& title
	);

	basic_local_document_info(
		const basic_local_buffer<selector_type>& buffer,
		net6::basic_local<selector_type>& net,
		const serialise::object& obj
	);

	/** Sends a subscribe request for the local user. If the subscribe
	 * request succeeded, the subscribe_event will be emitted.
	 */
	virtual void subscribe() = 0;

	/** Unsubscribes the local user from this document. signal_unsubscribe
	 * will be emitted if the request has been accepted.
	 */
	virtual void unsubscribe() = 0;

	/** Returns whether the local user is subscribed to this document.
	 */
	virtual bool is_subscribed() const;

	/** Returns whether the given user <em>user</em> is subscribed to
	 * this document.
	 */
	virtual bool is_subscribed(const user& user) const;

public:
	/** Returns the buffer this document belongs to.
	 */
	const basic_local_buffer<selector_type>& get_buffer() const;

private:
	/** Returns the underlaying net6 object.
	 */
	net6::basic_local<selector_type>& get_net6();

	/** Returns the underlaying net6 object.
	 */
	const net6::basic_local<selector_type>& get_net6() const;
};

typedef basic_local_document_info<net6::selector> local_document_info;

template<typename selector_type>
basic_local_document_info<selector_type>::basic_local_document_info(
	const basic_local_buffer<selector_type>& buffer,
	net6::basic_local<selector_type>& net,
	const user* owner, unsigned int id,
	const std::string& title
):
	basic_document_info<obby::document, selector_type>(buffer, net, owner, id, title)
{
}

template<typename selector_type>
basic_local_document_info<selector_type>::basic_local_document_info(
	const basic_local_buffer<selector_type>& buffer,
	net6::basic_local<selector_type>& net,
	const serialise::object& obj
):
	basic_document_info<obby::document, selector_type>(buffer, net, obj)
{
}

template<typename selector_type>
bool basic_local_document_info<selector_type>::is_subscribed() const
{
	return is_subscribed(get_buffer().get_self() );
}

template<typename selector_type>
bool basic_local_document_info<selector_type>::
	is_subscribed(const user& user) const
{
	return basic_document_info<obby::document, selector_type>::is_subscribed(user);
}

template<typename selector_type>
const basic_local_buffer<selector_type>&
basic_local_document_info<selector_type>::get_buffer() const
{
	return dynamic_cast<const basic_local_buffer<selector_type>&>(
		basic_document_info<obby::document, selector_type>::m_buffer
	);
}

template<typename selector_type>
net6::basic_local<selector_type>&
basic_local_document_info<selector_type>::get_net6()
{
	return dynamic_cast<net6::basic_local<selector_type>&>(
		basic_document_info<obby::document, selector_type>::m_net
	);
}

template<typename selector_type>
const net6::basic_local<selector_type>&
basic_local_document_info<selector_type>::get_net6() const
{
	return dynamic_cast<const net6::basic_local<selector_type>&>(
		basic_document_info<obby::document, selector_type>::m_net
	);
}

} // namespace obby

#endif // _OBBY_DOCUMENT_INFO_HPP_
