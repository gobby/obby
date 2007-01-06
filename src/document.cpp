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

obby::document::document(unsigned int id, const user_table& usertable)
 : m_id(id), m_history(), m_revision(0), m_usertable(usertable),
   m_lines(1, line())
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
	std::vector<line>::const_iterator iter;
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
	std::vector<line>::size_type i;
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

void obby::document::insert_nosync(position pos, const std::string& text,
                                   unsigned int author_id)
{
	// Convert position to row and column
	unsigned int pos_row, pos_col;
	position_to_coord(pos, pos_row, pos_col);

	// Verify them
	assert(pos_row < m_lines.size() );
	assert(pos_col <= m_lines[pos_row].length() );

	// Find author in user table
	const user_table::user* author;
	author = m_usertable.find_from_user_id(author_id);
	assert(author != NULL);

	// Move line iterator to the line where to insert text
	std::vector<line>::iterator iter = m_lines.begin();
	for(unsigned int i = 0; i < pos_row; ++ i)
		++ iter;

	// Line that holds a carry from the first line when text contains
	// a newline
	line first_line_carry;
	unsigned int ins_col = pos_col;

	// Insert line by line
	std::string::size_type nl_pos = 0, nl_prev = 0;
	while( (nl_pos = text.find('\n', nl_pos)) != std::string::npos)
	{
		// First line?
		if(nl_prev == 0)
		{
			// Store rest of line in first_line_carry
			first_line_carry = iter->substr(pos_col);
			// and remove it from the source line
			iter->erase(pos_col);
			// Insert the line at the beginning of the next line
			ins_col = 0;
		}

		// Append the text to this newline onto this line
		iter->append(text.substr(nl_prev, nl_pos - nl_prev), *author);
		// Insert next line
		++ iter;
		iter = m_lines.insert(iter, line());

		// Store newline position for substringing next line
		nl_prev = ++ nl_pos;
	}

	// Insert first_line_carry (if any) and the text in the last line
	iter->insert(ins_col, first_line_carry);
	iter->insert(ins_col, text.substr(nl_prev), *author);
}

void obby::document::erase_nosync(position from, position to,
                                  unsigned int author_id)
{
	// Convert positions to rows and columns
	unsigned int from_row, from_col, to_row, to_col;
	position_to_coord(from, from_row, from_col);
	position_to_coord(to, to_row, to_col);
	
	// Verify them
	assert(to >= from);
	assert(to_row < m_lines.size() );
	assert(to_col <= m_lines[to_row].length() );

	// Find author in user_table
	const user_table::user* author;
	author = m_usertable.find_from_user_id(author_id);
	assert(author != NULL);

	// Find the iterator for the given row
	std::vector<line>::iterator iter = m_lines.begin();
	for(unsigned int i = 0; i < from_row; ++ i)
		++ iter;

	// Do not remove any lines?
	if(from_row == to_row)
	{
		// Just erase text from the line
		iter->erase(from_col, to_col - from_col);
	}
	else
	{
		// Erase the rest of this line
		iter->erase(from_col);
		// And append the rest of the last line
		iter->append(m_lines[to_row].substr(to_col) );

		// Remove all lines between the first and the last one
		++ iter;
		std::vector<line>::iterator end_iter = iter;
		for(unsigned int i = from_row + 1; i <= to_row; ++ i)
			++ end_iter;
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
		pos += m_lines[i].length() + 1;
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

obby::position obby::document::position_eob() const
{
	return coord_to_position(m_lines.size() - 1,
	       	m_lines[m_lines.size() - 1].length() );
}

