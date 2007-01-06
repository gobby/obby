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
#include <iostream>
#include "buffer.hpp"
#include "line.hpp"

const obby::line::size_type obby::line::npos = obby::line::string_type::npos;

obby::line::line()
 : m_line(), m_authors()
{
	/*user_pos pos = { NULL, 0 };
	m_authors.push_back(pos);*/
}

obby::line::line(const string_type& text, const user_type* author)
 : m_line(text), m_authors()
{
	// It is just _one_ line.
	assert(text.find('\n') == string_type::npos);

	// Insert initial author
	user_pos pos = { author, 0 };
	m_authors.push_back(pos);
}

obby::line::line(const net6::packet& pack, const user_table& user_table)
{
	if(pack.get_param_count() < 3) return;
	// Parameters 0 and 1 are checked by buffer.
	if(pack.get_param(2).get_type() != net6::packet::param::STRING) return;

	// Parameter 0 is the document ID which we do not need here.
	m_line = pack.get_param(2).as_string();

	// We need odd parameter count (line, pos->author, pos->author, etc.)
	if( (pack.get_param_count() % 2) == 0) return;

	// Reserve space in author vector
	m_authors.reserve( (pack.get_param_count() - 3) / 2);
	
	// Add authors
	for(unsigned int i = 3; i < pack.get_param_count(); i += 2)
	{
		// Verify parameters
		if(pack.get_param(i).get_type() != net6::packet::param::INT)
			return;
		if(pack.get_param(i + 1).get_type() != net6::packet::param::INT)
			return;

		// Get position and author id
		unsigned int pos = pack.get_param(i).as_int();
		unsigned int user_id = pack.get_param(i + 1).as_int();

		// Find author
		const user* author = user_table.find_user(user_id);

		// Author may be NULL if user is 0, this means that a server
		// document has written something directly into a line.
		if(author == NULL && user_id != 0)
		{
			std::cerr << "obby::line::line: User " << user_id << " "
			          << "does not exist" << std::endl;
		}
		else
		{
			// Add to vector
			user_pos upos = { author, pos };
			m_authors.push_back(upos);
		}
	}
}

obby::line::line(const line& other)
 : m_line(other.m_line), m_authors(other.m_authors)
{
}

obby::line::~line()
{
}

obby::line& obby::line::operator=(const line& other)
{
	m_line = other.m_line;
	m_authors = other.m_authors;

	return *this;
}

obby::line::operator const std::string&() const
{
	return m_line;
}

obby::line::size_type obby::line::length() const
{
	return m_line.length();
}

void obby::line::insert(size_type pos, const line& text)
{
	// Build new author vector
	std::vector<user_pos> new_vec;
	new_vec.reserve(m_authors.size() + text.m_authors.size() + 1);
	std::vector<user_pos>::size_type i, j;

	// Insert authors before the insertion point
	for(i = 0; i < m_authors.size(); ++ i)
	{
		if(m_authors[i].position > pos)
			break;
		else
			new_vec.push_back(m_authors[i]);
	}

	// Insert authors of the insertion text
	for(j = 0; j < text.m_authors.size(); ++ j)
	{
		std::vector<user_pos>::size_type new_pos = new_vec.size();
		new_vec.push_back(text.m_authors[j]);
		new_vec[new_pos].position += pos;
	}

	// Insert authors after the insertion point. Insert the last author
	// before this point once again because he wrote the stuff up to
	// the next author.
	if(i > 0)
	{
		std::vector<user_pos>::size_type new_pos = new_vec.size();
		new_vec.push_back(m_authors[i - 1]);
		new_vec[new_pos].position = pos + text.length();
		
		for(; i < m_authors.size(); ++ i)
		{
			std::vector<user_pos>::size_type j = new_vec.size();
			new_vec.push_back(m_authors[i]);
			new_vec[j].position += text.length();
		}
	}

	// Use new vector
	m_authors.swap(new_vec);

	// Insert line content
	m_line.insert(pos, text.m_line);

	// Compress author list
	compress_authors();
}

void obby::line::insert(size_type pos, const string_type& text,
                        const user_type* author)
{
	insert(pos, line(text, author) );
}

void obby::line::append(const line& text)
{
	insert(m_line.length(), text);
}

void obby::line::append(const string_type& text, const user_type* author)
{
	append(line(text, author) );
}

void obby::line::erase(size_type from, size_type len)
{
	if(len == npos) len = m_line.length() - from;

	std::vector<user_pos>::iterator iter;
	for(iter = m_authors.begin(); iter != m_authors.end(); ++ iter)
	{
		// This position is in the range of deleted text: Move it to
		// the beginning, so compress_authors() will remove them.
		if(iter->position > from && iter->position <= from + len)
			iter->position = from;
		// Position after the text. Just move forward.
		else if(iter->position > from + len)
			iter->position -= len;
	}

	// Erase from line content.
	m_line.erase(from, len);
	compress_authors();
}

obby::line obby::line::substr(size_type from, size_type len) const
{
	// Set correct length for npos
	if(len == npos) len = m_line.length() - from;
	assert(from + len <= m_line.length() );

	// Create new line
	line new_line;
	new_line.m_authors.reserve(m_authors.size() );

	// Ignore authors before the given range
	std::vector<user_pos>::size_type i;
	for(i = 0; i < m_authors.size(); ++ i)
		if(m_authors[i].position > from)
			break;

	// Insert first author
	if(i > 0)
	{
		new_line.m_authors.push_back(m_authors[i - 1]);
		new_line.m_authors[0].position = 0;

		// Insert others
		for(; i < m_authors.size(); ++ i)
		{
			// After the given range? Forget them.
			if(m_authors[i].position >= from + len)
			{
				break;
			}
			else
			{
				std::vector<user_pos>::size_type new_pos =
					new_line.m_authors.size();

				new_line.m_authors.push_back(m_authors[i]);
				new_line.m_authors[new_pos].position -= from;
			}
		}
	}

	// Set content
	new_line.m_line = m_line.substr(from, len);
	
	// Compress author list
	new_line.compress_authors();

	return new_line;
}

obby::line::author_iterator obby::line::author_begin() const
{
	return m_authors.begin();
}

obby::line::author_iterator obby::line::author_end() const
{
	return m_authors.end();
}

void obby::line::compress_authors()
{
	std::vector<user_pos> new_vec;
	new_vec.reserve(m_authors.size() );

	for(std::vector<user_pos>::size_type i = 0; i < m_authors.size(); ++ i)
	{
		// Whether to add this user_pos to the new vector
		bool add = true;

		// Check for another user_pos at the same position
		if(i < m_authors.size() - 1)
			if(m_authors[i].position == m_authors[i + 1].position)
				add = false;
		if(!add) continue;

		// Check for the same author
		if(new_vec.size() > 0)
			if(new_vec[new_vec.size()-1].author ==
			   m_authors[i].author)
				add = false;
		if(!add) continue;

		// Ignore positions at the end of the line
		if(m_authors[i].position == m_line.length() ) break;

		// Add into new vector
		new_vec.push_back(m_authors[i]);
	}

	m_authors.swap(new_vec);
}

net6::packet obby::line::to_packet(unsigned int document_id) const
{
	net6::packet pack("obby_document", net6::packet::DEFAULT_PRIORITY,
	                  2 + m_authors.size() * 2);
	pack << document_id << "sync_line" << m_line;

	std::vector<user_pos>::const_iterator iter;
	for(iter = m_authors.begin(); iter != m_authors.end(); ++ iter)
		pack << static_cast<int>(iter->position) <<
			static_cast<int>(iter->author->get_id());

	return pack;
}

