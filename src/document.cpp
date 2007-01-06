/* libobby - Network text editing library
 * Copyright (C) 2005 0x539 dev group
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 i* version 2 of the License, or (at your option) any later version.
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
#include "document.hpp"

obby::document::document(unsigned int id)
 : m_id(id), m_history(), m_revision(0), m_lines(1, "")
{
}

obby::document::~document()
{
	std::list<record*>::iterator iter;
	for(iter = m_history.begin(); iter != m_history.end(); ++ iter)
		delete *iter;
}

unsigned int obby::document::get_id() const
{
	return m_id;
}

std::string obby::document::get_whole_buffer() const
{
	std::string content;
	std::vector<std::string>::const_iterator iter;
	for(iter = m_lines.begin(); iter != m_lines.end(); ++ iter)
		content += *iter;
	return content;
}

std::string obby::document::get_sub_buffer(position from, position to) const
{
	unsigned int from_row, from_col, to_row, to_col;
	position_to_coord(from, from_row, from_col);
	position_to_coord(to, to_row, to_col);

	assert(to >= from);
	assert(to_row < m_lines.size() );
	assert(to_col <= m_lines[to_row].length() );

	std::string buffer;
	std::vector<std::string>::size_type i;
	for(i = from_row; i <= to_row; ++ i)
	{
		std::string::size_type begin = 0, end = m_lines[i].length();
		if(i == from_row)
			begin = from_col;
		if(i == to_row)
			end = to_col;

		buffer += m_lines[i].substr(begin, end - begin);
		if(i != to_row)
			buffer += "\n";
	}
	return buffer;
}

void obby::document::insert_nosync(position pos, const std::string& text)
{
	unsigned int pos_row, pos_col;
	position_to_coord(pos, pos_row, pos_col);

	assert(pos_row < m_lines.size() );
	assert(pos_col <= m_lines[pos_row].length() );

	std::vector<std::string>::iterator iter = m_lines.begin();
	for(unsigned int i = 0; i < pos_row; ++ i)
		++ iter;

	std::string first_line_carry;
	unsigned int ins_col = pos_col;
	std::string::size_type nl_pos = 0, nl_prev = 0;
	while( (nl_pos = text.find('\n', nl_pos)) != std::string::npos)
	{
		if(nl_prev == 0)
		{
			first_line_carry = iter->substr(pos_col);
			iter->erase(pos_col);
			ins_col = 0;
		}

		iter->append(text.substr(nl_prev, nl_pos - nl_prev) );
		++ iter;
		iter = m_lines.insert(iter, "");

		nl_prev = ++ nl_pos;
	}

	iter->insert(ins_col, text.substr(nl_prev) + first_line_carry);
}

void obby::document::erase_nosync(position from, position to)
{
	unsigned int from_row, from_col, to_row, to_col;
	position_to_coord(from, from_row, from_col);
	position_to_coord(to, to_row, to_col);
	
	assert(to >= from);
	assert(to_row < m_lines.size() );
	assert(to_col <= m_lines[to_row].length() );

	std::vector<std::string>::iterator iter = m_lines.begin();
	for(unsigned int i = 0; i < from_row; ++ i)
		++ iter;

	if(from_row == to_row)
	{
		unsigned int erase_len = to_col - from_col;
		iter->erase(from_col, erase_len);
	}
	else
	{
		iter->erase(from_col);
		iter->append(m_lines[to_row].substr(to_col) );

		++ iter;
		std::vector<std::string>::iterator end_iter = iter;
		for(unsigned int i = from_row + 1; i <= to_row; ++ i)
			++ end_iter;
//		++ end_iter;
		m_lines.erase(iter, end_iter);
	}
}

obby::document::signal_insert_type obby::document::insert_event() const
{
	return m_signal_insert;
}

obby::document::signal_delete_type obby::document::delete_event() const
{
	return m_signal_delete;
}

std::string obby::document::get_line(unsigned int index) const
{
	assert(index < m_lines.size() );
	return m_lines[index];
}

unsigned int obby::document::get_line_count() const
{
	return m_lines.size();
}

obby::position obby::document::coord_to_position(unsigned int row,
                                                 unsigned int col) const
{
	assert(row < m_lines.size() );
	assert(col <= m_lines[row].length() );

	position pos = 0;
	for(std::vector<std::string>::size_type i = 0; i < row; ++ i)
		pos += m_lines[i].length();
	pos += col;
	return pos;
}

void obby::document::position_to_coord(position pos,
                                       unsigned int& row,
                                       unsigned int& col) const
{
	row = col = 0;
	position cur_pos = 0;
	std::vector<std::string>::size_type i;

	for(i = 0; i < m_lines.size(); ++ i)
	{
		cur_pos += m_lines[i].length() + 1;
		if(cur_pos > pos)
			break;
		else
			++ row;
	}

	assert(i < m_lines.size() );
	col = m_lines[i].length() + 1 - (cur_pos - pos);
}

