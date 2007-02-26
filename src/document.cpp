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

obby::document::chunk_iterator::chunk_iterator(const base_iterator& iter):
	m_iter(iter)
{
}

obby::document::chunk_iterator& obby::document::chunk_iterator::operator++()
{
	++ m_iter;
	return *this;
}

obby::document::chunk_iterator obby::document::chunk_iterator::operator++(int)
{
	chunk_iterator temp(*this);
	++ *this;
	return temp;
}

bool obby::document::chunk_iterator::
	operator==(const chunk_iterator& other) const
{
	return m_iter == other.m_iter;
}

bool obby::document::chunk_iterator::
	operator!=(const chunk_iterator& other) const
{
	return m_iter != other.m_iter;
}

const std::string& obby::document::chunk_iterator::get_text() const
{
	return m_iter->get_text();
}

const obby::user* obby::document::chunk_iterator::get_author() const
{
	return m_iter->get_author();
}

obby::document::document(const template_type& tmpl):
	m_text() // (0x3fff) // TODO: Use chunk size limit, but make sure it works!
{
}

bool obby::document::empty() const
{
	return m_text.empty();
}

obby::position obby::document::size() const
{
	return m_text.length();
}

obby::text obby::document::get_slice(position pos,
                                     position len) const
{
	return m_text.substr(pos, len);
}

std::string obby::document::get_text() const
{
	return m_text;
}

void obby::document::insert(position pos,
                            const text& str)
{
	m_text.insert(pos, str);
	m_signal_changed.emit();
}

void obby::document::insert(position pos,
                            const std::string& str,
                            const user* author)
{
	m_text.insert(pos, str, author);
	m_signal_changed.emit();
}

void obby::document::erase(position pos,
                           position len)
{
	m_text.erase(pos, len);
	m_signal_changed.emit();
}

void obby::document::append(const text& str)
{
	m_text.append(str);
	m_signal_changed.emit();
}

void obby::document::append(const std::string& str,
                            const user* author)
{
	m_text.append(str, author);
	m_signal_changed.emit();
}

obby::document::chunk_iterator obby::document::chunk_begin() const
{
	return m_text.chunk_begin();
}

obby::document::chunk_iterator obby::document::chunk_end() const
{
	return m_text.chunk_end();
}

obby::document::signal_changed_type obby::document::changed_event() const
{
	return m_signal_changed;
}
