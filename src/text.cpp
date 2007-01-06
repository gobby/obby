/* libobby - Network text editing library
 * Copyright (C) 2005, 2006 0x539 dev group
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

#include "text.hpp"

namespace
{
	const obby::text::size_type CHUNK_INIT =
		~static_cast<obby::text::size_type>(0);
}

obby::text::chunk::chunk(const chunk& other):
	m_text(other.m_text), m_author(other.m_author)
{
}

obby::text::chunk::chunk(const string_type& string,
                         const user* author):
	m_text(string),
	m_author(author)
{
}

obby::text::chunk::chunk(const net6::packet& pack,
                         unsigned int& index,
                         const user_table& table):
	m_text(pack.get_param(index + 0).as<std::string>() ),
	m_author(
		pack.get_param(index + 1).as<const user*>(
			::serialise::hex_context<const user*>(table)
		)
	)
{
	index += 2;
}

obby::text::chunk::chunk(const serialise::object& obj,
                         const user_table& table):
	m_text(obj.get_required_attribute("content").as<std::string>() ),
	m_author(
		obj.get_required_attribute("author").as<const user*>(
			::serialise::context<const user*>(table)
		)
	)
{
}

void obby::text::chunk::serialise(serialise::object& obj)
{
	obj.add_attribute("content").set_value(m_text);
	obj.add_attribute("author").set_value(m_author);
}

void obby::text::chunk::append_packet(net6::packet& pack)
{
	pack << m_text << m_author;
}

void obby::text::chunk::append(const string_type& text)
{
	m_text.append(text);
}

void obby::text::chunk::insert(size_type pos, const string_type& text)
{
	m_text.insert(pos, text);
}

void obby::text::chunk::erase(size_type pos, size_type len)
{
	m_text.erase(pos, len);
}

const obby::text::string_type& obby::text::chunk::get_text() const
{
	return m_text;
}

const obby::user* obby::text::chunk::get_author() const
{
	return m_author;
}

obby::text::size_type obby::text::chunk::get_length() const
{
	return m_text.length();
}

obby::text::text():
	m_max_chunk(CHUNK_INIT)
{
}

obby::text::text(const text& other):
	m_max_chunk(other.m_max_chunk)
{
	for(list_type::const_iterator iter = other.m_chunks.begin();
	    iter != other.m_chunks.end();
	    ++ iter)
	{
		m_chunks.push_back(new chunk(**iter) );
	}
}

obby::text::text(const string_type& string,
                 const user* author):
	m_max_chunk(CHUNK_INIT)
{
	m_chunks.push_back(new chunk(string, author) );
}

obby::text::text(const net6::packet& pack,
                 unsigned int& index,
                 const user_table& table):
	m_max_chunk(CHUNK_INIT)
{
	unsigned int count = pack.get_param(index ++).as<unsigned int>();
	for(unsigned int i = 0; i < count; ++ i)
		m_chunks.push_back(new chunk(pack, index, table) );
}

obby::text::text(const serialise::object& obj,
                 const user_table& table)
{
	for(serialise::object::child_iterator iter = obj.children_begin();
	    iter != obj.children_end();
	    ++ iter)
	{
		if(iter->get_name() == "chunk")
		{
			m_chunks.push_back(new chunk(*iter, table) );
		}
		else
		{
			// TODO: unexpected_child_error
			format_string str(_("Unexpected child node: '%0%'") );
			str << iter->get_name();
			throw serialise::error(str.str(), iter->get_line() );
		}
	}
}

obby::text::~text()
{
	clear();
}

obby::text& obby::text::operator=(const text& other)
{
	if(&other == this) return *this;

	clear();
	m_max_chunk = other.m_max_chunk;

	for(list_type::const_iterator iter = other.m_chunks.begin();
	    iter != other.m_chunks.end();
	    ++ iter)
	{
		m_chunks.push_back(new chunk(**iter) );
	}

	return *this;
}

void obby::text::serialise(serialise::object& obj)
{
	for(list_type::iterator it = m_chunks.begin();
	    it != m_chunks.end();
	    ++ it)
	{
		serialise::object& part = obj.add_child();
		part.set_name("chunk");
		(*it)->serialise(part);
//		part.add_attribute("content").set_value( (*it)->get_text() );
//		part.add_attribute("author").set_value( (*it)->get_author() );
	}
}

void obby::text::append_packet(net6::packet& pack)
{
	pack << m_chunks.size();
	for(list_type::iterator it = m_chunks.begin();
	    it != m_chunks.end();
	    ++ it)
	{
		(*it)->append_packet(pack);
	}
}

void obby::text::clear()
{
	for(list_type::iterator it = m_chunks.begin();
	    it != m_chunks.end();
	    ++ it)
	{
		delete *it;
	}

	m_chunks.clear();
}

obby::text obby::text::substr(size_type pos, size_type len)
{
	size_type cur_len = 0;
	chunk* cur_chunk = NULL;
	text new_text;

	new_text.m_max_chunk = m_max_chunk;
	for(list_type::iterator it = m_chunks.begin();
	    it != m_chunks.end();
	    ++ it)
	{
		size_type chunk_len = (*it)->get_length();
		cur_len += (*it)->get_text().length();

		if(cur_len <= pos)
			continue;

		// Chunk is part of substr
		size_type begin = 0;
		if(cur_len - chunk_len < pos)
			begin = pos - (cur_len - chunk_len);

		size_type end = chunk_len;
		if(len != npos && cur_len >= pos + len)
			end = cur_len - (pos + len);

		// Not necessary since m_max_chunk is copied from *this
		//if(end - begin > new_text.m_max_chunk)
		//	end = begin + new_text.m_max_chunk;

		if(cur_chunk != NULL &&
		   cur_chunk->get_author() == (*it)->get_author() &&
		   cur_chunk->get_length() < new_text.m_max_chunk)
		{
			// Merge with current
			cur_chunk->append(
				(*it)->get_text().substr(
					begin,
					end - begin
				)
			);
		}
		else
		{
			// No current chunk, make new one
			cur_chunk = new chunk(
				(*it)->get_text().substr(begin, end - begin),
				(*it)->get_author()
			);

			new_text.m_chunks.push_back(cur_chunk);
		}

		if(len != npos && cur_len >= pos + len)
			break;
	}

	return new_text;
}

void obby::text::insert(size_type pos,
                        const string_type& str,
                        const user* author)
{
	// TODO: Optimize to have not to construct a text object here!
	insert(pos, text(str, author) );
}

void obby::text::insert(size_type pos,
                        const text& str)
{
	size_type cur_len = 0;
	list_type::iterator it;
	for(it = m_chunks.begin(); it != m_chunks.end(); ++ it)
	{
		cur_len += (*it)->get_text().length();
		if(cur_len >= pos)
			break;
	}

	chunk* cur_chunk = NULL;
	list_type::iterator ins_pos = it;
	size_type chunk_pos = 0;

	if(it == m_chunks.end() )
	{
		// Insert first chunk
		if(pos != 0)
		{
			// No appropriate position found
			throw std::logic_error(
				"obby::text::insert:\n"
				"pos exceeds text's size"
			);
		}
	}
	else
	{
		cur_chunk = *it;
		chunk_pos = pos - (cur_len - (*it)->get_length() );
		++ ins_pos;
	}

	for(list_type::const_iterator it = str.m_chunks.begin();
	    it != str.m_chunks.end();
	    ++ it)
	{
		// No chunk available
		if(cur_chunk == NULL)
		{
			cur_chunk = new chunk(**it);
			m_chunks.insert(ins_pos, cur_chunk);
			chunk_pos = (*it)->get_length();
		}
		// Merge with previous chunk if possible
		else if(cur_chunk->get_author() == (*it)->get_author() &&
		        cur_chunk->get_length() + (*it)->get_length() <=
			m_max_chunk)
		{
			cur_chunk->insert(
				chunk_pos,
				(*it)->get_text()
			);

			chunk_pos += (*it)->get_length();
		}
		// Split current chunk if necessary
		else if(cur_chunk->get_author() != (*it)->get_author() &&
		        chunk_pos < cur_chunk->get_length() )
		{
			chunk* new_chunk = new chunk(
				cur_chunk->get_text().substr(chunk_pos),
				cur_chunk->get_author()
			);

			ins_pos = m_chunks.insert(ins_pos, new_chunk);
			cur_chunk->erase(chunk_pos);

			cur_chunk = new chunk(**it);
			m_chunks.insert(ins_pos, cur_chunk);
			chunk_pos = (*it)->get_length();
		}
		else
		{
			cur_chunk = new chunk(**it);
			m_chunks.insert(ins_pos, cur_chunk);
			chunk_pos = (*it)->get_length();
		}
	}

	// Merge ins_pos with cur_chunk if possible
	if(ins_pos != m_chunks.end() &&
	   cur_chunk->get_author() == (*ins_pos)->get_author() &&
	   cur_chunk->get_length() + (*ins_pos)->get_length() < m_max_chunk)
	{
		cur_chunk->append( (*ins_pos)->get_text() );

		delete *ins_pos;
		m_chunks.erase(ins_pos);
	}
}

obby::text::chunk_iterator obby::text::chunk_begin() const
{
	return chunk_iterator(m_chunks.begin() );
}

obby::text::chunk_iterator obby::text::chunk_end() const
{
	return chunk_iterator(m_chunks.end() );
}

#if 0
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

bool obby::line::operator<(const std::string& other) const
{
	return m_line < other;
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
#endif
