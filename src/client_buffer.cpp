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

#include <cassert>
#include "client_document.hpp"
#include "client_buffer.hpp"

obby::client_buffer::client_buffer()
 : buffer(), m_unsynced(), m_client(NULL), m_self(NULL)
{
}

obby::client_buffer::client_buffer(const std::string& hostname,
                                   unsigned int port)
 : buffer(), m_unsynced(), m_client(NULL), m_self(NULL)
{
	net6::ipv4_address addr(
		net6::ipv4_address::create_from_hostname(hostname, port)
	);

	m_client = new net6::client(addr);
	register_signal_handlers();
}

obby::client_buffer::~client_buffer()
{
	if(m_client)
	{
		delete m_client;
		m_client = NULL;
	}
}

void obby::client_buffer::login(const std::string& name, int red, int green,
                                int blue)
{
	net6::packet login_pack("net6_client_login");
	login_pack << name << red << green << blue;
	m_client->custom_login(login_pack);
}

void obby::client_buffer::request_create_document()
{
	net6::packet request_pack("obby_document_create");
	m_client->send(request_pack);
}

void obby::client_buffer::request_remove_document(unsigned int id)
{
	net6::packet request_pack("obby_document_remove");
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

	user* new_user = add_user(peer, red, green, blue);
	if(!m_self) m_self = new_user;
	m_signal_user_join.emit(*new_user);
}

void obby::client_buffer::on_part(net6::client::peer& peer)
{
	// Find user in user list and store iterator to it
	std::list<user*>::iterator iter;
	for(iter = m_userlist.begin(); iter != m_userlist.end(); ++ iter)
		if( (*iter)->get_id() == peer.get_id() )
			break;

	// Emit part singal
	assert(iter != m_userlist.end() );
	m_signal_user_part.emit(**iter);

	// Remove user from list
	delete *iter;
	m_userlist.erase(iter);
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
	if(pack.get_command() == "obby_document_remove")
		on_net_document_remove(pack);

	if(pack.get_command() == "obby_sync_init")
		on_net_sync_init(pack);
	if(pack.get_command() == "obby_sync_doc_init")
		on_net_sync_doc_init(pack);
	if(pack.get_command() == "obby_sync_doc_line")
		on_net_sync_doc_line(pack);
	if(pack.get_command() == "obby_sync_doc_final")
		on_net_sync_doc_final(pack);
	if(pack.get_command() == "obby_sync_final")
		on_net_sync_final(pack);
}

void obby::client_buffer::on_login_failed(const std::string& reason)
{
	m_signal_login_failed.emit(reason);
}

void obby::client_buffer::on_net_record(const net6::packet& pack)
{
	// Create record from packet
	record* rec = record::from_packet(pack);
	if(!rec) return;

	// TODO: Find suitable document
	document* doc = find_document(rec->get_document() );
	if(!doc) { delete rec; return; }

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
	if(pack.get_param_count() < 1) return;
	if(pack.get_param(0).get_type() != net6::packet::param::INT) return;

	unsigned int id = pack.get_param(0).as_int();
	assert(find_document(id) == NULL);

	document& new_doc = add_document(id);
	m_signal_insert_document.emit(new_doc);
}

void obby::client_buffer::on_net_document_remove(const net6::packet& pack)
{
	if(pack.get_param_count() < 1) return;
	if(pack.get_param(0).get_type() != net6::packet::param::INT) return;

	unsigned int id = pack.get_param(0).as_int();
	
	std::list<document*>::iterator iter;
	for(iter = m_doclist.begin(); iter != m_doclist.end(); ++ iter)
	{
		if( (*iter)->get_id() == id)
		{
			m_signal_remove_document.emit(**iter);

			delete *iter;
			m_doclist.erase(iter);
			break;
		}
	}
}

void obby::client_buffer::on_net_sync_init(const net6::packet& pack)
{
	// t0l.
}

void obby::client_buffer::on_net_sync_doc_init(const net6::packet& pack)
{
	if(pack.get_param_count() < 1) return;
	if(pack.get_param(0).get_type() != net6::packet::param::INT) return;

	unsigned int id = pack.get_param(0).as_int();

	client_document& doc = static_cast<client_document&>(add_document(id) );
	doc.on_net_sync_init(pack);
}

void obby::client_buffer::on_net_sync_doc_line(const net6::packet& pack)
{
	if(pack.get_param_count() < 1) return;
	if(pack.get_param(0).get_type() != net6::packet::param::INT) return;

	unsigned int id = pack.get_param(0).as_int();

	client_document* doc = static_cast<client_document*>(find_document(id));
	if(doc) doc->on_net_sync_line(pack);
}

void obby::client_buffer::on_net_sync_doc_final(const net6::packet& pack)
{
	if(pack.get_param_count() < 1) return;
	if(pack.get_param(0).get_type() != net6::packet::param::INT) return;

	unsigned int id = pack.get_param(0).as_int();

	client_document* doc = static_cast<client_document*>(find_document(id));
	if(doc) doc->on_net_sync_final(pack);
}

void obby::client_buffer::on_net_sync_final(const net6::packet& pack)
{
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
}

obby::document& obby::client_buffer::add_document(unsigned int id)
{
	document* doc = new client_document(id, *m_client);
	m_doclist.push_back(doc);
	return *doc;
}

