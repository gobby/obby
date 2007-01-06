/* libobby - Network text editing library
 * Copyright (C) 2005 0x539 dev group
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public
 * License as published by the Free Software Foundation; either
 i* version 2 of the License, or (at your option) any later version.
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

#include "local_document_info.hpp"
#include "local_buffer.hpp"

obby::local_document_info::local_document_info(const local_buffer& buf,
                                               unsigned int id,
                                               const std::string& title)
 : document_info(buf, id, title)
{
}

obby::local_document_info::~local_document_info()
{
}

const obby::local_buffer& obby::local_document_info::get_buffer() const
{
	return dynamic_cast<const obby::local_buffer&>(m_buffer);
}

obby::local_document* obby::local_document_info::get_document()
{
	return dynamic_cast<obby::local_document*>(m_document);
}

const obby::local_document* obby::local_document_info::get_document() const
{
	return dynamic_cast<const obby::local_document*>(m_document);
}
