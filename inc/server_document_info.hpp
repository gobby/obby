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

#ifndef _OBBY_SERVER_DOCUMENT_INFO_HPP_
#define _OBBY_SERVER_DOCUMENT_INFO_HPP_

#include <net6/server.hpp>
#include "server_document.hpp"
#include "document_info.hpp"

namespace obby
{

class server_buffer;

/** Information about a document that is provided without being subscribed to
 * a document.
 */

class server_document_info : virtual public document_info
{
public:
	server_document_info(const server_buffer& buf, net6::server& server,
	                     unsigned int id, const std::string& title);
	~server_document_info();

	/** Returns the buffer to which the document is assigned.
	 */
	const server_buffer& get_buffer() const;

	/** Returns the document for this info, if one is assigned.
	 */
	server_document* get_document();

	/** Returns the document for this info, if one is assigned.
	 */
	const server_document* get_document() const;

	/** Renames the given document.
	 */
	virtual void rename(const std::string& new_title);

	/** Subscribes the given user to this document.
	 */
	void subscribe_user(const user& user);

	/** Unsubscribes the given user from this document.
	 */
	void unsubscribe_user(const user& user);

	/** Called by the buffer if a network event occured that belongs to the
	 * document. Parameter 0 has to the document ID, parameter 1 a kind
	 * of sub-command, what to do. The real packet command is
	 * "obby_document" to notify that the packet belongs to a document.
	 */
	virtual void obby_data(const net6::packet& pack, user& from);

protected:
	/** Protected constructor that may be used by derived classes.
	 * server_document_infos do always assign their document immediately
	 * because they need it to share changes between clients. This
	 * constructor does not assign such a document, so a derived class
	 * is able to call this constructor and then its own assign_document
	 * function to create its own document. For example, the
	 * server_document_info creates a server_document in assign_document,
	 * but the host_document_info needs to create a host_document.
	 */
	server_document_info(const server_buffer& buf, net6::server& server,
	                     unsigned int id, const std::string& title,
	                     bool noassign);

	/** Assigns a document to the document info.
	 */
	virtual void assign_document();

	/** Renames the document. The operation is performed by 
	 * <em>user_id</em>.
	 */
	void rename_impl(const std::string& new_title, unsigned int user_id);

	/** Executes a network packet.
	 */
	bool execute_packet(const net6::packet& pack, obby::user& from);

	/** Rename request.
	 */
	void on_net_rename(const net6::packet& pack, obby::user& from);
	
	/** Change in a document.
	 */
	void on_net_record(const net6::packet& pack, obby::user& from);

	/** Subscribe request.
	 */
	void on_net_subscribe(const net6::packet& pack, obby::user& from);

	/** Unsubscribe request.
	 */
	void on_net_unsubscribe(const net6::packet& pack, obby::user& from);

	net6::server& m_server;
};

}

#endif // _OBBY_SERVER_DOCUMENT_INFO_HPP_
