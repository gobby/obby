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

#include "local_document.hpp"
#include <net6/client.hpp>

namespace obby
{

class client_document_info;

template<typename selector_type>
class basic_client_buffer;

/** Document used by client_buffer. Usually you do not have create or delete
 * document objects by youself, the equivalent buffers do this.
 */

class client_document : public local_document
{
public:
	/** Creates a new client_document with given ID. <em>client</em> is
	 * a net6::client object to synchronise changes to.
	 */
	client_document(const client_document_info& info, net6::client& client);
	virtual ~client_document();

	/** Returns the document info for this document.
	 */
	const client_document_info& get_info() const;

	/** Returns the buffer to which the document is assigned.
	 */
	const basic_client_buffer<net6::selector>& get_buffer() const;

	/** Inserts <em>text</em> at <em>pos</em>.
	 */
	virtual void insert(position pos, const std::string& text);

	/** Deletes the given area.
	 */
	virtual void erase(position from, position to);

	/** Returns the amount of unsynced changes.
	 */
	std::list<record*>::size_type unsynced_count() const;

	/** Applys the given record to the document. Note that the operation
	 * will not be synchronised to other users. Use the insert() and
	 * erase() functions instead.
	 */
	void apply_record(const record& rec);

	/** Initialises the document with a given revision.
	 */
	void initialise(unsigned int revision);

	/** Adds a line to the document. This is only useful during the
	 * synchronisation process and called by the document_info.
	 * TODO: document_info should provide insert/erase functions that
	 * are forwarded to the document and only provide a const document*
	 * to the user.
	 */
	void add_line(const line& line);

#if 0
	/** Synchronization functions called while synchronizing documents.
	 */
	virtual void on_net_sync_init(const net6::packet& pack);
	virtual void on_net_sync_line(const net6::packet& pack);
	virtual void on_net_sync_final(const net6::packet& pack);
#endif
protected:
	std::list<record*> m_unsynced;
	net6::client& m_client;
};

}

#endif // _OBBY_CLIENT_DOCUMENT_HPP_
