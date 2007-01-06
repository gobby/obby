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

#include "delete_operation.hpp"

#if 0
#include "no_operation.hpp"
#include "split_operation.hpp"
#include "delete_operation.hpp"
#include "reversible_insert_operation.hpp"

obby::delete_operation::delete_operation(position pos, position len)
 : operation(), m_pos(pos), m_len(len)
{
}

obby::delete_operation::delete_operation(const net6::packet& pack,
                                         unsigned int& index)
 : operation(),
   m_pos(pack.get_param(index + 0).as<int>() ),
   m_len(pack.get_param(index + 1).as<int>() )
{
	index += 2;
}

obby::operation* obby::delete_operation::clone() const
{
	return new delete_operation(m_pos, m_len);
}

obby::operation* obby::delete_operation::reverse(const document& doc) const
{
	// Need to convert it to a reversible_insert_operation
	// (TODO: Rename to reversed_insert_operation) becasue the normal
	// insert operation does not hold user information but just the text
	// to insert.
	return new reversible_insert_operation(
		m_pos,
		doc.get_slice(m_pos, m_len)
	);
}

void obby::delete_operation::apply(document& doc, const user* author) const
{
	doc.erase(m_pos, m_len, author);
}

obby::operation*
obby::delete_operation::transform(const operation& base_op) const
{
	return base_op.transform_delete(m_pos, m_len);
}

obby::operation*
obby::delete_operation::transform_insert(position pos,
                                         const std::string& text) const
{
	if(m_pos + m_len < pos)
	{
		// Case 6
		return clone();
	}
	else if(pos <= m_pos)
	{
		// Case 7
		return new delete_operation(
			m_pos + text.length(),
			m_len
		);
	}
	else
	{
		// Case 8
		return new split_operation(
			new delete_operation(
				m_pos,
				pos - m_pos
			),
			new delete_operation(
				pos + text.length(),
				m_len - (pos - m_pos)
			)
		);
	}
}

obby::operation*
obby::delete_operation::transform_delete(position pos, position len) const
{
	if(m_pos + m_len < pos)
	{
		// Case 9
		return clone();
	}
	else if(m_pos >= pos + len)
	{
		// Case 10
		return new delete_operation(m_pos - len, m_len);
	}
	else if(pos <= m_pos && pos + len >= m_pos + m_len)
	{
		// Case 11
		return new no_operation;
	}
	else if(pos <= m_pos && pos + len < m_pos + m_len)
	{
		// Case 12
		return new delete_operation(
			pos,
			m_len - (pos + len - m_pos)
		);
	}
	else if(pos > m_pos && pos + len >= m_pos + m_len)
	{
		// Case 13
		return new delete_operation(
			m_pos,
			pos - m_pos
		);
	}
	else
	{
		// Case 14
		return new delete_operation(
			m_pos,
			m_len - len
		);
	}
}

void obby::delete_operation::append_packet(net6::packet& pack) const
{
	pack << "del" << m_pos << m_len;
}
#endif
