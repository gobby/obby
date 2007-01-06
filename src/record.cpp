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

#include "record.hpp"

unsigned int obby::record::m_counter = 0;

obby::record::record(unsigned int revision, unsigned int from)
 : m_id(++ m_counter), m_revision(revision) m_from(from)
{
}

obby::record::record(unsigned int revision, unsigned int from, unsigned int id)
 : m_id(id), m_revision(revision), m_from(from)
{
}

obby::record::~record()
{
}

unsigned int obby::record::get_id() const
{
	return m_id;
}

unsigned int obby::record::get_revision() const
{
	return m_revision;
}
