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
#include "common.hpp"
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
	//assert(text.find('\n') == string_type::npos);

	// Insert initial author
	user_pos pos = { author, 0 };
	m_authors.push_back(pos);
}

obby::line::line(const net6::packet& pack,
                 unsigned int& index,
                 const user_table& user_table)
{
	// First there is the string
	m_line = pack.get_param(index ++).as<std::string>();

	// Reserve space in author vector
	m_authors.reserve( (pack.get_param_count() - index) / 2);
	
	// Add authors
	while(index < pack.get_param_count() )
	{
		// Get position and author
		unsigned int pos =
			pack.get_param(index ++).as<unsigned int>();
		const user* author = pack.get_param(index ++).as<const user*>(
			::serialise::hex_context<const user*>(user_table));

		// Add to vector
		user_pos upos = { author, pos };
		m_authors.push_back(upos);
	}
}

obby::line::line(const serialise::object& obj,
                 const user_table& user_table)
{
	for(serialise::object::child_iterator iter = obj.children_begin();
	    iter != obj.children_end();
	    ++ iter)
	{
		if(iter->get_name() == "part")
		{
			// Part of the line
			user_pos pos = {
				iter->get_required_attribute("author").
					as<const user*>(
						::serialise::context<
							const user*
						>(user_table)
					),
				m_line.length()
			};

			// Line content
			m_line += iter->get_required_attribute("content").
				as<std::string>();

			m_authors.push_back(pos);
		}
		else
		{
			// Unexpected child
			// TODO: unexpected_child_error
			format_string str(_("Unexpected child node: '%0%'") );
			str << iter->get_name();
			throw serialise::error(str.str(), iter->get_line() );
		}
	}
}

void obby::line::serialise(serialise::object& obj) const
{
	for(author_iterator iter = author_begin();
	    iter != author_end();
	    ++ iter)
	{
		std::string::size_type to_pos = m_line.length();
		author_iterator to_iter = iter; ++ to_iter;
		if(to_iter != author_end() ) to_pos = to_iter->position;

		serialise::object& part = obj.add_child();
		part.set_name("part");

		part.add_attribute("content").set_value(
			m_line.substr(iter->position, to_pos - iter->position)
		);

		part.add_attribute("author").set_value(iter->author);
	}
}

obby::line::line(const line& other)
 : m_line(other.m_line), m_authors(other.m_authors)
{
}

void obby::line::reserve(size_type len, size_type pos)
{
	m_line.reserve(len);
	m_authors.reserve(pos);
}

obby::line& obby::line::operator=(const line& other)
{
	m_line = other.m_line;
	m_authors = other.m_authors;

	return *this;
}

void obby::line::append_packet(net6::packet& pack) const
{
	pack << m_line;
	for(std::vector<user_pos>::size_type i = 0; i < m_authors.size(); ++ i)
		pack << m_authors[i].position << m_authors[i].author;
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
	// TODO: Compress directly 4 great performance, something like a
	// private (anonymous namespace) function.
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
	// TODO: Is this really necessary if the current line is compressed?
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

