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

#include <string>
#include <list>
#include <sigc++/signal.h>
#include <net6/non_copyable.hpp>
#include "position.hpp"
#include "record.hpp"
#include "insert_record.hpp"
#include "delete_record.hpp"
#include "user_table.hpp"
#include "line.hpp"

namespace obby
{

/** Abstract base class for obby documents. A document contains an amount of
 * text that is synchronised to all other participants in the obby session.
 */
	
class document : private net6::non_copyable
{
public:
	typedef sigc::signal<void, const insert_record&> signal_insert_type;
	typedef sigc::signal<void, const delete_record&> signal_delete_type;

	document(unsigned int id, const user_table& usertable);
	virtual ~document();

	/** Returns a unique ID for this document.
	 */
	unsigned int get_id() const;

	/** Returns the title set for this document.
	 */
	std::string get_title() const;

	/** Sets a new title for this document.
	 */
	void set_title(const std::string& title);

	/** Returns the whole content of the document.
	 */
	std::string get_whole_buffer() const;

	/** Returns a part of the document's contents.
	 */
	std::string get_sub_buffer(position from, position to) const;

	/** Inserts <em>text</em> at <em>pos</em> and synchronises this change
	 * to other users.
	 */
	virtual void insert(position pos, const std::string& text) = 0;

	/** Remove the text at the speciefied position and synchronises this
	 * change to other users.
	 */
	virtual void erase(position from, position to) = 0;

	/** Inserts text without syncing it to other users. This is an low-level
	 * function libobby uses within itself, so USE WITH CARE! You may
	 * destroy the complete obby session by performing unsynced operations.
	 * @param pos Position where to insert text
	 * @param text text to insert
	 * @param author_id ID of the user who performed this operation.
	 */
	void insert_nosync(position pos, const std::string& text,
	                   unsigned int author_id);

	/** Removes text without syncing it to other users. This is an low-level
	 * function libobby uses within itself, so USE WITH CARE! You may
	 * destroy the complete obby session by performing unsynced operations.
	 * @param from Beginning of the position where to remove text
	 * @param to End of the area where to remove text
	 * @param author_id ID of the user who performed this operation.
	 */
	void erase_nosync(position from, position to, unsigned int author_id);

	/** Signal which will be emitted if text has been inserted into the
	 * document. A call to insert or insert_nosync will not emit this
	 * signal.
	 */
	signal_insert_type insert_event() const;

	/** Signal which will be emitted if text has been deleted from the
	 * document. erase and erase_nosync will not emit this signal.
	 */
	signal_delete_type delete_event() const;

	/** Returns the given line of text.
	 */
	std::string get_line(unsigned int index) const;

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

	/** Called by the buffer if another user changed anything.
	 */
	virtual void on_net_record(record& rec) = 0;
protected:
	unsigned int m_id;
	std::string m_title;

	std::list<record*> m_history;
	unsigned int m_revision;
	const user_table& m_usertable;

	std::vector<line> m_lines;

	signal_insert_type m_signal_insert;
	signal_delete_type m_signal_delete;
};

}

#endif // _OBBY_DOCUMENT_HPP_
