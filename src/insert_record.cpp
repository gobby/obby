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

#include "insert_record.hpp"
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

void obby::insert_record::apply(buffer& buf)
{
	buf.insert(m_pos, m_text);
}

bool obby::insert_record::is_valid() const
{
	return !m_text.empty();
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
	if(m_pos >= from && m_pos < to)
	{
		m_text = "";
	}
	else if(m_pos >= to)
	{
		// TODO: Put this into a method of position?
		if(to.get_line() == m_pos.get_line() )
			m_pos.move_by(
				from.get_line() - to.get_line(),
				from.get_col() - to.get_col()
			);
		else
			m_pos.move_by(from.get_line() - to.get_line(), 0);
	}
}

