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

#include <cassert>
#include "buffer.hpp"

obby::buffer::buffer()
 : m_history(), m_revision(0)
{
}

obby::buffer::~buffer()
{
	std::list<record*>::iterator iter;
	for(iter = m_history.begin(); iter != m_history.end(); ++ iter)
		delete *iter;
}

const std::string& obby::buffer::get_whole_buffer() const
{
	return m_buffer;
}

std::string obby::buffer::get_sub_buffer(position from, position to) const
{
	assert(to >= from);
	return m_buffer.substr(from, to - from);
}

void obby::buffer::insert_nosync(position pos, const std::string& text)
{
	m_buffer.insert(pos, text);
}

void obby::buffer::erase_nosync(position from, position to)
{
	assert(to >= from);
	m_buffer.erase(from, to - from);
}

obby::buffer::signal_insert_type obby::buffer::insert_event() const
{
	return m_signal_insert;
}

obby::buffer::signal_delete_type obby::buffer::delete_event() const
{
	return m_signal_delete;
}

