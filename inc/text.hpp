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

#ifndef _OBBY_TEXT_HPP_
#define _OBBY_TEXT_HPP_

#include <string>
#include <list>
#include <net6/packet.hpp>
#include "ptr_iterator.hpp"
#include "user.hpp"

/** @brief Obby text that stores which user wrote what.
 */
namespace obby
{

class text
{
public:
	typedef std::string string_type;
	typedef string_type::size_type size_type;
	static const size_type npos = string_type::npos;

protected:
	class chunk
	{
	public:
		chunk(const chunk& other);
		chunk(const string_type& string,
		      const user* author);
		chunk(const net6::packet& pack,
		      unsigned int& index,
		      const user_table& table);
		chunk(const serialise::object& obj,
		      const user_table& table);

		void serialise(serialise::object& obj);
		void append_packet(net6::packet& pack);

		void prepend(const string_type& text);
		void append(const string_type& text);
		void insert(size_type pos, const string_type& text);
		void erase(size_type pos, size_type len = npos);

		const string_type& get_text() const;
		size_type get_length() const;
		const user* get_author() const;
	protected:
		string_type m_text;
		const user* m_author;
	};

	typedef std::list<chunk*> list_type;

public:
	// TODO: Cache iterators to serveral points in the text (1/8, 1/4, ...)
	// to optimize insert, erase and other functions
	typedef ptr_iterator<
		const chunk,
		list_type,
		list_type::const_iterator
	> chunk_iterator;

	text();
	text(const text& other);
	text(const string_type& string,
	     const user* author);
	text(const net6::packet& pack,
	     unsigned int& index,
	     const user_table& table);
	text(const serialise::object& obj,
	     const user_table& table);
	~text();

	text& operator=(const text& other);

	void serialise(serialise::object& obj);
	void append_packet(net6::packet& pack);

	void clear();

	text substr(size_type pos,
	            size_type len = npos);
	void insert(size_type pos,
	            const string_type& str,
	            const user* author);
	void insert(size_type pos,
	            const text& str);
	void erase(size_type pos,
	           size_type len = npos);

	size_type length() const;

	bool operator==(const text& other);
	bool operator!=(const text& other);
	bool operator==(const string_type& other);
	bool operator!=(const string_type& other);
	bool operator<(const string_type& other);
	bool operator>(const string_type& other);
	bool operator<=(const string_type& other);
	bool operator>=(const string_type& other);

	chunk_iterator chunk_begin() const;
	chunk_iterator chunk_end() const;

	void set_max_chunk_size(size_type max_chunk);

	operator const std::string&() const;

protected:
	size_type m_max_chunk;
	list_type m_chunks;

private:
	list_type::iterator insert_chunk(list_type::iterator chunk_it,
	                                 size_type& chunk_pos,
	                                 const string_type& str,
	                                 const user* author);

	list_type::iterator find_chunk(size_type& pos);
};

#if 0
class document;

/** Line in a obby document. It stores the content of the line and which user
 * edited which chunk of the line.
 */

class line
{
public:
	typedef std::string string_type;
	typedef string_type::size_type size_type;
	typedef user user_type;
	static const size_type npos;

protected:
	/** Internal struct used to store author and the position where the
	 * text in the line has been written by that user. Note that author
	 * may be NULL if the text was written by someone who has not a user
	 * object assigned to him (like a server_document).
	 */
	struct user_pos
	{
		const user_type* author;
		size_type position;
	};
public:
	/** TODO: Is there a need to use vector here?
	 * Concider using std::list...
	 */
	typedef std::vector<user_pos>::const_iterator author_iterator;

	/** Creates a new line.
	 */
	line();

	/** Creates a new line.
	 * @param text Initial line content
	 * @param author User who wrote this line
	 */
	line(const string_type& text, const user_type* author);

	/** Reads a line from a network packet as appended by append_packet
	 * @param pack The net6::packet to read the line from.
	 * @param index Parameter were to start to read.
	 * @param user_table obby::user_table where to find the authors of this
	 * line
	 */
	line(const net6::packet& pack,
	     unsigned int& index,
	     const user_table& user_table);

	/** Deserialises a line from a serialisation object.
	 */
	line(const serialise::object& obj,
	     const user_table& user_table);

	/** Copy constructor, it copies a line with its authors.
	 */
	line(const line& other);
	
	/** Copies a line with its authors.
	 */
	line& operator=(const line& other);

	/** Reserves space for <em>len</em> characters of the string chunk and
	 * <em>pos</em> user positions.
	 */
	void reserve(size_type len, size_type pos);

	/** Serialises the line into a serialisation object.
	 */
	void serialise(serialise::object& obj) const;

	/** Appends the line to a packet.
	 */
	void append_packet(net6::packet& pack) const;

	/** Returns the line contents.
	 */
	operator const std::string&() const;

	/** Returns the length of the line.
	 */
	size_type length() const;

	/** Insert another line into this one.
	 * @param pos Position where to insert text.
	 * @param text Line to insert.
	 */
	void insert(size_type pos, const line& text);

	/** Inserts text into the line.
	 * @param pos Position where to insert text.
	 * @param text Text to insert into the line.
	 * @param author User who has written the newly inserted text.
	 */
	void insert(size_type pos, const string_type& text,
	            const user_type* author);
	
	/** Appends another line to this one.
	 * @param text Line to append.
	 */
	void append(const line& text);
	
	/** Appends text to the line.
	 * @param text Text to append.
	 * @param author User who has written the new text.
	 */
	void append(const string_type& text, const user_type* author);

	/** Erases chunks of the line.
	 * @param from Position where to erase text.
	 * @param len Number of characters to erase.
	 */
	void erase(size_type from, size_type len = npos);

	/** Returns a sub string of the line.
	 */
	line substr(size_type from, size_type len = npos) const;

	/** Returns the begin of the author list.
	 */
	author_iterator author_begin() const;

	/** Returns the end of the author list.
	 */
	author_iterator author_end() const;

	/** TODO: Doc me. Implement others.
	 */
	bool operator<(const std::string& other) const;

protected:
	/** Internal function that simplifies the m_authors vector without
	 * losing data.
	 */
	void compress_authors();

	/** Line content.
	 */
	string_type m_line;

	/** Stores which line segments was written by which user.
	 */
	std::vector<user_pos> m_authors;
};
#endif

}

#endif // _OBBY_TEXT_HPP_

