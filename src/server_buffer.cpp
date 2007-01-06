/* libobby - Network text editing library
 * Copyright (C) 2005 0x539 dev group
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this program; if not, write to the Free
 * Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include "server_document.hpp"
#include "server_buffer.hpp"

obby::server_buffer::server_buffer()
 : buffer(), m_doc_counter(0), m_server(NULL)
{
}

obby::server_buffer::server_buffer(unsigned int port)
 : buffer(), m_doc_counter(0), m_server(new net6::server(port, false) )
{
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

obby::document& obby::server_buffer::create_document()
{
	unsigned int id = ++ m_doc_counter;
	document& doc = add_document(id);
	
	net6::packet pack("obby_document_create");
	pack << id;
	m_server->send(pack);
}

void obby::server_buffer::remove_document(document* doc)
{
	m_doclist.erase(std::remove(m_doclist.begin(), m_doclist.end(), doc),
	                m_doclist.end() );
	
	net6::packet pack("obby_document_remove");
	pack << doc->get_id();
	m_server->send(pack);

	delete doc;

	
}

obby::server_buffer::signal_join_type obby::server_buffer::join_event() const
{
	return m_signal_join;
}

obby::server_buffer::signal_login_type obby::server_buffer::login_event() const
{
	return m_signal_login;
}

obby::server_buffer::signal_part_type obby::server_buffer::part_event() const
{
	return m_signal_part;
}

void obby::server_buffer::on_join(net6::server::peer& peer)
{
	m_signal_join.emit(peer);
}

void obby::server_buffer::on_login(net6::server::peer& peer,
                                   const net6::packet& pack)
{
	int red = pack.get_param(1).as_int();
	int green = pack.get_param(2).as_int();
	int blue = pack.get_param(3).as_int();
	user* new_user = add_user(peer, red, green, blue);

	// Client logged in. Synchronise the complete buffer, but
	// seperate it into multiple packets to not block other high-priority
	// network packets like chat packets.
	net6::packet init_sync("obby_sync_init");
	init_sync << static_cast<unsigned int>(m_doclist.size() );
	m_server->send(init_sync, peer);

	std::list<document*>::iterator iter;
	for(iter = m_doclist.begin(); iter != m_doclist.end(); ++ iter)
		static_cast<server_document*>(*iter)->synchronize(peer);

	net6::packet final_sync("obby_sync_final");
	m_server->send(final_sync, peer);

	m_signal_login.emit(*new_user);
}

void obby::server_buffer::on_part(net6::server::peer& peer)
{
	// Delete user from user list
	std::list<user*>::iterator iter;
	for(iter = m_userlist.begin(); iter != m_userlist.end(); ++ iter)
	{
		if( (*iter)->get_id() == peer.get_id() )
		{
			delete *iter;
			m_userlist.erase(iter);
			break;
		}
	}

	m_signal_part.emit(peer);
}

void obby::server_buffer::on_data(const net6::packet& pack,
                                  net6::server::peer& peer)
{
	user* from_user = find_user(peer.get_id() );
	assert(from_user != NULL);

	if(pack.get_command() == "obby_record")
		on_net_record(pack, *from_user);
}

bool obby::server_buffer::on_auth(net6::server::peer& peer,
                                  const net6::packet& pack,
				  std::string& reason)
{
	reason = "Invalid login request";
	if(pack.get_param_count() < 4) return false;
	if(pack.get_param(1).get_type() != net6::packet::param::INT)
		return false;
	if(pack.get_param(2).get_type() != net6::packet::param::INT)
		return false;
	if(pack.get_param(3).get_type() != net6::packet::param::INT)
		return false;
	reason.clear();

	int red = pack.get_param(1).as_int();
	int green = pack.get_param(2).as_int();
	int blue = pack.get_param(3).as_int();

	std::list<user*>::iterator iter;
	for(iter = m_userlist.begin(); iter != m_userlist.end(); ++ iter)
	{
		if(abs(red   - (*iter)->get_red()) < 32 &&
		   abs(green - (*iter)->get_green()) < 32 &&
		   abs(blue  - (*iter)->get_blue()) < 32)
		{
			reason = "Color is already in use";
			return false;
		}
	}

	return true;
}

void obby::server_buffer::on_extend(net6::server::peer& peer,
                                    net6::packet& pack)
{
	user* ideq_user = find_user(peer.get_id() );
	if(!ideq_user) return;

	pack << ideq_user->get_red() << ideq_user->get_green()
	     << ideq_user->get_blue();
}

void obby::server_buffer::register_signal_handlers()
{
	m_server->join_event().connect(
		sigc::mem_fun(*this, &server_buffer::on_join) );
	m_server->login_event().connect(
		sigc::mem_fun(*this, &server_buffer::on_login) );
	m_server->login_auth_event().connect(
		sigc::mem_fun(*this, &server_buffer::on_auth) );
	m_server->login_extend_event().connect(
		sigc::mem_fun(*this, &server_buffer::on_extend) );
	m_server->part_event().connect(
		sigc::mem_fun(*this, &server_buffer::on_part) );
	m_server->data_event().connect(
		sigc::mem_fun(*this, &server_buffer::on_data) );
}

obby::document& obby::server_buffer::add_document(unsigned int id)
{
	document* doc = new server_document(id, *m_server);
	m_doclist.push_back(doc);
	return *doc;
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
	document& doc = create_document();
	m_signal_insert_document.emit(doc);
}

void obby::server_buffer::on_net_document_remove(const net6::packet& pack,
                                                 user& from)
{
	if(pack.get_param_count() < 1) return;
	if(pack.get_param(0).get_type() != net6::packet::param::INT) return;

	unsigned int id = pack.get_param(0).as_int();
	document* doc = find_document(id);
	if(!doc) return;

	m_signal_remove_document.emit(*doc);
	remove_document(doc);
}

