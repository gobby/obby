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
#include "delete_record.hpp"
#include "insert_record.hpp"
#include "document.hpp"

obby::delete_record::delete_record(position pos, const std::string& text,
                                   unsigned int document, unsigned int revision,                                   unsigned int from)
 : record(document, revision, from), m_pos(pos), m_text(text)
{
}

obby::delete_record::delete_record(position pos, const std::string& text,
                                   unsigned int document, unsigned int revision,
                                   unsigned int from, unsigned int id)
 : record(document, revision, from, id), m_pos(pos), m_text(text)
{
}

obby::delete_record::~delete_record()
{
}

obby::record* obby::delete_record::clone() const
{
	return new delete_record(m_pos, m_text, m_document,
	                         m_revision, m_from, m_id);
}

void obby::delete_record::apply(document& doc) const
{
	assert(doc.get_slice(m_pos, m_pos + m_text.length()) == m_text);
	doc.erase_nosync(*this);
}

void obby::delete_record::apply(record& rec) const
{
	rec.on_delete(m_pos, m_pos + m_text.length() );
}

net6::packet obby::delete_record::to_packet() const
{
	net6::packet pack("obby_document");
	pack << m_document << "record" << "delete" << m_id << m_revision
	     << m_from << static_cast<unsigned int>(m_pos) << m_text;
	return pack;
}

obby::record* obby::delete_record::reverse()
{
	return new insert_record(m_pos, m_text, m_document,
	                         m_revision, m_from, m_id);
}

void obby::delete_record::on_insert(position pos, const std::string& text)
{
	if(pos <= m_pos)
		m_pos += text.length();
	else if(pos < m_pos + m_text.length() )
		m_text.insert(pos - m_pos, text);
}

void obby::delete_record::on_delete(position from, position to)
{
	assert(to >= from);

	// Deletion after the range
	if(from >= m_pos + m_text.length() )
	{
		return;
	}
	// Delete before the range
	else if(to <= m_pos)
	{
		m_pos -= (to - from);
	}
	// Deletion around the range
	else if(from <= m_pos && to >= m_pos + m_text.length() )
	{
		invalidate();
	}
	// Deletion of the first part of the range
	else if(from <= m_pos && to > m_pos)
	{
		m_text.erase(0, to - m_pos);
		m_pos = from;
	}
	// Deletion of the last part of the range
	else if(from < m_pos + m_text.length() && to >= m_pos + m_text.length())
	{
		m_text.erase(m_pos + m_text.length() - from);
	}
	// Deletion in the range
	else
	{
		m_text.erase(from - m_pos, to - from);
	}
}

#ifndef NDEBUG
#include <sstream>

std::string obby::delete_record::inspect() const
{
	std::stringstream stream;
	stream << "delete " << m_text << " from " << m_pos;
	return stream.str();
}
#endif

obby::position obby::delete_record::get_begin() const
{
	return m_pos;
}

obby::position obby::delete_record::get_end() const
{
	return m_pos + m_text.length();
}

const std::string& obby::delete_record::get_text() const
{
	return m_text;
}

