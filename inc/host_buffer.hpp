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

#ifndef _OBBY_HOST_BUFFER_HPP_
#define _OBBY_HOST_BUFFER_HPP_

#include <sigc++/signal.h>
#include <net6/host.hpp>
#include "host_document_info.hpp"
#include "server_buffer.hpp"
#include "local_buffer.hpp"

namespace obby
{

template<typename selector_type>
class basic_host_buffer : virtual public basic_local_buffer<selector_type>,
                          virtual public basic_server_buffer<selector_type>
{
public: 
	/** Creates a new host_buffer.
	 * @param port Port to listen to for incoming connections.
	 * @param username User name for the local user.
	 * @param red Red color component for the local user.
	 * @param green Green color component for the local user.
	 * @param blue Blue color component for the local user.
	 */
	basic_host_buffer(unsigned int port, const std::string& username,
	                  int red, int green, int blue);

	/** Creates a new host_buffer.
	 * @param port Port to listen to for incoming connections.
	 * @param username User name for the local user.
	 * @param red Red color component for the local user.
	 * @param green Green color component for the local user.
	 * @param blue Blue color component for the local user.
	 * @param public_key Public RSA key of the server's key pair.
	 * @param private_key Corresponding private key. Must match
	 * the public one.
	 */
	basic_host_buffer(unsigned int port, const std::string& username,
			  int red, int green, int blue,
	                  const RSA::Key& public_key,
	                  const RSA::Key& private_key);

	/** Looks for a document with the given ID which belongs to the user
	 * with the given owner ID. Note that we do not take a real user object
	 * here because the ID is enough and one might not have a user object
	 * to the corresponding ID. So a time-consuming lookup is obsolete.
	 */
	host_document_info* document_find(unsigned int owner_id,
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
	                             const std::string& content,
	                             bool open_as_edited = false);

	/** Sets a new colour for the local user.
	 */
	virtual void set_colour(int red, int green, int blue);
	
protected:
	/** Private constructor used by derived classed. It does not create
	 * a net6::basic_host object to allow derived classed to create
	 * derivates from net6::basic_host
	 */
	basic_host_buffer();
	basic_host_buffer(const RSA::Key& public_key,
	                  const RSA::Key& private_key);

	/** Creates a new document info object according to the type of buffer.
	 */
	virtual document_info* new_document_info(const user* owner,
	                                         unsigned int id,
	                                         const std::string& title);

	user* m_self;
private:
	/** Private function which executes the constructor code, called by
	 * both public constructors.
	 */
	void init_impl(unsigned int port, const std::string& username,
	               int red, int green, int blue);

	/** This function provides access to the underlaying net6::basic_host
	 * object.
	 */
	net6::basic_host<selector_type>& net6_host();

	/** This function provides access to the underlaying net6::basic_host
	 * object.
	 */
	const net6::basic_host<selector_type>& net6_host() const;
};

typedef basic_host_buffer<net6::selector> host_buffer;

template<typename selector_type>
basic_host_buffer<selector_type>::basic_host_buffer()
 : basic_buffer<selector_type>(),
   basic_local_buffer<selector_type>(),
   basic_server_buffer<selector_type>(),
   m_self(NULL)
{
}

template<typename selector_type>
basic_host_buffer<selector_type>::
	basic_host_buffer(const RSA::Key& public_key,
	                  const RSA::Key& private_key)
 : basic_buffer<selector_type>(),
   basic_local_buffer<selector_type>(),
   basic_server_buffer<selector_type>(public_key, private_key),
   m_self(NULL)
{
}

template<typename selector_type>
basic_host_buffer<selector_type>::
	basic_host_buffer(unsigned int port, const std::string& username,
	                  int red, int green, int blue)
 : basic_buffer<selector_type>(),
   basic_local_buffer<selector_type>(),
   basic_server_buffer<selector_type>(),
   m_self(NULL)
{
	init_impl(port, username, red, green, blue);
}

template<typename selector_type>
basic_host_buffer<selector_type>::
	basic_host_buffer(unsigned int port, const std::string& username,
	                  int red, int green, int blue,
	                  const RSA::Key& public_key,
	                  const RSA::Key& private_key)
 : basic_buffer<selector_type>(),
   basic_local_buffer<selector_type>(),
   basic_server_buffer<selector_type>(public_key, private_key),
   m_self(NULL)
{
	init_impl(port, username, red, green, blue);
}

template<typename selector_type>
void basic_host_buffer<selector_type>::
	init_impl(unsigned int port, const std::string& username,
	          int red, int green, int blue)
{
	// Create host object
	basic_buffer<selector_type>::m_net =
		new net6::host(port, username, false);

	// Create local user
	m_self = basic_buffer<selector_type>::m_user_table.add_user(
		net6_host().get_self(), red, green, blue
	);

	// Register signal handlers
	basic_server_buffer<selector_type>::register_signal_handlers();
}

template<typename selector_type>
host_document_info* basic_host_buffer<selector_type>::
	document_find(unsigned int owner_id, unsigned int id) const
{
	return dynamic_cast<host_document_info*>(
		basic_buffer<selector_type>::document_find(owner_id, id)
	);
}

template<typename selector_type>
const user& basic_host_buffer<selector_type>::get_self() const
{
	if(m_self == NULL)
		throw std::logic_error("obby::host_buffer::get_self");

	return *m_self;
}

template<typename selector_type>
void basic_host_buffer<selector_type>::send_message(const std::string& message)
{
	if(m_self == NULL)
		throw std::logic_error("obby::host_buffer::send_message");

	// Send message from local user instead of server
	// TODO: Move signal emission to send_message_impl
	basic_buffer<selector_type>::m_signal_message.emit(*m_self, message);
	basic_server_buffer<selector_type>::send_message_impl(message, m_self);
}

template<typename selector_type>
void basic_host_buffer<selector_type>::
	document_create(const std::string& title, const std::string& content,
	                bool open_as_edited)
{
	if(m_self == NULL)
		throw std::logic_error("obby::host_buffer::document_create");

	unsigned int id = ++ basic_buffer<selector_type>::m_doc_counter;

	// Create document with local user as owner instead of NULL indicating
	// that it is the server's docuent.
	basic_server_buffer<selector_type>::document_create_impl(
		title, content, m_self, id, open_as_edited
	);
}

template<typename selector_type>
void basic_host_buffer<selector_type>::set_colour(int red, int green, int blue)
{
	if(m_self == NULL)
		throw std::logic_error("obby::host_buffer::set_colour");

	// Check for colour conflicts
	// TODO: user_colour_impl should check this
	if(!basic_buffer<selector_type>::check_colour(red, green, blue, m_self))
	{
		basic_local_buffer<selector_type>::
			m_signal_user_colour_failed.emit();
	}
	else
	{
		// TODO: user_colour_impl should take const user&, user_table
		// performs the necessary operation
		basic_server_buffer<selector_type>::user_colour_impl(
			const_cast<user&>(*m_self), red, green, blue
		);
	}
}

template<typename selector_type>
document_info* basic_host_buffer<selector_type>::
	new_document_info(const user* owner, unsigned int id,
	                  const std::string& title)
{
	// Create host_document_info, according to host_buffer
	return new host_document_info(*this, net6_host(), owner, id, title);
}

template<typename selector_type>
net6::basic_host<selector_type>& basic_host_buffer<selector_type>::
	net6_host()
{
	return dynamic_cast<net6::basic_host<selector_type>&>(
		*basic_buffer<selector_type>::m_net
	);
}

template<typename selector_type>
const net6::basic_host<selector_type>& basic_host_buffer<selector_type>::
	net6_host() const
{
	return dynamic_cast<const net6::basic_host<selector_type>&>(
		*basic_buffer<selector_type>::m_net
	);
}

} // namespace obby

#endif // _OBBY_HOST_BUFFER_HPP_
