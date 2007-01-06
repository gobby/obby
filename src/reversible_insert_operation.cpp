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
#include "reversible_insert_operation.hpp"

obby::reversible_insert_operation::
	reversible_insert_operation(position pos,
	                            const line& text):
	operation(), m_pos(pos), m_text(text)
{
}

obby::reversible_insert_operation::
	reversible_insert_operation(position pos,
	                            const line& text,
	                            const operation& original):
	operation(original), m_pos(pos), m_text(text)
{
}

obby::reversible_insert_operation::
	reversible_insert_operation(position pos,
	                            const line& text,
	                            original_operation* original):
	operation(original), m_pos(pos), m_text(text)
{
}

obby::reversible_insert_operation::
	reversible_insert_operation(const net6::packet& pack,
                                    unsigned int& index,
				    const user_table& user_table):
	operation(),
	m_pos(pack.get_param(index ++).as<int>() ),
	m_text(pack, index, user_table)
{
}

obby::operation* obby::reversible_insert_operation::clone() const
{
	return new reversible_insert_operation(m_pos, m_text, m_original);
}

obby::operation*
obby::reversible_insert_operation::reverse(const document& doc) const
{
	return new delete_operation(m_pos, m_text.length() );
}

void obby::reversible_insert_operation::apply(document& doc,
                                              const user* author) const
{
	doc.insert(m_pos, m_text);
}

obby::operation*
obby::reversible_insert_operation::transform(const operation& base_op) const
{
	return base_op.transform_insert(m_pos, m_text);
}

obby::operation* obby::reversible_insert_operation::
	transform_insert(position pos,
	                 const std::string& text) const
{
	if(m_pos < pos)
	{
		// Case 1 - nothing to do
		return clone();
	}
	else if(m_pos == pos)
	{
		// Special case
		// TODO
		if(static_cast<const std::string&>(m_text) <
		   static_cast<const std::string&>(text) )
		{
			return clone();
		}
		else
		{
			return new reversible_insert_operation(
				m_pos + text.length(),
				m_text,
				*this
			);
		}
	}
	else
	{
		// Case 2
		return new reversible_insert_operation(
			m_pos + text.length(),
			m_text,
			*this
		);
	}
}

obby::operation* obby::reversible_insert_operation::
	transform_delete(position pos,
	                 position len) const
{
	if(m_pos <= pos)
	{
		// Case 3
		return clone();
	}
	else if(m_pos > pos + len)
	{
		// Case 4
		return new reversible_insert_operation(
			m_pos - len,
			m_text,
			*this
		);
	}
	else
	{
		// Case 5
		return new reversible_insert_operation(
			pos,
			m_text,
			*this
		);
	}
}

void obby::reversible_insert_operation::append_packet(net6::packet& pack) const
{
	// Should not be sent through the net
	throw std::logic_error(
		"obby::reversible_insert_operation::append_packet"
	);

	pack << "revins";
	m_text.append_packet(pack);
}


