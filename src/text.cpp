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

	inline obby::text::size_type CHUNK_SIZE(obby::text::size_type size)
	{
		return size == obby::text::npos ? CHUNK_INIT : size;
	}

	// find_chunk implementation to avoid code duplication for
	// iterator and const_iterator versions
	template<typename List, typename Iter>
	Iter find_chunk(List list, obby::text::size_type& pos)
	{
		for(Iter it = list.begin(); it != list.end(); ++ it)
		{
			if( (*it)->get_length() > pos)
				return it;
			else
				pos -= (*it)->get_length();
		}

		if(pos == 0) return list.end();

		throw std::logic_error(
			"obby::text::find_chunk:\n"
			"Requested position exceeds text's size"
		);
	}
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

void obby::text::chunk::serialise(serialise::object& obj) const
{
	obj.add_attribute("content").set_value(m_text);
	obj.add_attribute("author").set_value(m_author);
}

void obby::text::chunk::append_packet(net6::packet& pack) const
{
	pack << m_text << m_author;
}

void obby::text::chunk::prepend(const string_type& text)
{
	m_text.insert(0, text);
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

obby::text::text(size_type initial_chunk_size):
	m_max_chunk(CHUNK_SIZE(initial_chunk_size) )
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
                 const user* author,
                 size_type initial_chunk_size):
	m_max_chunk(CHUNK_SIZE(initial_chunk_size) )
{
	for(size_type n = 0; n < string.length(); ++ n)
	{
		size_type len = std::min(string.length() - n, m_max_chunk);
		m_chunks.push_back(new chunk(string.substr(n, len), author) );
	}
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
                 const user_table& table):
	m_max_chunk(CHUNK_INIT)
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

void obby::text::serialise(serialise::object& obj) const
{
	for(list_type::const_iterator it = m_chunks.begin();
	    it != m_chunks.end();
	    ++ it)
	{
		serialise::object& part = obj.add_child();
		part.set_name("chunk");
		(*it)->serialise(part);
	}
}

void obby::text::append_packet(net6::packet& pack) const
{
	pack << m_chunks.size();
	for(list_type::const_iterator it = m_chunks.begin();
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

obby::text obby::text::substr(size_type pos, size_type len) const
{
	text new_text;
	list_type::const_iterator iter = find_chunk(pos);

	chunk* prev_chunk = NULL;
	while( (len == npos || len > 0) && (iter != m_chunks.end()) )
	{
		chunk* cur_chunk = *iter;
		size_type count = cur_chunk->get_length() - pos;

		if(len != npos)
		{
			count = std::min(count, len);
			len -= count;
		}

		if(prev_chunk != NULL &&
		   prev_chunk->get_author() == cur_chunk->get_author() &&
		   prev_chunk->get_length() + cur_chunk->get_length() <=
		   m_max_chunk)
		{
			prev_chunk->append(
				cur_chunk->get_text().substr(pos, count)
			);
		}
		else
		{
			prev_chunk = new chunk(
				cur_chunk->get_text().substr(pos, count),
				cur_chunk->get_author()
			);

			new_text.m_chunks.push_back(prev_chunk);
		}

		++ iter; pos = 0;
	}

	if(len > 0 && len != npos)
	{
		throw std::logic_error(
			"obby::text::substr:\n"
			"len is out or range"
		);
	}

	return new_text;
}

void obby::text::insert(size_type pos,
                        const string_type& str,
                        const user* author)
{
	list_type::iterator ins_pos = find_chunk(pos);
	insert_chunk(ins_pos, pos, str, author);
}

void obby::text::insert(size_type pos,
                        const text& str)
{
	list_type::iterator ins_pos = find_chunk(pos);
	for(list_type::const_iterator it = str.m_chunks.begin();
	    it != str.m_chunks.end();
	    ++ it)
	{
		ins_pos = insert_chunk(
			ins_pos,
			pos,
			(*it)->get_text(),
			(*it)->get_author()
		);
	}
}

void obby::text::erase(size_type pos, size_type len)
{
	list_type::iterator ers_pos = find_chunk(pos);
	while( (len == npos || len > 0) && ers_pos != m_chunks.end() )
	{
		size_type count = (*ers_pos)->get_length() - pos;
		if(len != npos)
		{
			count = std::min(count, len);
			len -= count;
		}

		ers_pos = erase_chunk(ers_pos, pos, count);
		pos = 0;
	}

	if(len != npos && len > 0)
	{
		throw std::logic_error(
			"obby::text::erase:\n"
			"len is out of range"
		);
	}
}

void obby::text::append(const string_type& str,
                        const user* author)
{
	chunk* last_chunk = NULL;
	if(!m_chunks.empty() ) last_chunk = *m_chunks.rbegin();
	size_type pos = 0;

	// Merge beginning of str with last chunk if possible
	if(last_chunk != NULL &&
	   last_chunk->get_author() == author &&
	   last_chunk->get_length() < m_max_chunk)
	{
		pos = std::min(
			m_max_chunk - last_chunk->get_length(),
			str.length()
		);

		last_chunk->append(str.substr(0, pos) );
	}

	// Append rest of string
	for(; pos < str.length(); pos += m_max_chunk)
	{
		size_type count = std::min(str.length() - pos, m_max_chunk);
		m_chunks.push_back(new chunk(str.substr(pos, count), author) );
	}
}

void obby::text::append(const text& str)
{
	// Simply append all chunks of str
	for(list_type::const_iterator it = str.m_chunks.begin();
	    it != str.m_chunks.end();
	    ++ it)
	{
		append( (*it)->get_text(), (*it)->get_author() );
	}
}

void obby::text::prepend(const string_type& str,
                         const user* author)
{
	chunk* first_chunk = NULL;
	if(!m_chunks.empty() ) first_chunk = *m_chunks.begin();

	size_type len = str.length();

	// Prepend end of str to first chunk if possible
	if(first_chunk != NULL &&
	   first_chunk->get_author() == author &&
	   first_chunk->get_length() < m_max_chunk)
	{
		size_type count = std::min(
			m_max_chunk - first_chunk->get_length(),
			len
		);

		len -= count;
		first_chunk->prepend(str.substr(len, count) );
	}

	// Insert chunks before for the rest of str
	while(len > 0)
	{
		size_type count = std::min(
			len,
			m_max_chunk
		);

		len -= count;
		m_chunks.push_front(new chunk(str.substr(len, count), author));
	}
}

void obby::text::prepend(const text& str)
{
	for(list_type::const_reverse_iterator it = str.m_chunks.rbegin();
	    it != str.m_chunks.rend();
	    ++ it)
	{
		prepend( (*it)->get_text(), (*it)->get_author() );
	}
}

obby::text::size_type obby::text::length() const
{
	// TODO: Cache this value?
	size_type len = 0;
	for(list_type::const_iterator it = m_chunks.begin();
	    it != m_chunks.end();
	    ++ it)
	{
		len += (*it)->get_length();
	}

	return len;
}

bool obby::text::operator==(const text& other) const
{
	return compare(other) == EQUAL_OWNERMATCH;
}

bool obby::text::operator!=(const text& other) const
{
	return compare(other) != EQUAL_OWNERMATCH;
}

bool obby::text::operator<(const text& other) const
{
	return compare(other) == LESS;
}

bool obby::text::operator>(const text& other) const
{
	return compare(other) == GREATER;
}

bool obby::text::operator<=(const text& other) const
{
	return compare(other) != GREATER;
}

bool obby::text::operator>=(const text& other) const
{
	return compare(other) != LESS;
}

bool obby::text::operator==(const string_type& other) const
{
	return compare(other) == EQUAL;
}

bool obby::text::operator!=(const string_type& other) const
{
	return compare(other) != EQUAL;
}

bool obby::text::operator<(const string_type& other) const
{
	return compare(other) == LESS;
}

bool obby::text::operator>(const string_type& other) const
{
	return compare(other) == GREATER;
}

bool obby::text::operator<=(const string_type& other) const
{
	return compare(other) != GREATER;
}

bool obby::text::operator>=(const string_type& other) const
{
	return compare(other) != LESS;
}

bool obby::text::empty() const
{
	return m_chunks.empty();
}

obby::text::chunk_iterator obby::text::chunk_begin() const
{
	return chunk_iterator(m_chunks.begin() );
}

obby::text::chunk_iterator obby::text::chunk_end() const
{
	return chunk_iterator(m_chunks.end() );
}

void obby::text::set_max_chunk_size(size_type max_chunk)
{
	m_max_chunk = max_chunk;
	if(m_chunks.empty() ) return;

	// Merge and/or split current chunks
	list_type::iterator next = m_chunks.begin(); ++ next;
	for(list_type::iterator it = m_chunks.begin();
	    it != m_chunks.end();
	    ++ it, ++ next)
	{
		chunk* cur_chunk = *it;

		chunk* next_chunk = NULL;
		if(next != m_chunks.end() ) next_chunk = *next;

		// Split current chunk if necessary
		if(cur_chunk->get_length() > m_max_chunk)
		{
			size_type pos = m_max_chunk;
			while(cur_chunk->get_length() - pos > 0)
			{
				// Merge with next if possible
				if(next_chunk != NULL &&
				   next_chunk->get_author() ==
				   cur_chunk->get_author() &&
				   cur_chunk->get_length() - pos +
				   next_chunk->get_length() <= m_max_chunk)
				{
					// Note that this could also be done
					// in the merging process below but
					// then we would split the chunk up
					// just to merge it then...
					next_chunk->prepend(
						cur_chunk->get_text().substr(
							pos
						)
					);

					pos += (cur_chunk->get_length() - pos);
				}
				// Split otherwise
				else
				{
					size_type len = std::min(
						cur_chunk->get_length() - pos,
						m_max_chunk
					);

					const std::string& text =
						cur_chunk->get_text();

					it = m_chunks.insert(
						next,
						new chunk(
							text.substr(
								pos,
								len
							),
							cur_chunk->get_author()
						)
					);

					pos += len;
				}

			}

			// Remove splitted/merged stuff from current one
			cur_chunk->erase(m_max_chunk);
			cur_chunk = *it;
		}
		// Merge chunk with next
		else if(next_chunk != NULL &&
		        cur_chunk->get_author() == next_chunk->get_author() &&
		        cur_chunk->get_length() + next_chunk->get_length() <=
		        m_max_chunk)
		{
			cur_chunk->append(next_chunk->get_text() );

			delete next_chunk;
			next = m_chunks.erase(next);
		}
	}
}

obby::text::operator string_type() const
{
	string_type str;
	str.reserve(length() );

	for(list_type::const_iterator it = m_chunks.begin();
	    it != m_chunks.end();
	    ++ it)
	{
		str.append( (*it)->get_text() );
	}

	return str;
}

obby::text::list_type::iterator
obby::text::find_chunk(size_type& pos)
{
	return ::find_chunk<list_type&, list_type::iterator>(
		m_chunks,
		pos
	);
}

obby::text::list_type::const_iterator
obby::text::find_chunk(size_type& pos) const
{
	return ::find_chunk<const list_type&, list_type::const_iterator>(
		m_chunks,
		pos
	);
}

obby::text::list_type::iterator
obby::text::insert_chunk(list_type::iterator chunk_it,
                         size_type& chunk_pos,
                         const string_type& str,
                         const user* author)
{
	chunk* cur_chunk = NULL;
	if(chunk_it != m_chunks.end() ) cur_chunk = *chunk_it;

	list_type::iterator ins_pos = chunk_it;

	// Get previous chunk
	chunk* prev_chunk = NULL;
	list_type::iterator prev_pos = ins_pos;
	if(prev_pos != m_chunks.begin() )
	{
		-- prev_pos;
		prev_chunk = *prev_pos;
	}

	// Merge with previous
	if(prev_chunk != NULL &&
	   chunk_pos == 0 &&
	   author == prev_chunk->get_author() &&
	   str.length() + prev_chunk->get_length() <= m_max_chunk)
	{
		prev_chunk->append(str);
		return chunk_it;
	}
	else if(cur_chunk == NULL)
	{
		// Insertion at end (no current chunk) and cannot merge with
		// previous: Need to create a new one, do nothing here - this
		// is done below
	}
	// Merge with current
	else if(author == cur_chunk->get_author() &&
	        str.length() + cur_chunk->get_length() <= m_max_chunk)
	{
		cur_chunk->insert(chunk_pos, str);
		chunk_pos += str.length();
		return chunk_it;
	}
	// Insert new chunk after current chunk if str is inserted at
	// end of current chunk
	else if(chunk_pos == cur_chunk->get_length() )
	{
		++ ins_pos;
	}
	// Insert new chunk before current chunk if str is inserted at
	// the beginning of current chunk
	else if(chunk_pos > 0)
	{
		// Split up otherwise
		chunk* new_chunk = new chunk(
			cur_chunk->get_text().substr(chunk_pos),
			cur_chunk->get_author()
		);

		cur_chunk->erase(chunk_pos);
		chunk_pos = 0;

		++ ins_pos;
		ins_pos = m_chunks.insert(
			ins_pos,
			new_chunk
		);

		// Try to merge with both chunks - they may be smaller
		// and thus fit into max chunk size
		if(cur_chunk->get_author() == author)
		{
			if(cur_chunk->get_length() + str.length() <=
			   m_max_chunk)
			{
				cur_chunk->append(str);
				chunk_pos = cur_chunk->get_length();
				-- ins_pos;
				return ins_pos;
			}
			else if(new_chunk->get_length() +
			        str.length() <= m_max_chunk)
			{
				new_chunk->prepend(str);
				chunk_pos = str.length();
				return ins_pos;
			}
		}

		// If not insert another new chunk between
		// the chunks split up - ins_pos points already
		// to the correct position
	}

	// Insert one new chunk if str fits into
	if(str.length() <= m_max_chunk)
	{
		chunk_pos = 0;
		m_chunks.insert(ins_pos, new chunk(str, author) );
		return ins_pos;
	}
	else
	{
		// Make multiple chunks otherwise
		// TODO: Fill up previous chunk if author matches and ins_pos
		// is != m_chunks.begin()
		cur_chunk = ( (ins_pos == m_chunks.end()) ? NULL : *ins_pos);
		for(size_type n = 0; n < str.length(); n += m_max_chunk)
		{
			size_type len = std::min(str.length() - n, m_max_chunk);

			// Merge with next
			if(cur_chunk &&
			   cur_chunk->get_author() == author &&
			   len + cur_chunk->get_length() <= m_max_chunk)
			{
				// Must be last chunk to insert since all
				// others are m_max_chunk in size and thus
				// may not be merged
				cur_chunk->prepend(str.substr(n, len) );
				chunk_pos = len;
				return ins_pos;
			}
			else
			{
				/*result =*/ m_chunks.insert(
					ins_pos,
					new chunk(str.substr(n, len), author)
				);
			}
		}

		chunk_pos = 0;
		return ins_pos;
	}
}

obby::text::list_type::iterator
obby::text::erase_chunk(list_type::iterator chunk_it,
                        size_type pos,
                        size_type len)
{
	chunk* prev_chunk = NULL;
	chunk* next_chunk = NULL;

	list_type::iterator prev_it = chunk_it;
	if(prev_it != m_chunks.begin() ) { -- prev_it; prev_chunk = *prev_it; }

	list_type::iterator next_it = chunk_it;
	++ next_it;
	if(next_it != m_chunks.end() ) { next_chunk = *next_it; }

	chunk* cur_chunk = *chunk_it;
	//if(len == npos) len = cur_chunk->get_length() - pos;

	if(pos + len > cur_chunk->get_length() )
	{
		throw std::logic_error(
			"obby::text::erase_chunk:\n"
			"Chunk len exceeded"
		);
	}

	// Complete erasure
	if(len == cur_chunk->get_length() )
	{
		delete cur_chunk;
		m_chunks.erase(chunk_it);

		// Merge surrounding chunks if possible
		if(next_chunk != NULL && prev_chunk != NULL &&
		   next_chunk->get_author() == prev_chunk->get_author() &&
		   next_chunk->get_length() + prev_chunk->get_length() <
		   m_max_chunk)
		{
			prev_chunk->append(next_chunk->get_text() );

			delete next_chunk;
			next_it = m_chunks.erase(next_it);
		}

		return next_it;
	}

	// Merge with previous
	if(prev_chunk != NULL &&
	   prev_chunk->get_author() == cur_chunk->get_author() &&
	   cur_chunk->get_length() - len + prev_chunk->get_length() <
	   m_max_chunk)
	{
		if(pos > 0)
		{
			prev_chunk->append(
				cur_chunk->get_text().substr(0, pos)
			);
		}

		if(pos + len < cur_chunk->get_length() )
		{
			prev_chunk->append(
				cur_chunk->get_text().substr(pos + len)
			);
		}

		delete cur_chunk;
		m_chunks.erase(chunk_it);

		// Merge result with next if possible
		if(next_chunk != NULL &&
		   prev_chunk->get_author() == next_chunk->get_author() &&
		   prev_chunk->get_length() + next_chunk->get_length() <=
		   m_max_chunk)
		{
			prev_chunk->append(next_chunk->get_text() );

			delete next_chunk;
			next_it = m_chunks.erase(next_it);
		}

		return next_it;
	}

	// Merge with next
	if(next_chunk != NULL &&
	   next_chunk->get_author() == cur_chunk->get_author() &&
	   cur_chunk->get_length() - len + next_chunk->get_length() <
	   m_max_chunk)
	{
		if(pos + len < cur_chunk->get_length() )
		{
			next_chunk->prepend(
				cur_chunk->get_text().substr(pos)
			);
		}

		if(pos > 0)
		{
			next_chunk->prepend(
				cur_chunk->get_text().substr(0, pos)
			);
		}

		delete cur_chunk;
		m_chunks.erase(chunk_it);

		// No need to try to merge with previous since the check
		// above would already have done it.

		++ next_it;
		return next_it;
	}

	// No merging possible...
	cur_chunk->erase(pos, len);
	return next_it;
}

obby::text::compare_result obby::text::compare(const text& other) const
{
	list_type::const_iterator it1 = m_chunks.begin();
	list_type::const_iterator it2 = other.m_chunks.begin();

	size_type pos1 = 0, pos2 = 0;
	bool author_match = true;

	while(it1 != m_chunks.end() && it2 != other.m_chunks.end() )
	{
		// Authors do not match: Remember this for later use. If
		// the strings differ LESS or GREATER is returned, author
		// match does only play a role when the text content is equal
		if( (*it1)->get_author() != (*it2)->get_author() )
			author_match = false;

		size_type len = std::min(
			(*it1)->get_length() - pos1,
			(*it2)->get_length() - pos2
		);

		int res = (*it1)->get_text().compare(
			pos1,
			len,
			(*it2)->get_text(),
			pos2,
			len
		);

		if(res != 0) return res < 0 ? LESS : GREATER;

		pos1 += len;
		pos2 += len;

		if(pos1 == (*it1)->get_length() )
			{ ++ it1; pos1 = 0; }
		if(pos2 == (*it2)->get_length() )
			{ ++ it2; pos2 = 0; }
	}

	// *this has more length than other
	if(it1 != m_chunks.end() )
		return GREATER;
	// other has more length
	else if(it2 != m_chunks.end() )
		return LESS;
	// both text's content are equal
	else
		return author_match ? EQUAL_OWNERMATCH : EQUAL;
}

obby::text::compare_result obby::text::compare(const string_type& text) const
{
	size_type pos = 0;
	for(list_type::const_iterator it = m_chunks.begin();
	    it != m_chunks.end();
	    ++ it)
	{
		size_type len = (*it)->get_length();
		int res = text.compare(pos, len, (*it)->get_text());
		if(res != 0) return res < 0 ? LESS : GREATER;
		pos += len;
	}

	return EQUAL;
}
