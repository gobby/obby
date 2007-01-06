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
	typedef duplex_signal<
		sigc::signal<void, position, const std::string&, const user*>
	> signal_insert_type;

	typedef duplex_signal<
		sigc::signal<void, position, position, const user*>
	> signal_erase_type;

	typedef text::chunk_iterator chunk_iterator;

	/** @brief Default constructor, creates an empty document.
	 */
	document();

	/** @brief Clears the document content.
	 */
	void clear();

	/** @brief Returns the size (in bytes) of the document.
	 */
	position size() const;

	/** @brief Returns the whole document as a string.
	 */
	std::string get_text() const;

	/** @brief Extracts a part from the document.
	 */
	text get_slice(position pos,
	               position len) const;

	/** @brief Inserts text into the document.
	 */
	void insert(position pos,
	            const text& str);

	/** @brief Erases text from the document.
	 */
	void erase(position pos,
	           position len);

	/** @brief Returns the beginning of the chunk list.
	 */
	chunk_iterator chunk_begin() const;

	/** @brief Returns the end of the chunk list.
	 */
	chunk_iterator chunk_end() const;

	/** @brief Signal that is emitted when text has been inserted into
	 * the document.
	 */
	signal_insert_type insert_event() const;

	/** @brief Signal that is emitted when text has been erased from
	 * the document.
	 */
	signal_erase_type erase_event() const;

protected:
	text m_text;

	signal_insert_type m_signal_insert;
	signal_erase_type m_signal_erase;
};

}

#endif // _OBBY_DOCUMENT_HPP_
