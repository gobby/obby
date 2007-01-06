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

#include "host_document.hpp"
#include "host_buffer.hpp"

obby::host_document::host_document(unsigned int id, net6::host& host,
                                   const host_buffer& buf)
 : server_document(id, host, buf)
{
}

obby::host_document::~host_document()
{
}

const obby::host_buffer& obby::host_document::get_buffer() const
{
	return static_cast<const host_buffer&>(m_buffer);
}

void obby::host_document::insert(position pos, const std::string& text)
{
	server_document::insert(pos, text, get_buffer().get_self().get_id() );
}

void obby::host_document::erase(position begin, position end)
{
	server_document::erase(begin, end, get_buffer().get_self().get_id() );
}

