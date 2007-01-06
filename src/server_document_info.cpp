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
                                                 const user* owner,
                                                 unsigned int id,
                                                 const std::string& title)
 : document_info(buf, owner, id, title), m_server(server)
{
	// Server documents are always assigned.
	assign_document();
	// Subscribe the owner to the document
	if(owner) m_userlist.push_back(owner);
}

obby::server_document_info::server_document_info(const server_buffer& buf,
                                                 net6::server& server,
                                                 const user* owner,
                                                 unsigned int id,
                                                 const std::string& title,
                                                 bool noassign)
 : document_info(buf, owner, id, title), m_server(server)
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
	rename_impl(new_title, NULL);
}

void obby::server_document_info::subscribe_user(const user& user)
{
	// Synchronise document content to the new user
	get_document()->synchronise(user);

	// Add user to the list of subscribed users
	m_userlist.push_back(&user);

	// Emit corresponding signal
	m_signal_subscribe.emit(user);

	// Tell others about subscription
	net6::packet pack("obby_document");
	pack << static_cast<document_info*>(this) << "subscribe" << &user;
	m_server.send(pack);
}

void obby::server_document_info::unsubscribe_user(const user& user)
{
	// Remove user from the list of subscribed documents
	m_userlist.erase(
		std::remove(m_userlist.begin(), m_userlist.end(), &user),
		m_userlist.end()
	);

	// Emit corresponding signal
	m_signal_unsubscribe.emit(user);

	// Tell others about unsubscripition
	net6::packet pack("obby_document");
	pack << static_cast<document_info*>(this) << "unsubscribe" << &user;
}

void obby::server_document_info::obby_data(const net6::packet& pack, user& from)
{
	// Parameters 0 and 1 have been checked by the buffer
	if(!execute_packet(pack, from) )
	{
		std::cerr << "obby::server_document_info::obby_data: Document "
		          << "command " << pack.get_param(1).as<std::string>()
		          << " does not exist" << std::endl;
	}
}

void obby::server_document_info::assign_document()
{
	m_document = new server_document(*this, m_server);
}

void obby::server_document_info::rename_impl(const std::string& new_title,
                                             const user* user)
{
	// Send rename request
	net6::packet pack("obby_document");
	pack << static_cast<document_info*>(this) << "rename"
	     << user << new_title;
	m_server.send(pack);

	// Emit changed signal
	m_signal_rename.emit(new_title);

	// Rename document
	m_title = new_title;
}

bool obby::server_document_info::execute_packet(const net6::packet& pack,
                                                user& from)
{
	const std::string& command = pack.get_param(1).as<std::string>();
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
	// TODO: Authentication

	const std::string& new_title = pack.get_param(2).as<std::string>();
	rename_impl(new_title, &from);
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
	rec->set_user(&from);

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

