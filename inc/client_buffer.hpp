/* libobby - Network text editing library
 * Copyright (C) 2005, 2006 0x539 dev group
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
#include "sha1.hpp"
#include "rsa.hpp"
#include "local_buffer.hpp"
#include "client_document_info.hpp"

namespace obby
{

/** Buffer to establish a connection to a basic_server_buffer.
 */
template<typename Document, typename Selector>
class basic_client_buffer:
	virtual public basic_local_buffer<Document, Selector>
{
public:
	struct connection_settings {
		std::string name;
		obby::colour colour;
		std::string global_password;
		std::string user_password;
	};

	// Document info
	typedef typename basic_local_buffer<Document, Selector>::
		base_document_info_type base_document_info_type;

	typedef basic_client_document_info<Document, Selector>
		document_info_type;

	// Network
	typedef typename basic_local_buffer<Document, Selector>::
		base_net_type base_net_type;

	typedef net6::basic_client<Selector> net_type;

	// Signal
	typedef net6::default_accumulator<bool, false> login_accumulator;

	typedef sigc::signal<void>
		signal_welcome_type;
	typedef sigc::signal<void, login::error>
		signal_login_failed_type;
	typedef sigc::signal<void>
		signal_close_type;
	typedef sigc::signal<void>
		signal_encrypted_type;

	typedef typename sigc::signal<bool, connection_settings&>
		::template accumulated<login_accumulator>
		signal_prompt_name_type;
	typedef typename sigc::signal<bool, connection_settings&>
		::template accumulated<login_accumulator>
		signal_prompt_colour_type;
	typedef typename sigc::signal<bool, connection_settings&>
		::template accumulated<login_accumulator>
		signal_prompt_global_password_type;
	typedef typename sigc::signal<bool, connection_settings&>
		::template accumulated<login_accumulator>
		signal_prompt_user_password_type;

	/** Creates a new client_buffer that is not connected to anywhere.
	 */
	basic_client_buffer();

	/** Connects to the given host where a obby server is assumed to be
	 * running. After the connection has been established, signal_welcome
	 * will be emitted after the server sent us some initial data (like its
	 * public RSA key). At this point the login function may be used to
	 * login as a user with a given colour.
	 * TODO: Ask username and colour parameters already here and login
	 * implicitly after having called connect().
	 *
	 * @param hostname Host name to connect to. If hostname is not an IP
	 * address, a DNS lookup will be performed.
	 * @param port Port to connect to. 6522 is the default obby port.
	 */
	void connect(const std::string& hostname, unsigned int port = 6522);

	/** Disconnects from a server. Note that documents and users are
	 * still available until reconnection. get_self() will still return
	 * the local user. is_logged_in() will returns false since the
	 * connection is lost.
	 */
	void disconnect();

	/** Checks if we are currently connected to an obby session.
	 */
	//bool is_connected() const;

	/** Requests encryption of the connection.
	 */
	void request_encryption();

	/** Sends a login request for this client. If either the login request
	 * failed because of name or colour are already in use or a password
	 * is required, prompt_*_event() will be emitted. It may be used to
	 * choose another name/colour or password. Returning false from those
	 * signal handlers tell obby to abort the login process,
	 * signal_login_failed will _not_ be emitted in this case.
	 *
	 * TODO: Take connection_settings?
	 * @param name User name for this client.
	 * @param colour User colour.
	 */
	void login(const std::string& name,
	           const obby::colour& colour);

	/** Returns TRUE if the client is already logged in.
	 */
	bool is_logged_in() const;

	/** Requests a new document at the server and sync its initial
	 * contents. signal_document_insert will be emitted if the server
	 * authorised the creation process.
	 */
	virtual void document_create(const std::string& title,
	                             const std::string& encoding,
	                             const std::string& content = "");

	/** Requests the deletion of a document at the server.
	 * signal_document_remove will be emitted if the server
	 * authorized the deletion.
	 */
	virtual void document_remove(base_document_info_type& doc);

	/** Looks for a document with the given ID which belongs to the user
	 * with the given owner ID. Note that we do not take a real user object
	 * here because the ID is enough and one might not have a user object
	 * to the corresponding ID. So a time-consuming lookup is obsolete.
	 */
	document_info_type* document_find(unsigned int owner_id,
	                                  unsigned int id) const;

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

	/** Sends a global message to all users.
	 */
	virtual void send_message(const std::string& message);

	/** Set user password.
	 */
	virtual void set_password(const std::string& password);

	/** Set user colour.
	 */
	virtual void set_colour(const colour& colour);

	/** Signal which will be emitted after the first packet, the welcome
	 * packet, is received. This is a good place to perform a call to
	 * the login function. Note that you cannot login earlier because the
	 * server's public key is not known at that time.
	 */
	signal_welcome_type welcome_event() const;

	/** Signal which will be emitted if the connection to the server
	 * has been lost.
	 */
	signal_close_type close_event() const;

	/** Signal which will be emitted as soon as the connection is
	 * guaranteed to be encrypted.
	 */
	signal_encrypted_type encrypted_event() const;

	/** Signal which will be emitted if a login request did not succeed.
	 */
	signal_login_failed_type login_failed_event() const;

	/** Signal which will be emitted if the name is already in use.
	 */
	signal_prompt_name_type prompt_name_event() const;

	/** Signal which will be emitted if the colour is already in use.
	 */
	signal_prompt_colour_type prompt_colour_event() const;

	/** Signal which will be emitted if a global password is required.
	 */
	signal_prompt_global_password_type prompt_global_password_event() const;

	/** Signal which will be emitted if a user password is required.
	 */
	signal_prompt_user_password_type prompt_user_password_event() const;

protected:
	/** Registers the signal handlers for the net6::client object. It may
	 * be used by derived classes to register these signal handlers.
	 */
	void register_signal_handlers();

        /** Creates a new document info object according to the type of buffer.
	 */
	virtual base_document_info_type*
	new_document_info(const user* owner,
	                  unsigned int id,
	                  const std::string& title,
	                  const std::string& encoding);

        /** Creates a new document info object according to the type of buffer.
	 */
	virtual base_document_info_type*
	new_document_info(const user* owner,
	                  unsigned int id,
	                  const std::string& title,
	                  const std::string& encoding,
	                  const std::string& content);

        /** Creates a new document info object according to the type of buffer.
	 */
	virtual base_document_info_type*
	new_document_info(const net6::packet& pack);

	/** Creates the underlaying net6 network object corresponding to the
	 * buffer's type.
	 * TODO: Make server_buffer's and host_buffer's new_net parameterless
	 * and call open() appropriately
	 */
	virtual base_net_type* new_net();

	/** net6 signal handlers.
	 */
	void on_join(const net6::user& user6, const net6::packet& pack);
	void on_part(const net6::user& user6, const net6::packet& pack);
	void on_close();
	void on_encrypted();
	void on_data(const net6::packet& pack);
	void on_login_failed(net6::login::error error);
	void on_login_extend(net6::packet& pack);

	/** Executes a given network packet.
	 */
	virtual bool execute_packet(const net6::packet& pack);

	/** Welcome handling.
	 */
	virtual void on_net_welcome(const net6::packet& pack);

	/** Document commands.
	 */
	virtual void on_net_document_create(const net6::packet& pack);
	virtual void on_net_document_remove(const net6::packet& pack);

	/** Messaging commands.
	 */
	virtual void on_net_message(const net6::packet& pack);

	/** User colour commands.
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

	/** @brief Closes the session.
	 */
	virtual void session_close();

	/** @brief Implementation of session_close() that does not call
	 * a base function.
	 */
	void session_close_impl();

	const user* m_self;

	std::string m_token;
	RSA::Key m_public;

	connection_settings m_settings;

	signal_welcome_type m_signal_welcome;
	signal_close_type m_signal_close;
	signal_encrypted_type m_signal_encrypted;
	signal_login_failed_type m_signal_login_failed;

	signal_prompt_name_type m_signal_prompt_name;
	signal_prompt_colour_type m_signal_prompt_colour;
	signal_prompt_global_password_type m_signal_prompt_global_password;
	signal_prompt_user_password_type m_signal_prompt_user_password;
private:
	/** This function provides access to the underlaying net6::basic_client
	 * object.
	 */
	net_type& net6_client();

	/** This function provides access to the underlaying net6::basic_client
	 * object.
	 */
	const net_type& net6_client() const;
};

typedef basic_client_buffer<obby::document, net6::selector> client_buffer;

template<typename Document, typename Selector>
basic_client_buffer<Document, Selector>::basic_client_buffer():
	basic_local_buffer<Document, Selector>(), m_self(NULL)
{
}

template<typename Document, typename Selector>
void basic_client_buffer<Document, Selector>::
	connect(const std::string& hostname,
                unsigned int port)
{
	if(basic_buffer<Document, Selector>::is_open() )
	{
		throw std::logic_error(
			"obby::basic_client_buffer::connect:\n"
			"Connection already established"
		);
	}

	// Create connection object
	// TODO: Make the same with server_buffer
	// (create object, register signal handlers, open server)
	basic_buffer<Document, Selector>::m_net.reset(new_net() );

	// Register signal handlers
	register_signal_handlers();

	// Make sure that the signal handlers to the net6::client object
	// are registered before the connection is made, otherwise we may
	// lose a welcome packet if the remote site sends the packet between
	// the connection and the signal handler registration (which _may_
	// occur with connections to localhost).
	net6_client().connect(
		net6::ipv4_address::create_from_hostname(hostname, port)
	);
}

template<typename Document, typename Selector>
void basic_client_buffer<Document, Selector>::disconnect()
{
	if(!basic_buffer<Document, Selector>::is_open() )
	{
		throw std::logic_error(
			"obby::basic_client_buffer::disconnect:\n"
			"Client is not connected"
		);
	}

	// Close session
	session_close();
}

#if 0
template<typename Document, typename Selector>
bool basic_client_buffer<Document, Selector>::is_connected() const
{
	return basic_buffer<Document, Selector>::m_net.get() != NULL;
}
#endif

template<typename Document, typename Selector>
void basic_client_buffer<Document, Selector>::request_encryption()
{
	if(!basic_buffer<Document, Selector>::is_open() )
	{
		throw std::logic_error(
			"obby::basic_client_buffer::request_encryption:\n"
			"Client buffer is not connected"
		);
	}

	net6_client().request_encryption();
}

template<typename Document, typename Selector>
void basic_client_buffer<Document, Selector>::login(const std::string& name,
                                                    const colour& colour)
{
	// Need public key (which comes with welcome packet) before login
	if(!m_public)
	{
		throw std::logic_error(
			"obby::basic_client_buffer::login:\n"
			"No public key available - wait for welcome packet "
			"before calling login"
		);
	}

	m_settings.name = name;
	m_settings.colour = colour;

	net6_client().login(name);
}

template<typename Document, typename Selector>
bool basic_client_buffer<Document, Selector>::is_logged_in() const
{
	// TODO: Return true or false when being disconnected and still
	// having an old session?
	//
	// Some functions like document_remove depend on the current
	// behavior. Change them accordingly if you change something here!
	if(basic_buffer<Document, Selector>::m_net.get() == NULL) return false;
	return net6_client().is_logged_in();
}

template<typename Document, typename Selector>
void basic_client_buffer<Document, Selector>::
	document_create(const std::string& title,
	                const std::string& encoding,
	                const std::string& content)
{
	// TODO: Allow this and create the document just locally?
	if(!is_logged_in() )
	{
		throw std::logic_error(
			"obby::basic_client_buffer::document_create:\n"
			"Cannot create document without being logged in"
		);
	}

	// TODO: m_doc_counter does not belong into the base class
	unsigned int id = ++ basic_buffer<Document, Selector>::m_doc_counter;
	// Create document
	base_document_info_type* info =
		new_document_info(m_self, id, title, encoding, content);
	// Add document to list
	basic_buffer<Document, Selector>::document_add(*info);
	// Tell server
	net6::packet request_pack("obby_document_create");
	request_pack << id << title << encoding << content;
	net6_client().send(request_pack);
}

template<typename Document, typename Selector>
void basic_client_buffer<Document, Selector>::
	document_remove(base_document_info_type& document)
{
	if(is_logged_in() )
	{
		// Send remove request, remove document on server reply
		// to ensure synchronisation
		net6::packet request_pack("obby_document_remove");
		request_pack << &document;
		net6_client().send(request_pack);
	}
	else
	{
		// Delete document
		basic_buffer<Document, Selector>::document_delete(document);
	}
}

template<typename Document, typename Selector>
typename basic_client_buffer<Document, Selector>::document_info_type*
basic_client_buffer<Document, Selector>::
	document_find(unsigned int owner_id,
	              unsigned int id) const
{
	return dynamic_cast<document_info_type*>(
		basic_buffer<Document, Selector>::document_find(owner_id, id)
	);
}

template<typename Document, typename Selector>
const obby::user& basic_client_buffer<Document, Selector>::get_self() const
{
	if(m_self == NULL)
	{
		throw std::logic_error(
			"obby::basic_client_buffer::get_self:\n"
			"Client is not logged in"
		);
	}

	return *m_self;
}

template<typename Document, typename Selector>
const std::string& basic_client_buffer<Document, Selector>::get_name() const
{
	// TODO: Do we still need this?
	if(m_self == NULL) return m_settings.name;
	return basic_local_buffer<Document, Selector>::get_name();
}

template<typename Document, typename Selector>
void basic_client_buffer<Document, Selector>::
	send_message(const std::string& message)
{
	if(is_logged_in() )
	{
		// Wait for server response to ensure synchronisation
		net6::packet message_pack("obby_message");
		message_pack << message;
		net6_client().send(message_pack);
	}
	else
	{
		// If we have never been connected to a session we have no
		// user that might send this message
		if(m_self == NULL)
		{
			throw std::logic_error(
				"obby::basic_client_buffer::send_message:\n"
				"No self user available. Probably the client "
				"buffer never has been connected to a session."
			);
		}

		basic_buffer<Document, Selector>::m_chat.add_user_message(
			message, get_self()
		);
	}
}

template<typename Document, typename Selector>
void basic_client_buffer<Document, Selector>::
	set_password(const std::string& password)
{
	if(is_logged_in() )
	{
		net6::packet password_pack("obby_user_password");
		password_pack << RSA::encrypt(m_public, password);
		net6_client().send(password_pack);
	}
	else
	{
		throw std::logic_error(
			"obby::basic_client_buffer::set_password:\n"
			"Cannot set password without being logged in"
		);
	}
}

template<typename Document, typename Selector>
void basic_client_buffer<Document, Selector>::set_colour(const colour& colour)
{
	if(is_logged_in() )
	{
		net6::packet colour_pack("obby_user_colour");
		colour_pack << colour;
		net6_client().send(colour_pack);
	}
	else
	{
		throw std::logic_error(
			"obby::basic_client_buffer::::set_colour:\n"
			"Cannot change colour without being logged in"
		);
	}
}

template<typename Document, typename Selector>
typename basic_client_buffer<Document, Selector>::signal_welcome_type
basic_client_buffer<Document, Selector>::welcome_event() const
{
	return m_signal_welcome;
}

template<typename Document, typename Selector>
typename basic_client_buffer<Document, Selector>::signal_login_failed_type
basic_client_buffer<Document, Selector>::login_failed_event() const
{
	return m_signal_login_failed;
}

template<typename Document, typename Selector>
typename basic_client_buffer<Document, Selector>::signal_prompt_name_type
basic_client_buffer<Document, Selector>::prompt_name_event() const
{
	return m_signal_prompt_name;
}

template<typename Document, typename Selector>
typename basic_client_buffer<Document, Selector>::signal_prompt_colour_type
basic_client_buffer<Document, Selector>::prompt_colour_event() const
{
	return m_signal_prompt_colour;
}

template<typename Document, typename Selector>
typename basic_client_buffer<Document, Selector>::
	signal_prompt_global_password_type
basic_client_buffer<Document, Selector>::prompt_global_password_event() const
{
	return m_signal_prompt_global_password;
}

template<typename Document, typename Selector>
typename basic_client_buffer<Document, Selector>::
	signal_prompt_user_password_type
basic_client_buffer<Document, Selector>::prompt_user_password_event() const
{
	return m_signal_prompt_user_password;
}

template<typename Document, typename Selector>
typename basic_client_buffer<Document, Selector>::signal_close_type
basic_client_buffer<Document, Selector>::close_event() const
{
	return m_signal_close;
}

template<typename Document, typename Selector>
typename basic_client_buffer<Document, Selector>::signal_encrypted_type
basic_client_buffer<Document, Selector>::encrypted_event() const
{
	return m_signal_encrypted;
}

template<typename Document, typename Selector>
void basic_client_buffer<Document, Selector>::on_join(const net6::user& user6,
                                                      const net6::packet& pack)
{
	unsigned int id =
		pack.get_param(2).net6::parameter::as<unsigned int>();
	colour colour =
		pack.get_param(3).net6::parameter::as<obby::colour>();

	// Add user
	const user* new_user = basic_buffer<Document, Selector>::
		m_user_table.add_user(id, user6, colour);

	// The first joining user is the local one
	if(m_self == NULL) m_self = new_user;

	basic_buffer<Document, Selector>::user_join(*new_user);
}

template<typename Document, typename Selector>
void basic_client_buffer<Document, Selector>::on_part(const net6::user& user6,
                                                      const net6::packet& pack)
{
	// Find user
	const user* cur_user =
		basic_buffer<Document, Selector>::m_user_table.find(
			user6,
			user::flags::CONNECTED,
			user::flags::NONE
		);

	// Should never happen
	if(cur_user == NULL)
	{
		format_string str("User %0% is not connected");
		str << user6.get_id();
		throw net6::bad_value(str.str() );
	}

	basic_buffer<Document, Selector>::user_part(*cur_user);
}

template<typename Document, typename Selector>
void basic_client_buffer<Document, Selector>::on_close()
{
	// Disconnect
	disconnect();
	// TODO: Emit signal_close in the disconnect() function?
	m_signal_close.emit();
}

template<typename Document, typename Selector>
void basic_client_buffer<Document, Selector>::on_encrypted()
{
	m_signal_encrypted.emit();
}

template<typename Document, typename Selector>
void basic_client_buffer<Document, Selector>::on_data(const net6::packet& pack)
{
	if(!execute_packet(pack) )
	{
		throw net6::bad_value(
			"Unexpected command: " + pack.get_command()
		);
	}
}

template<typename Document, typename Selector>
void basic_client_buffer<Document, Selector>::
	on_login_failed(net6::login::error error)
{
	if(error == net6::login::ERROR_NAME_IN_USE)
	{
		if(m_signal_prompt_name.emit(m_settings) )
			login(m_settings.name, m_settings.colour);
	}
	else if(error == login::ERROR_COLOUR_IN_USE)
	{
		if(m_signal_prompt_colour.emit(m_settings) )
			login(m_settings.name, m_settings.colour);
	}
	else if(error == login::ERROR_WRONG_GLOBAL_PASSWORD)
	{
		if(m_signal_prompt_global_password.emit(m_settings) )
			login(m_settings.name, m_settings.colour);
	}
	else if(error == login::ERROR_WRONG_USER_PASSWORD)
	{
		if(m_signal_prompt_user_password.emit(m_settings) )
			login(m_settings.name, m_settings.colour);
	}
	else
	{
		m_signal_login_failed.emit(error);
	}
}

template<typename Document, typename Selector>
void basic_client_buffer<Document, Selector>::
	on_login_extend(net6::packet& pack)
{
	// Add user colour and, if given, (hashed) passwords.
	pack << m_settings.colour;
	if(!m_settings.global_password.empty() ||
	   !m_settings.user_password.empty() )
	{
		pack << SHA1::hash(m_token + m_settings.global_password);
		if(!m_settings.user_password.empty() )
			pack << SHA1::hash(m_token + m_settings.user_password);
	}
}

template<typename Document, typename Selector>
bool basic_client_buffer<Document, Selector>::
	execute_packet(const net6::packet& pack)
{
	// TODO: std::map<> from command to function
	if(pack.get_command() == "obby_welcome")
		{ on_net_welcome(pack); return true; }

	if(pack.get_command() == "obby_document_create")
		{ on_net_document_create(pack); return true; }

	if(pack.get_command() == "obby_document_remove")
		{ on_net_document_remove(pack); return true; }

	if(pack.get_command() == "obby_message")
		{ on_net_message(pack); return true; }

	if(pack.get_command() == "obby_user_colour")
		{ on_net_user_colour(pack); return true; }

	if(pack.get_command() == "obby_user_colour_failed")
		{ on_net_user_colour_failed(pack); return true; }

	if(pack.get_command() == "obby_sync_init")
		{ on_net_sync_init(pack); return true; }

	if(pack.get_command() == "obby_sync_usertable_user")
		{ on_net_sync_usertable_user(pack); return true; }

	if(pack.get_command() == "obby_sync_doclist_document")
		{ on_net_sync_doclist_document(pack); return true; }

	if(pack.get_command() == "obby_sync_final")
		{ on_net_sync_final(pack); return true; }

	if(pack.get_command() == "obby_document")
		{ on_net_document(pack); return true; }

	return false;
}

template<typename Document, typename Selector>
void basic_client_buffer<Document, Selector>::
	on_net_welcome(const net6::packet& pack)
{
	// Get the OBBY version the server is running and compare to the version
	// of this library.
	unsigned long server_version =
		pack.get_param(0).net6::parameter::as<unsigned long>();

	if(server_version !=
	   basic_buffer<Document, Selector>::PROTOCOL_VERSION)
	{
		on_login_failed(login::ERROR_PROTOCOL_VERSION_MISMATCH);
		return;
	}

	// This token is prepended to every hashed password that is sent over
	// the line. Because every client has its individual token, it is
	// not possible to take one's password hash and send it to the server.
	m_token = pack.get_param(1).net6::parameter::as<std::string>();

	// The server's public key, used to encrypt the password if we want
	// to change it.
	m_public.set_n(mpz_class(
		pack.get_param(2).net6::parameter::as<std::string>(),
		36
	) );

	m_public.set_k(mpz_class(
		pack.get_param(3).net6::parameter::as<std::string>(),
		36
	) );

	// Emit welcome signal to indicate that the user may now perform a
	// login() call.
	m_signal_welcome.emit();
}

template<typename Document, typename Selector>
void basic_client_buffer<Document, Selector>::
	on_net_document_create(const net6::packet& pack)
{
	// Get owner, id and title
	const user* owner = pack.get_param(0).net6::parameter::as<const user*>(
		::serialise::hex_context<const user*>(
			basic_buffer<Document, Selector>::get_user_table()
		)
	);

	unsigned int id =
		pack.get_param(1).net6::parameter::as<unsigned int>();
	const std::string& title =
		pack.get_param(2).net6::parameter::as<std::string>();
	const std::string& encoding =
		pack.get_param(3).net6::parameter::as<std::string>();

	// Get owner ID
	unsigned int owner_id = (owner == NULL ? 0 : owner->get_id() );

	// Document owner must not be the local user because if we created the
	// document, the create_document request is not sent back to us.
	if(owner == m_self)
	{
		format_string str("Owner of document %0%/%1% is self");
		str << owner_id << id;
		throw net6::bad_value(str.str() );
	}

	// Is there already such a document?
	if(document_find(owner_id, id) )
	{
		format_string str("Document %0%/%1% exists already");
		str << owner_id << id;
		throw net6::bad_value(str.str() );
	}

	// Add new document
	base_document_info_type* info =
		new_document_info(owner, id, title, encoding);

	basic_buffer<Document, Selector>::document_add(*info);
}

template<typename Document, typename Selector>
void basic_client_buffer<Document, Selector>::
	on_net_document_remove(const net6::packet& pack)
{
	// Get document to remove
	document_info_type& doc = dynamic_cast<document_info_type&>(
		*pack.get_param(0).net6::parameter::as<
			base_document_info_type*
		>(::serialise::hex_context<base_document_info_type*>(*this))
	);

	// Emit unsubscribe singal for users who were subscribed to this doc
	// TODO: Do this is in document_delete!
	for(typename document_info_type::user_iterator user_iter =
		doc.user_begin();
	    user_iter != doc.user_end();
	    ++ user_iter)
	{
		doc.unsubscribe_event().emit(*user_iter);
	}

	// Delete document
	basic_buffer<Document, Selector>::document_delete(doc);
}

template<typename Document, typename Selector>
void basic_client_buffer<Document, Selector>::
	on_net_message(const net6::packet& pack)
{
	const user* writer = pack.get_param(0).net6::parameter::as<const user*>(
		::serialise::hex_context<const user*>(
			basic_buffer<Document, Selector>::get_user_table()
		)
	);

	const std::string& message =
		pack.get_param(1).net6::parameter::as<std::string>();

	// Valid user id indicates that the message comes from a user, otherwise
	// the server sent the message directly
	if(writer != NULL)
	{
		basic_buffer<Document, Selector>::m_chat.add_user_message(
			message,
			*writer
		);
	}
	else
	{
		basic_buffer<Document, Selector>::m_chat.add_server_message(
			message
		);
	}
}

template<typename Document, typename Selector>
void basic_client_buffer<Document, Selector>::
	on_net_user_colour(const net6::packet& pack)
{
	const user* from = pack.get_param(0).net6::parameter::as<const user*>(
		::serialise::hex_context<const user*>(
			basic_buffer<Document, Selector>::get_user_table()
		)
	);

	basic_buffer<Document, Selector>::m_user_table.set_user_colour(
		*from,
		pack.get_param(1).net6::parameter::as<obby::colour>()
	);

	// TODO: user::set_colour should emit the signal
	basic_buffer<Document, Selector>::m_signal_user_colour.emit(*from);
}

template<typename Document, typename Selector>
void basic_client_buffer<Document, Selector>::
	on_net_user_colour_failed(const net6::packet& pack)
{
	basic_local_buffer<Document, Selector>::
		m_signal_user_colour_failed.emit();
}

template<typename Document, typename Selector>
void basic_client_buffer<Document, Selector>::
	on_net_sync_init(const net6::packet& pack)
{
	// Login was successful, synchronisation begins. Clear users and
	// documents from old session that have been kept to be able to still
	// access them while being disconnected.
	basic_buffer<Document, Selector>::m_user_table.clear();
	basic_buffer<Document, Selector>::document_clear();
	m_self = NULL;

	basic_buffer<Document, Selector>::m_signal_sync_init.emit(
		pack.get_param(0).net6::parameter::as<unsigned int>()
	);
}

template<typename Document, typename Selector>
void basic_client_buffer<Document, Selector>::
	on_net_sync_usertable_user(const net6::packet& pack)
{
	// User that was already in the obby session, but isn't anymore.

	// Extract data from packet
	unsigned int id =
		pack.get_param(0).net6::parameter::as<unsigned int>();
	const std::string& name =
		pack.get_param(1).net6::parameter::as<std::string>();
	colour colour =
		pack.get_param(2).net6::parameter::as<obby::colour>();

	// Add user into user table
	basic_buffer<Document, Selector>::m_user_table.add_user(
		id, name, colour
	);

	// TODO: Emit user_join_signal. Should be done automatically by the
	// function call above
}

template<typename Document, typename Selector>
void basic_client_buffer<Document, Selector>::
	on_net_sync_doclist_document(const net6::packet& pack)
{
	// Get data from packet
	const user* owner = pack.get_param(0).net6::parameter::as<const user*>(
		::serialise::hex_context<const user*>(
			basic_buffer<Document, Selector>::get_user_table()
		)
	);

	unsigned int id =
		pack.get_param(1).net6::parameter::as<unsigned int>();

	// Get document owner ID
	unsigned int owner_id = (owner == NULL ? 0 : owner->get_id() );

	// Check for duplicates, should not happen
	if(document_find(owner_id, id) != NULL)
	{
		format_string str("Document %0%/%1% exists already");
		str << owner_id << id;
		throw net6::bad_value(str.str() );
	}

	// Create document_info from packet
	base_document_info_type* info = new_document_info(pack);
	// Add to buffer
	basic_buffer<Document, Selector>::document_add(*info);
}

template<typename Document, typename Selector>
void basic_client_buffer<Document, Selector>::
	on_net_sync_final(const net6::packet& pack)
{
	basic_buffer<Document, Selector>::m_signal_sync_final.emit();
}

template<typename Document, typename Selector>
void basic_client_buffer<Document, Selector>::
	on_net_document(const net6::packet& pack)
{
	// Get document, forward packet
	document_info_type& info = dynamic_cast<document_info_type&>(
		*pack.get_param(0).net6::parameter::as<
			base_document_info_type*
		>(::serialise::hex_context<base_document_info_type*>(*this))
	);

	// TODO: Rename this function. Think about providing a signal that may
	// be emitted.
	info.on_net_packet(document_packet(pack) );
}

template<typename Document, typename Selector>
void basic_client_buffer<Document, Selector>::session_close()
{
	session_close_impl();
	basic_local_buffer<Document, Selector>::session_close_impl();
	basic_buffer<Document, Selector>::session_close_impl();
}

template<typename Document, typename Selector>
void basic_client_buffer<Document, Selector>::session_close_impl()
{
	// Reset passwords to prevent using them for the next connection
	m_settings.global_password = m_settings.user_password = "";
}

template<typename Document, typename Selector>
void basic_client_buffer<Document, Selector>::register_signal_handlers()
{
	net6_client().join_event().connect(
		sigc::mem_fun(*this, &basic_client_buffer::on_join) );
	net6_client().part_event().connect(
		sigc::mem_fun(*this, &basic_client_buffer::on_part) );
	net6_client().close_event().connect(
		sigc::mem_fun(*this, &basic_client_buffer::on_close) );
	net6_client().encrypted_event().connect(
		sigc::mem_fun(*this, &basic_client_buffer::on_encrypted) );
	net6_client().data_event().connect(
		sigc::mem_fun(*this, &basic_client_buffer::on_data) );
	net6_client().login_failed_event().connect(
		sigc::mem_fun(*this, &basic_client_buffer::on_login_failed) );
	net6_client().login_extend_event().connect(
		sigc::mem_fun(*this, &basic_client_buffer::on_login_extend) );
}

template<typename Document, typename Selector>
typename basic_client_buffer<Document, Selector>::base_document_info_type*
basic_client_buffer<Document, Selector>::
	new_document_info(const user* owner,
	                  unsigned int id,
	                  const std::string& title,
	                  const std::string& encoding)
{
	// Create client_document_info, according to client_buffer
	return new document_info_type(
		*this, net6_client(), owner, id, title, encoding
	);
}

template<typename Document, typename Selector>
typename basic_client_buffer<Document, Selector>::base_document_info_type*
basic_client_buffer<Document, Selector>::
	new_document_info(const user* owner,
	                  unsigned int id,
	                  const std::string& title,
	                  const std::string& encoding,
	                  const std::string& content)
{
	return new document_info_type(
		*this, net6_client(), owner, id, title, encoding, content
	);
}

template<typename Document, typename Selector>
typename basic_client_buffer<Document, Selector>::base_document_info_type*
basic_client_buffer<Document, Selector>::
	new_document_info(const net6::packet& pack)
{
	return new document_info_type(*this, net6_client(), pack);
}

template<typename Document, typename Selector>
typename basic_client_buffer<Document, Selector>::base_net_type*
basic_client_buffer<Document, Selector>::new_net()
{
	// Connect to remote host
	return new net_type;
}

template<typename Document, typename Selector>
typename basic_client_buffer<Document, Selector>::net_type&
basic_client_buffer<Document, Selector>::net6_client()
{
	return dynamic_cast<net_type&>(
		*basic_buffer<Document, Selector>::m_net.get()
	);
}

template<typename Document, typename Selector>
const typename basic_client_buffer<Document, Selector>::net_type&
basic_client_buffer<Document, Selector>::net6_client() const
{
	return dynamic_cast<const net_type&>(
		*basic_buffer<Document, Selector>::m_net.get()
	);
}

} // namespace obby

#endif // _OBBY_CLIENT_BUFFER_HPP_
