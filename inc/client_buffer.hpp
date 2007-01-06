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
	typedef basic_client_document_info<selector_type> document_info;

	typedef net6::default_accumulator<bool, false> password_accumulator;

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

	typedef typename sigc::signal<bool, std::string&>
		::template accumulated<password_accumulator>
		signal_global_password_type;
	typedef typename sigc::signal<bool, std::string&>
		::template accumulated<password_accumulator>
		signal_user_password_type;

	/** Creates a new client_buffer and connects to <em>hostname</em>
	 * at <em>port</em>
	 */
	basic_client_buffer(const std::string& hostname, unsigned int port);

	/** Sends a login request for this client.
	 * @param name User name for this client.
	 * @param red Red color component for the user color.
	 * @param green Green color component for the user color.
	 * @param blue Blue color component for the user color.
	 * @param global_password Password that is used as a global session
	 * password. If it is not provided, signal_global_password will be
	 * emitted to prompt for a session password.
	 * @param user_password Same as global password, but is used as
	 * user password.
	 */
	void login(const std::string& name, int red, int green, int blue,
	           const std::string& global_password = "",
	           const std::string& user_password = "");

	/** Requests a new document at the server and sync its initial
	 * contents. signal_document_insert will be emitted if the server
	 * authorised the creation process.
	 */
	virtual void document_create(const std::string& title,
	                             const std::string& content = "",
	                             bool open_as_edited = false);

	/** Requests the deletion of a document at the server.
	 * signal_document_remove will be emitted if the server
	 * authorized the deletion.
	 */
	virtual void document_remove(document_info& doc);

	/** Looks for a document with the given ID which belongs to the user
	 * with the given owner ID. Note that we do not take a real user object
	 * here because the ID is enough and one might not have a user object
	 * to the corresponding ID. So a time-consuming lookup is obsolete.
	 */
	document_info* document_find(unsigned int owner_id,
	                             unsigned int id) const;

	/** Returns the local user.
	 */
	//virtual user& get_self();

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
	virtual void set_colour(int red, int green, int blue);

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

	/** Signal which will be emitted if a global password is required.
	 */
	signal_global_password_type global_password_event() const;

	/** Signal which will be emitted if a user password is required.
	 */
	signal_user_password_type user_password_event() const;

protected:
	/** Private constructor which may be used by derived objects from
	 * client_buffer to create a derived version of net6::client.
	 */
	basic_client_buffer();

	/** Registers the signal handlers for the net6::client object. It may
	 * be used by derived classed to register these signal handlers.
	 */
	void register_signal_handlers();

        /** Creates a new document info object according to the type of buffer.
	 */
	virtual typename basic_buffer<selector_type>::document_info*
	new_document_info(const user* owner, unsigned int id,
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
private:
	/** This function provides access to the underlaying net6::basic_client
	 * object.
	 */
	net6::basic_client<selector_type>& net6_client();

	/** This function provides access to the underlaying net6::basic_client
	 * object.
	 */
	const net6::basic_client<selector_type>& net6_client() const;
};

typedef basic_client_buffer<net6::selector> client_buffer;

template<typename selector_type>
basic_client_buffer<selector_type>::basic_client_buffer()
 : basic_local_buffer<selector_type>(), m_self(NULL)
{
}

template<typename selector_type>
basic_client_buffer<selector_type>::
	basic_client_buffer(const std::string& hostname, unsigned int port)
{
	// Reslve hostname
	net6::ipv4_address addr(
		net6::ipv4_address::create_from_hostname(hostname, port)
	);

	// Connect to host
	basic_buffer<selector_type>::m_net =
		new net6::basic_client<selector_type>(addr);

	// Register signal handlers
	register_signal_handlers();
}

template<typename selector_type>
void basic_client_buffer<selector_type>::
	login(const std::string& name, int red, int green, int blue,
              const std::string& global_password,
	      const std::string& user_password)
{
	if(!m_public)
		throw std::logic_error("obby::basic_client_buffer::login");

	m_name = name;
	m_red = red;
	m_green = green;
	m_blue = blue;
	m_global_password = global_password;
	m_user_password = user_password;

	net6_client().login(name);
}

template<typename selector_type>
void basic_client_buffer<selector_type>::
	document_create(const std::string& title, const std::string& content,
	                bool open_as_edited)
{
	// TODO: Special handling if not connected
	// Choose new ID
	// TODO: m_doc_counter does not belong into the base class
	unsigned int id = ++ basic_buffer<selector_type>::m_doc_counter;

	// Add document to list
	document_info& info = dynamic_cast<document_info&>(
		basic_buffer<selector_type>::document_add(m_self, id, title)
	);

	// TODO: Emit signal in document_add
	basic_buffer<selector_type>::m_signal_document_insert.emit(info);
	// Assign new document to info, subscribe local user
	// TODO: Do it otherwhere. Maybe another client_document_info
	// constructor that is called somehow...
	info.obby_local_init(content, open_as_edited);
	info.obby_sync_subscribe(*m_self);
	// Emit subscription signal: TODO: Do it in obby_sync_subscribe
	info.subscribe_event().emit(*m_self);
	// Tell others
	net6::packet request_pack("obby_document_create");
	request_pack << id << title << content << open_as_edited;
	net6_client().send(request_pack);
}

template<typename selector_type>
void basic_client_buffer<selector_type>::
	document_remove(document_info& document)
{
	// Send remove request
	net6::packet request_pack("obby_document_remove");
	request_pack << document;
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
	if(m_self == NULL) return m_name;
	return basic_local_buffer<selector_type>::get_name();
}

template<typename selector_type>
void basic_client_buffer<selector_type>::
	send_message(const std::string& message)
{
	net6::packet message_pack("obby_message");
	message_pack << message;
	net6_client().send(message_pack);
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
	set_colour(int red, int green, int blue)
{
	net6::packet colour_pack("obby_user_colour");
	colour_pack << red << green << blue;
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
typename basic_client_buffer<selector_type>::signal_global_password_type
basic_client_buffer<selector_type>::global_password_event() const
{
	return m_signal_global_password;
}

template<typename selector_type>
typename basic_client_buffer<selector_type>::signal_user_password_type
basic_client_buffer<selector_type>::user_password_event() const
{
	return m_signal_user_password;
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
	unsigned int red = pack.get_param(2).net6::basic_parameter::as<int>();
	unsigned int green = pack.get_param(3).net6::basic_parameter::as<int>();
	unsigned int blue = pack.get_param(4).net6::basic_parameter::as<int>();

	// Add user
	user* new_user = basic_buffer<selector_type>::m_user_table.add_user(
		user6, red, green, blue
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
	user* cur_user = basic_buffer<selector_type>::
		m_user_table.user_table::find_user<user::CONNECTED>(user6);

	// Should never happen
	if(cur_user == NULL)
	{
		format_string str("User %0% is not connected");
		str << user6.get_id();
		throw net6::basic_parameter::bad_value(str.str() );
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
	// TODO: This function should take reference, not pointer.
	basic_buffer<selector_type>::m_user_table.remove_user(cur_user);
}

template<typename selector_type>
void basic_client_buffer<selector_type>::on_close()
{
	m_signal_close.emit();
}

template<typename selector_type>
void basic_client_buffer<selector_type>::on_data(const net6::packet& pack)
{
	if(!execute_packet(pack) )
	{
		throw net6::basic_parameter::bad_value(
			"Unexpected command: " + pack.get_command()
		);
	}
}

template<typename selector_type>
void basic_client_buffer<selector_type>::
	on_login_failed(net6::login::error error)
{
	if(error == login::ERROR_WRONG_GLOBAL_PASSWORD)
	{
		// Wrong global password; prompt for new one
		std::string global_password;
		if(m_signal_global_password.emit(global_password) )
		{
			login(
				m_name, m_red, m_green, m_blue,
				global_password, m_user_password
			);
		}
	}
	else if(error == login::ERROR_WRONG_USER_PASSWORD)
	{
		// Wrong user password; prompt for new one
		std::string user_password;
		if(m_signal_user_password.emit(user_password) )
		{
			login(
				m_name, m_red, m_green, m_blue,
				m_global_password, user_password
			);
		}
	}
	else
	{
		m_signal_login_failed.emit(error);
	}
}

template<typename selector_type>
void basic_client_buffer<selector_type>::on_login_extend(net6::packet& pack)
{
	// Add user colour and (hashed) passwords
	pack << m_red << m_green << m_blue
	     << SHA1::hash(m_token + m_global_password)
	     << SHA1::hash(m_token + m_user_password);
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
		pack.get_param(0).net6::basic_parameter::as<int>();

	if(server_version != basic_buffer<selector_type>::PROTOCOL_VERSION)
	{
		on_login_failed(login::ERROR_PROTOCOL_VERSION_MISMATCH);
		return;
	}

	// This token is prepended to every hashed password that is sent over
	// the line. Because every client has its individual token, it is
	// not possible to take one's password hash and send it to the server.
	m_token = pack.get_param(1).net6::basic_parameter::as<std::string>();

	// The server's public key, used to encrypt the password if we want
	// to change it.
	m_public.set_n(mpz_class(
		pack.get_param(2).net6::basic_parameter::as<std::string>(),
		36
	) );

	m_public.set_k(mpz_class(
		pack.get_param(3).net6::basic_parameter::as<std::string>(),
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
	const user* owner =
		pack.get_param(0).net6::basic_parameter::as<user*>();
	unsigned int id =
		pack.get_param(1).net6::basic_parameter::as<int>();
	const std::string& title =
		pack.get_param(2).net6::basic_parameter::as<std::string>();

	// Ignore the packet if it is a document from us because we added it
	// already without waiting for server acknowledge
	if(owner == m_self) return;

	// Get owner ID
	unsigned int owner_id = (owner == NULL ? 0 : owner->get_id() );

	// Is there already such a document
	if(document_find(owner_id, id) )
	{
		format_string str("Document %0%/%1% exists already");
		str << owner_id << id;
		throw net6::basic_parameter::bad_value(str.str() );
	}

	// Add new document
	document_info& new_doc =
		basic_buffer<selector_type>::document_add(owner, id, title);

	// TODO: document_add should emit this signal
	basic_buffer<selector_type>::m_signal_document_insert.emit(new_doc);
	// Emit subscription signal for owner
	// TODO: Should be done implicitly by calling some other function, like
	// new_doc.obby_sync_subscribe(owner)
	new_doc.subscribe_event().emit(*owner);
}

template<typename selector_type>
void basic_client_buffer<selector_type>::
	on_net_document_remove(const net6::packet& pack)
{
	// Get document to remove
	document_info* doc =
		pack.get_param(0).net6::basic_parameter::as<document_info*>();

	// Emit unsubscribe singal for users who were subscribed to this doc
	// TODO: Do this is in document_delete!
	for(typename document_info::user_iterator user_iter = doc->user_begin();
	    user_iter != doc->user_end();
	    ++ user_iter)
		doc->unsubscribe_event().emit(*user_iter);

	// Emit document remove signal
	// TODO: Should be done in document_delete!
	basic_buffer<selector_type>::m_signal_document_remove.emit(*doc);

	// Delete document
	basic_buffer<selector_type>::document_delete(*doc);
}

template<typename selector_type>
void basic_client_buffer<selector_type>::
	on_net_message(const net6::packet& pack)
{
	user* writer =
		pack.get_param(0).net6::basic_parameter::as<user*>();
	const std::string& message =
		pack.get_param(1).net6::basic_parameter::as<std::string>();

	// Valid user id indicates that the message comes from a user, otherwise
	// the server sent the message directly
	if(writer != NULL)
	{
		basic_buffer<selector_type>::m_signal_message.emit(
			*writer, message
		);
	}
	else
	{
		basic_buffer<selector_type>::m_signal_server_message.emit(
			message
		);
	}
}

template<typename selector_type>
void basic_client_buffer<selector_type>::
	on_net_user_colour(const net6::packet& pack)
{
	user* from = pack.get_param(0).net6::basic_parameter::as<user*>();

	// TODO: Should be done by a call to the user_table
	from->set_colour(
		pack.get_param(1).net6::basic_parameter::as<int>(),
		pack.get_param(2).net6::basic_parameter::as<int>(),
		pack.get_param(3).net6::basic_parameter::as<int>()
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
		pack.get_param(0).net6::basic_parameter::as<int>()
	);
}

template<typename selector_type>
void basic_client_buffer<selector_type>::
	on_net_sync_usertable_user(const net6::packet& pack)
{
	// User that was already in the obby session, but isn't anymore.

	// Extract data from packet
	unsigned int id =
		pack.get_param(0).net6::basic_parameter::as<int>();
	const std::string& name =
		pack.get_param(1).net6::basic_parameter::as<std::string>();
	unsigned int red =
		pack.get_param(2).net6::basic_parameter::as<int>();
	unsigned int green =
		pack.get_param(3).net6::basic_parameter::as<int>();
	unsigned int blue =
		pack.get_param(4).net6::basic_parameter::as<int>();

	// Add user into user table
	basic_buffer<selector_type>::m_user_table.add_user(
		id, name, red, green, blue
	);

	// TODO: Emit user_join_signal. Should be done automatically by the
	// function call above
}

template<typename selector_type>
void basic_client_buffer<selector_type>::
	on_net_sync_doclist_document(const net6::packet& pack)
{
	// Get data from packet
	user* owner =
		pack.get_param(0).net6::basic_parameter::as<user*>();
	unsigned int id =
		pack.get_param(1).net6::basic_parameter::as<int>();
	const std::string& title =
		pack.get_param(2).net6::basic_parameter::as<std::string>();

	// Get document owner ID
	unsigned int owner_id = (owner == NULL ? 0 : owner->get_id() );

	// Check for duplicates, should not happen
	if(document_find(owner_id, id) != NULL)
	{
		format_string str("Document %0%/%1% exists already");
		str << owner_id << id;
		throw net6::basic_parameter::bad_value(str.str() );
	}

	// TODO: Emit document_insert signal, should be done by
	// document_add
	document_info& info = dynamic_cast<document_info&>(
		basic_buffer<selector_type>::document_add(owner, id, title)
	);

	// Add users who subscribed to this document.
	// TODO: Refactor this, somehow.
	info.obby_sync_init();
	for(unsigned int i = 3; i < pack.get_param_count(); ++ i)
	{
		// Get user from parameter
		const user* cur_user =
			pack.get_param(i).net6::basic_parameter::as<user*>();

		// Subscribe it
		// TODO: Emit subscription signal (by obby_sync_subscribe)
		info.obby_sync_subscribe(*cur_user);
	}
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
		*pack.get_param(0).net6::basic_parameter::as<
			obby::document_info*
		>()
	);

	// TODO: Rename this function. Think about providing a signal that may
	// be emitted.
	info.on_net_record(document_packet(pack) );
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
typename basic_buffer<selector_type>::document_info*
basic_client_buffer<selector_type>::
	new_document_info(const user* owner, unsigned int id,
	                  const std::string& title)
{
	// Create client_document_info, according to client_buffer
	return new document_info(*this, net6_client(), owner, id, title);
}

template<typename selector_type>
net6::basic_client<selector_type>& basic_client_buffer<selector_type>::
	net6_client()
{
	return dynamic_cast<net6::basic_client<selector_type>&>(
		*basic_buffer<selector_type>::m_net.get()
	);
}

template<typename selector_type>
const net6::basic_client<selector_type>& basic_client_buffer<selector_type>::
	net6_client() const
{
	return dynamic_cast<const net6::basic_client<selector_type>&>(
		*basic_buffer<selector_type>::m_net.get()
	);
}

} // namespace obby

#endif // _OBBY_CLIENT_BUFFER_HPP_
