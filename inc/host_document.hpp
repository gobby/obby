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

#ifndef _OBBY_HOST_DOCUMENT_HPP_
#define _OBBY_HOST_DOCUMENT_HPP_

#include <sigc++/signal.h>
#include <net6/host.hpp>
#include "host_user_table.hpp"
#include "server_document.hpp"

namespace obby
{

/** host_document used by host_buffer. Usually you do not have to create or
 * delete documents yourself, the buffers manage them.
 */
	
class host_document : public server_document
{
public: 
	/** Creates a new host_document.
	 * @param id Unique ID for this document.
	 * @param host net6::host object to synchronise data to.
	 */
	host_document(unsigned int id, net6::host& host,
	              const host_user_table& usertable);
	virtual ~host_document();

protected:
};

}

#endif // _OBBY_HOST_DOCUMENT_HPP_
