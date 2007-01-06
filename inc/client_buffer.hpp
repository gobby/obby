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
#include "sha1.hpp"
#include "rsa.hpp"
#include "local_buffer.hpp"
#include "client_document_info.hpp"

namespace obby
{

/** Buffer to establish a connection to a basic_server_buffer.
 */
template<typename selector_type>
class basic_client_buffer : virtual public basic_local_buffer<selector_type>
{
public:
	struct connection_settings {
		std::string name;
		obby::colour colour;
		std::string global_password;
		std::string user_password;
	};

	typedef net6::basic_client<selector_type> net_type;

	typedef typename basic_buffer<selector_type>::base_document_info
		base_document_info;
	typedef basic_client_document_info<selector_type>
		document_info;

	typedef net6::default_accumulator<bool, false> login_accumulator;

	typedef sigc::signal<void>
		signal_welcome_type;
	typedef sigc::signal<void, login::error>
		signal_login_failed_type;
	typedef sigc::signal<void, unsigned int>
		signal_sync_init_type;
	typedef sigc::signal<void>
		signal_sync_final_type;
	typedef sigc::signal<void>
		signal_close_type;

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

	/** Disconnects from a server.
	 */
	void disconnect();

	/** Checks if we are currently connected to an obby session.
	 */
	bool is_connected() const;

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
	                             const std::string& content = "");

	/** Requests the deletion of a document at the server.
	 * signal_document_remove will be emitted if the server
	 * authorized the deletion.
	 */
	virtual void document_remove(base_document_info& doc);

	/** Looks for a document with the given ID which belongs to the user
	 * with the given owner ID. Note that we do not take a real user object
	 * here because the ID is enough and one might not have a user object
	 * to the corresponding ID. So a time-consuming lookup is obsolete.
	 */
	document_info* document_find(unsigned int owner_id,
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
	void set_password(const std::string& password);

	/** Set user colour.
	 */
	virtual void set_colour(const colour& colour);

	/** Signal which will be emitted after the first packet, the welcome
	 * packet, is received. This is a good place to perform a call to
	 * the login function. Note that you cannot login earlier because the
	 * server's public key is not known at that time.
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
	 * be used by derived classed to register these signal handlers.
	 */
	void register_signal_handlers();

        /** Creates a new document info object according to the type of buffer.
	 */
	virtual document_info* new_document_info(const user* owner,
	                                         unsigned int id,
	                                         const std::string& title);

        /** Creates a new document info object according to the type of buffer.
	 */
	virtual document_info* new_document_info(const user* owner,
	                                         unsigned int id,
	                                         const std::string& title,
	                                         const std::string& content);

        /** Creates a new document info object according to the type of buffer.
	 */
	virtual document_info* new_document_info(const net6::packet& pack);

	/** Creates the underlaying net6 network object corresponding to the
	 * buffer's type.
	 * TODO: Make server_buffer's and host_buffer's new_net parameterless
	 * and callopen() approproately
	 */
	virtual net_type* new_net();

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

	user* m_self;

	std::string m_token;
	RSA::Key m_public;

	connection_settings m_settings;

	signal_welcome_type m_signal_welcome;
	signal_sync_init_type m_signal_sync_init;
	signal_sync_final_type m_signal_sync_final;
	signal_close_type m_signal_close;
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

typedef basic_client_buffer<net6::selector> client_buffer;

template<typename selector_type>
basic_client_buffer<selector_type>::basic_client_buffer()
 : basic_local_buffer<selector_type>(), m_self(NULL)
{
}

template<typename selector_type>
void basic_client_buffer<selector_type>::connect(const std::string& hostname,
                                                 unsigned int port)
{
	if(is_connected() )
		throw std::logic_error("obby::basic_client_buffer::connect");

	// Create connection object
	// TODO: Make the same with server_buffer
	// (create object, register signal handlers, open server)
	basic_buffer<selector_type>::m_net.reset(new_net() );

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

template<typename selector_type>
void basic_client_buffer<selector_type>::disconnect()
{
	if(!is_connected() )
		throw std::logic_error("obby::basic_client_buffer::disconnect");

	// TODO: Keep documents and users until reconnection
	basic_buffer<selector_type>::document_clear();
	basic_buffer<selector_type>::m_user_table.clear();
	basic_buffer<selector_type>::m_net.reset(NULL);
	m_self = NULL;

	// Empty passwords
	m_settings.global_password = m_settings.user_password = "";
}

template<typename selector_type>
bool basic_client_buffer<selector_type>::is_connected() const
{
	return basic_buffer<selector_type>::m_net.get() != NULL;
}

template<typename selector_type>
void basic_client_buffer<selector_type>::
	login(const std::string& name,
	      const colour& colour)
{
	// Need public key (which comes with welcome packet) before login
	if(!m_public)
		throw std::logic_error("obby::basic_client_buffer::login");

	m_settings.name = name;
	m_settings.colour = colour;

	net6_client().login(name);
}

template<typename selector_type>
bool basic_client_buffer<selector_type>::is_logged_in() const
{
	return m_self != NULL;
}

template<typename selector_type>
void basic_client_buffer<selector_type>::
	document_create(const std::string& title, const std::string& content)
{
	// TODO: Special handling if not connected
	// Choose new ID
	// TODO: m_doc_counter does not belong into the base class
	unsigned int id = ++ basic_buffer<selector_type>::m_doc_counter;
	// Create document
	document_info* info = new_document_info(m_self, id, title, content);
	// Add document to list
	basic_buffer<selector_type>::document_add(*info);
	// Tell server
	net6::packet request_pack("obby_document_create");
	request_pack << id << title << content;
	net6_client().send(request_pack);
}

template<typename selector_type>
void basic_client_buffer<selector_type>::
	document_remove(base_document_info& document)
{
	// Send remove request
	net6::packet request_pack("obby_document_remove");
	request_pack << &document;
	net6_client().send(request_pack);
}

template<typename selector_type>
typename basic_client_buffer<selector_type>::document_info*
basic_client_buffer<selector_type>::
	document_find(unsigned int owner_id, unsigned int id) const
{
	return dynamic_cast<document_info*>(
		basic_buffer<selector_type>::document_find(owner_id, id)
	);
}

template<typename selector_type>
const obby::user& basic_client_buffer<selector_type>::get_self() const
{
	if(m_self == NULL)
		throw std::logic_error("obby::basic_client_buffer::get_self");

	return *m_self;
}

template<typename selector_type>
const std::string& basic_client_buffer<selector_type>::get_name() const
{
	if(m_self == NULL) return m_settings.name;
	return basic_local_buffer<selector_type>::get_name();
}

template<typename selector_type>
void basic_client_buffer<selector_type>::
	send_message(const std::string& message)
{
	net6::packet message_pack("obby_message");
	message_pack << message;
	net6_client().send(message_pack);

	// Do not add directly, wait for confirmation to ensure synchronisation
/*	basic_buffer<selector_type>::m_chat.add_user_message(
		message, get_self()
	);*/
}

template<typename selector_type>
void basic_client_buffer<selector_type>::
	set_password(const std::string& password)
{
	net6::packet password_pack("obby_user_password");
	password_pack << RSA::encrypt(m_public, password);
	net6_client().send(password_pack);
}

template<typename selector_type>
void basic_client_buffer<selector_type>::
	set_colour(const colour& colour)
{
	net6::packet colour_pack("obby_user_colour");
	colour_pack << colour;
	net6_client().send(colour_pack);
}

template<typename selector_type>
typename basic_client_buffer<selector_type>::signal_welcome_type
basic_client_buffer<selector_type>::welcome_event() const
{
	return m_signal_welcome;
}

template<typename selector_type>
typename basic_client_buffer<selector_type>::signal_sync_init_type
basic_client_buffer<selector_type>::sync_init_event() const
{
	return m_signal_sync_init;
}

template<typename selector_type>
typename basic_client_buffer<selector_type>::signal_sync_final_type
basic_client_buffer<selector_type>::sync_final_event() const
{
	return m_signal_sync_final;
}

template<typename selector_type>
typename basic_client_buffer<selector_type>::signal_login_failed_type
basic_client_buffer<selector_type>::login_failed_event() const
{
	return m_signal_login_failed;
}

template<typename selector_type>
typename basic_client_buffer<selector_type>::signal_prompt_name_type
basic_client_buffer<selector_type>::prompt_name_event() const
{
	return m_signal_prompt_name;
}

template<typename selector_type>
typename basic_client_buffer<selector_type>::signal_prompt_colour_type
basic_client_buffer<selector_type>::prompt_colour_event() const
{
	return m_signal_prompt_colour;
}

template<typename selector_type>
typename basic_client_buffer<selector_type>::signal_prompt_global_password_type
basic_client_buffer<selector_type>::prompt_global_password_event() const
{
	return m_signal_prompt_global_password;
}

template<typename selector_type>
typename basic_client_buffer<selector_type>::signal_prompt_user_password_type
basic_client_buffer<selector_type>::prompt_user_password_event() const
{
	return m_signal_prompt_user_password;
}

template<typename selector_type>
typename basic_client_buffer<selector_type>::signal_close_type
basic_client_buffer<selector_type>::close_event() const
{
	return m_signal_close;
}

template<typename selector_type>
void basic_client_buffer<selector_type>::
	on_join(const net6::user& user6, const net6::packet& pack)
{
	unsigned int id =
		pack.get_param(2).net6::parameter::as<unsigned int>();
	colour colour =
		pack.get_param(3).net6::parameter::as<obby::colour>();

	// Add user
	user* new_user = basic_buffer<selector_type>::m_user_table.add_user(
		id, user6, colour
	);

	// The first joining user is the local one
	if(m_self == NULL) m_self = new_user;

	// Forward join message to documents
	// TODO: Document shall connect to user_join_event
	for(typename basic_buffer<selector_type>::document_iterator doc_iter =
		basic_buffer<selector_type>::document_begin();
	    doc_iter != basic_buffer<selector_type>::document_end();
	    ++ doc_iter)
		doc_iter->obby_user_join(*new_user);

	// TODO: Move signal emission to user_table::add_user
	basic_buffer<selector_type>::m_signal_user_join.emit(*new_user);
}

template<typename selector_type>
void basic_client_buffer<selector_type>::
	on_part(const net6::user& user6, const net6::packet& pack)
{
	// Find user
	const user* cur_user = basic_buffer<selector_type>::
		m_user_table.find(user6);

	// Should never happen
	if(cur_user == NULL)
	{
		format_string str("User %0% is not connected");
		str << user6.get_id();
		throw net6::bad_value(str.str() );
	}

	// Forward part message to the documents
	// TODO: Documents should connect to user_part_event
	for(typename basic_buffer<selector_type>::document_iterator doc_iter =
		basic_buffer<selector_type>::document_begin();
	    doc_iter != basic_buffer<selector_type>::document_end();
	    ++ doc_iter)
		doc_iter->obby_user_part(*cur_user);

	// TODO: Should be done by user_table::remove_user
	basic_buffer<selector_type>::m_signal_user_part.emit(*cur_user);

	// Remove user
	basic_buffer<selector_type>::m_user_table.remove_user(*cur_user);
}

template<typename selector_type>
void basic_client_buffer<selector_type>::on_close()
{
	// Disconnect
	disconnect();
	// TODO: Emit signal_close in the disconnect() function?
	m_signal_close.emit();
}

template<typename selector_type>
void basic_client_buffer<selector_type>::on_data(const net6::packet& pack)
{
	if(!execute_packet(pack) )
	{
		throw net6::bad_value(
			"Unexpected command: " + pack.get_command()
		);
	}
}

template<typename selector_type>
void basic_client_buffer<selector_type>::
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

template<typename selector_type>
void basic_client_buffer<selector_type>::on_login_extend(net6::packet& pack)
{
	// Add user colour and, if given, (hashed) passwords.
	pack << m_settings.colour;
	if(!m_settings.global_password.empty() )
	{
		pack << SHA1::hash(m_token + m_settings.global_password);
		if(!m_settings.user_password.empty() )
			pack << SHA1::hash(m_token + m_settings.user_password);
	}
}

template<typename selector_type>
bool basic_client_buffer<selector_type>::
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

template<typename selector_type>
void basic_client_buffer<selector_type>::
	on_net_welcome(const net6::packet& pack)
{
	// Get the OBBY version the server is running and compare to the version
	// of this library.
	unsigned long server_version =
		pack.get_param(0).net6::parameter::as<unsigned long>();

	if(server_version != basic_buffer<selector_type>::PROTOCOL_VERSION)
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

template<typename selector_type>
void basic_client_buffer<selector_type>::
	on_net_document_create(const net6::packet& pack)
{
	// Get owner, id and title
	const user* owner = pack.get_param(0).net6::parameter::
		as<const user*>(basic_buffer<selector_type>::get_user_table());
	unsigned int id =
		pack.get_param(1).net6::parameter::as<unsigned int>();
	const std::string& title =
		pack.get_param(2).net6::parameter::as<std::string>();

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
	document_info* info = new_document_info(owner, id, title);
	basic_buffer<selector_type>::document_add(*info);
}

template<typename selector_type>
void basic_client_buffer<selector_type>::
	on_net_document_remove(const net6::packet& pack)
{
	// Get document to remove
	obby::document_info* doc = pack.get_param(0).net6::
		parameter::as<obby::document_info*>(*this);

	// Emit unsubscribe singal for users who were subscribed to this doc
	// TODO: Do this is in document_delete!
	for(typename document_info::user_iterator user_iter = doc->user_begin();
	    user_iter != doc->user_end();
	    ++ user_iter)
		doc->unsubscribe_event().emit(*user_iter);

	// Delete document
	basic_buffer<selector_type>::document_delete(*doc);
}

template<typename selector_type>
void basic_client_buffer<selector_type>::
	on_net_message(const net6::packet& pack)
{
	const user* writer = pack.get_param(0).net6::parameter::
		as<const user*>(basic_buffer<selector_type>::get_user_table() );
	const std::string& message =
		pack.get_param(1).net6::parameter::as<std::string>();

	// Valid user id indicates that the message comes from a user, otherwise
	// the server sent the message directly
	if(writer != NULL)
	{
		basic_buffer<selector_type>::m_chat.add_user_message(
			message,
			*writer
		);
	}
	else
	{
		basic_buffer<selector_type>::m_chat.add_server_message(
			message
		);
	}
}

template<typename selector_type>
void basic_client_buffer<selector_type>::
	on_net_user_colour(const net6::packet& pack)
{
	const user* from = pack.get_param(0).net6::parameter::
		as<const user*>(basic_buffer<selector_type>::get_user_table() );

	// TODO: Should be done by a call to the user_table
	const_cast<user*>(from)->set_colour(
		pack.get_param(1).net6::parameter::as<obby::colour>()
	);

	// TODO: user::set_colour should emit the signal
	basic_buffer<selector_type>::m_signal_user_colour.emit(*from);
}

template<typename selector_type>
void basic_client_buffer<selector_type>::
	on_net_user_colour_failed(const net6::packet& pack)
{
	basic_local_buffer<selector_type>::m_signal_user_colour_failed.emit();
}

template<typename selector_type>
void basic_client_buffer<selector_type>::
	on_net_sync_init(const net6::packet& pack)
{
	m_signal_sync_init.emit(
		pack.get_param(0).net6::parameter::as<unsigned int>()
	);
}

template<typename selector_type>
void basic_client_buffer<selector_type>::
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
	basic_buffer<selector_type>::m_user_table.add_user(
		id, name, colour
	);

	// TODO: Emit user_join_signal. Should be done automatically by the
	// function call above
}

template<typename selector_type>
void basic_client_buffer<selector_type>::
	on_net_sync_doclist_document(const net6::packet& pack)
{
	// Get data from packet
	const user* owner = pack.get_param(0).net6::parameter::
		as<const user*>(basic_buffer<selector_type>::get_user_table() );
	unsigned int id =
		pack.get_param(1).net6::parameter::as<unsigned int>();
//	const std::string& title =
//		pack.get_param(2).net6::basic_parameter::as<std::string>();

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
	document_info* info = new_document_info(pack);
	// Add to buffer
	basic_buffer<selector_type>::document_add(*info);
}

template<typename selector_type>
void basic_client_buffer<selector_type>::
	on_net_sync_final(const net6::packet& pack)
{
	m_signal_sync_final.emit();
}

template<typename selector_type>
void basic_client_buffer<selector_type>::
	on_net_document(const net6::packet& pack)
{
	// Get document, forward packet
	document_info& info = dynamic_cast<document_info&>(
		*pack.get_param(0).net6::parameter::as<
			obby::document_info*
		>(*this)
	);

	// TODO: Rename this function. Think about providing a signal that may
	// be emitted.
	info.on_net_packet(document_packet(pack) );
}

template<typename selector_type>
void basic_client_buffer<selector_type>::register_signal_handlers()
{
	net6_client().join_event().connect(
		sigc::mem_fun(*this, &basic_client_buffer::on_join) );
	net6_client().part_event().connect(
		sigc::mem_fun(*this, &basic_client_buffer::on_part) );
	net6_client().close_event().connect(
		sigc::mem_fun(*this, &basic_client_buffer::on_close) );
	net6_client().data_event().connect(
		sigc::mem_fun(*this, &basic_client_buffer::on_data) );
	net6_client().login_failed_event().connect(
		sigc::mem_fun(*this, &basic_client_buffer::on_login_failed) );
	net6_client().login_extend_event().connect(
		sigc::mem_fun(*this, &basic_client_buffer::on_login_extend) );
}

template<typename selector_type>
typename basic_client_buffer<selector_type>::document_info*
basic_client_buffer<selector_type>::
	new_document_info(const user* owner, unsigned int id,
	                  const std::string& title)
{
	// Create client_document_info, according to client_buffer
	return new document_info(*this, net6_client(), owner, id, title);
}

template<typename selector_type>
typename basic_client_buffer<selector_type>::document_info*
basic_client_buffer<selector_type>::
	new_document_info(const user* owner, unsigned int id,
	                  const std::string& title, const std::string& content)
{
	return new document_info(
		*this, net6_client(), owner, id, title, content
	);
}

template<typename selector_type>
typename basic_client_buffer<selector_type>::document_info*
basic_client_buffer<selector_type>::new_document_info(const net6::packet& pack)
{
	return new document_info(*this, net6_client(), pack);
}

template<typename selector_type>
typename basic_client_buffer<selector_type>::net_type*
basic_client_buffer<selector_type>::new_net()
{
	// Connect to remote host
	return new net_type;
}

template<typename selector_type>
typename basic_client_buffer<selector_type>::net_type&
basic_client_buffer<selector_type>::net6_client()
{
	return dynamic_cast<net_type&>(
		*basic_buffer<selector_type>::m_net.get()
	);
}

template<typename selector_type>
const typename basic_client_buffer<selector_type>::net_type&
basic_client_buffer<selector_type>::net6_client() const
{
	return dynamic_cast<const net_type&>(
		*basic_buffer<selector_type>::m_net.get()
	);
}

} // namespace obby

#endif // _OBBY_CLIENT_BUFFER_HPP_
