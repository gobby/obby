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
	// Send create document request.
	net6::packet request_pack("obby_document_create");
	request_pack << title << content;
	m_client->send(request_pack);
}

void obby::client_buffer::rename_document(obby::document& document,
                                          const std::string& new_title)
{
	// Send rename document request
	net6::packet request_pack("obby_document_rename");
	request_pack << document.get_id() << new_title;
	m_client->send(request_pack);
}

void obby::client_buffer::remove_document(obby::document& document)
{
	// Send remove document request
	net6::packet request_pack("obby_document_remove");
	request_pack << document.get_id();
	m_client->send(request_pack);
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
	if(pack.get_param_count() < 5) return;
	if(pack.get_param(2).get_type() != net6::packet::param::INT) return;
	if(pack.get_param(3).get_type() != net6::packet::param::INT) return;
	if(pack.get_param(4).get_type() != net6::packet::param::INT) return;

	int red = pack.get_param(2).as_int();
	int green = pack.get_param(3).as_int();
	int blue = pack.get_param(4).as_int();

	user* new_user = m_usertable.add_user(peer, red, green, blue);

	// The first joining user is the local one
	if(!m_self) m_self = new_user;
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
	// TODO: std::map<> from command to function?
	if(pack.get_command() == "obby_record")
		on_net_record(pack);

	if(pack.get_command() == "obby_document_create")
		on_net_document_create(pack);
	if(pack.get_command() == "obby_document_rename")
		on_net_document_rename(pack);
	if(pack.get_command() == "obby_document_remove")
		on_net_document_remove(pack);

	if(pack.get_command() == "obby_message")
		on_net_message(pack);

	if(pack.get_command() == "obby_sync_init")
		on_net_sync_init(pack);
	if(pack.get_command() == "obby_sync_usertable_user")
		on_net_sync_usertable_user(pack);
	if(pack.get_command() == "obby_sync_doc_init")
		on_net_sync_doc_init(pack);
	if(pack.get_command() == "obby_sync_doc_line")
		on_net_sync_doc_line(pack);
	if(pack.get_command() == "obby_sync_doc_final")
		on_net_sync_doc_final(pack);
	if(pack.get_command() == "obby_sync_final")
		on_net_sync_final(pack);
}

void obby::client_buffer::on_login_failed(net6::login::error error)
{
	m_signal_login_failed.emit(error);
}

void obby::client_buffer::on_login_extend(net6::packet& pack)
{
	pack << st_red << st_green << st_blue;
}

void obby::client_buffer::on_net_record(const net6::packet& pack)
{
	// Create record from packet
	record* rec = record::from_packet(pack);
	if(!rec)
	{
		// Record could not be created
		std::cerr << "Got invalid record" << std::endl;
		return;
	}

	// Find suitable document
	document* doc = find_document(rec->get_document() );
	if(!doc)
	{
		// Document not found
		std::cerr << "Record " << rec->get_id() << ": "
		          << "Document " << rec->get_document() << " "
		          << "does not exist" << std::endl;
		delete rec;
		return;
	}

	try
	{
		doc->on_net_record(*rec);
	}
	catch(...)
	{
		delete rec;
		throw;
	}
}

void obby::client_buffer::on_net_document_create(const net6::packet& pack)
{
	// Check for a valid packet
	if(pack.get_param_count() < 4) return;
	if(pack.get_param(0).get_type() != net6::packet::param::INT) return;
	if(pack.get_param(1).get_type() != net6::packet::param::STRING) return;
	if(pack.get_param(2).get_type() != net6::packet::param::INT) return;
	if(pack.get_param(3).get_type() != net6::packet::param::STRING) return;

	// Get id and document
	unsigned int id = pack.get_param(0).as_int();
	const std::string& title = pack.get_param(1).as_string();

	if(find_document(id) )
	{
		std::cerr << "Got invalid create document request: "
		          << "Document " << id << " exists already"
	                  << std::endl;
		return;
	}

	// Create new document and initialize title
	document& new_doc = add_document(id);
	new_doc.set_title(title);

	// Get author id and initial content
	unsigned int author_id = pack.get_param(2).as_int();
	const std::string& content = pack.get_param(3).as_string();

	// Build initial insert record and insert text
	insert_record rec(0, content, id, 0, author_id, 0);
	new_doc.insert_nosync(rec);

	// Document inserted successfully
	m_signal_insert_document.emit(new_doc);
}

void obby::client_buffer::on_net_document_rename(const net6::packet& pack)
{
	// Check for valid packet
	if(pack.get_param_count() < 2) return;
	if(pack.get_param(0).get_type() != net6::packet::param::INT) return;
	if(pack.get_param(1).get_type() != net6::packet::param::STRING) return;

	// Extract id and new title
	unsigned int id = pack.get_param(0).as_int();
	const std::string& name = pack.get_param(1).as_string();

	// Search document
	document* doc = find_document(id);
	if(doc == NULL)
	{
		std::cerr << "Got invalid document rename request: "
		          << "Document " << id << " does not exist"
		          << std::endl;
		return;
	}

	// Rename document and emit signal
	doc->set_title(name);
	m_signal_rename_document.emit(*doc, name);
}

void obby::client_buffer::on_net_document_remove(const net6::packet& pack)
{
	// Check for valid packet
	if(pack.get_param_count() < 1) return;
	if(pack.get_param(0).get_type() != net6::packet::param::INT) return;

	// Extract ID
	unsigned int id = pack.get_param(0).as_int();
	
	// Look for the document to remove
	std::list<document*>::iterator iter;
	for(iter = m_doclist.begin(); iter != m_doclist.end(); ++ iter)
	{
		// Correct ID?
		if( (*iter)->get_id() == id)
		{
			// Emit signal
			m_signal_remove_document.emit(**iter);

			// Delete document
			delete *iter;
			m_doclist.erase(iter);
			return;
		}
	}

	// Document could not be deleted: Invalid request.
	std::cerr << "Got invalid remove document request: "
	          << "Document " << id << " does not exist";
}

void obby::client_buffer::on_net_message(const net6::packet& pack)
{
	if(pack.get_param_count() < 2) return;
	if(pack.get_param(0).get_type() != net6::packet::param::INT) return;
	if(pack.get_param(1).get_type() != net6::packet::param::STRING) return;

	unsigned int uid = pack.get_param(0).as_int();
	const std::string& message = pack.get_param(1).as_string();

	// Valid user id => Message comes from a user
	if(uid != 0)
	{
		// Find the writer of this message
		user* writer = m_usertable.find_user<user::CONNECTED>(uid);

		// Did not found a connected writer?
		if(!writer)
		{
			// Drop warning
			std::cerr << "Got invalid message: "
			          << "Sender " << uid << " is invalid"
			          << std::endl;
			return;
		}

		// Emit message signal
		m_signal_message.emit(*writer, message);
	}
	else
	{
		// Got server message
		m_signal_server_message.emit(message);
	}
}

void obby::client_buffer::on_net_sync_init(const net6::packet& pack)
{
	// noop
}

void obby::client_buffer::on_net_sync_usertable_user(const net6::packet& pack)
{
	// User that was already in the obby session, but isn't anymore. The
	// server tells us ID, name, red, green and blue values.
	if(pack.get_param_count() < 5) return;
	if(pack.get_param(0).get_type() != net6::packet::param::INT) return;
	if(pack.get_param(1).get_type() != net6::packet::param::STRING) return;
	if(pack.get_param(2).get_type() != net6::packet::param::INT) return;
	if(pack.get_param(3).get_type() != net6::packet::param::INT) return;
	if(pack.get_param(4).get_type() != net6::packet::param::INT) return;

	// Extract data from packet
	unsigned int id = pack.get_param(0).as_int();
	const std::string& name = pack.get_param(1).as_string();
	int red = pack.get_param(2).as_int();
	int green = pack.get_param(3).as_int();
	int blue = pack.get_param(4).as_int();

	// Add user into user table
	m_usertable.add_user(id, name, red, green, blue);
}

void obby::client_buffer::on_net_sync_doc_init(const net6::packet& pack)
{
	// Check packet validness
	if(pack.get_param_count() < 1) return;
	if(pack.get_param(0).get_type() != net6::packet::param::INT) return;

	// Extract id
	unsigned int id = pack.get_param(0).as_int();

	// Check for duplicates
	if(find_document(id) )
	{
		std::cerr << "Got invalid create document request: "
		          << "Document " << id << " exists already"
		          << std::endl;
		return;
	}

	// Add new document to sync
	client_document& doc = static_cast<client_document&>(add_document(id) );

	// Forward packet
	doc.on_net_sync_init(pack);
}

void obby::client_buffer::on_net_sync_doc_line(const net6::packet& pack)
{
	// Check packet validness
	if(pack.get_param_count() < 1) return;
	if(pack.get_param(0).get_type() != net6::packet::param::INT) return;

	// Extract id
	unsigned int id = pack.get_param(0).as_int();

	// Find document
	client_document* doc = static_cast<client_document*>(find_document(id));

	// Forward message
	if(doc)
		doc->on_net_sync_line(pack);
	else
		std::cerr << "Got invalid sync line request: "
		          << "Document " << id << " does not exist"
		          << std::endl;
}

void obby::client_buffer::on_net_sync_doc_final(const net6::packet& pack)
{
	// Check packet validness
	if(pack.get_param_count() < 1) return;
	if(pack.get_param(0).get_type() != net6::packet::param::INT) return;

	// Extract id
	unsigned int id = pack.get_param(0).as_int();

	// Find document
	client_document* doc = static_cast<client_document*>(find_document(id));

	// Forward message
	if(doc)
		doc->on_net_sync_final(pack);
	else
		std::cerr << "Got invalid sync final request: "
		          << "Document " << id << " does not exist"
		          << std::endl;
}

void obby::client_buffer::on_net_sync_final(const net6::packet& pack)
{
	// Sync has been completed: Emit signal
	m_signal_sync.emit();
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

obby::document& obby::client_buffer::add_document(unsigned int id)
{
	document* doc = new client_document(id, *m_client, *this);
	m_doclist.push_back(doc);
	return *doc;
}

