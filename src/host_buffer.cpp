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
#include "host_user_table.hpp"
#include "host_document.hpp"
#include "host_buffer.hpp"

obby::host_buffer::host_buffer()
 : server_buffer(), m_self(NULL)
{
}

obby::host_buffer::host_buffer(unsigned int port, const std::string& username,
                               int red, int green, int blue)
 : server_buffer(), m_self(NULL)
{
	net6::host* host = new net6::host(port, username, false);
	m_server = host;

	m_usertable = new host_user_table(*host, *this);
	
	assert(host->get_self() );
	m_self = add_user(*host->get_self(), red, green, blue);
	/*
	m_self = new user(*host->get_self(), red, green, blue);
	m_userlist.push_back(m_self);
	m_usertable->insert(*m_self);
	*/
	register_signal_handlers();
}

obby::host_buffer::~host_buffer()
{
	if(m_server)
	{
		delete m_server;
		m_server = NULL;
	}

	if(m_usertable)
	{
		delete m_usertable;
		m_usertable = NULL;
	}
}

obby::user& obby::host_buffer::get_self()
{
	return *m_self;
}

const obby::user& obby::host_buffer::get_self() const
{
	return *m_self;
}

void obby::host_buffer::send_message(const std::string& message)
{
	m_signal_message.emit(*m_self, message);
	relay_message(m_self->get_id(), message);
}

obby::document& obby::host_buffer::add_document(unsigned int id)
{
	host_user_table* table = static_cast<host_user_table*>(m_usertable);
	net6::host* host = static_cast<net6::host*>(m_server);

	document* doc = new host_document(id, *host, *table);

	m_doclist.push_back(doc);
	return *doc;
}

void obby::host_buffer::create_document(const std::string& title,
                                        const std::string& content)
{
	server_buffer::create_document(title, content, m_self->get_id() );
}

