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

#ifndef _OBBY_DOCUMENT_HPP_
#define _OBBY_DOCUMENT_HPP_

#include "position.hpp"
#include "duplex_signal.hpp"
#include "text.hpp"

namespace obby
{

/** @brief Default document that uses obby::text to store its content.
 */
class document: private net6::non_copyable
{
public:
	class chunk_iterator: public text::chunk_iterator
	{
	public:
		typedef text::chunk_iterator base_iterator;

		chunk_iterator(const base_iterator& iter);

		chunk_iterator& operator++();
		chunk_iterator operator++(int);

		bool operator==(const chunk_iterator& other) const;
		bool operator!=(const chunk_iterator& other) const;

		const std::string& get_text() const;
		const user* get_author() const;
	private:
		base_iterator m_iter;
	};

	class template_type { };

	/** @brief Default constructor, creates an empty document.
	 */
	document(const template_type& tmpl);

	/** @brief Returns TRUE when the document does not contain any text.
	 */
	bool empty() const;

	/** @brief Returns the size (in bytes) of the document.
	 */
	position size() const;

	/** @brief Extracts a part from the document.
	 */
	text get_slice(position pos,
	               position len) const;

	/** @brief Returns the contents of the document in a string.
	 */
	std::string get_text() const;

	/** @brief Inserts text into the document.
	 */
	void insert(position pos,
	            const text& str);

	/** @brief Inserts text written by <em>author</em>.
	 */
	void insert(position pos,
	            const std::string& str,
	            const user* author);

	/** @brief Erases text from the document.
	 */
	void erase(position pos,
	           position len);

	/** @brief Inserts the given text at the end of the document.
	 */
	void append(const text& str);

	/** @brief Inserts text written by <em>author</em> at the end
	 * of the document.
	 */
	void append(const std::string& str,
	            const user* author);

	/** @brief Returns the beginning of the chunk list.
	 */
	chunk_iterator chunk_begin() const;

	/** @brief Returns the end of the chunk list.
	 */
	chunk_iterator chunk_end() const;

protected:
	text m_text;
};

}

#endif // _OBBY_DOCUMENT_HPP_
