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
	// Create the document with the special server id 0.
	create_document(title, content, 0);
}

void obby::server_buffer::rename_document(document& doc,
                                          const std::string& new_title)
{
	doc.set_title(new_title);

	net6::packet pack("obby_document_rename");
	pack << doc.get_id() << new_title;
	m_server->send(pack);

	m_signal_rename_document.emit(doc, new_title);
}

void obby::server_buffer::remove_document(document& doc)
{
	m_signal_remove_document.emit(doc);
	m_doclist.erase(std::remove(m_doclist.begin(), m_doclist.end(), &doc),
	                m_doclist.end() );
	
	net6::packet pack("obby_document_remove");
	pack << doc.get_id();
	m_server->send(pack);

	delete &doc;
}

void obby::server_buffer::send_message(const std::string& message)
{
	m_signal_server_message.emit(message);
	relay_message(0, message);
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

obby::document& obby::server_buffer::add_document(unsigned int id)
{
	document* doc = new server_document(id, *m_server, *this);
	m_doclist.push_back(doc);
	return *doc;
}

void obby::server_buffer::create_document(const std::string& title,
                                          const std::string& content,
                                          unsigned int author_id)
{
	// Internally create the document
	unsigned int id = ++ m_doc_counter;
	document& doc = add_document(id);
	doc.set_title(title);

	// Publish the new document to the users
	net6::packet pack("obby_document_create");
	pack << id << title << author_id << content;
	m_server->send(pack);

	// Insert the document's content, syncing is done by the create packet.
	insert_record rec(0, content, id, 0, author_id, 0);
	doc.insert_nosync(rec);

	// Emit the signal
	m_signal_insert_document.emit(doc);
}

void obby::server_buffer::relay_message(unsigned int uid,
                                        const std::string& message)
{
	net6::packet pack("obby_message");
	pack << uid << message;
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

	// Client logged in. Synchronise the complete buffer, but
	// seperate it into multiple packets to not block other high-priority
	// network packets like chat packets.
	net6::packet init_sync("obby_sync_init");
	init_sync << static_cast<unsigned int>(m_doclist.size() );
	m_server->send(init_sync, peer);

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

	// Synchronise the documents
	for(document_iterator iter = document_begin();
	    iter != document_end();
	    ++ iter)
	{
		static_cast<server_document&>(*iter).synchronise(peer);
	}

	// Done with synchronising
	net6::packet final_sync("obby_sync_final");
	m_server->send(final_sync, peer);

	m_signal_user_join.emit(*new_user);
}

void obby::server_buffer::on_part(net6::server::peer& peer)
{
	// Find user object for given peer
	user* cur_user = m_usertable.find_user<user::CONNECTED>(peer);
	if(!cur_user)
	{
		// Not found: Drop error message...
		// TODO: Throw localied exceptions when we have format strings.
		std::cerr << "obby::server_buffer::on_part: User "
		          << peer.get_id() << " is not connected" << std::endl;
	}
	else
	{
		// Emit part signal, remove user from user list.
		m_signal_user_part.emit(*cur_user);
		m_usertable.remove_user(cur_user);
	}
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
	user* from_user = m_usertable.find_user<user::CONNECTED>(peer);
	if(!from_user)
	{
		std::cerr << "obby::server_buffer::on_data: User "
		          << peer.get_id() << " is not connected" << std::endl;
		return;
	}

	if(pack.get_command() == "obby_record")
		on_net_record(pack, *from_user);
	if(pack.get_command() == "obby_document_create")
		on_net_document_create(pack, *from_user);
	if(pack.get_command() == "obby_document_rename")
		on_net_document_rename(pack, *from_user);
	if(pack.get_command() == "obby_document_remove")
		on_net_document_remove(pack, *from_user);
	if(pack.get_command() == "obby_message")
		on_net_message(pack, *from_user);
}

void obby::server_buffer::on_net_record(const net6::packet& pack, user& from)
{
	// Create record from packet
	record* rec = record::from_packet(pack);
	if(!rec) return;

	// Set correct sender
	rec->set_from(from.get_id() );
	
	// Find correct document
	document* doc = find_document(rec->get_document() );
	if(!doc) { delete rec; return; }
		
	try
	{
		// Delegate to document
		doc->on_net_record(*rec);
	}
	catch(...)
	{
		// Release the record if document::on_net_record throws an
		// exception.
		delete rec;
		throw;
	}
}

void obby::server_buffer::on_net_document_create(const net6::packet& pack,
                                                 user& from)
{
	if(pack.get_param_count() < 2) return;
	if(pack.get_param(0).get_type() != net6::packet::param::STRING) return;
	if(pack.get_param(1).get_type() != net6::packet::param::STRING) return;

	const std::string& title = pack.get_param(0).as_string();
	const std::string& content = pack.get_param(1).as_string();

	create_document(title, content, from.get_id() );
}

void obby::server_buffer::on_net_document_rename(const net6::packet& pack,
		                                 user& from)
{
	if(pack.get_param_count() < 2) return;
	if(pack.get_param(0).get_type() != net6::packet::param::INT) return;
	if(pack.get_param(1).get_type() != net6::packet::param::STRING) return;

	unsigned int id = pack.get_param(0).as_int();
	const std::string& title = pack.get_param(1).as_string();
	
	document* doc = find_document(id);
	if(!doc) return;

	rename_document(*doc, title);
}

void obby::server_buffer::on_net_document_remove(const net6::packet& pack,
                                                 user& from)
{
	if(pack.get_param_count() < 1) return;
	if(pack.get_param(0).get_type() != net6::packet::param::INT) return;

	unsigned int id = pack.get_param(0).as_int();
	document* doc = find_document(id);
	if(!doc) return;

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
		relay_message(uid, message);
	}
}
