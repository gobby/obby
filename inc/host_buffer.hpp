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

class host_buffer : public local_buffer,
                    public server_buffer
{
public: 
	/** Returns a new host_buffer.
	 * @param port Port to listen to for incoming connections.
	 * @param username User name for the local user.
	 * @param red Red color component for the local user.
	 * @param green Green color component for the local user.
	 * @param blue Blue color component for the local user.
	 */
	host_buffer(unsigned int port, const std::string& username, int red,
	            int green, int blue);
	host_buffer(unsigned int port, const std::string& username, int red,
	            int green, int blue, const RSA::Key& public_key,
	            const RSA::Key& private_key);
	virtual ~host_buffer();

	/** Looks for a document with the given ID which belongs to the user
	 * with the given owner ID. Note that we do not take a real user object
	 * here because the ID is enough and one might not have a user object
	 * to the corresponding ID. So a time-consuming lookup is obsolete.
	 */
	host_document_info* find_document(unsigned int owner_id,
	                                  unsigned int id) const;

	/** Returns the local user.
	 */
	virtual user& get_self();

	/** Returns the local user.
	 */
	virtual const user& get_self() const;

	/** Sends a global message to all users.
	 */
	virtual void send_message(const std::string& message);
	
	/** Creates a new document with predefined content.
	 * signal_insert_document will be emitted and may be used to access
	 * the resulting obby::document. Additional signals will be emitted
	 * for the synced content.
	 */
	virtual void create_document(const std::string& title,
	                             const std::string& content);

protected:
	/** Private constructor used by derived classed. It does not create
	 * a net6::host object to allow derived classed to create derived
	 * classes from net6::host
	 */
	host_buffer();

        /** Adds a new document with the given title to the buffer.
	 */
	virtual document_info& add_document_info(const user* owner,
	                                         unsigned int id,
	                                         const std::string& title);

	user* m_self;
};

}

#endif // _OBBY_HOST_BUFFER_HPP_
