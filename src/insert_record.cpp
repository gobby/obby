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

namespace
{
	// TODO: We need this function more than once, I guess. Maybe put
	// it into obby::position?
	obby::position string_size(const std::string& str)
	{
		unsigned int lines = 1;
		std::string::size_type cur = 0, prev = 0;
		while((cur = str.find('\n', cur)) != std::string::npos)
			{ prev = ++cur; ++ lines; }
		
		return obby::position(lines, str.length() - prev);
	}
}

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
		position size = string_size(text);
		m_pos.move_by(size.get_line() - 1, 0);
		if(size.get_line() > 1)
			m_pos.move_to(m_pos.get_line(), size.get_col() );
		else
			m_pos.move_by(0, size.get_col() );
	}
}

void obby::insert_record::on_delete(const position& from, const position& to)
{
	if(m_pos < from)
	{
		if(to.get_line() == m_pos.get_line() )
			m_pos.move_by(
				from.get_line() - to.get_line(),
				from.get_col() - to.get_col()
			);
		else
			m_pos.move_move_by(from.get_line() - to.get_line(), 0);
	}
	else if(m_pos >= from && m_pos < to)
	{
		m_text = "";
	}
}

