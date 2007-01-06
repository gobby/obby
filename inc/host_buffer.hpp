/* libobby - Network text editing library
 * Copyright (C) 2005 0x539 dev group
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this program; if not, write to the Free
 * Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#ifndef _OBBY_HOST_BUFFER_HPP_
#define _OBBY_HOST_BUFFER_HPP_

#include <sigc++/signal.h>
#include <net6/host.hpp>
#include "server_buffer.hpp"

namespace obby
{

class host_buffer : public server_buffer
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
	virtual ~host_buffer();

	/** Returns the local user.
	 */
	user& get_self();

	/** Returns the local user.
	 */
	const user& get_self() const;

	/** Adds a new document with the given ID to the buffer. The internal
	 * ID counter is set to the new given document ID.
	 */
	virtual document& add_document(unsigned int id);
protected:
	/** Private constructor used by derived classed. It does not create
	 * a net6::host object to allow derived classed to create derived
	 * classes from net6::host
	 */
	host_buffer();
	
	user* m_self;
};

}

#endif // _OBBY_HOST_BUFFER_HPP_
