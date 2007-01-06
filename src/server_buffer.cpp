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

#include <cassert>
#include <iostream>
#include "error.hpp"
#include "rsa.hpp"
#include "sha1.hpp"
#include "server_document.hpp"
#include "server_document_info.hpp"
#include "server_buffer.hpp"

obby::server_buffer::server_buffer()
 : buffer(), m_server(NULL)
{
	// TODO: Key length as ctor parameter
	std::pair<RSA::Key, RSA::Key> keys = RSA::generate(m_rclass, 256);
	m_public = keys.second; m_private = keys.first;
}

obby::server_buffer::server_buffer(const RSA::Key& public_key,
                                   const RSA::Key& private_key)
 : buffer(), m_server(NULL), m_public(public_key), m_private(private_key)
{
}

obby::server_buffer::server_buffer(unsigned int port)
 : buffer(), m_server(NULL)
{
	init_impl(port);

	// TODO: Key length as ctor parameter
	std::pair<RSA::Key, RSA::Key> keys = RSA::generate(m_rclass, 256);
	m_public = keys.second; m_private = keys.first;
}

obby::server_buffer::server_buffer(unsigned int port,
                                   const RSA::Key& private_key,
                                   const RSA::Key& public_key)
 : buffer(), m_server(NULL), m_public(public_key), m_private(private_key)
{
	init_impl(port);
}

obby::server_buffer::~server_buffer()
{
	if(m_server)
	{
		delete m_server;
		m_server = NULL;
	}
}

void obby::server_buffer::init_impl(unsigned int port)
{
	m_server = new net6::server(port);
	register_signal_handlers();
}

void obby::server_buffer::select()
{
	m_server->select();
}

void obby::server_buffer::select(unsigned int timeout)
{
	m_server->select(timeout);
}

void obby::server_buffer::set_global_password(const std::string& password)
{
	m_global_password = password;
}

void obby::server_buffer::create_document(const std::string& title,
                                          const std::string& content)
{
	// Create the document with the special owner NULL,
	// which means that this document is created by the server.
	create_document_impl(title, content, NULL, ++ m_doc_counter);
}

void obby::server_buffer::remove_document(document_info& doc)
{
	// Emit unsubscribe singal for all users that were
	// subscribed to this document.
	for(document_info::user_iterator user_iter = doc.user_begin();
	    user_iter != doc.user_end();
	    ++ user_iter)
		doc.unsubscribe_event().emit(*user_iter);

	// Emit signal that the document will be removed
	m_signal_document_remove.emit(doc);

	// Remove from list
	m_doclist.erase(
		std::remove(m_doclist.begin(), m_doclist.end(), &doc),
		m_doclist.end()
	);

	// Tell other clients that the document will be removed
	net6::packet pack("obby_document_remove");
	pack << doc;
	m_server->send(pack);

	// Free associated resources.
	delete &doc;
}

obby::server_document_info*
obby::server_buffer::find_document(unsigned int owner_id, unsigned int id) const
{
	return dynamic_cast<server_document_info*>(
		buffer::find_document(owner_id, id)
	);
}

void obby::server_buffer::send_message(const std::string& message)
{
	// The server sent a message: Emit corresponding signal.
	m_signal_server_message.emit(message);

	// Call send_message_impl with the ID 0 which means, that the server
	// has sent this message.
	send_message_impl(message, 0);
}

obby::server_buffer::signal_connect_type
obby::server_buffer::connect_event() const
{
	return m_signal_connect;
}

obby::server_buffer::signal_disconnect_type
obby::server_buffer::disconnect_event() const
{
	return m_signal_disconnect;
}

void obby::server_buffer::create_document_impl(const std::string& title,
                                               const std::string& content,
                                               const user* owner,
                                               unsigned int id)
{
	// Internally create the document
	document_info& doc = add_document_info(owner, id, title);
	// Publish the new document to the users
	// TODO: Do not send the packet back to the owner?
	net6::packet pack("obby_document_create");
	pack << owner << id << title;
	m_server->send(pack);
	// Insert the document's initial content.
	insert_record rec(0, content, *doc.get_document(), owner, 0, 0);
	doc.get_document()->insert_nosync(rec);
	// Emit the signal
	m_signal_document_insert.emit(doc);
	// Emit subscription signal for the owner
	doc.subscribe_event().emit(*owner);
}

void obby::server_buffer::send_message_impl(const std::string& message,
                                            const user* writer)
{
	net6::packet pack("obby_message");
	pack << writer << message;
	m_server->send(pack);
}

void obby::server_buffer::on_connect(net6::server::peer& peer)
{
	// Create random token for this connection
	mpz_class t = m_rclass.get_z_bits(48);
	std::string token = t.get_str(36);

	// Add the token for this peer into the token map
	m_tokens[&peer] = token;

	// Send token and the server's public key with the welcome packet
	net6::packet pack("obby_welcome");
	pack << PROTOCOL_VERSION << token << m_private.get_n().get_str(36)
	     << m_public.get_k().get_str(36);
	m_server->send(pack, peer);

	m_signal_connect.emit(peer);
}

void obby::server_buffer::on_disconnect(net6::server::peer& peer)
{
	m_signal_disconnect.emit(peer);
}

void obby::server_buffer::on_join(net6::server::peer& peer)
{
	// Find user in user list
	user* new_user = m_usertable.find_user<user::CONNECTED>(peer);
	if(!new_user)
	{
		std::cerr << "obby::server_buffer::on_join: User "
		          << peer.get_id() << " is not connected" << std::endl;
		return;
	}

	// Send initial sync packet
	net6::packet init_pack("obby_sync_init");
	init_pack << (m_usertable.user_count<user::CONNECTED, true>() +
	             document_count() );
	m_server->send(init_pack, peer);

	// Synchronise not-connected users.
	for(user_table::user_iterator<user::CONNECTED, true> iter =
		m_usertable.user_begin<user::CONNECTED, true>();
	    iter != m_usertable.user_end<user::CONNECTED, true>();
	    ++ iter)
	{
		net6::packet user_pack("obby_sync_usertable_user");
		user_pack << iter->get_id() << iter->get_name()
		          << iter->get_red() << iter->get_green()
		          << iter->get_blue();
		m_server->send(user_pack, peer);
	}

	// Synchronise the document list
	for(document_iterator iter = document_begin();
	    iter != document_end();
	    ++ iter)
	{
		// Setup document packet with owner, ID and title
		net6::packet doc_pack("obby_sync_doclist_document");
		doc_pack << iter->get_owner() << iter->get_id()
		         << iter->get_title();

		// Add users that are subscribed to this document
		for(document_info::user_iterator user_iter = iter->user_begin();
		    user_iter != iter->user_end();
		    ++ user_iter)
			doc_pack << &(*user_iter);

		m_server->send(doc_pack, peer);
	}

	// Done with synchronising
	net6::packet final_sync("obby_sync_final");
	m_server->send(final_sync, peer);

	// Forward join message to documents
	for(document_iterator i = document_begin(); i != document_end(); ++ i)
		i->obby_user_join(*new_user);

	// User joined successfully.
	m_signal_user_join.emit(*new_user);
}

void obby::server_buffer::on_part(net6::server::peer& peer)
{
	// Find user object for given peer
	user* cur_user = m_usertable.find_user<user::CONNECTED>(peer);
	if(!cur_user)
	{
		// Not found: Drop error message...
		// TODO: Throw localised exceptions when we have format strings.
		std::cerr << "obby::server_buffer::on_part: User "
		          << peer.get_id() << " is not connected" << std::endl;
		return;
	}

	// Forward part message to documents
	for(document_iterator i = document_begin(); i != document_end(); ++ i)
		i->obby_user_part(*cur_user);

	// Emit part signal, remove user from user list.
	m_signal_user_part.emit(*cur_user);
	m_usertable.remove_user(cur_user);
}

bool obby::server_buffer::on_auth(net6::server::peer& peer,
                                  const net6::packet& pack,
                                  net6::login::error& error)
{
	// Extract user name
	std::string name = pack.get_param(0).as<std::string>();

	// Extract colour components
	int red = pack.get_param(1).as<int>();
	int green = pack.get_param(2).as<int>();
	int blue = pack.get_param(3).as<int>();

	// Get password, if given
	std::string global_password, user_password;
	if(pack.get_param_count() > 4)
		global_password = pack.get_param(4).as<std::string>();
	if(pack.get_param_count() > 5)
		user_password = pack.get_param(5).as<std::string>();

	// Check for existing colors
	// TODO: Check for colors that non-connected user occupy?
	std::list<user*>::iterator iter;
	for(user_table::user_iterator<user::CONNECTED> iter =
		m_usertable.user_begin<user::CONNECTED>();
	    iter != m_usertable.user_end<user::CONNECTED>();
	    ++ iter)
	{
		if((abs(red   - iter->get_red()) +
		    abs(green - iter->get_green()) +
		    abs(blue  - iter->get_blue())) < 32)
		{
			error = login::ERROR_COLOR_IN_USE;
			return false;
		}
	}

	// Non-empty password?
	if(!m_global_password.empty() )
	{
		assert(m_tokens.find(&peer) != m_tokens.end() );

		// Compare passwords
		if(global_password !=
		   SHA1::hash(m_tokens[&peer] + m_global_password))
		{
			error = login::ERROR_WRONG_GLOBAL_PASSWORD;
			return false;
		}
	}

	// Check user password
	obby::user* user = m_usertable.find_user<user::CONNECTED, true>(name);
	if(user && !user->get_password().empty() )
	{
		// Compare passwords
		if(user_password !=
		   SHA1::hash(m_tokens[&peer] + user->get_password() ))
		{
			error = login::ERROR_WRONG_USER_PASSWORD;
			return false;
		}
	}

	return true;
}

unsigned int obby::server_buffer::on_login(net6::server::peer& peer,
                                           const net6::packet& pack)
{
	// Get colour from packet
	int red = pack.get_param(1).as<int>();
	int green = pack.get_param(2).as<int>();
	int blue = pack.get_param(3).as<int>();

	// Insert user into list
	user* new_user = m_usertable.add_user(peer, red, green, blue);

	std::map<net6::peer*, std::string>::iterator i = m_tokens.find(&peer);
	assert(i != m_tokens.end() );
	new_user->set_token(i->second);
	m_tokens.erase(i);

	// Tell net6 to use already existing ID, if any
	return new_user->get_id();
}

void obby::server_buffer::on_extend(net6::server::peer& peer,
                                    net6::packet& pack)
{
	// Find corresponding user in user list
	user* ideq_user = m_usertable.find_user<user::CONNECTED>(peer);
	if(!ideq_user)
	{
		std::cerr << "obby::server_buffer::on_extend: User "
		          << peer.get_id() << " is not connected" << std::endl;
		return;
	}

	// Extend user-join packet with user color
	pack << ideq_user->get_red() << ideq_user->get_green()
	     << ideq_user->get_blue();
}

void obby::server_buffer::on_data(net6::server::peer& peer,
                                  const net6::packet& pack)
{
	// Find user from peer
	user* from_user = m_usertable.find_user<user::CONNECTED>(peer);
	if(!from_user)
	{
		std::cerr << "obby::server_buffer::on_data: User "
		          << peer.get_id() << " is not connected" << std::endl;
		return;
	}

	// Execute packet
	if(!execute_packet(pack, *from_user) )
	{
		std::cerr << "obby::server_buffer::on_data: Command "
		          << pack.get_command() << " "
		          << "does not exist" << std::endl;
	}
}

bool obby::server_buffer::execute_packet(const net6::packet& pack, user& from)
{
	// TODO: std::map<> mapping command to function

	if(pack.get_command() == "obby_document_create")
	{
		// Document create request
		on_net_document_create(pack, from);
		return true;
	}

	if(pack.get_command() == "obby_document_remove")
	{
		// Document remove request
		on_net_document_remove(pack, from);
		return true;
	}

	if(pack.get_command() == "obby_message")
	{
		// Chat message
		on_net_message(pack, from);
		return true;
	}

	if(pack.get_command() == "obby_user_password")
	{
		// User password
		on_net_user_password(pack, from);
		return true;
	}

	if(pack.get_command() == "obby_document")
	{
		// Forward to the document sub system
		on_net_document(pack, from);
		return true;
	}

	// Command unknown
	return false;
}

void obby::server_buffer::on_net_document_create(const net6::packet& pack,
                                                 user& from)
{
	unsigned int id = pack.get_param(0).as<int>();
	const std::string& title = pack.get_param(1).as<std::string>();
	const std::string& content = pack.get_param(2).as<std::string>();

	create_document_impl(title, content, &from, id);
}

void obby::server_buffer::on_net_document_remove(const net6::packet& pack,
                                                 user& from)
{
	// Get document to remove
	document_info* doc = pack.get_param(0).as<document_info*>();

	// TODO: Auth

	// Remove it
	remove_document(*doc);
}

void obby::server_buffer::on_net_message(const net6::packet& pack, user& from)
{
	// Get message
	const std::string& message = pack.get_param(0).as<std::string>();
	// Send it
	m_signal_message.emit(from, message);
	send_message_impl(message, &from);
}

void obby::server_buffer::on_net_user_password(const net6::packet& pack,
                                               user& from)
{
	// Set password for this user
	from.set_password(RSA::decrypt(m_private,
		pack.get_param(0).as<std::string>() ));
}

void obby::server_buffer::on_net_document(const net6::packet& pack, user& from)
{
	// Get target document
	document_info* doc = pack.get_param(0).as<document_info*>();
	// Cast to server document
	server_document_info* server_doc =
		dynamic_cast<server_document_info*>(doc);
	// Forward packet
	server_doc->obby_data(pack, from);
}

void obby::server_buffer::register_signal_handlers()
{
	m_server->connect_event().connect(
		sigc::mem_fun(*this, &server_buffer::on_connect) );
	m_server->disconnect_event().connect(
		sigc::mem_fun(*this, &server_buffer::on_disconnect) );
	m_server->join_event().connect(
		sigc::mem_fun(*this, &server_buffer::on_join) );
	m_server->part_event().connect(
		sigc::mem_fun(*this, &server_buffer::on_part) );
	m_server->login_auth_event().connect(
		sigc::mem_fun(*this, &server_buffer::on_auth) );
	m_server->login_event().connect(
		sigc::mem_fun(*this, &server_buffer::on_login) );
	m_server->login_extend_event().connect(
		sigc::mem_fun(*this, &server_buffer::on_extend) );
	m_server->data_event().connect(
		sigc::mem_fun(*this, &server_buffer::on_data) );
}

obby::document_info&
obby::server_buffer::add_document_info(const user* owner,
                                       unsigned int id,
                                       const std::string& title)
{
	document_info* doc =
		new server_document_info(*this, *m_server, owner, id, title);
	m_doclist.push_back(doc);
	return *doc;
}

