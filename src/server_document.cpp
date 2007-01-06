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

#include "server_document.hpp"
#include "server_buffer.hpp"

obby::server_document::server_document(unsigned int id, net6::server& server,
                                       const server_buffer& buf)
 : document(id, buf), m_server(server)
{
}

obby::server_document::~server_document()
{
}

const obby::server_buffer& obby::server_document::get_buffer() const
{
	// static_cast does not work with virtual inheritance
	return dynamic_cast<const obby::server_buffer&>(m_buffer);
}

void obby::server_document::insert(position pos, const std::string& text)
{
	insert(pos, text, 0);
}

void obby::server_document::erase(position begin, position end)
{
	erase(begin, end, 0);
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
}

void obby::server_document::synchronise(net6::server::peer& peer)
{
	// Send doc initial sync packet with document revision
	net6::packet init_pack("obby_sync_doc_init");
	init_pack << m_id << m_title << m_revision;
	m_server.send(init_pack, peer);

	// Send buffer
	std::vector<line>::const_iterator iter;
	for(iter = m_lines.begin(); iter != m_lines.end(); ++ iter)
		m_server.send(iter->to_packet(m_id), peer);

	// Send final sync packet
	net6::packet final_pack("obby_sync_doc_final");
	final_pack << m_id;
	m_server.send(final_pack, peer);
}

void obby::server_document::insert(position pos, const std::string& text,
                                   unsigned int author_id)
{
	// Build record
	record* rec =
		new insert_record(pos, text, m_id, ++ m_revision, author_id);
	// Apply on document
	rec->apply(*this);
	// Insert into history
	m_history.push_front(rec);
	// Synchronize to clients
	m_server.send(rec->to_packet() );
}

void obby::server_document::erase(position from, position to,
                                  unsigned int author_id)
{
	// Get erased text
	std::string erased = get_sub_buffer(from, to);
	// Create record
	record* rec =
		new delete_record(from, erased, m_id, ++ m_revision, author_id);
	// Apply on document
	rec->apply(*this);
	// Insert into history
	m_history.push_front(rec);
	// Synchronize to clients
	m_server.send(rec->to_packet() );
}

