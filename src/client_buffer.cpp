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

#include <iostream>
#include "client_document.hpp"
#include "client_buffer.hpp"

/** These variables are used to store the wished user color from the login()
 * call to the login_extend signal handler.
 * TODO: This is not thread-safe. But, does anyone want to use threads to login
 * with two client_buffers at the same time..?
 * Another solution would be to establish the connection to the login_extend
 * just in the login() call and bind the colour components to it.
 */
namespace { int st_red, st_green, st_blue; }

obby::client_buffer::client_buffer()
 : buffer(), m_client(NULL), m_self(NULL)
{
}

obby::client_buffer::client_buffer(const std::string& hostname,
                                   unsigned int port)
 : local_buffer(), m_client(NULL), m_self(NULL)
{
	// Resolve hostname
	net6::ipv4_address addr(
		net6::ipv4_address::create_from_hostname(hostname, port)
	);

	// Connect to server
	m_client = new net6::client(addr);

	// Register signal handlers
	register_signal_handlers();
}

obby::client_buffer::~client_buffer()
{
	// Check that the client is not already deleted by a subclass.
	if(m_client)
	{
		delete m_client;
		m_client = NULL;
	}
}

void obby::client_buffer::login(const std::string& name, int red, int green,
                                int blue)
{
	st_red = red; st_green = green; st_blue = blue;
	m_client->login(name);
}

void obby::client_buffer::create_document(const std::string& title,
                                          const std::string& content)
{
	// TODO: Authentication
	// Choose new ID for the document
	unsigned int id = ++ m_doc_counter;
	// Add the new document to the list
	document_info& info = add_document_info(m_self, id, title);
	client_document_info& client_info =
		dynamic_cast<client_document_info&>(info);
	// Document has been created: Emit corresponding signal
	m_signal_document_insert.emit(info);
	// Assign a new document to the info
	client_info.obby_local_init(content);
	// Subscribe the local user to the new document
	client_info.obby_sync_subscribe(*m_self);
	// Emit subscription signal
	client_info.subscribe_event().emit(*m_self);
	// Tell others
	net6::packet request_pack("obby_document_create");
	request_pack << id << title << content;
	m_client->send(request_pack);
}

void obby::client_buffer::remove_document(obby::document_info& document)
{
	// Send remove document request
	net6::packet request_pack("obby_document_remove");
	request_pack << document;
	m_client->send(request_pack);
}

obby::client_document_info*
obby::client_buffer::find_document(unsigned int owner_id, unsigned int id) const
{
	return dynamic_cast<obby::client_document_info*>(
		buffer::find_document(owner_id, id)
	);
}

obby::user& obby::client_buffer::get_self()
{
	return *m_self;
}

const obby::user& obby::client_buffer::get_self() const
{
	return *m_self;
}

void obby::client_buffer::select()
{
	m_client->select();
}

void obby::client_buffer::select(unsigned int timeout)
{
	m_client->select(timeout);
}

void obby::client_buffer::send_message(const std::string& message)
{
	// Sends a chat message
	net6::packet pack("obby_message");
	pack << message;
	m_client->send(pack);
}

obby::client_buffer::signal_sync_type obby::client_buffer::sync_event() const
{
	return m_signal_sync;
}

obby::client_buffer::signal_close_type obby::client_buffer::close_event() const
{
	return m_signal_close;
}

obby::client_buffer::signal_login_failed_type
obby::client_buffer::login_failed_event() const
{
	return m_signal_login_failed;
}

void obby::client_buffer::on_join(net6::client::peer& peer,
                                  const net6::packet& pack)
{
	int red = pack.get_param(2).as<int>();
	int green = pack.get_param(3).as<int>();
	int blue = pack.get_param(4).as<int>();

	// Add user into user table
	user* new_user = m_usertable.add_user(peer, red, green, blue);

	// The first joining user is the local one
	if(!m_self) m_self = new_user;

	// Forward join message to the documents
	for(document_iterator i = document_begin(); i != document_end(); ++ i)
		i->obby_user_join(*new_user);
	
	m_signal_user_join.emit(*new_user);
}

void obby::client_buffer::on_part(net6::client::peer& peer,
                                  const net6::packet& pack)
{
	// Find user
	user* cur_user = m_usertable.find_user<user::CONNECTED>(peer);
	
	// Make sure that the user we are removing was connected
	if(cur_user == NULL)
	{
		std::cerr << "obby::client_buffer::on_part: User "
		          << peer.get_id() << " is not connected" << std::endl;
		return;
	}

	// Forward part message to the documents.
	for(document_iterator i = document_begin(); i != document_end(); ++ i)
		i->obby_user_part(*cur_user);

	// Emit signal for the removed user
	m_signal_user_part.emit(*cur_user);

	// Remove user
	m_usertable.remove_user(cur_user);
}

void obby::client_buffer::on_close()
{
	m_signal_close.emit();
}

void obby::client_buffer::on_data(const net6::packet& pack)
{
	if(!execute_packet(pack) )
	{
		std::cerr << "obby::client_buffer::on_data: Command "
		          << pack.get_command() << " does not exist"
		          << std::endl;
	}
}

void obby::client_buffer::on_login_failed(net6::login::error error)
{
	m_signal_login_failed.emit(error);
}

void obby::client_buffer::on_login_extend(net6::packet& pack)
{
	pack << st_red << st_green << st_blue;
}

bool obby::client_buffer::execute_packet(const net6::packet& pack)
{
	// TODO: std::map from command to function
	if(pack.get_command() == "obby_document_create")
	{
		// Create document request
		on_net_document_create(pack);
		return true;
	}

	if(pack.get_command() == "obby_document_remove")
	{
		// Remove document request
		on_net_document_remove(pack);
		return true;
	}

	if(pack.get_command() == "obby_message")
	{
		// Chat message
		on_net_message(pack);
		return true;
	}

	if(pack.get_command() == "obby_sync_usertable_user")
	{
		// Usertable synchronisation
		on_net_sync_usertable_user(pack);
		return true;
	}
	
	if(pack.get_command() == "obby_sync_doclist_document")
	{
		// Document list synchronisation
		on_net_sync_doclist_document(pack);
		return true;
	}
		
	if(pack.get_command() == "obby_sync_final")
	{
		// End-of-synchronisation
		on_net_sync_final(pack);
		return true;
	}

	if(pack.get_command() == "obby_document")
	{
		// Document forwarding
		on_net_document(pack);
		return true;
	}

	// Command not understood
	return false;
}

void obby::client_buffer::on_net_document_create(const net6::packet& pack)
{
	// Get owner, id and document title
	user* owner = pack.get_param(0).as<user*>();
	unsigned int id = pack.get_param(1).as<int>();
	const std::string& title = pack.get_param(2).as<std::string>();

	// Ignore the packet if it's a document from us because we added it
	// already without having the acknowledge from the server.
	if(owner == m_self) return;

	// Is there already such a document?
	if(find_document(owner->get_id(), id) )
	{
		std::cerr << "obby::client_buffer::on_net_document_create: "
		          << "Document " << id << " from " << owner->get_id()
		          << " exists already" << std::endl;
		return;
	}

	// Add new document
	document_info& new_doc = add_document_info(owner, id, title);
	// Emit document insert signal
	m_signal_document_insert.emit(new_doc);
	// Emit subscription singal for the owner
	new_doc.subscribe_event().emit(*owner);
}

void obby::client_buffer::on_net_document_remove(const net6::packet& pack)
{
	// Extract document
	document_info* doc = pack.get_param(0).as<document_info*>();

	// Emit unsubscribe signal for users who were subscribed to this
	// document
	for(document_info::user_iterator user_iter = doc->user_begin();
	    user_iter != doc->user_end();
	    ++ user_iter)
		doc->unsubscribe_event().emit(*user_iter);

	// Emit document remove signal
	m_signal_document_remove.emit(*doc);

	// Delete document
	delete doc;
	m_doclist.erase(
		std::remove(m_doclist.begin(), m_doclist.end(), doc),
		m_doclist.end()
	);
}

void obby::client_buffer::on_net_message(const net6::packet& pack)
{
	user* writer = pack.get_param(0).as<user*>();
	const std::string& message = pack.get_param(1).as<std::string>();

	// Valid user id => Message comes from a user
	if(writer != NULL)
	{
		// Emit message signal
		m_signal_message.emit(*writer, message);
	}
	else
	{
		// Got server message
		m_signal_server_message.emit(message);
	}
}

void obby::client_buffer::on_net_sync_usertable_user(const net6::packet& pack)
{
	// User that was already in the obby session, but isn't anymore. The
	// server tells us ID, name, red, green and blue values.

	// Extract data from packet
	unsigned int id = pack.get_param(0).as<int>();
	const std::string& name = pack.get_param(1).as<std::string>();
	int red = pack.get_param(2).as<int>();
	int green = pack.get_param(3).as<int>();
	int blue = pack.get_param(4).as<int>();

	// Add user into user table
	m_usertable.add_user(id, name, red, green, blue);
}

void obby::client_buffer::on_net_sync_doclist_document(const net6::packet& pack)
{
	// Extract owner, id and title
	user* owner = pack.get_param(0).as<user*>();
	unsigned int id = pack.get_param(1).as<int>();
	const std::string& title = pack.get_param(2).as<std::string>();

	// Check if there is such a document
	if(find_document(owner->get_id(), id) )
	{
		std::cerr << "obby::client_buffer::on_net_sync_doclist_document"
		          << ": Document " << id << " from " << owner->get_id()
		          << " exists already" << std::endl;
		return;
	}

	obby::document_info& info = add_document_info(owner, id, title);
	obby::client_document_info& client_info =
		dynamic_cast<client_document_info&>(info);

	// Add users who subscribed to this document
	for(unsigned int i = 3; i < pack.get_param_count(); ++ i)
	{
		// Read user from parameter
		user* cur_user = pack.get_param(i).as<user*>();
		// Synchronise this user's subscription.
		client_info.obby_sync_subscribe(*cur_user);
	}
}

void obby::client_buffer::on_net_sync_final(const net6::packet& pack)
{
	// Sync has been completed: Emit signal
	m_signal_sync.emit();
}

void obby::client_buffer::on_net_document(const net6::packet& pack)
{
	// Read document from info
	obby::document_info* doc = pack.get_param(0).as<obby::document_info*>();
	// Forward packet
	dynamic_cast<obby::client_document_info*>(doc)->obby_data(pack);
}

void obby::client_buffer::register_signal_handlers()
{
	m_client->join_event().connect(
		sigc::mem_fun(*this, &client_buffer::on_join) );
	m_client->part_event().connect(
		sigc::mem_fun(*this, &client_buffer::on_part) );
	m_client->close_event().connect(
		sigc::mem_fun(*this, &client_buffer::on_close) );
	m_client->data_event().connect(
		sigc::mem_fun(*this, &client_buffer::on_data) );
	m_client->login_failed_event().connect(
		sigc::mem_fun(*this, &client_buffer::on_login_failed) );
	m_client->login_extend_event().connect(
		sigc::mem_fun(*this, &client_buffer::on_login_extend) );
}

obby::document_info&
obby::client_buffer::add_document_info(const user* owner,
                                       unsigned int id,
                                       const std::string& title)
{
	// Create client document info
	document_info* doc = new client_document_info(
		*this, *m_client, owner, id, title
	);
	// Push to list
	m_doclist.push_back(doc);
	return *doc;
}

