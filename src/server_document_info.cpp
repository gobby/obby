/* libobby - Network text editing library
 * Copyright (C) 2005 0x539 dev group
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public
 * License as published by the Free Software Foundation; either
 i* version 2 of the License, or (at your option) any later version.
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
#include "server_document_info.hpp"
#include "server_buffer.hpp"

obby::server_document_info::server_document_info(const server_buffer& buf,
                                                 net6::server& server,
                                                 unsigned int id,
                                                 const std::string& title)
 : document_info(buf, id, title), m_server(server)
{
	// Server documents are always assigned.
	assign_document();
}

obby::server_document_info::server_document_info(const server_buffer& buf,
                                                 net6::server& server,
                                                 unsigned int id,
                                                 const std::string& title,
                                                 bool noassign)
 : document_info(buf, id, title), m_server(server)
{
}

obby::server_document_info::~server_document_info()
{
}

const obby::server_buffer& obby::server_document_info::get_buffer() const
{
	return dynamic_cast<const obby::server_buffer&>(m_buffer);
}

obby::server_document* obby::server_document_info::get_document()
{
	return dynamic_cast<obby::server_document*>(m_document);
}

const obby::server_document* obby::server_document_info::get_document() const
{
	return dynamic_cast<const obby::server_document*>(m_document);
}

void obby::server_document_info::rename(const std::string& new_title)
{
	// Rename the document with the special ID 0 which means that the
	// server has changed the name.
	rename_impl(new_title, 0);
}

void obby::server_document_info::subscribe_user(const user& user)
{
	// Synchronise document content to the new user
	get_document()->synchronise(user);

	// Add user to the list of subscribed users
	m_userlist.push_back(&user);
	// Add document to the user's list of documents he is subscribed to
//	user.subscribe(*this);

	// Emit corresponding signal
	m_signal_subscribe.emit(user);

	// Tell others about subscription
	net6::packet pack("obby_document");
	pack << m_id << "subscribe" << user.get_id();
	m_server.send(pack);
}

void obby::server_document_info::unsubscribe_user(const user& user)
{
	// Remove user from the list of subscribed documents
	m_userlist.erase(
		std::remove(m_userlist.begin(), m_userlist.end(), &user),
		m_userlist.end()
	);
	// Remove document from the user's list of documents he is subscribed to
//	user.unsubscribe(*this);

	// Emit corresponding signal
	m_signal_unsubscribe.emit(user);

	// Tell others about unsubscripition
	net6::packet pack("obby_document");
	pack << m_id << "unsubscribe" << user.get_id();
}

void obby::server_document_info::obby_data(const net6::packet& pack, user& from)
{
	// Parameters 0 and 1 have been checked by the buffer
	if(!execute_packet(pack, from) )
	{
		std::cerr << "obby::server_document_info::obby_data: Document "
		          << "command " << pack.get_param(1).as_string() << " "
		          << "does not exist" << std::endl;
	}
}

void obby::server_document_info::assign_document()
{
	m_document = new server_document(*this, m_server);
}

void obby::server_document_info::rename_impl(const std::string& new_title,
                                             unsigned int user_id)
{
	// Send rename request
	net6::packet pack("document");
	pack << m_id << "rename" << user_id << new_title;
	m_server.send(pack);

	// Emit changed signal
	m_signal_rename.emit(new_title);

	// Rename document
	m_title = new_title;
}

bool obby::server_document_info::execute_packet(const net6::packet& pack,
                                                user& from)
{
	const std::string& command = pack.get_param(1).as_string();
	if(command == "rename")
	{
		// Rename request
		on_net_rename(pack, from);
		return true;
	}

	if(command == "record")
	{
		// Record
		on_net_record(pack, from);
		return true;
	}

	if(command == "subscribe")
	{
		// Subscribe request
		on_net_subscribe(pack, from);
		return true;
	}

	if(command == "unsubscribe")
	{
		// Unsubscribe request
		on_net_unsubscribe(pack, from);
		return true;
	}

	return false;
}

void obby::server_document_info::on_net_rename(const net6::packet& pack,
                                               user& from)
{
	if(pack.get_param_count() < 3) return;
	if(pack.get_param(2).get_type() != net6::packet::param::STRING) return;

	// TODO: Authentication

	const std::string& new_title = pack.get_param(2).as_string();
	rename_impl(new_title, from.get_id() );
}

void obby::server_document_info::on_net_record(const net6::packet& pack,
                                               user& from)
{
	record* rec = record::from_packet(pack);
	if(!rec)
	{
		std::cerr << "obby::server_document_info::on_net_record: Got "
		          << "invalid record packet from " << from.get_id()
		          << " (" << from.get_name() << ")" << std::endl;
		return;
	}

	// Set correct sender
	rec->set_from(from.get_id() );

	try
	{
		// Apply record
		get_document()->apply_record(*rec);
	}
	catch(...)
	{
		// Free record on error
		delete rec;
		throw;
	}
}

void obby::server_document_info::on_net_subscribe(const net6::packet& pack,
                                                  user& from)
{
	// Users can't subscribe twice
	if(is_subscribed(from) )
	{
		std::cerr << "obby::server_document_info::on_net_subscribe: "
		          << "User " << from.get_id() << " ("
		          << from.get_name() << ") is already subscribed to "
		          << "document " << m_id << std::endl;
	}
	else
	{
		subscribe_user(from);
	}
}

void obby::server_document_info::on_net_unsubscribe(const net6::packet& pack,
                                                    user& from)
{
	// Users can't unsubscribe if they are already subscribed
	if(!is_subscribed(from) )
	{
		std::cerr << "obby::server_document_info::on_net_unsubscribe: "
		          << "User " << from.get_id() << " ("
		          << from.get_name() << ") is not subscribed to "
		          << "document " << m_id << std::endl;
	}
	else
	{
		unsubscribe_user(from);
	}
}

