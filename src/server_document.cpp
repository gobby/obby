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

obby::server_document::server_document(unsigned int id, net6::server& server)
 : document(id), m_server(server)
{
}

obby::server_document::~server_document()
{
}

void obby::server_document::insert(position pos, const std::string& text)
{
	// TODO: Insert to buffer
	// TODO: Add to history
	// TODO: Tell clients new revision
}

void obby::server_document::erase(position from, position to)
{
	// TODO: Delete from buffer
	// TODO: Add to history
	// TODO: Tell clients new revision
}

void obby::server_document::on_net_record(record& rec)
{
	// Look for wished revision
	std::list<record*>::iterator iter;
	for(iter = m_history.begin(); iter != m_history.end(); ++ iter)
		if( (*iter)->get_revision() == rec.get_revision() )
			break;

	// Wished Revision does not exist
	if(iter == m_history.end() && rec.get_revision() != 0)
		return;

	// Apply newer Revision on the new record
	while(iter != m_history.begin() )
	{
		-- iter;

		if( (*iter)->get_from() != rec.get_from() )
			(*iter)->apply(rec);
	}

	// Ignore record if it got invalid
	if(!rec.is_valid() )
		return;

	// Apply record on buffer
	rec.apply(*this);

	// Increment revision
	rec.set_revision(++ m_revision);

	// Add change to history
	m_history.push_front(rec.clone() );

	// Tell clients
	m_server.send(rec.to_packet() );

	// Emit changed signal
	rec.emit_document_signal(*this);
}
