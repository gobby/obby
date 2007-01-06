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

#include "client_document.hpp"
#include "client_document_info.hpp"
#include "client_buffer.hpp"

obby::client_document::client_document(const client_document_info& info,
                                       net6::client& client)
 : document(info), local_document(info), m_client(client)
{
}

obby::client_document::~client_document()
{
	std::list<record*>::iterator iter;
	for(iter = m_unsynced.begin(); iter != m_unsynced.end(); ++ iter)
		delete *iter;
}

const obby::client_document_info& obby::client_document::get_info() const
{
	return dynamic_cast<const client_document_info&>(m_info);
}

const obby::client_buffer& obby::client_document::get_buffer() const
{
	return dynamic_cast<const client_buffer&>(m_info.get_buffer() );
}

void obby::client_document::insert(position pos, const std::string& text)
{
	// Add to unsynced changes
	record* rec = new insert_record(pos, text, get_id(), m_revision,
	                                m_client.get_self()->get_id() );
	m_unsynced.push_back(rec);
	// Apply on document
	rec->apply(*this);
	// Send sync request to server
	m_client.send(rec->to_packet() );
}

void obby::client_document::erase(position from, position to)
{
	std::string text = get_slice(from, to);
	// Add to unsynced changes
	record* rec = new delete_record(from, text, get_id(), m_revision,
	                                m_client.get_self()->get_id() );
	m_unsynced.push_back(rec);
	// Delete from document
	rec->apply(*this);
	// Send sync request to server
	m_client.send(rec->to_packet() );
}

std::list<obby::record*>::size_type
obby::client_document::unsynced_count() const
{
	// Just return size of list which is the amount of unsynced changes
	return m_unsynced.size();
}

void obby::client_document::apply_record(const record& rec)
{
	// We are about to change anything
	m_signal_change.before().emit();

	// Apply the record to the history
	m_history.push_front(rec.clone() );

	// Look for a unsynced change we are syncing
	std::list<record*>::iterator iter;

	for(iter = m_unsynced.end(); iter-- != m_unsynced.begin(); )
	{
		record* rev = (*iter)->reverse();
		rev->apply(*this);
		delete rev;
	}

	// Insert rec.
	rec.apply(*this);

	for(iter = m_unsynced.begin(); iter != m_unsynced.end(); )
	{
		if( (*iter)->get_from() == rec.get_from() &&
		    (*iter)->get_id() == rec.get_id() )
		{
			delete *iter;
			iter = m_unsynced.erase(iter);
		}
		else
		{
			if((*iter)->get_from() != rec.get_from() )
				rec.apply(**iter);

			if(!(*iter)->is_valid() )
			{
				delete *iter;
				iter = m_unsynced.erase(iter);
			}
			else
			{
				(*iter)->apply(*this);
				++ iter;
			}
		}
	}

	// Update revision
	m_revision = rec.get_revision();

	// Change has been done
	m_signal_change.after().emit();
}

void obby::client_document::initialise(unsigned int revision)
{
	m_revision = revision;
	m_lines.clear();
}

void obby::client_document::add_line(const line& line)
{
	m_lines.push_back(line);
}

