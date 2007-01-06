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
#include "client_document.hpp"
#include "client_buffer.hpp"

obby::client_document::client_document(unsigned int id, net6::client& client,
                                       const client_buffer& buf)
 : document(id, buf), m_unsynced(), m_client(client)
{
}

obby::client_document::~client_document()
{
	std::list<record*>::iterator iter;
	for(iter = m_unsynced.begin(); iter != m_unsynced.end(); ++ iter)
		delete *iter;
}

const obby::client_buffer& obby::client_document::get_buffer() const
{
	// static_cast does not work with virtual inheritance
	return dynamic_cast<const client_buffer&>(m_buffer);
}

void obby::client_document::insert(position pos, const std::string& text)
{
	// Add to unsynced changes
	record* rec = new insert_record(pos, text, m_id, m_revision,
	                                m_client.get_self()->get_id() );
	m_unsynced.push_back(rec);
	// Apply on document
	rec->apply(*this);
	// Send sync request to server
	m_client.send(rec->to_packet() );
}

void obby::client_document::erase(position from, position to)
{
	std::string text = get_sub_buffer(from, to);
	// Add to unsynced changes
	record* rec = new delete_record(from, text, m_id, m_revision,
	                                m_client.get_self()->get_id() );
	m_unsynced.push_back(rec);
	// Delete from document
	rec->apply(*this);
	// Send sync request to server
	m_client.send(rec->to_packet() );
}

unsigned int obby::client_document::get_unsynced_changes_count() const
{
	// Just return size of list which is the amount of unsynced changes
	return m_unsynced.size();
}

void obby::client_document::on_net_record(record& rec)
{
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

	m_revision = rec.get_revision();

	// THE FOLLOWING DOES NOT WORK :((99
	// TODO: What happens with invalid records?
	// Is this change made from this client?
#if 0
	if(rec.get_from() == m_client.get_self()->get_id() )
	{
		// Look in unsynced changes
		for(iter = m_unsynced.begin(); iter != m_unsynced.end(); )
		{
			// Is it the incoming record?
			if( (*iter)->get_id() == rec.get_id() )
			{
				// Found
				sync_record = *iter;
				iter = m_unsynced.erase(iter);
				break;
			}
			else
			{
				// No, check next one
				++ iter;
			}
		}
	}

	if(sync_record && !sync_record->is_valid() )
		goto end;

	// Apply unsynced changes on new record and the syncing record to move
	// them to the current position
	for(iter = m_unsynced.begin(); iter != m_unsynced.end(); ++ iter)
	{
		if( (*iter)->is_valid() )
		{
			if(sync_record != NULL)
				(*iter)->apply(*sync_record);
			(*iter)->apply(rec);
		}
	}

	if(!rec.is_valid() )
		goto end;

	// Undo the unsynced change that we are syncing
	if(sync_record)
	{
		if(!sync_record->is_valid() )
			goto end;

		record* undo_record = sync_record->reverse();
		for(iter = m_unsynced.begin(); iter != m_unsynced.end(); ++iter)
			if( (*iter)->is_valid() )
				undo_record->apply(**iter);
		undo_record->apply(*this);
		undo_record->emit_document_signal(*this);
		delete undo_record;
	}

	// Redo synced change
	for(iter = m_unsynced.begin(); iter != m_unsynced.end(); ++ iter)
		if( (*iter)->is_valid() )
			rec.apply(**iter);
	rec.apply(*this);
	rec.emit_document_signal(*this);

	// Update revision number
	m_revision = rec.get_revision();

end:
	delete sync_record;
#endif
}

void obby::client_document::on_net_sync_init(const net6::packet& pack)
{
	if(pack.get_param_count() < 2) return;
	if(pack.get_param(0).get_type() != net6::packet::param::INT) return;
	if(pack.get_param(1).get_type() != net6::packet::param::STRING) return;
	if(pack.get_param(2).get_type() != net6::packet::param::INT) return;

	m_id = pack.get_param(0).as_int();
	m_title = pack.get_param(1).as_string();
	m_revision = pack.get_param(2).as_int();

	m_lines.clear();
}

void obby::client_document::on_net_sync_line(const net6::packet& pack)
{
	m_lines.push_back(line(pack, get_buffer().get_user_table()) );
/*	if(pack.get_param_count() < 2) return;
	if(pack.get_param(0).get_type() != net6::packet::param::INT) return;
	if(pack.get_param(1).get_type() != net6::packet::param::STRING) return;

	m_lines.push_back(pack.get_param(1).as_string() );*/
}

void obby::client_document::on_net_sync_final(const net6::packet& pack)
{
	if(pack.get_param_count() < 1) return;
	if(pack.get_param(0).get_type() != net6::packet::param::INT) return;

	// Auch t0l.
}

