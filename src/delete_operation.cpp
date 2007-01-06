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

#include "no_operation.hpp"
#include "split_operation.hpp"
#include "delete_operation.hpp"

obby::delete_operation::delete_operation(position pos, const std::string& text)
 : operation(), m_pos(pos), m_text(text)
{
}

obby::delete_operation::delete_operation(position pos, const std::string& text,
                                         const operation& original)
 : operation(original), m_pos(pos), m_text(text)
{
}

obby::delete_operation::delete_operation(position pos, const std::string& text,
                                         original_operation* original)
 : operation(original), m_pos(pos), m_text(text)
{
}

obby::delete_operation::delete_operation(const net6::packet& pack,
                                         unsigned int& index)
 : operation(),
   m_pos(pack.get_param(index ++).as<int>() ),
   m_text(pack.get_param(index ++).as<std::string>() )
   // TODO: unreversible delete_operation with only len instead of m_text
{
}

obby::operation* obby::delete_operation::clone() const
{
	return new delete_operation(m_pos, m_text, m_original);
}

void obby::delete_operation::apply(document& doc, const user* author) const
{
	if(doc.get_slice(m_pos, m_text.length()) != m_text)
		throw std::logic_error("obby::delete_operation::apply");

	doc.erase(m_pos, m_text.length(), author);
}

obby::operation*
obby::delete_operation::transform(const operation& base_op) const
{
	return base_op.transform_delete(m_pos, m_text.length() );
}

obby::operation*
obby::delete_operation::transform_insert(position pos,
                                         const std::string& text) const
{
	if(m_pos + m_text.length() < pos)
	{
		// Case 6
		return clone();
	}
	else if(pos <= m_pos)
	{
		// Case 7
		return new delete_operation(
			m_pos + text.length(),
			m_text,
			*this
		);
	}
	else
	{
		// Case 8
		// TODO: Both delete_operation's originals should refer to the
		// same object.
		return new split_operation(
			new delete_operation(
				m_pos,
				m_text.substr(0, pos - m_pos),
				*this
			),
			new delete_operation(
				pos + text.length(),
				m_text.substr(pos - m_pos),
				*this
			)
			// TODO: What about split_operation's original
		);
	}
}

obby::operation*
obby::delete_operation::transform_delete(position pos, position len) const
{
	if(m_pos + m_text.length() < pos)
	{
		// Case 9
		return clone();
	}
	else if(m_pos >= pos + len)
	{
		// Case 10
		return new delete_operation(m_pos - len, m_text, *this);
	}
	else if(pos <= m_pos && pos + len >= m_pos + m_text.length() )
	{
		// Case 11
		return new no_operation(*this);
	}
	else if(pos <= m_pos && pos + len < m_pos + m_text.length() )
	{
		// Case 12
		return new delete_operation(
			pos,
			m_text.substr(pos + len - m_pos),
			*this
		);
	}
	else if(pos > m_pos && pos + len >= m_pos + m_text.length() )
	{
		// Case 13
		return new delete_operation(
			m_pos,
			m_text.substr(0, pos - m_pos),
			*this
		);
	}
	else
	{
		// Case 14
		return new delete_operation(
			m_pos,
			m_text.substr(0, pos - m_pos) +
			m_text.substr(pos + len - m_pos),
			*this
		);
	}
}

void obby::delete_operation::append_packet(net6::packet& pack) const
{
	pack << "del" << m_pos << m_text;
}
