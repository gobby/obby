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
#include "user.hpp"
#include "line.hpp"

namespace obby
{

/** Contains the content of a document. Note that all positions pointing into
 * the document refer to byte offsets.
 */
class document : private net6::non_copyable
{
public:
	typedef duplex_signal<
		sigc::signal<void, position, const std::string&, const user*>
	> signal_insert_type;
	typedef duplex_signal<
		sigc::signal<void, position, position, const user*>
	> signal_delete_type;

	document();

	/** Returns the whole content of the document.
	 */
	std::string get_text() const;

	/** Returns a part of the document's contents.
	 */
	std::string get_slice(position from, position len) const;

	/** Inserts text into the document.
	 * @param pos Position where to insert text.
	 * @param text Text to insert.
	 * @param author User that has written this text.
	 */
	void insert(position pos, const std::string& text, const user* author);

	/** Removes text from the document.
	 * @param pos Beginning of the range where to delete text.
	 * @param len Amount of bytes to delete.
	 * @param author User who deleted the text.
	 */
	void erase(position pos, position len, const user* author);

	/** Signal which will be emitted if text has been inserted into the
	 * document.
	 */
	signal_insert_type insert_event() const;

	/** Signal which will be emitted if text has been deleted from the
	 * document.
	 */
	signal_delete_type delete_event() const;

	/** Clears all the lines in the document. Note that a document with no
	 * lines is invalid and cannot be operated with insert() or erase().
	 * Insert at least one line with add_line() after the call to this
	 * function. Note that a call to this function does not emit
	 * insert or erase signals.
	 */
	void clear_lines();

	/** Adds a line to the document. Note that a call to this function does
	 * not emit insert or erase signals.
	 */
	void add_line(const line& line);

	/** Returns the given line of text.
	 */
	const line& get_line(unsigned int index) const;

	/** Returns the amount of lines in the buffer.
	 */
	unsigned int get_line_count() const;

	/** Converts a row/column pair to a obby::position.
	 */
	position coord_to_position(unsigned int row, unsigned int col) const;

	/** Convert an obby::position to a row/column pair.
	 */
	void position_to_coord(position pos,
	                       unsigned int& row,
	                       unsigned int& col) const;

	/** Returns an obby::position pointing at the end of the buffer.
	 */
	position position_eob() const;

protected:
	// TODO: Add history
	std::vector<line> m_lines;

	signal_insert_type m_signal_insert;
	signal_delete_type m_signal_delete;
};

}

#endif // _OBBY_DOCUMENT_HPP_
