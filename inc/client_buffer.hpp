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

#ifndef _OBBY_CLIENT_BUFFER_HPP_
#define _OBBY_CLIENT_BUFFER_HPP_

#include <string>
#include <list>
#include <sigc++/signal.h>
#include <net6/client.hpp>
#include "record.hpp"
#include "user.hpp"
#include "insert_record.hpp"
#include "delete_record.hpp"
#include "buffer.hpp"

namespace obby
{

/** Buffer for establish a connection to a server_buffer.
 */

class client_buffer : public buffer, public sigc::trackable
{
public:
	typedef sigc::signal<void>                     signal_sync_type;
	typedef sigc::signal<void>                     signal_close_type;
	typedef sigc::signal<void, const std::string&> signal_login_failed_type;
	
	/** Creates a new client_buffer and connects to <em>hostname</em>
	 * at <em>port</em>
	 */
	client_buffer(const std::string& hostname, unsigned int port);
	virtual ~client_buffer();
	
	/** Sends a login request for this client.
	 * @param name User name for this client.
	 * @param red Red color component for the user color.
	 * @param green Green color component for the user color.
	 * @param blue Blue color component for the user color.
	 */
	void login(const std::string& name, int red, int green, int blue);

	/** Requests a new document at the server and sync its initial
	 * contents. signal_insert_document will be emitted if the server
	 * authorised the creation process.
	 */
	virtual void create_document(const std::string& title,
	                             const std::string& content = "");

	/** Requests a new name for an existing document.
	 */
	virtual void rename_document(document& doc,
	                             const std::string& new_title);

	/** Requests the deletion of a document at the server.
	 * signal_remove_document will be emitted if the server
	 * authorized the deletion.
	 */
	virtual void remove_document(document& doc);

	/** Returns the local user.
	 */
	user& get_self();

	/** Returns the local user.
	 */
	const user& get_self() const;

	/** Waits indefinitly for incoming events.
	 */
	virtual void select();

	/** Waits for incoming events or <em>timeout</em> expired.
	 */
	virtual void select(unsigned int timeout);

	/** Sends a global message to all users.
	 */
	virtual void send_message(const std::string& message);

	/** Signal which will be emitted if the initial synchronization of
	 * the documents has been completed.
	 */
	signal_sync_type sync_event() const;

	/** Signal which will be emitted if the connection to the server
	 * has been lost.
	 */
	signal_close_type close_event() const;

	/** Signal which will be emitted if a login request did not succeed.
	 */
	signal_login_failed_type login_failed_event() const;

protected:
	/** Private constructor which may be used by derived objects from
	 * client_buffer to create a derived vesion of net6::client.
	 */
	client_buffer();

	/** Registers the signal handlers for the net6::client object. It may
	 * be used by derived classed to register these signal handlers.
	 */
	void register_signal_handlers();

        /** Adds a new client_document with the given ID to the buffer.
	 */
	virtual document& add_document(unsigned int id);

	void on_join(net6::client::peer& peer, const net6::packet& pack);
	void on_part(net6::client::peer& peer, const net6::packet& pack);
	void on_close();
	void on_data(const net6::packet& pack);
	void on_login_failed(const std::string& reason);
	void on_login_extend(net6::packet& pack);

	void on_net_record(const net6::packet& pack);

	void on_net_document_create(const net6::packet& pack);
	void on_net_document_rename(const net6::packet& pack);
	void on_net_document_remove(const net6::packet& pack);

	void on_net_message(const net6::packet& pack);
	
	void on_net_sync_init(const net6::packet& pack);
	void on_net_sync_usertable_init(const net6::packet& pack);
	void on_net_sync_usertable_record(const net6::packet& pack);
	void on_net_sync_usertable_final(const net6::packet& pack);
	void on_net_sync_doc_init(const net6::packet& pack);
	void on_net_sync_doc_line(const net6::packet& pack);
	void on_net_sync_doc_final(const net6::packet& pack);
	void on_net_sync_final(const net6::packet& pack);

	std::list<record*> m_unsynced;
	net6::client* m_client;
	user* m_self;

	signal_sync_type m_signal_sync;
	signal_close_type m_signal_close;
	signal_login_failed_type m_signal_login_failed;
};

}

#endif // _OBBY_CLIENT_BUFFER_HPP_
