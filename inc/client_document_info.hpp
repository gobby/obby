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

#ifndef _OBBY_CLIENT_DOCUMENT_INFO_HPP_
#define _OBBY_CLIENT_DOCUMENT_INFO_HPP_

#include <net6/client.hpp>
#include "client_document.hpp"
#include "local_document_info.hpp"

namespace obby
{

class client_buffer;

/** Information about a document that is provided without being subscribed to
 * a document.
 */

class client_document_info : public local_document_info
{
public:
	client_document_info(const client_buffer& buf, net6::client& client,
	                     const user* owner, unsigned int id,
	                     const std::string& title);
	~client_document_info();

	/** Returns the buffer to which the document is assigned.
	 */
	const client_buffer& get_buffer() const;

	/** Returns the document for this info, if one is assigned.
	 */
	client_document* get_document();

	/** Returns the document for this info, if one is assigned.
	 */
	const client_document* get_document() const;

	/** Sends a rename request for the document.
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

	// TODO: Make the following functions friend to client buffer.
	/** Called by the buffer if a network event occured that belongs to the
	 * document. Parameter 0 has to be the document ID, parameter 1 a kind
	 * of sub-command, what to do. The real packet command is
	 * "obby_document" to notify that the packet belongs to a document.
	 */
	virtual void obby_data(const net6::packet& pack);

	/** Called by the client buffer when user synchronisation begings.
	 * This clears all the correcntly subscribed users.
	 */
	virtual void obby_sync_init();

	/** Adds a user to the list of subscribed users. This function is
	 * called by the buffer while synchronising the document list.
	 */
	virtual void obby_sync_subscribe(const user& user);

	/** Called by the buffer if the local user created a document. The
	 * document content is assigned immediately in this case because
	 * the clients does not wait for server acknowledgement to show the
	 * new document without delay.
	 */
	virtual void obby_local_init(const std::string& initial_content,
	                             bool open_as_edited = false);

protected:
	/** Assigns a document to the document info.
	 */
	virtual void assign_document();

	/** Executes a packet.
	 */
	bool execute_packet(const net6::packet& pack);

	/** Rename command.
	 */
	void on_net_rename(const net6::packet& pack);

	/** Record command: Change in the document.
	 */
	void on_net_record(const net6::packet& pack);

	/** Synchronisation initialisation command.
	 */
	void on_net_sync_init(const net6::packet& pack);

	/** Synchronisation of a line of the document.
	 */
	void on_net_sync_line(const net6::packet& pack);

	/** User subscription command.
	 */
	void on_net_subscribe(const net6::packet& pack);

	/** User unsubscription.
	 */
	void on_net_unsubscribe(const net6::packet& pack);

	net6::client& m_client;
};

}

#endif // _OBBY_CLIENT_DOCUMENT_INFO_HPP_
