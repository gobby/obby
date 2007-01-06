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

#ifndef _OBBY_HOST_DOCUMENT_INFO_HPP_
#define _OBBY_HOST_DOCUMENT_INFO_HPP_

#include <net6/host.hpp>
#include "local_document_info.hpp"
#include "server_document_info.hpp"

namespace obby
{

template<typename selector_type>
class basic_host_buffer;

/** Information about a document that is provided without being subscribed to
 * a document.
 */
template<typename selector_type>
class basic_host_document_info :
	virtual public basic_local_document_info<selector_type>,
	virtual public basic_server_document_info<selector_type>
{
public:
	basic_host_document_info(
		const basic_host_buffer<selector_type>& buffer,
		const net6::basic_host<selector_type>& net,
		const user* owner, unsigned int id,
		const std::string& title
	);

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

private:
	/** Returns the buffer to which this document_info belongs.
	 */
	const basic_host_buffer<selector_type>& get_buffer() const;

	/** Returns the underlaying net6 object through which requests are
	 * transmitted.
	 */
	const net6::basic_host<selector_type>& get_net6() const;
};

typedef basic_host_document_info<net6::selector> host_document_info;

template<typename selector_type>
basic_host_document_info<selector_type>::basic_host_document_info(
	const basic_host_buffer<selector_type>& buffer,
	const net6::basic_host<selector_type>& net,
	const user* owner, unsigned int id,
	const std::string& title
) : basic_document_info<selector_type>(buffer, net, owner, id, title),
    basic_local_document_info<selector_type>(buffer, net, owner, id, title),
    basic_server_document_info<selector_type>(buffer, net, owner, id, title)
{
}

template<typename selector_type>
void basic_host_document_info<selector_type>::
	insert(position pos, const std::string& text)
{
	basic_server_document_info<selector_type>::
		insert_impl(pos, text, &get_buffer().get_self() );
}

template<typename selector_type>
void basic_host_document_info<selector_type>::
	erase(position pos, position len)
{
	basic_server_document_info<selector_type>::
		erase_impl(pos, len, &get_buffer().get_self() );
}

template<typename selector_type>
void basic_host_document_info<selector_type>::
	rename(const std::string& new_title)
{
	basic_server_document_info<selector_type>::
		rename_impl(new_title, &get_buffer().get_self() );
}

template<typename selector_type>
void basic_host_document_info<selector_type>::
	subscribe()
{
	// TODO: Call a base-class method that does not try to sync the document
	// contents to the user, it is not senseful in our case.
	basic_document_info<selector_type>::
		subscribe_user(get_buffer().get_self() );
}

template<typename selector_type>
void basic_host_document_info<selector_type>::
	unsubscribe()
{
	basic_document_info<selector_type>::
		unsubscribe_user(get_buffer().get_self() );
}

template<typename selector_type>
const basic_host_buffer<selector_type>&
basic_host_document_info<selector_type>::get_buffer() const
{
	return dynamic_cast<const basic_host_buffer<selector_type>&>(
		basic_document_info<selector_type>::m_buffer
	);
}

template<typename selector_type>
const net6::basic_host<selector_type>&
basic_host_document_info<selector_type>::get_net6() const
{
	return dynamic_cast<const net6::basic_host<selector_type>&>(
		basic_document_info<selector_type>::m_net
	);
}

} // namespace obby

#endif // _OBBY_HOST_DOCUMENT_INFO_HPP_
