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
 : m_history(), m_revision(0), m_lines(1, "")
{
}

obby::buffer::~buffer()
{
	std::list<record*>::iterator iter;
	for(iter = m_history.begin(); iter != m_history.end(); ++ iter)
		delete *iter;
}

std::string obby::buffer::get_sub_buffer(const position& from,
                                         const position& to) const
{
	assert(to >= from);
	assert(to.get_line() < m_lines.size() );
	assert(from.get_col() <= m_lines[from.get_line()].size() );
	assert(to.get_col() <= m_lines[to.get_line()].size() );

	std::string buf;
	for(unsigned int i = from.get_line(); i <= to.get_line(); ++ i)
	{
		unsigned int begin = 0;
		unsigned int end = m_lines[i].length();
		
		if(i == from.get_line() ) begin = from.get_col();
		if(i == to.get_line() )  end = to.get_col();

		//buf += m_lines[i].substr(begin, end - begin);
		buf.append(&m_lines[i][begin], end - begin);
	}

	return buf;
}

void obby::buffer::insert_nosync(const position& pos, const std::string& text)
{
	assert(pos.get_line() < m_lines.size() );
	assert(pos.get_col() <= m_lines[pos.get_line()].size() );

	std::string::size_type cur = 0, prev = 0;
	std::vector<std::string>::size_type line = pos.get_line();
	while((cur = text.find('\n', cur)) != std::string::npos)
	{
		m_lines[line].append(&text[prev], cur - prev);
		insert_lines(++line, 1);
	}
	m_lines[line].append(&text[prev], text.size() - prev);
}

void obby::buffer::erase_nosync(const position& from, const position& to)
{
	assert(to >= from);
	assert(to.get_line() < m_lines.size() );
	assert(from.get_col() <= m_lines[from.get_line()].size() );
	assert(to.get_col() <= m_lines[to.get_line()].size() );

	if(from.get_line() == to.get_line() )
	{
		m_lines[from.get_line()].erase(
			from.get_col(),
			to.get_col() - from.get_col()
		);
	}
	else
	{
		// TODO: Replace the following two calls by one call to
		// std::string::replace
		m_lines[from.get_line()].erase(from.get_col() );

		m_lines[from.get_line()].append(
			&m_lines[to.get_line()][to.get_col()],
			m_lines[to.get_line()].length() - to.get_col()
		);

		erase_lines(
			from.get_line() + 1,
			to.get_line() - from.get_line()
		);
	}
}

// TODO: Improve these functions. But how? :/
void obby::buffer::insert_lines(unsigned int pos, unsigned int count)
{
	std::vector<std::string>::iterator iter = m_lines.begin();
	for(unsigned int i = 0; i < pos; ++ i) ++ iter;
	m_lines.insert(iter, count, "");
}

void obby::buffer::erase_lines(unsigned int pos, unsigned int count)
{
	std::vector<std::string>::iterator beg_iter = m_lines.begin();
	for(unsigned int i = 0; i < pos; ++ i) ++ beg_iter;
	std::vector<std::string>::iterator end_iter = beg_iter;
	for(unsigned int i = 0; i < count; ++ i) ++ end_iter;
	m_lines.erase(beg_iter, end_iter);
}

