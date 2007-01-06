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

#include "common.hpp"
#include "document.hpp"

obby::document::document()
 : m_lines(1, line() )
{
}

void obby::document::serialise(serialise::object& obj) const
{
	for(std::vector<line>::const_iterator iter = m_lines.begin();
	    iter != m_lines.end();
	    ++ iter)
	{
		serialise::object& line = obj.add_child();
		line.set_name("line");
		iter->serialise(line);
	}
}

void obby::document::deserialise(const serialise::object& obj,
                                 const user_table& user_table)
{
	// Clear lines
	m_lines.clear();
	// Load from serialisation object
	for(serialise::object::child_iterator iter = obj.children_begin();
	    iter != obj.children_end();
	    ++ iter)
	{
		if(iter->get_name() == "line")
		{
			m_lines.push_back(line(*iter, user_table) );
		}
		else
		{
			// Unexpected child node
			// TODO: unexpected_child_error
			format_string str(_("Unexpected child node: '%0%'") );
			str << iter->get_name();
			throw serialise::error(str.str(), iter->get_line() );
		}
	}
	// Do not allow documents with no lines
	if(m_lines.empty() )
	{
		throw serialise::error(
			_("Document needs at least one line"),
			obj.get_line()
		);
	}
}

std::string obby::document::get_text() const
{
	// TODO: Preallocation
	std::string content;
	std::vector<line>::const_iterator iter;

	for(iter = m_lines.begin(); iter != m_lines.end(); ++ iter)
	{
		content += *iter;
		content += (iter != m_lines.end() - 1) ? "\n" : "";
	}

	return content;
}

obby::line obby::document::get_slice(position from, position len) const
{
	// Find range to extract
	unsigned int from_row, from_col, to_row, to_col;
	position_to_coord(from, from_row, from_col);
	position_to_coord(from + len, to_row, to_col);

	// Preallocate memory for the string
	line buffer;
	// TODO: Stupid assumption here, make better one - or leave zero.
	buffer.reserve(len, (to_col - from_col) * 10);

	// Iterate through affected rows
	for(std::vector<line>::size_type i = from_row; i <= to_row; ++ i)
	{
		// Find columns in this row where to extract text
		line::size_type begin = 0, end = m_lines[i].length();
		if(i == from_row)
			begin = from_col;
		if(i == to_row)
			end = to_col;

		// Append to target buffer
		buffer.append(m_lines[i].substr(begin, end - begin) );

		// Append newline, if this is not the last line
		if(i != to_row)
		{
			line::author_iterator last =
				m_lines[i].author_end() - 1;

			// TODO: Last author may not neccesarily be author
			// of the newline character
			buffer.append(line("\n", last->author) );
		}
	}

	// Verify that the returned length is the same as the required one
	if(buffer.length() != len)
		throw std::logic_error("obby::document::get_slice");

	return buffer;
}

void obby::document::insert(position pos, const std::string& text,
                            const user* author)
{
	insert(pos, line(text, author) );
}

void obby::document::insert(position pos, const line& text)
{
	// Convert position to row and column
	unsigned int pos_row, pos_col;
	position_to_coord(pos, pos_row, pos_col);

	const std::string& str = static_cast<const std::string&>(text);
	std::vector<line>::iterator iter = m_lines.begin() + pos_row;

	// Line that holds a carry from the first line when text contains
	// a newline
	line first_line_carry;
	unsigned int ins_col = pos_col;

	// Notify signal handlers before making any changes
	// TODO: Sigal handler taking line. The here given author is just the
	// first one, this is wrong and works only with
	// insert(position, const std::string&, const user*)
	m_signal_insert.before().emit(pos, text, text.author_begin()->author);

	// Insert line by line
	std::string::size_type nl_pos = 0, nl_prev = 0;
	while( (nl_pos = str.find('\n', nl_pos)) != std::string::npos)
	{
		// First line?
		if(nl_prev == 0)
		{
			// Store rest of line in first_line_carry
			first_line_carry = iter->substr(pos_col);
			// and remove it from source line
			iter->erase(pos_col);
			// Insert the line at the beginning of the next line
			ins_col = 0;
		}

		// Append line
		iter->append(text.substr(nl_prev, nl_pos - nl_prev) );
		// Insert new line
		iter = m_lines.insert(++ iter, line() );
		nl_prev = ++ nl_pos;
	}

	// Insert first_line_carry (if any) and the text in the last line
	iter->insert(ins_col, first_line_carry);
	iter->insert(ins_col, text.substr(nl_prev) );

	// Notify signal handlers after operation
	// TODO: Sigal handler taking line. The here given author is just the
	// first one, this is wrong and works only with
	// insert(position, const std::string&, const user*)
	m_signal_insert.after().emit(pos, text, text.author_begin()->author);
}

void obby::document::erase(position pos, position len, const user* author)
{
	// Convert position into row/column pairs
	unsigned int from_row, from_col, to_row, to_col;
	position_to_coord(pos, from_row, from_col);
	position_to_coord(pos + len, to_row, to_col);

	// Get first affected line
	std::vector<line>::iterator iter = m_lines.begin() + from_row;
	
	// Tell signal handlers before doing anything
	m_signal_delete.before().emit(pos, len, author);

	if(from_row == to_row)
	{
		// Only one row affected: Delete given range
		iter->erase(from_col, to_col - from_col);
	}
	else
	{
		// Erase text right from start point in first line
		iter->erase(from_col);
		// Append rest of last line
		iter->append(m_lines[to_row].substr(to_col) );

		// Remove other lines
		++ iter;
		m_lines.erase(iter, iter + (to_row - from_row) );
	}

	// Notify signal handlers after operation
	m_signal_delete.after().emit(pos, len, author);
}

obby::document::signal_insert_type obby::document::insert_event() const
{
	return m_signal_insert;
}

obby::document::signal_delete_type obby::document::delete_event() const
{
	return m_signal_delete;
}

void obby::document::clear_lines()
{
	m_lines.clear();
}

void obby::document::add_line(const line& line)
{
	m_lines.push_back(line);
}

const obby::line& obby::document::get_line(unsigned int index) const
{
	if(index >= m_lines.size() )
		throw std::logic_error("obby::document::get_line");

	return m_lines[index];
}

unsigned int obby::document::get_line_count() const
{
	return m_lines.size();
}

obby::position obby::document::coord_to_position(unsigned int row,
                                                 unsigned int col) const
{
	if(row >= m_lines.size() )
		throw std::logic_error("obby::document::coord_to_position");
	if(col > m_lines[row].length() )
		throw std::logic_error("obby::document::coord_to_position");

	position pos = 0;
	for(std::vector<std::string>::size_type i = 0; i < row; ++ i)
		pos += m_lines[i].length() + 1;

	return pos + col;
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

	if(i >= m_lines.size() )
		throw std::logic_error("obby::document::position_to_coord");

	col = m_lines[i].length() + 1 - (cur_pos - pos);
}

obby::position obby::document::position_eob() const
{
	return coord_to_position(m_lines.size() - 1,
	       	m_lines[m_lines.size() - 1].length() );
}

