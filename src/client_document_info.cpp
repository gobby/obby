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

#include "client_document_info.hpp"

#if 0
obby::client_document_info::client_document_info(const basic_client_buffer<net6::selector>& buf,
                                                 net6::client& client,
                                                 const user* owner,
                                                 unsigned int id,
                                                 const std::string& title)
 : document_info(buf, owner, id, title),
   local_document_info(buf, owner, id, title),
   m_client(client)
{
}

obby::client_document_info::~client_document_info()
{
}

const obby::basic_client_buffer<net6::selector>& obby::client_document_info::get_buffer() const
{
	return dynamic_cast<const basic_client_buffer<net6::selector>&>(m_buffer);
}

obby::client_document* obby::client_document_info::get_document()
{
	return dynamic_cast<obby::client_document*>(m_document);
}

const obby::client_document* obby::client_document_info::get_document() const
{
	return dynamic_cast<const obby::client_document*>(m_document);
}

void obby::client_document_info::rename(const std::string& new_title)
{
	net6::packet pack("obby_document");
	pack << m_id << "rename" << new_title;
	m_client.send(pack);
}

void obby::client_document_info::subscribe()
{
	if(m_document)
	{
		std::cerr << "obby::client_document_info::subscribe: "
		          << "Already subscribed to document " <<  m_id
		          << std::endl;
	}
	else
	{
		// Send subscribe request
		net6::packet pack("obby_document");
		pack << static_cast<document_info*>(this) << "subscribe";
		m_client.send(pack);
	}
}

void obby::client_document_info::unsubscribe()
{
	if(m_document)
	{
		// Send unsubscribe request
		net6::packet pack("obby_document");
		pack << static_cast<document_info*>(this) << "unsubscribe";
		m_client.send(pack);
	}
	else
	{
		std::cerr << "obby::client_document_info::unsubscribe: "
		          << "Not subscribed to document " << m_id
		          << std::endl;
	}
}

void obby::client_document_info::obby_data(const net6::packet& pack)
{
	// Parameters 0 and 1 have been checked by the buffer
	if(!execute_packet(pack) )
	{
		std::cerr << "obby::client_document_info::obby_data: Document "
		          << "command " << pack.get_param(1).as<std::string>()
		          << " does not exist" << std::endl;
	}
}

void obby::client_document_info::obby_sync_init()
{
	// The subscription list gets synchronised now, so m_userlist has
	// to be cleared to hold exactly the synchonised users and no others.
	m_userlist.clear();
}

void obby::client_document_info::obby_sync_subscribe(const user& user)
{
	// The signal does not have to be emitted because this function is
	// called during the initial synchronisation process, where no one
	// knows that the document exists and therefore no one has the
	// possibility to attach a signal handler.
	m_userlist.push_back(&user);
}

void
obby::client_document_info::obby_local_init(const std::string& initial_content,
                                            bool open_as_edited)
{
	// Assign a document to the function
	assign_document();
	// Create initial content
	insert_record rec(
		0, initial_content, *m_document,
		(open_as_edited) ? &get_buffer().get_self() : NULL,
		0, 0
	);
	get_document()->insert_nosync(rec);
}

bool obby::client_document_info::execute_packet(const net6::packet& pack)
{
	// TODO: std::map<> from command to function
	const std::string& command = pack.get_param(1).as<std::string>();

	if(command == "rename")
	{
		// Rename request
		on_net_rename(pack);
		return true;
	}

	if(command == "record")
	{
		// Record
		on_net_record(pack);
		return true;
	}

	if(command == "sync_init")
	{
		// Synchronisation initialisation
		on_net_sync_init(pack);
		return true;
	}

	if(command == "sync_line")
	{
		// Synchronisation of a line
		on_net_sync_line(pack);
		return true;
	}

	if(command == "subscribe")
	{
		// User subscription
		on_net_subscribe(pack);
		return true;
	}

	if(command == "unsubscribe")
	{
		// User unsubscription
		on_net_unsubscribe(pack);
		return true;
	}

	return false;
}

void obby::client_document_info::on_net_rename(const net6::packet& pack)
{
	unsigned int user_id = pack.get_param(2).as<int>();
	const std::string& new_title = pack.get_param(3).as<std::string>();

	// Emit signal before setting new title to allow the signal handler
	// to access the previous title.
	m_signal_rename.emit(new_title);
	m_title = new_title;
}

void obby::client_document_info::on_net_record(const net6::packet& pack)
{
	// We have to be subscribed to receive changes to the document.
	if(!m_document)
	{
		std::cerr << "obby::client_document_info::on_net_record: "
		          << "Got record packet without being subscribed "
		          << "to document " << m_id << std::endl;
		return;
	}

	// Convert packet to a record
	record* rec = record::from_packet(pack);
	if(!rec)
	{
		std::cerr << "obby::client_document_info::on_net_record: "
		          << "Got invalid record packet" << std::endl;
		return;
	}

	try
	{
		// Apply record
		get_document()->apply_record(*rec);
	}
	catch(...)
	{
		delete rec;
		throw;
	}
}

void obby::client_document_info::on_net_sync_init(const net6::packet& pack)
{
	// Already subscribed? No synchronisation needed.
	if(m_document)
	{
		std::cerr << "obby::client_document_info::on_net_sync_init: "
		          << "Got sync_init packet with being subscribed "
		          << "to document " << m_id << std::endl;
		return;
	}

	// Extract revision
	unsigned int revision = pack.get_param(2).as<int>();

	// Create document and initialise it
	assign_document();
	get_document()->initialise(revision);
}

void obby::client_document_info::on_net_sync_line(const net6::packet& pack)
{
	// No document assigned?
	if(!m_document)
	{
		std::cerr << "obby::client_document_info::on_net_sync_line: "
		          << "Got sync_line packet without having received "
		          << "sync_init before in document " << m_id
		          << std::endl;
		return;
	}

	line new_line(pack);
	get_document()->add_line(new_line);
}

void obby::client_document_info::on_net_subscribe(const net6::packet& pack)
{
	// Find user which subscribed
	user* new_user = pack.get_param(2).as<user*>();

	// Verify that the user is not already subscribed
	if(is_subscribed(*new_user) )
	{
		std::cerr << "obby::client_document_info::on_net_subscribe: "
		          << "User " << new_user->get_id() << " ("
		          << new_user->get_name() << ") is already subscribed"
		          << std::endl;
		return;
	}

	// Put it into the list of subscribed users
	m_userlist.push_back(new_user);
	// Emit signal
	m_signal_subscribe.emit(*new_user);
}

void obby::client_document_info::on_net_unsubscribe(const net6::packet& pack)
{
	// Find user which unsubscribed
	user* old_user = pack.get_param(2).as<obby::user*>();

	// Verify that the user is not already subscribed
	if(!is_subscribed(*old_user) )
	{
		std::cerr << "obby::client_document_info::on_net_unsubscribe: "
		          << "User " << old_user->get_name() << " ("
		          << old_user->get_name() << ") is not subscribed"
		          << std::endl;
		return;
	}

	// Remove it from the list of subscribed users
	m_userlist.erase(
		std::remove(m_userlist.begin(), m_userlist.end(), old_user),
		m_userlist.end()
	);

	// Emit signal
	m_signal_unsubscribe.emit(*old_user);
}

void obby::client_document_info::assign_document()
{
	m_document = new client_document(*this, m_client);
}
#endif
