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
#include "buffer.hpp"

obby::insert_record::insert_record(const position& pos, const std::string& text,
                                   unsigned int revision, unsigned int from)
 : record(revision, from), m_pos(pos), m_text(text)
{
}

obby::insert_record::insert_record(const position& pos, const std::string& text,
                                   unsigned int revision, unsigned int from,
                                   unsigned int id)
 : record(revision, from, id), m_pos(pos), m_text(text)
{
}

obby::insert_record::~insert_record()
{
}

void obby::insert_record::apply(buffer& buf) const
{
	buf.insert_nosync(m_pos, m_text);
}

void obby::insert_record::apply(record& rec) const
{
	rec.on_insert(m_pos, m_text);
}

net6::packet obby::insert_record::to_packet()
{
	net6::packet pack("obby_record");
	pack << "insert" << static_cast<int>(m_id)
	     << static_cast<int>(m_revision) << static_cast<int>(m_from)
	     << static_cast<int>(m_pos.get_line() )
	     << static_cast<int>(m_pos.get_col() )
	     << m_text;
	return pack;
}

obby::record* obby::insert_record::reverse(const buffer& buf)
{
	position text_size(m_text);
	return new delete_record(m_pos, m_pos + text_size, m_revision, 
	                         m_from, m_id);
}

void obby::insert_record::on_insert(const position& pos,
                                    const std::string& text)
{
	if(pos <= m_pos)
	{
		position size(text);
		m_pos += size;
	}
}

void obby::insert_record::on_delete(const position& from, const position& to)
{
	assert(to >= from);

	if(m_pos >= from && m_pos < to)
	{
		// The position where to insert text has been deleted
		invalidate();
	}
	else if(m_pos >= to)
	{
		m_pos.sub_range(from, to);
	}
}

const obby::position& obby::insert_record::get_position() const
{
	return m_pos;
}

const std::string& obby::insert_record::get_text() const
{
	return m_text;
}

