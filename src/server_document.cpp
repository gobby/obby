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
#include "server_document_info.hpp"
#include "server_buffer.hpp"

obby::server_document::server_document(const server_document_info& info,
                                       net6::server& server)
 : document(info), m_server(server)
{
}

obby::server_document::~server_document()
{
}

const obby::server_document_info& obby::server_document::get_info() const
{
	return dynamic_cast<const server_document_info&>(m_info);
}

const obby::server_buffer& obby::server_document::get_buffer() const
{
	return dynamic_cast<const server_buffer&>(m_info.get_buffer() );
}

void obby::server_document::insert(position pos, const std::string& text)
{
	insert_impl(pos, text, NULL);
}

void obby::server_document::erase(position begin, position end)
{
	erase_impl(begin, end, NULL);
}

void obby::server_document::apply_record(const record& rec_)
{
	// TODO: Use the same algorithm as in the client (undo changes in the
	// document instead of moving the record around), then this evil hack
	// may be removed, too. Maybe, a mysterious bug will also be fixed
	// with this, which strangly merges texts if two users begin to
	// type at the same position.
	//record& rec = const_cast<record&>(rec_);
	record& rec = *rec_.clone();

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

		if( (*iter)->get_user() != rec.get_user() )
			(*iter)->apply(rec);
	}

	// Ignore record if it got invalid
	if(!rec.is_valid() )
		return;

	// Emit change signal before changing anything
	m_signal_change.before().emit();

	// Apply record on buffer
	rec.apply();

	// Increment revision
	rec.set_revision(++ m_revision);

	// Add change to history
	m_history.push_front(&rec); //.clone() );

	// Changes have been performed
	m_signal_change.after().emit();

	// Tell clients
	forward_record(rec);
}

void obby::server_document::insert_impl(position pos, const std::string& text,
                                        const user* author)
{
	// Emit change signal before changing anything
	m_signal_change.before().emit();
	// Build record
	record* rec = new insert_record(
		pos, text, *this, author, ++ m_revision
	);
	// Apply on document
	rec->apply();
	// Insert into history
	m_history.push_front(rec);
	// Changes have been performed
	m_signal_change.after().emit();
	// Synchronize to clients
	forward_record(*rec);
}

void obby::server_document::erase_impl(position from, position to,
                                       const user* author)
{
	// Emit change signal before changing anything
	m_signal_change.before().emit();
	// Get erased text
	std::string erased = get_slice(from, to);
	// Create record
	record* rec = new delete_record(
		from, erased, *this, author, ++ m_revision
	);
	// Apply on document
	rec->apply();
	// Insert into history
	m_history.push_front(rec);
	// Changes have been performed
	m_signal_change.after().emit();
	// Synchronize to clients
	forward_record(*rec);
}

void obby::server_document::synchronise(const user& to)
{
	net6::server::peer& peer =
		*static_cast<net6::server::peer*>(to.get_peer() );

	// Send doc initial sync packet with document revision
	net6::packet init_pack("obby_document");
	init_pack << m_info << "sync_init" << m_revision;
	m_server.send(init_pack, peer);

	// Send buffer
	std::vector<line>::const_iterator iter;
	for(iter = m_lines.begin(); iter != m_lines.end(); ++ iter)
		m_server.send(iter->to_packet(*this), peer);
}

void obby::server_document::forward_record(const record& rec) const
{
	// Build packet from record
	net6::packet pack = rec.to_packet();
	// Get info
	const server_document_info& info = get_info();

	for(server_document_info::user_iterator iter = info.user_begin();
	    iter != info.user_end();
	    ++ iter)
	{
		// Send the packet to each subscribed user
		m_server.send(
			pack,
			*static_cast<net6::server::peer*>(iter->get_peer())
		);
	}
}

