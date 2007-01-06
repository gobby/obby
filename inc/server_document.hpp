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

#ifndef _OBBY_SERVER_DOCUMENT_HPP_
#define _OBBY_SERVER_DOCUMENT_HPP_

#include <sigc++/signal.h>
#include <net6/server.hpp>
#include "insert_record.hpp"
#include "delete_record.hpp"
#include "server_document.hpp"
#include "buffer.hpp"

namespace obby
{

/** Document used by server_buffer. Usually you do not create or delete
 * documents yourself, the buffers manage them.
 */

class server_document : public document
{
public: 
	/** Creates a new server document.
	 * @param id Unique ID for this document.
	 * @param server net6::server object to synchronize data to.
	 */
	server_document(unsigned int id, net6::server& server);
	virtual ~server_document();

	/** Inserts <em>text</em> at <em>pos</em> and synchronizes it with
	 * the clients.
	 */
	virtual void insert(position pos, const std::string& text);

	/** Removes text from the given area and synchronizes it with the
	 * clients.
	 */
	virtual void erase(position from, position end);

	/** Call this function if a record from a client has arrived.
	 */
	virtual void on_net_record(record& rec);

protected:
	net6::server& m_server;
};

}

#endif // _OBBY_SERVER_DOCUMENT_HPP_
