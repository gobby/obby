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
#include "line.hpp"

const obby::line::size_type obby::line::npos = obby::line::string_type::npos;

obby::line::line()
 : m_line(), m_authors()
{
}

obby::line::line(const string_type& text, const user_type& author)
 : m_line(text), m_authors()
{
	// It is just _one_ line.
	assert(text.find('\n') == string_type::npos);

	// Insert initial author
	user_pos pos = { &author, 0 };
	m_authors.push_back(pos);
}

obby::line::line(const net6::packet& pack, const user_table& usertable)
{
	if(pack.get_param_count() < 2) return;
	if(pack.get_param(0).get_type() != net6::packet::param::INT) return;
	if(pack.get_param(1).get_type() != net6::packet::param::STRING) return;

	// Parameter 0 is the document ID which we do not need here.
	m_line = pack.get_param(1).as_string();

	// We need even parameter count (pos->author, pos->author, etc.)
	if(pack.get_param_count() % 2) return;

	// Reserve space in author vector
	m_authors.reserve( (pack.get_param_count() - 2) / 2);
	
	// Add authors
	for(unsigned int i = 2; i < pack.get_param_count(); i += 2)
	{
		// Verify parameters
		if(pack.get_param(i).get_type() != net6::packet::param::INT)
			return;
		if(pack.get_param(i + 1).get_type() != net6::packet::param::INT)
			return;

		// Get position and author id
		unsigned int pos = pack.get_param(i).as_int();
		unsigned int user_id = pack.get_param(i + 1).as_int();
		const obby::user_table::user* author = usertable.find(user_id);
		assert(author != NULL);

		// Add to vector
		user_pos upos = { author, pos };
		m_authors.push_back(upos);
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
	// Search the correct position where to insert the new authors
	std::vector<user_pos>::iterator iter;
	// Count steps
	std::vector<user_pos>::size_type count = 0;
	for(iter = m_authors.begin(); iter != m_authors.end(); ++iter, ++count)
		if(iter->position >= pos)
			break;

	// Insert them
	m_authors.insert(iter, text.m_authors.begin(), text.m_authors.end() );

	// Adjust positions at the inserted position. std::vector<>::insert
	// does not return an iterator, so we need to find the position where
	// the new user_pos have been inserted
	std::vector<user_pos>::size_type i = 0;
	for(iter = m_authors.begin(); iter != m_authors.end(); ++ iter, ++ i)
		if(i >= count)
			iter->position += text.length();

	// Insert line content
	m_line.insert(pos, text.m_line);
}

void obby::line::insert(size_type pos, const string_type& text,
                        const user_type& author)
{
	insert(pos, line(text, author) );
}

void obby::line::append(const line& text)
{
	insert(m_line.length(), text);
}

void obby::line::append(const string_type& text, const user_type& author)
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

	// New line
	line new_line;
	// Set content
	new_line.m_line = m_line.substr(from, len);
	
	// Set authors
	std::vector<user_pos>::const_iterator iter;
	for(iter = m_authors.begin(); iter != m_authors.end(); ++ iter)
	{
		// Take only authors from within the given range
		if(iter->position >= from && iter->position < from + len)
		{
			// Adjust position
			user_pos new_pos = *iter;
			new_pos.position -= len;
			// Put into new line's authors list
			new_line.m_authors.push_back(new_pos);
		}
	}

	return new_line;
}

void obby::line::compress_authors()
{
	std::vector<user_pos>::iterator iter;
	std::vector<user_pos>::iterator prev_iter = m_authors.end();
	std::vector<user_pos>::iterator old_iter = m_authors.end();

	// Filter mutliple positions at the same position in the text
	for(iter = m_authors.begin(); iter != m_authors.end(); ++ iter)
	{
		// Ignore the first entry.
		if(prev_iter != m_authors.end() )
		{
			// Another position?
			if(iter->position > old_iter->position)
			{
				// Delete the ones before
				iter = m_authors.erase(old_iter, prev_iter);
				++ iter;
				old_iter = iter;
			}
		}
		else
		{
			// Initialize old_iter
			old_iter = iter;
		}

		prev_iter = iter;
	}

	// Filter the same author
	prev_iter = old_iter = m_authors.end();
	for(iter = m_authors.begin(); iter != m_authors.end(); ++ iter)
	{
		// Ignore the first entry.
		if(prev_iter != m_authors.end() )
		{
			// Another author?
			if(iter->author != old_iter->author)
			{
				iter = m_authors.erase(old_iter, iter);
				old_iter = iter;
			}
		}
		else
		{
			// Initialize old_iter
			old_iter = iter;
		}

		prev_iter = iter;
	}
}

net6::packet obby::line::to_packet(unsigned int document_id) const
{
	net6::packet pack("obby_sync_doc_line", net6::packet::DEFAULT_PRIORITY,
	                  2 + m_authors.size() * 2);
	pack << document_id << m_line;

	std::vector<user_pos>::const_iterator iter;
	for(iter = m_authors.begin(); iter != m_authors.end(); ++ iter)
		pack << static_cast<int>(iter->position) <<
			static_cast<int>(iter->author->get_id());

	return pack;
}

