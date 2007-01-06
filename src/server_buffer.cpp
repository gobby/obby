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
#include "server_document.hpp"
#include "server_document_info.hpp"
#include "server_buffer.hpp"

obby::server_buffer::server_buffer()
 : buffer(), m_doc_counter(0), m_server(NULL)
{
}

obby::server_buffer::server_buffer(unsigned int port)
 : buffer(), m_doc_counter(0), m_server(NULL)
{
	m_server = new net6::server(port, false);

	register_signal_handlers();
}

obby::server_buffer::~server_buffer()
{
	if(m_server)
	{
		delete m_server;
		m_server = NULL;
	}
}

void obby::server_buffer::select()
{
	m_server->select();
}

void obby::server_buffer::select(unsigned int timeout)
{
	m_server->select(timeout);
}

void obby::server_buffer::create_document(const std::string& title,
                                          const std::string& content)
{
	// Create the document with the special ID 0, which means that this
	// document is created by the server.
	create_document_impl(title, content, 0);
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
	pack << doc.get_id();
	m_server->send(pack);

	// Free associated resources.
	delete &doc;
}

obby::server_document_info*
obby::server_buffer::find_document(unsigned int id) const
{
	return dynamic_cast<server_document_info*>(buffer::find_document(id) );
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
                                               unsigned int author_id)
{
	// Internally create the document
	unsigned int id = ++ m_doc_counter;
	document_info& doc = add_document_info(id, title);

	// Publish the new document to the users
	net6::packet pack("obby_document_create");
	pack << id << title;
	m_server->send(pack);

	// Insert the document's initial content.
	insert_record rec(0, content, id, 0, author_id, 0);
	doc.get_document()->insert_nosync(rec);

	// Emit the signal
	m_signal_document_insert.emit(doc);
}

void obby::server_buffer::send_message_impl(const std::string& message,
                                            unsigned int user_id)
{
	net6::packet pack("obby_message");
	pack << user_id << message;
	m_server->send(pack);
}

void obby::server_buffer::on_connect(net6::server::peer& peer)
{
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

	// Client logged in. Begin with synchronising.

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
		// Setup document packet with ID and title
		net6::packet doc_pack("obby_sync_doclist_document");
		doc_pack << iter->get_id() << iter->get_title();

		// Add users that are subscribed to this document
		for(document_info::user_iterator user_iter = iter->user_begin();
		    user_iter != iter->user_end();
		    ++ user_iter)
			doc_pack << iter->get_id();

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
	// Verify packet correctness
	if(pack.get_param_count() < 4) return false;
	if(pack.get_param(1).get_type() != net6::packet::param::INT)
		return false;
	if(pack.get_param(2).get_type() != net6::packet::param::INT)
		return false;
	if(pack.get_param(3).get_type() != net6::packet::param::INT)
		return false;

	// Extract colour components
	int red = pack.get_param(1).as_int();
	int green = pack.get_param(2).as_int();
	int blue = pack.get_param(3).as_int();

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

	return true;
}

unsigned int obby::server_buffer::on_login(net6::server::peer& peer,
                                           const net6::packet& pack)
{
	// Get colour from packet
	int red = pack.get_param(1).as_int();
	int green = pack.get_param(2).as_int();
	int blue = pack.get_param(3).as_int();

	// Insert user into list
	user* new_user = m_usertable.add_user(peer, red, green, blue);

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
	if(pack.get_param_count() < 2) return;
	if(pack.get_param(0).get_type() != net6::packet::param::STRING) return;
	if(pack.get_param(1).get_type() != net6::packet::param::STRING) return;

	const std::string& title = pack.get_param(0).as_string();
	const std::string& content = pack.get_param(1).as_string();

	create_document_impl(title, content, from.get_id() );
}

void obby::server_buffer::on_net_document_remove(const net6::packet& pack,
                                                 user& from)
{
	if(pack.get_param_count() < 1) return;
	if(pack.get_param(0).get_type() != net6::packet::param::INT) return;

	// Get document to remove
	unsigned int id = pack.get_param(0).as_int();
	document_info* doc = find_document(id);

	// TODO: Auth

	// Check if we remove a valid document
	if(!doc)
	{
		std::cerr << "obby::server_buffer::on_net_document_remove: "
		          << "Document " << id << " does not exist"
		          << std::endl;
		return;
	}

	remove_document(*doc);
}

void obby::server_buffer::on_net_message(const net6::packet& pack, user& from)
{
	if(pack.get_param_count() < 1) return;
	if(pack.get_param(0).get_type() != net6::packet::param::STRING) return;

	unsigned int uid = from.get_id();
	const std::string& message = pack.get_param(0).as_string();

	obby::user* user = m_usertable.find_user<user::CONNECTED>(uid);
	if(!user)
	{
		std::cerr << "obby::server_buffer::on_net_message: User "
		          << uid << " is not connected" << std::endl;
	}
	else
	{
		m_signal_message.emit(*user, message);
		send_message_impl(message, uid);
	}
}

void obby::server_buffer::on_net_document(const net6::packet& pack, user& from)
{
	if(pack.get_param_count() < 2) return;
	if(pack.get_param(0).get_type() != net6::packet::param::INT) return;
	if(pack.get_param(1).get_type() != net6::packet::param::STRING) return;

	unsigned int id = pack.get_param(0).as_int();
	// Param 1 is a kind of sub-command for the document.

	// Find document to which the packet belongs
	server_document_info* doc = find_document(id);
	if(!doc)
	{
		// No such document
		std::cerr << "obby::server_buffer::on_net_document: Document "
		          << id << " does not exist" << std::endl;
		return;
	}

	// Forward packet
	doc->obby_data(pack, from);
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
obby::server_buffer::add_document_info(unsigned int id,
                                       const std::string& title)
{
	document_info* doc =
		new server_document_info(*this, *m_server, id, title);
	m_doclist.push_back(doc);
	return *doc;
}

