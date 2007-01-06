/* libobby - Network text editing library
 * Copyright (C) 2005 0x539 dev group
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public
 * License as published by the Free Software Foundation; either
 i* version 2 of the License, or (at your option) any later version.
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
#include <iostream>
#include "document.hpp"
#include "buffer.hpp"

obby::document::document(const obby::document_info& info)
 : m_info(info), m_history(), m_revision(0), m_lines(1, line())
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
	return m_info.get_id();
}

const std::string& obby::document::get_title() const
{
	return m_info.get_title();
}

const obby::document_info& obby::document::get_info() const
{
	return m_info;
}

const obby::buffer& obby::document::get_buffer() const
{
	return m_info.get_buffer();
}

unsigned int obby::document::get_revision() const
{
	return m_revision;
}

std::string obby::document::get_text() const
{
	std::string content;
	std::vector<line>::const_iterator iter;
	for(iter = m_lines.begin(); iter != m_lines.end(); ++ iter)
	{
		content += *iter;
		content += (iter != m_lines.end() - 1) ? "\n" : "";
	}
	return content;
}

std::string obby::document::get_slice(position from, position to) const
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

void obby::document::insert_nosync(const insert_record& record)
{
	// Convert position to row and column
	unsigned int pos_row, pos_col;
	position_to_coord(record.get_position(), pos_row, pos_col);

	// Verify them
	assert(pos_row < m_lines.size() );
	assert(pos_col <= m_lines[pos_row].length() );

	// Get user_table from the buffer
	const user_table& user_table = get_buffer().get_user_table();

	// Find author in user table
	// user::CONNECTED should be correct here: Only conncted users
	// can edit this document.
	const user* author =
		user_table.find_user<user::CONNECTED>(record.get_from() );

	// Author may be NULL if the user_id is 0, then a server_document,
	// which has no user, has inserted something directly
	if(author == NULL && record.get_from() != 0)
	{
		std::cerr << "obby::document::insert_nosync: User "
		          << record.get_from() << " is not connected"
		          << std::endl;
		return;
	}

	// Move line iterator to the line where to insert text
	// TODO: std::vector::iterator is a random access iterator, we can
	// do things like +=
	std::vector<line>::iterator iter = m_lines.begin();
	for(unsigned int i = 0; i < pos_row; ++ i)
		++ iter;

	// Line that holds a carry from the first line when text contains
	// a newline
	line first_line_carry;
	unsigned int ins_col = pos_col;

	// Notify signal handlers before making any changes
	m_signal_insert.before().emit(record);
	
	// Insert line by line
	std::string::size_type nl_pos = 0, nl_prev = 0;
	const std::string& text = record.get_text();
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
		iter->append(text.substr(nl_prev, nl_pos - nl_prev), author);
		// Insert next line
		++ iter;
		iter = m_lines.insert(iter, line());

		// Store newline position for substringing next line
		nl_prev = ++ nl_pos;
	}

	// Insert first_line_carry (if any) and the text in the last line
	iter->insert(ins_col, first_line_carry);
	iter->insert(ins_col, text.substr(nl_prev), author);

	// Notify signal handlers after changes have been performed
	m_signal_insert.after().emit(record);
}

void obby::document::erase_nosync(const delete_record& record)
{
	// Convert positions to rows and columns
	unsigned int from_row, from_col, to_row, to_col;
	position_to_coord(record.get_begin(), from_row, from_col);
	position_to_coord(record.get_end(), to_row, to_col);
	
	// Verify them
	// TODO: record ensures already that end >= begin, doesn't it?
	assert(record.get_end() >= record.get_begin() );
	assert(to_row < m_lines.size() );
	assert(to_col <= m_lines[to_row].length() );

	// Get user_table from the buffer
	const user_table& user_table = get_buffer().get_user_table();

	// Find author in user table
	// user::CONNECTED should be correct here: Only conncted users
	// can edit this document.
	const user* author =
		user_table.find_user<user::CONNECTED>(record.get_from() );

	// Author may be NULL if the user_id is 0, then a server_document,
	// which has no user, has inserted something directly
	if(author == NULL && record.get_from() != 0)
	{
		std::cerr << "obby::document::insert_nosync: User "
		          << record.get_from() << " does not exist"
		          << std::endl;
		return;
	}

	// Find the iterator for the given row
	// TODO: std::vector::iterator is a random access iterator, we can
	// do things like +=
	std::vector<line>::iterator iter = m_lines.begin();
	for(unsigned int i = 0; i < from_row; ++ i)
		++ iter;

	// Notify signal handlers before making any changes
	m_signal_delete.before().emit(record);

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

	// Notify signal handlers after changes have been performed
	m_signal_delete.after().emit(record);
}

obby::document::signal_insert_type obby::document::insert_event() const
{
	return m_signal_insert;
}

obby::document::signal_delete_type obby::document::delete_event() const
{
	return m_signal_delete;
}

obby::document::signal_change_type obby::document::change_event() const
{
	return m_signal_change;
}

const obby::line& obby::document::get_line(unsigned int index) const
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

