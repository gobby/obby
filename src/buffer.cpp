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
#include "buffer.hpp"

obby::buffer::buffer()
 : m_history(), m_revision(0), m_lines()
{
}

obby::buffer::~buffer()
{
}

std::string obby::buffer::get_sub_buffer(const position& from,
                                         const position& to)
{
	assert(to > from);
	assert(to.get_line() < m_lines.size() );

	std::string buf;
	for(unsigned int i = from.get_line(); i <= to.get_line(); ++ i)
	{
		unsigned int begin = 0;
		unsigned int end = m_lines[i].length();
		
		if(i == from.get_line() ) begin = from.get_col();
		if(i == end.get_line() )  end = to.get_col();

		//buf += m_lines[i].substr(begin, end - begin);
		buf.append(&m_lines[i][begin], end - begin);
	}

	return buf;
}

void obby::buffer::insert(const position& pos, const std::string& text)
{
}

void obby::buffer::erase(const position& from, const position& to)
{
}

