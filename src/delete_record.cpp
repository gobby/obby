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
#include "delete_record.hpp"
#include "insert_record.hpp"
#include "buffer.hpp"

obby::delete_record::delete_record(const position& begin, const position& end,
                                   unsigned int revision, unsigned int from)
 : record(revision, from), m_from(begin), m_to(end)
{
	assert(end > begin);
}

obby::delete_record::delete_record(const position& begin, const position& end,
                                   unsigned int revision, unsigned int from,
                                   unsigned int id)
 : record(revision, from, id), m_from(begin), m_to(end)
{
	assert(end > begin);
}

obby::delete_record::~delete_record()
{
}

void obby::delete_record::apply(buffer& buf) const
{
	buf.erase_nosync(m_from, m_to);
}

void obby::delete_record::apply(record& rec) const
{
	rec.on_delete(m_from, m_to);
}

net6::packet obby::delete_record::to_packet()
{
	net6::packet pack("obby_record");
	pack << "delete" << static_cast<int>(m_id)
	     << static_cast<int>(m_revision) << static_cast<int>(record::m_from)
	     << static_cast<int>(m_from.get_line() )
	     << static_cast<int>(m_from.get_col() )
	     << static_cast<int>(m_to.get_line() )
	     << static_cast<int>(m_to.get_col() );
	return pack;
}

obby::record* obby::delete_record::reverse(const buffer& buf)
{
	std::string sub_buf = buf.get_sub_buffer(m_from, m_to);
	return new insert_record(m_from, sub_buf,
	                         m_revision, record::m_from, m_id);
}

void obby::delete_record::on_insert(const position& pos,
                                    const std::string& text)
{
	position size(text);

	if(pos < m_from)
		{ m_from += size; m_to += size; }
	else if(pos < m_to)
		m_to += size;
}

void obby::delete_record::on_delete(const position& from, const position& to)
{
	assert(to >= from);

	// Deletion after the range
	if(from >= m_to)
	{
		return;
	}
	// Delete before the range
	else if(to <= m_from)
	{
		m_from.sub_range(from, to);
		m_to.sub_range(from, to);
	}
	// Deletion of the first part of the range
	else if(from <= m_from && to > m_from)
	{
		m_from = from;
		m_to.sub_range(from, to);
	}
	// Deletion of the last part of the range
	else if(from < m_to && to >= from)
	{
		m_to = from;
	}
	// Deletion in the range
	else
	{
		m_to.sub_range(from, to);
	}

	if(m_to <= m_from)
		invalidate();
}

const obby::position& obby::delete_record::get_begin() const
{
	return m_from;
}

const obby::position& obby::delete_record::get_end() const
{
	return m_to;
}

