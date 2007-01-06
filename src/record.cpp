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

#include "record.hpp"

#if 0
obby::record::record(const vector_time& timestamp, const operation& op)
 : m_timestamp(timestamp), m_operation(op.clone() )
{
}

obby::record::record(const vector_time& timestamp, operation* op)
 : m_timestamp(timestamp), m_operation(op)
{
}

obby::record::record(const net6::packet& pack,
                     unsigned int& index,
                     const user_table& user_table)
 : m_timestamp(
	pack.get_param(index).as<int>(),
	pack.get_param(index + 1).as<int>()
   ),
   m_operation(NULL)
{
	index += 2;
	m_operation = operation::from_packet(pack, index, user_table);
}

const obby::operation& obby::record::get_operation() const
{
	return *m_operation;
}

const obby::vector_time& obby::record::get_time() const
{
	return m_timestamp;
}

void obby::record::append_packet(net6::packet& pack) const
{
	pack << m_timestamp.get_local() << m_timestamp.get_remote();
	m_operation->append_packet(pack);
}
#endif
