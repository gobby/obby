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

#include "jupiter_server.hpp"

obby::jupiter_server::jupiter_server(document& doc)
 : m_document(doc)
{
}

obby::jupiter_server::~jupiter_server()
{
	for(client_map::iterator iter = m_clients.begin();
	    iter != m_clients.end();
	    ++ iter)
	{
		delete iter->second;
	}
}

void obby::jupiter_server::client_add(const user& client)
{
	if(m_clients.find(&client) != m_clients.end() )
		throw std::logic_error("obby::jupiter_server::client_add");

	m_clients[&client] = new jupiter_algorithm;
}

void obby::jupiter_server::client_remove(const user& client)
{
	client_map::iterator iter = m_clients.find(&client);
	if(iter == m_clients.end() )
		throw std::logic_error("obby::jupiter_server::client_remove");

	delete iter->second;
	m_clients.erase(iter);
}

void obby::jupiter_server::local_op(const operation& op, const user* from)
{
	// Apply locally
	op.apply(m_document, from);
	// Delegate to clients
	for(client_map::iterator iter = m_clients.begin();
	    iter != m_clients.end();
	    ++ iter)
	{
		// Get resulting record
		std::auto_ptr<record> rec = iter->second->local_op(op);
		// Emit corresponding signal
		m_signal_local.emit(*rec, *iter->first, from);
	}
}

void obby::jupiter_server::remote_op(const record& rec, const user* from)
{
	// Find client from which the record comes from
	client_map::iterator iter = m_clients.find(from);
	if(iter == m_clients.end() )
		throw std::logic_error("obby::jupiter_server::remote_op");

	// Transform operation
	std::auto_ptr<operation> op = iter->second->remote_op(rec);
	// Apply to local document
	op->apply(m_document, from);

	// Delegate to other clients
	for(iter = m_clients.begin(); iter != m_clients.end(); ++ iter)
	{
		// Not the client who sent the record
		if(iter->first != from)
		{
			// Generate record for this client
			std::auto_ptr<record> rec = iter->second->local_op(*op);
			// Emit remote signal
			m_signal_remote.emit(*rec, *iter->first, from);
		}
	}
}

obby::jupiter_server::signal_local_type
obby::jupiter_server::local_event() const
{
	return m_signal_local;
}

obby::jupiter_server::signal_remote_type
obby::jupiter_server::remote_event() const
{
	return m_signal_remote;
}

