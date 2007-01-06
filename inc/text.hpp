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

namespace obby
{

/** @brief Obby text that stores which user wrote what.
 *
 * An obby text consists of a list of so-called chunks. A chunk is a part of
 * the text that a specified user has written. It is possible to iterate
 * through the chunks of the text to find out which user wrote what.
 *
 * It is also possible to limit the maximum chunk size (in bytes) to speed
 * up text manipulating in large text documents where a single user wrote
 * a large part. However, chunk size limitation has currently not been
 * tested and might be broken.
 *
 * Normally, the class tries to merge chunks from the same author if
 * the size limitation allows this. Such merging is performed when inserting
 * or deleting text. It ensures that the text does not consist of hundreds
 * of chunks but the algorithm is not optimal - meaning it does not find the
 * solution which needs the least number of chunks in any case. This would
 * require too much performance when text is inserted or deleted.
 */
class text
{
public:
	/** @brief String type used in chunks.
	 */
	typedef std::string string_type;

	/** @brief Size type.
	 */
	typedef string_type::size_type size_type;

	/** @brief Invalid position marker.
	 */
	static const size_type npos = string_type::npos;

protected:
	/** @brief Single text chunk.
	 */
	class chunk
	{
	public:
		chunk(const chunk& other);
		chunk(const string_type& string,
		      const user* author);

		/** @brief Reads a chunk from a net6::packet.
		 *
		 * The user table is used to lookup user IDs.
		 */
		chunk(const net6::packet& pack,
		      unsigned int& index,
		      const user_table& table);

		/** @brief Reads a chunk from a serialisation object.
		 *
		 * The user table is used to lookup user IDs.
		 */
		chunk(const serialise::object& obj,
		      const user_table& table);

		/** @brief Serialises the chunk to the given serialisation
		 * object.
		 */
		void serialise(serialise::object& obj) const;

		/** @brief Reads the chunk from a net6::packet.
		 */
		void append_packet(net6::packet& pack) const;

		/** @brief Prepends text to the chunk.
		 */
		void prepend(const string_type& text);

		/** @brief Appends text to the chunk.
		 */
		void append(const string_type& text);

		/** @brief Inserts text into the chunk.
		 */
		void insert(size_type pos, const string_type& text);

		/** @brief Erases text from the chunk.
		 */
		void erase(size_type pos, size_type len = npos);

		/** @brief Returns the chunk's text.
		 */
		const string_type& get_text() const;

		/** @brief Returns the length of this chunk.
		 *
		 * Equivalent to get_text().length()
		 */
		size_type get_length() const;

		/** @brief Returns the user that has written this chunk.
		 */
		const user* get_author() const;
	protected:
		string_type m_text;
		const user* m_author;
	};

	typedef std::list<chunk*> list_type;

public:
	// TODO: Cache iterators to serveral points in the text (1/8, 1/4, ...)
	// to optimize insert, erase and other functions

	/** @brief Iterator type to iterate over the chunks of a text.
	 */
	typedef ptr_iterator<
		const chunk,
		list_type,
		list_type::const_iterator
	> chunk_iterator;

	text(size_type initial_chunk_size = npos);
	text(const text& other);
	text(const string_type& string,
	     const user* author,
	     size_type initial_chunk_size = npos);

	/** @brief Reads a text from a net6::packet.
	 */
	text(const net6::packet& pack,
	     unsigned int& index,
	     const user_table& table);

	/** @brief Reads a text from a serialisation object.
	 */
	text(const serialise::object& obj,
	     const user_table& table);
	~text();

	text& operator=(const text& other);

	/** @brief Writes the text to a serialisation object.
	 */
	void serialise(serialise::object& obj) const;

	/** @brief Appends the text to a net6 packet.
	 */
	void append_packet(net6::packet& pack) const;

	/** @brief Removes any chunks in the text.
	 */
	void clear();

	/** @brief Extracts a subtext from this text.
	 */
	text substr(size_type pos,
	            size_type len = npos) const;

	/** @brief Inserts a string into the text.
	 */
	void insert(size_type pos,
	            const string_type& str,
	            const user* author);

	/** @brief Inserts another text into this one.
	 */
	void insert(size_type pos,
	            const text& str);

	/** @brief Erases a substring from the text.
	 */
	void erase(size_type pos,
	           size_type len = npos);

	/** @brief Appends a string to the text.
	 *
	 * This function should be used instead of insert if text is inserted
	 * at the end of the text because no lookup of the insertion point
	 * in the chunk list has to be performed.
	 */
	void append(const string_type& str,
	            const user* author);

	/** @brief Appends another text to this text.
	 *
	 * This function should be used instead of insert if text is inserted
	 * at the end of the text because no lookup of the insertion point
	 * in the chunk list has to be performed.
	 */
	void append(const text& str);

	/** @brief Prepends a string to this text.
	 */
	void prepend(const string_type& str,
	             const user* author);

	/** @brief Prepends another text to this text.
	 */
	void prepend(const text& str);

	/** @brief Returns the length of this text, in bytes.
	 */
	size_type length() const;

	/** @brief Returns TRUE if the text's contents are equal to other's
	 * and if the same users wrote the same chunks.
	 *
	 * Chunk size limitation between the texts may differ.
	 */
	bool operator==(const text& other) const;

	/** @brief Returns TRUE if the text's contents differ from other's or
	 * if a chunk has been written by another user.
	 *
	 * Chunk size limitaion between the texts may differ.
	 */
	bool operator!=(const text& other) const;

	/** @brief Relational operator, ignores chunk ownership.
	 */
	bool operator<(const text& other) const;

	/** @brief Relational operator, ignores chunk ownership.
	 */
	bool operator>(const text& other) const;

	/** @brief Relational operator, ignores chunk ownership.
	 */
	bool operator<=(const text& other) const;

	/** @brief Relational operator, ignores chunk ownership.
	 */
	bool operator>=(const text& other) const;

	/** @brief Checks whether this text's content matches other.
	 */
	bool operator==(const string_type& other) const;

	/** @brief Checks whether this text's content differs from other.
	 */
	bool operator!=(const string_type& other) const;

	/** @brief Relational operator.
	 */
	bool operator<(const string_type& other) const;

	/** @brief Relational operator.
	 */
	bool operator>(const string_type& other) const;

	/** @brief Relational operator.
	 */
	bool operator<=(const string_type& other) const;

	/** @brief Relational operator.
	 */
	bool operator>=(const string_type& other) const;

	/** @brief Returns TRUE if the text is empty e.g. has no chunks.
	 */
	bool empty() const;

	/** @brief Returns the beginning of tho chunk list.
	 */
	chunk_iterator chunk_begin() const;

	/** @brief Returns the end of tho chunk list.
	 */
	chunk_iterator chunk_end() const;

	/** @brief Changes the chunk size limitation.
	 *
	 * If the limitation is decreased, chunks exceeding the new limitation
	 * are splitted up, if the limition in increased, chunks may be merged.
	 */
	void set_max_chunk_size(size_type max_chunk);

	/** @brief Converts the text to a string, loosing chunk ownership.
	 */
	operator std::string() const;

protected:
	size_type m_max_chunk;
	list_type m_chunks;

private:
	/** @brief Internal function to find the chunk at the given position
	 * in the chunk list.
	 *
	 * pos is changed to point to the required position in the returned
	 * chunk.
	 */
	list_type::iterator find_chunk(size_type& pos);

	/** @brief Internal function to find the chunk at the given position
	 * in the chunk list.
	 *
	 * pos is changed to point to the required position in the returned
	 * chunk.
	 */
	list_type::const_iterator find_chunk(size_type& pos) const;

	/** @brief Inserts a chunk at the given position in the given chunk.
	 *
	 * The given chunk is splitten up to insert the new chunk, if
	 * necessary. Sometimes, the new text may be merged with surrounding
	 * chunks.
	 *
	 * The function changen chunk_pos to point behind the insertion
	 * point in the returned iterater. This may or may not be a newly
	 * created chunk.
	 */
	list_type::iterator insert_chunk(list_type::iterator chunk_it,
	                                 size_type& chunk_pos,
	                                 const string_type& str,
	                                 const user* author);

	/** @brief Erases the given range from the given chunk.
	 *
	 * If the beginning and/or end is erased the function tries to merge
	 * the chunk with surrounding chunks. It returns the chunk after the
	 * chunk where text has been erased from.
	 */
	list_type::iterator erase_chunk(list_type::iterator chunk_it,
	                                size_type pos,
	                                size_type len);

	/** @brief Result of a compare() call.
	 */
	enum compare_result {
		/** This text is relationally greater than the
		 * compared one.
		 */
		GREATER,

		/** Both texts are equal and all chunks are written by
		 * the same authers.
		 */
		EQUAL_OWNERMATCH,

		/** Both texts are equal, but chunk ownership differs.
		 */
		EQUAL,

		/** This text is relationally less than the
		 * compared one.
		 */
		LESS
	};

	compare_result compare(const text& other) const;
	compare_result compare(const string_type& text) const;
};

}

#endif // _OBBY_TEXT_HPP_
