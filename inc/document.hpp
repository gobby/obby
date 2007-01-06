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

namespace obby
{

/** Abstract base class for obby documents. A document contains an amount of
 * text that is synchronized to all other participiants in the obby session.
 */
	
class document : private net6::non_copyable
{
public:
	typedef sigc::signal<void, const insert_record&> signal_insert_type;
	typedef sigc::signal<void, const delete_record&> signal_delete_type;

	document(unsigned int id);
	virtual ~document();

	/** Returns a unique ID for this document.
	 */
	unsigned int get_id() const;

	/** Returns the whole content of the document.
	 */
	std::string get_whole_buffer() const;

	/** Returns a part of the document's contents.
	 */
	std::string get_sub_buffer(position from, position to) const;

	/** Inserts <em>text</em> at <em>pos</em> and synchronizes this change
	 * to other users.
	 */
	virtual void insert(position pos, const std::string& text) = 0;

	/** Remove the text at the speciefied position and synchronizes this
	 * change to other users.
	 */
	virtual void erase(position from, position to) = 0;

	/** Inserts text without syncing it to other users. USE WITH CARE!
	 */
	void insert_nosync(position pos, const std::string& text);

	/** Removes text without syncing it to other users. USE WITH CARE!
	 */
	void erase_nosync(position from, position to);

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

	/** Called by the buffer if another user changed anything.
	 */
	virtual void on_net_record(record& rec) = 0;
protected:
	unsigned int m_id;

	std::list<record*> m_history;
	unsigned int m_revision;

	std::vector<std::string> m_lines;

	signal_insert_type m_signal_insert;
	signal_delete_type m_signal_delete;
};

}

#endif // _OBBY_DOCUMENT_HPP_
