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

#ifndef _OBBY_CLIENT_DOCUMENT_HPP_
#define _OBBY_CLIENT_DOCUMENT_HPP_

#include <net6/client.hpp>
#include "position.hpp"
#include "record.hpp"
#include "client_user_table.hpp"
#include "document.hpp"

namespace obby
{

class client_buffer;

/** Document used by client_buffer. Usually you do not have create or delete
 * document objects by youself, the equivalent buffers do this.
 */

class client_document : public document
{
public:
	/** Creates a new client_document with given ID. <em>client</em> is
	 * a net6::client object to synchronise changes to.
	 */
	client_document(unsigned int id, net6::client& client,
	                const client_buffer& buf);
	virtual ~client_document();

	/** Returns the buffer to which the document is assigned.
	 */
	const client_buffer& get_buffer() const;

	/** Inserts <em>text</em> at <em>pos</em>.
	 */
	virtual void insert(position pos, const std::string& text);

	/** Deletes the given area.
	 */
	virtual void erase(position from, position to);

	/** Called by the client_buffer if another user changed anything in this
	 * document.
	 */
	virtual void on_net_record(record& rec);

	/** Synchronization functions called while synchronizing documents.
	 */
	virtual void on_net_sync_init(const net6::packet& pack);
	virtual void on_net_sync_line(const net6::packet& pack);
	virtual void on_net_sync_final(const net6::packet& pack);

	/** Returns the amount of unsynced changes.
	 */
	unsigned int get_unsynced_changes_count() const;
protected:
	std::list<record*> m_unsynced;
	net6::client& m_client;
};

}

#endif // _OBBY_CLIENT_DOCUMENT_HPP_
