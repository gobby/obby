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
#include "insert_record.hpp"
#include "delete_record.hpp"
#include "document.hpp"

obby::insert_record::insert_record(position pos, const std::string& text,
                                   unsigned int document, unsigned int revision,
                                   unsigned int from)
 : record(document, revision, from), m_pos(pos), m_text(text)
{
}

obby::insert_record::insert_record(position pos, const std::string& text,
                                   unsigned int document, unsigned int revision,
                                   unsigned int from, unsigned int id)
 : record(document, revision, from, id), m_pos(pos), m_text(text)
{
}

obby::insert_record::~insert_record()
{
}

obby::record* obby::insert_record::clone() const
{
	return new insert_record(m_pos, m_text, m_revision, m_from, m_id);
}

void obby::insert_record::apply(document& doc) const
{
	doc.insert_nosync(m_pos, m_text);
}

void obby::insert_record::apply(record& rec) const
{
	rec.on_insert(m_pos, m_text);
}

net6::packet obby::insert_record::to_packet()
{
	net6::packet pack("obby_record");
	pack << "insert" << m_id << m_document << m_revision << m_from
	     << m_pos << m_text;
	return pack;
}

obby::record* obby::insert_record::reverse()
{
	return new delete_record(m_pos, m_text, m_document,
	                         m_revision, m_from, m_id);
}

void obby::insert_record::on_insert(position pos,
                                    const std::string& text)
{
	if(pos <= m_pos)
		m_pos += text.length();
}

void obby::insert_record::on_delete(position from, position to)
{
	assert(to >= from);

	if(m_pos >= from && m_pos < to)
		// The position where to insert text has been deleted
		invalidate();
	else if(m_pos >= to)
		m_pos -= (to - from);
}

obby::position obby::insert_record::get_position() const
{
	return m_pos;
}

const std::string& obby::insert_record::get_text() const
{
	return m_text;
}

void obby::insert_record::emit_document_signal(const document& doc) const
{
	doc.insert_event().emit(*this);
}

