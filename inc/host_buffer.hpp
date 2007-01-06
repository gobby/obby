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

#ifndef _OBBY_HOST_BUFFER_HPP_
#define _OBBY_HOST_BUFFER_HPP_

#include <sigc++/signal.h>
#include <net6/host.hpp>
#include "host_document_info.hpp"
#include "server_buffer.hpp"
#include "local_buffer.hpp"

namespace obby
{

template<typename Document, typename Selector>
class basic_host_buffer:
	virtual public basic_local_buffer<Document, Selector>,
	virtual public basic_server_buffer<Document, Selector>
{
public:
	// Document info
	typedef typename basic_server_buffer<Document, Selector>::
		base_document_info_type base_document_info_type;

	typedef basic_host_document_info<Document, Selector>
		document_info_type;

	// Network
	typedef typename basic_server_buffer<Document, Selector>::
		base_net_type base_net_type;

	typedef net6::basic_host<Selector> net_type;

	// TODO: Move this parameters into the open() call. The call would have
	// to lose its virtualness, but should be better nevertheless.

	/** Creates a new host_buffer.
	 * @param username User name for the local user.
	 * @param colour Local user colour.
	 */
	basic_host_buffer(const std::string& username,
	                  const colour& colour);

	/** Creates a new host_buffer.
	 * @param username User name for the local user.
	 * @param colour Local user colour.
	 * @param public_key Public RSA key of the server's key pair.
	 * @param private_key Corresponding private key. Must match
	 * the public one.
	 */
	basic_host_buffer(const std::string& username,
	                  const colour& colour,
	                  const RSA::Key& public_key,
	                  const RSA::Key& private_key);

	/** Opens the server on the given port.
	 */
	virtual void open(unsigned int port);

	/** Opens the server on the given port and resumes the obby session
	 * stored in the given file.
	 */
	virtual void open(const std::string& session, unsigned int port);

	/** Closes the session.
	 */
	virtual void close();

	/** Looks for a document with the given ID which belongs to the user
	 * with the given owner ID. Note that we do not take a real user object
	 * here because the ID is enough and one might not have a user object
	 * to the corresponding ID. So a time-consuming lookup is obsolete.
	 */
	document_info_type* document_find(unsigned int owner_id,
	                                  unsigned int id) const;

	/** Returns the local user.
	 */
	virtual const user& get_self() const;

	/** Sends a global message to all users.
	 */
	virtual void send_message(const std::string& message);
	
	/** Creates a new document with predefined content.
	 * signal_insert_document will be emitted and may be used to access
	 * the resulting obby::document.
	 */
	virtual void document_create(const std::string& title,
	                             const std::string& content);

	/** Sets a new colour for the local user.
	 */
	virtual void set_colour(const colour& colour);
	
protected:
	/** Creates a new document info object according to the type of buffer.
	 */
	virtual base_document_info_type*
	new_document_info(const user* owner,
	                  unsigned int id,
	                  const std::string& title,
	                  const std::string& content);

	/** Creates a new document info deserialised from a serialisation
	 * object according to the type of buffer.
	 */
	virtual base_document_info_type*
	new_document_info(const serialise::object& obj);

	/** Creates the underlaying net6 network object corresponding to the
	 * buffer's type.
	 */
	virtual base_net_type* new_net();

	std::string m_username;
	colour m_colour;

	const user* m_self;
private:
	/** This function provides access to the underlaying net6::basic_host
	 * object.
	 */
	net_type& net6_host();

	/** This function provides access to the underlaying net6::basic_host
	 * object.
	 */
	const net_type& net6_host() const;
};

typedef basic_host_buffer<obby::document, net6::selector> host_buffer;

template<typename Document, typename Selector>
basic_host_buffer<Document, Selector>::
	basic_host_buffer(const std::string& username,
	                  const colour& colour):
	basic_buffer<Document, Selector>(),
	basic_local_buffer<Document, Selector>(),
	basic_server_buffer<Document, Selector>(),
	m_username(username), m_colour(colour), m_self(NULL)
{
}

template<typename Document, typename Selector>
basic_host_buffer<Document, Selector>::
	basic_host_buffer(const std::string& username,
	                  const colour& colour,
	                  const RSA::Key& public_key,
	                  const RSA::Key& private_key):
	basic_buffer<Document, Selector>(),
	basic_local_buffer<Document, Selector>(),
	basic_server_buffer<Document, Selector>(public_key, private_key),
	m_username(username), m_colour(colour), m_self(NULL)
{
}

template<typename Document, typename Selector>
void basic_host_buffer<Document, Selector>::open(unsigned int port)
{
	basic_server_buffer<Document, Selector>::open(port);

	// Create local user
	m_self = basic_buffer<Document, Selector>::m_user_table.add_user(
		basic_buffer<Document, Selector>::m_user_table.find_free_id(),
		net6_host().get_self(), m_colour
	);

	basic_buffer<Document, Selector>::m_signal_user_join.emit(*m_self);
}

template<typename Document, typename Selector>
void basic_host_buffer<Document, Selector>::open(const std::string& content,
                                                 unsigned int port)
{
	basic_server_buffer<Document, Selector>::open(content, port);

	// Create local user
	m_self = basic_buffer<Document, Selector>::m_user_table.add_user(
		basic_buffer<Document, Selector>::m_user_table.find_free_id(),
		net6_host().get_self(), m_colour
	);

	basic_buffer<Document, Selector>::m_signal_user_join.emit(*m_self);
}

template<typename Document, typename Selector>
void basic_host_buffer<Document, Selector>::close()
{
	basic_server_buffer<Document, Selector>::close();
	m_self = NULL;
}

template<typename Document, typename Selector>
typename basic_host_buffer<Document, Selector>::document_info_type*
basic_host_buffer<Document, Selector>::
	document_find(unsigned int owner_id,
	              unsigned int id) const
{
	return dynamic_cast<document_info_type*>(
		basic_server_buffer<Document, Selector>::document_find(
			owner_id,
			id
		)
	);
}

template<typename Document, typename Selector>
const user& basic_host_buffer<Document, Selector>::get_self() const
{
	if(m_self == NULL)
	{
		throw std::logic_error(
			"obby::host_buffer::get_self:\n"
			"Server has not been opened"
		);
	}

	return *m_self;
}

template<typename Document, typename Selector>
void basic_host_buffer<Document, Selector>::
	send_message(const std::string& message)
{
	if(m_self == NULL)
	{
		throw std::logic_error(
			"obby::host_buffer::send_message:\n"
			"Server has net been opened"
		);
	}

	// Send message from local user instead of server
	basic_server_buffer<Document, Selector>::send_message_impl(
		message,
		m_self
	);
}

template<typename Document, typename Selector>
void basic_host_buffer<Document, Selector>::
	document_create(const std::string& title,
	                const std::string& content)
{
	if(m_self == NULL)
	{
		throw std::logic_error(
			"obby::host_buffer::document_create:\n"
			"Server has not been opened"
		);
	}

	unsigned int id = ++ basic_buffer<Document, Selector>::m_doc_counter;

	// Create document with local user as owner instead of NULL indicating
	// that it is the server's document.
	basic_server_buffer<Document, Selector>::document_create_impl(
		m_self, id, title, content
	);
}

template<typename Document, typename Selector>
void basic_host_buffer<Document, Selector>::set_colour(const colour& colour)
{
	if(m_self == NULL)
	{
		throw std::logic_error(
			"obby::host_buffer::set_colour:\n"
			"Server has not been opened"
		);
	}

	// Check for colour conflicts
	// TODO: user_colour_impl should check this
	if(!basic_buffer<Document, Selector>::check_colour(colour, m_self))
	{
		basic_local_buffer<Document, Selector>::
			m_signal_user_colour_failed.emit();
	}
	else
	{
		basic_server_buffer<Document, Selector>::user_colour_impl(
			*m_self,
			colour
		);
	}
}

template<typename Document, typename Selector>
typename basic_host_buffer<Document, Selector>::base_document_info_type*
basic_host_buffer<Document, Selector>::
	new_document_info(const user* owner,
	                  unsigned int id,
	                  const std::string& title,
	                  const std::string& content)
{
	// Create host_document_info, according to host_buffer
	return new document_info_type(
		*this,
		net6_host(),
		owner,
		id,
		title,
		content
	);
}

template<typename Document, typename Selector>
typename basic_host_buffer<Document, Selector>::base_document_info_type*
basic_host_buffer<Document, Selector>::
	new_document_info(const serialise::object& obj)
{
	// Create host_document_info, according to host_buffer
	return new document_info_type(*this, net6_host(), obj);
}

template<typename Document, typename Selector>
typename basic_host_buffer<Document, Selector>::base_net_type*
basic_host_buffer<Document, Selector>::new_net()
{
	return new net_type(m_username, false);
}

template<typename Document, typename Selector>
typename basic_host_buffer<Document, Selector>::net_type&
basic_host_buffer<Document, Selector>::net6_host()
{
	return dynamic_cast<net_type&>(
		*basic_server_buffer<Document, Selector>::m_net
	);
}

template<typename Document, typename Selector>
const typename basic_host_buffer<Document, Selector>::net_type&
basic_host_buffer<Document, Selector>::net6_host() const
{
	return dynamic_cast<const net_type&>(
		*basic_server_buffer<Document, Selector>::m_net
	);
}

} // namespace obby

#endif // _OBBY_HOST_BUFFER_HPP_
