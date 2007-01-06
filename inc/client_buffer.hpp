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

#include <net6/default_accumulator.hpp>
#include <net6/client.hpp>
#include "error.hpp"
#include "rsa.hpp"
#include "local_buffer.hpp"
#include "client_document_info.hpp"

namespace obby
{

/** Buffer for establish a connection to a server_buffer.
 */

class client_buffer : public local_buffer
{
public:
	typedef net6::default_accumulator<bool, false> password_accumulator;

	typedef sigc::signal<void>               signal_welcome_type;
	typedef sigc::signal<void, login::error> signal_login_failed_type;
	typedef sigc::signal<void, unsigned int> signal_sync_init_type;
	typedef sigc::signal<void>               signal_sync_final_type;
	typedef sigc::signal<void>               signal_close_type;

	typedef sigc::signal<bool, std::string&>
		::accumulated<password_accumulator> signal_global_password_type;
	typedef sigc::signal<bool, std::string&>
		::accumulated<password_accumulator> signal_user_password_type;

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
	 * @param global_password Password that is used as a global session
	 * password. If it is not provided, signal_global_password will be
	 * emitted to prompt for a session password.
	 */
	void login(const std::string& name, int red, int green, int blue,
	           const std::string& global_password = "",
	           const std::string& user_password = "");

	/** Requests a new document at the server and sync its initial
	 * contents. signal_document_insert will be emitted if the server
	 * authorised the creation process.
	 */
	virtual void create_document(const std::string& title,
	                             const std::string& content = "",
	                             bool open_as_edited = false);

	/** Requests the deletion of a document at the server.
	 * signal_document_remove will be emitted if the server
	 * authorized the deletion.
	 */
	virtual void remove_document(document_info& doc);

	/** Looks for a document with the given ID which belongs to the user
	 * with the given owner ID. Note that we do not take a real user object
	 * here because the ID is enough and one might not have a user object
	 * to the corresponding ID. So a time-consuming lookup is obsolete.
	 */
	client_document_info* find_document(unsigned int owner_id,
	                                    unsigned int id) const;

	/** Returns the local user.
	 */
	virtual user& get_self();

	/** Returns the local user.
	 */
	virtual const user& get_self() const;

	/** Returns the name of the local user even if the login process has
	 * not already completed.
	 */
	virtual const std::string& get_name() const;

	/** Returns the public key of the remote server.
	 */
	const RSA::Key& get_public_key() const;

	/** Waits indefinitly for incoming events.
	 */
	virtual void select();

	/** Waits for incoming events or <em>timeout</em> expired.
	 */
	virtual void select(unsigned int timeout);

	/** Sends a global message to all users.
	 */
	virtual void send_message(const std::string& message);

	/** Set user password.
	 */
	void set_password(const std::string& password);

	/** Set user colour.
	 */
	virtual void set_colour(int red, int green, int blue);

	/** Signal which will be emitted after the first packet, the welcome
	 * packet, is received.
	 */
	signal_welcome_type welcome_event() const;

	/** Signal which will be emitted if the initial synchronisation begins.
	 * This means, that the client has logged in successfully.
	 */
	signal_sync_init_type sync_init_event() const;

	/** Signal which will be emitted if the initial synchronisation of the
	 * user list and the document list has been completed.
	 */
	signal_sync_final_type sync_final_event() const;

	/** Signal which will be emitted if the connection to the server
	 * has been lost.
	 */
	signal_close_type close_event() const;

	/** Signal which will be emitted if a login request did not succeed.
	 */
	signal_login_failed_type login_failed_event() const;

	/** Signal which will be emitted if a global password is required.
	 */
	signal_global_password_type global_password_event() const;

	/** Signal which will be emitted if a user password is required.
	 */
	signal_user_password_type user_password_event() const;

protected:
	/** Private constructor which may be used by derived objects from
	 * client_buffer to create a derived vesion of net6::client.
	 */
	client_buffer();

	/** Registers the signal handlers for the net6::client object. It may
	 * be used by derived classed to register these signal handlers.
	 */
	void register_signal_handlers();

        /** Adds a new document with the given title to the buffer.
	 */
	virtual document_info& add_document_info(const user* owner,
	                                         unsigned int id,
	                                         const std::string& title);

	/** net6 signal handlers.
	 */
	void on_join(const net6::user& user6, const net6::packet& pack);
	void on_part(const net6::user& user6, const net6::packet& pack);
	void on_close();
	void on_data(const net6::packet& pack);
	void on_login_failed(net6::login::error error);
	void on_login_extend(net6::packet& pack);

	/** Executes a given network packet.
	 */
	virtual bool execute_packet(const net6::packet& pack);

	/** Welcome handling.
	 */
	virtual void on_net_welcome(const net6::packet& pack);

	/** Document concerning network commands.
	 */
	virtual void on_net_document_create(const net6::packet& pack);
	virtual void on_net_document_remove(const net6::packet& pack);

	/** Messaging commands.
	 */
	virtual void on_net_message(const net6::packet& pack);

	/** User colour change commands.
	 */
	virtual void on_net_user_colour(const net6::packet& pack);
	virtual void on_net_user_colour_failed(const net6::packet& pack);

	/** Synchronisation commands.
	 */
	virtual void on_net_sync_init(const net6::packet& pack);
	virtual void on_net_sync_usertable_user(const net6::packet& pack);
	virtual void on_net_sync_doclist_document(const net6::packet& pack);
	virtual void on_net_sync_final(const net6::packet& pack);

	/** Forwarding commands.
	 */
	virtual void on_net_document(const net6::packet& pack);

	net6::client* m_client;
	user* m_self;

	std::string m_name;
	int m_red;
	int m_green;
	int m_blue;
	std::string m_global_password;
	std::string m_user_password;

	std::string m_token;
	RSA::Key m_public;

	signal_welcome_type m_signal_welcome;
	signal_sync_init_type m_signal_sync_init;
	signal_sync_final_type m_signal_sync_final;
	signal_close_type m_signal_close;
	signal_login_failed_type m_signal_login_failed;
	signal_global_password_type m_signal_global_password;
	signal_user_password_type m_signal_user_password;
};

}

#endif // _OBBY_CLIENT_BUFFER_HPP_
