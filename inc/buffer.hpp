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

#ifndef _OBBY_BUFFER_HPP_
#define _OBBY_BUFFER_HPP_

#include <string>
#include <list>
#include <sigc++/signal.h>
#include "position.hpp"
#include "record.hpp"
#include "insert_record.hpp"
#include "delete_record.hpp"

namespace obby
{

class buffer
{
public:
	typedef sigc::signal<void, const insert_record&> signal_insert_type;
	typedef sigc::signal<void, const delete_record&> signal_delete_type;

	buffer();
	~buffer();

	const std::string& get_whole_buffer() const;
	std::string get_sub_buffer(position from, position to) const;

	virtual void insert(position pos, const std::string& text) = 0;
	virtual void erase(position from, position to) = 0;

	void insert_nosync(position pos, const std::string& text);
	void erase_nosync(position from, position to);

	signal_insert_type insert_event() const;
	signal_delete_type delete_event() const;

	std::string get_line(unsigned int index) const;
	unsigned int get_line_count() const;

	position coord_to_position(unsigned int x, unsigned int y) const;
	void position_to_coord(position pos, unsigned int& x,
	                       unsigned int& y) const;
protected:
	std::list<record*> m_history;
	unsigned int m_revision;

	std::string m_buffer;
	std::vector<position> m_lines;

	signal_insert_type m_signal_insert;
	signal_delete_type m_signal_delete;
};

}

#endif // _OBBY_BUFFER_HPP_
