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

#ifndef _OBBY_SERVER_BUFFER_HPP_
#define _OBBY_SERVER_BUFFER_HPP_

#include "rsa.hpp"
#include "buffer.hpp"
#include "server_document_info.hpp"

namespace obby
{

/** Buffer that serves as (dedicated) server. It listens for incoming
 * connections from client_buffers and synchronises their changes.
 */

class server_buffer : virtual public buffer,
                      public sigc::trackable
{
public: 
	typedef sigc::signal<void, net6::server::peer&> signal_connect_type;
	typedef sigc::signal<void, net6::server::peer&> signal_disconnect_type;

	/** Creates a new server buffer listening on port <em>port</em>.
	 */
	server_buffer(unsigned int port);
	server_buffer(unsigned int port, const RSA::Key& public_key,
	              const RSA::Key& private_key);
	virtual ~server_buffer();

	/** Waits indefinitely for incoming events.
	 */
	virtual void select();

	/** Waits for incoming events or until <em>timeout</em> expires.
	 */
	virtual void select(unsigned int timeout);

	/** Changes the global password for this session.
	 */
	void set_global_password(const std::string& password);
	
	/** Creates a new document with predefined content.
	 * signal_document_insert will be emitted and may be used to access
	 * the resulting obby::document_info.
	 */
	virtual void create_document(const std::string& title,
	                             const std::string& content = "");

	/** Removes an existing document.
	 */
	virtual void remove_document(document_info& doc);

	/** Looks for a document with the given ID which belongs to the user
	 * with the given owner ID. Note that we do not take a real user object
	 * here because the ID is enough and one might not have a user object
	 * to the corresponding ID. So a time-consuming lookup is obsolete.
	 */
	server_document_info* find_document(unsigned int owner_id,
	                                    unsigned int id) const;

	/** Sends a global message to all users.
	 */
	virtual void send_message(const std::string& message);

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

        /** Adds a new document with the given title to the buffer.
	 */
	virtual document_info& add_document_info(const user* owner,
	                                         unsigned int id,
	                                         const std::string& title);

	/** Internal function to create a document with predefined content,
	 * that the given user created.
	 * obby::host_buffer uses this function to create documents with
	 * the ID of its local user.
	 */
	virtual void create_document_impl(const std::string& title,
	                                  const std::string& content,
				          const obby::user* owner,
	                                  unsigned int id);

	/** Relays a message to the other users. The message is originally
	 * sent by the user with the ID <em>user_id</em>.
	 */
	void send_message_impl(const std::string& message,
	                       const obby::user* writer);
	
	/** net6 signal handlers.
	 */
	void on_connect(net6::server::peer& peer);
	void on_disconnect(net6::server::peer& peer);
	void on_join(net6::server::peer& peer);
	void on_part(net6::server::peer& peer);
	bool on_auth(net6::server::peer& peer, const net6::packet& pack,
	             net6::login::error& error);
	unsigned int on_login(net6::server::peer& peer,
	                      const net6::packet& pack);
	void on_extend(net6::server::peer& peer, net6::packet& pack);
	void on_data(net6::server::peer& from, const net6::packet& pack);

	/** Executes a network packet.
	 */
	bool execute_packet(const net6::packet& pack, user& from);

	/** Document commands.
	 */
	void on_net_document_create(const net6::packet& pack, user& from);
	void on_net_document_remove(const net6::packet& pack, user& from);

	/** Messaging commands.
	 */
	void on_net_message(const net6::packet& pack, user& from);
	void on_net_user_password(const net6::packet& pack, user& from);

	/** Forwarding commands.
	 */
	void on_net_document(const net6::packet& pack, user& from);

	/** net6 server object.
	 */
	net6::server* m_server;

	/** This map temporarily caches all tokens issued to the various
	 * clients, they get removed as soon as they are copied into the
	 * corresponding user object.
	 */
	std::map<net6::peer*, std::string> m_tokens;

	/** RSA keys to enable secure authentication.
	 */
	RSA::Key m_public;
	RSA::Key m_private;

	/** Global session password. Only people who know this password are
	 * allowed to join the session.
	 */
	std::string m_global_password;

	signal_connect_type m_signal_connect;
	signal_disconnect_type m_signal_disconnect;
};

}

#endif // _OBBY_SERVER_BUFFER_HPP_
