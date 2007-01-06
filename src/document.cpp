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

#include "common.hpp"
#include "document.hpp"

obby::document::document():
	m_text(0x3fff)
{
}

void obby::document::clear()
{
	m_text.clear();
}

obby::position obby::document::size() const
{
	return m_text.length();
}

std::string obby::document::get_text() const
{
	return m_text;
}

obby::text obby::document::get_slice(position pos,
                                     position len) const
{
	return m_text.substr(pos, len);
}

void obby::document::insert(position pos,
                            const text& str)
{
	m_signal_insert.before().emit(pos, str, str.chunk_begin()->get_author() );
	m_text.insert(pos, str);
	m_signal_insert.after().emit(pos, str, str.chunk_begin()->get_author() );
}

void obby::document::erase(position pos,
                           position len)
{
	m_signal_erase.before().emit(pos, len, NULL);
	m_text.erase(pos, len);
	m_signal_erase.after().emit(pos, len, NULL);
}

obby::document::chunk_iterator obby::document::chunk_begin() const
{
	return m_text.chunk_begin();
}

obby::document::chunk_iterator obby::document::chunk_end() const
{
	return m_text.chunk_end();
}

obby::document::signal_insert_type obby::document::insert_event() const
{
	return m_signal_insert;
}

obby::document::signal_erase_type obby::document::erase_event() const
{
	return m_signal_erase;
}
