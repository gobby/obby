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

#ifndef _OBBY_SERVER_BUFFER_HPP_
#define _OBBY_SERVER_BUFFER_HPP_

#include <sigc++/signal.h>
#include <net6/server.hpp>
#include "insert_record.hpp"
#include "delete_record.hpp"
#include "buffer.hpp"

namespace obby
{

/** Buffer that serves as (dedicated) server. It listens for incoming
 * connections from client_buffers and synchronises their changes.
 */
	
class server_buffer : public buffer, public sigc::trackable
{
public: 
	typedef sigc::signal<void, net6::server::peer&> signal_connect_type;
	typedef sigc::signal<void, net6::server::peer&> signal_disconnect_type;

	/** Creates a new server buffer listening on port <em>port</em>.
	 */
	server_buffer(unsigned int port);
	virtual ~server_buffer();

	/** Waits indefinitely for incoming events.
	 */
	virtual void select();

	/** Waits for incoming events or until <em>timeout</em> expires.
	 */
	virtual void select(unsigned int timeout);
	
	/** Creates a new document.
	 */
	document& create_document();

	/** Removes an existing document.
	 */
	void remove_document(document* doc);
	
	/** Signal which will be emitted if a new client has connected.
	 */
	signal_connect_type connect_event() const;

	/** Signal which will be emitted if a connected client has quit without
	 * having been logged in.
	 */
	signal_disconnect_type disconnect_event() const;

protected:
	/** Private constuctor used by derived objects. It does not create
	 * a net6::server object to allow derived object creating derived
	 * classes from net6::server
	 */
	server_buffer();

	/** Internal function that registers the signal handlers for the
	 * net6::server signals. It may be used by derived classes after
	 * having created their server object.
	 */
	void register_signal_handlers();

	/** Adds a new document with the given ID to the buffer. The internal
	 * ID counter is set to the new given document ID.
	 */
	virtual document& add_document(unsigned int id);

	void on_join(net6::server::peer& peer);
	void on_pre_login(net6::server::peer& peer, const net6::packet& pack);
	void on_post_login(net6::server::peer& peer, const net6::packet& pack);
	void on_part(net6::server::peer& peer);
	void on_data(const net6::packet& pack, net6::server::peer& from);
	bool on_auth(net6::server::peer& peer, const net6::packet& pack,
	             std::string& reason);
	void on_extend(net6::server::peer& peer, net6::packet& pack);

	void on_net_record(const net6::packet& pack, user& from);

	void on_net_document_create(const net6::packet& pack, user& from);
	void on_net_document_remove(const net6::packet& pack, user& from);

	unsigned int m_doc_counter;
	net6::server* m_server;

	signal_connect_type m_signal_connect;
	signal_disconnect_type m_signal_disconnect;
};

}

#endif // _OBBY_SERVER_BUFFER_HPP_
