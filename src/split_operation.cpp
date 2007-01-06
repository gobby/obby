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

#include "split_operation.hpp"

obby::split_operation::split_operation(const operation& first,
                                       const operation& second)
 : operation(), m_first(first.clone() ), m_second(second.clone() )
{
}

obby::split_operation::split_operation(operation* first, operation* second)
 : operation(), m_first(first), m_second(second)
{
}

obby::split_operation::split_operation(const net6::packet& pack,
                                       unsigned int& index,
                                       const user_table& user_table)
 : operation(),
   m_first(operation::from_packet(pack, index, user_table).release() ),
   m_second(operation::from_packet(pack, index, user_table).release() )
{
//	std::auto_ptr<operation> first = operation::from_packet(pack, index);
//	std::auto_ptr<operation> second = operation::from_packet(pack, index);
//
//	m_first = first.get();
//	m_second = second.get();
//
//	m_first.release();
//	m_second.release();
}

obby::operation* obby::split_operation::clone() const
{
	return new split_operation(*m_first, *m_second);
}

obby::operation* obby::split_operation::reverse(const document& doc) const
{
	return new split_operation(
		m_first->reverse(doc),
		m_second->reverse(doc)
	);
}

void obby::split_operation::apply(document& doc, const user* author) const
{
	m_first->apply(doc, author);

	// Transform second operation against first because the first one
	// has already been applied to the document.
	std::auto_ptr<operation> second(m_first->transform(*m_second) );
	second->apply(doc, author);
}

obby::operation*
obby::split_operation::transform(const operation& base_op) const
{
	// Transform base_op against second operation
	std::auto_ptr<operation> op1(m_second->transform(base_op) );
	// Transform result against first
	return m_first->transform(*op1);
}

obby::operation*
obby::split_operation::transform_insert(position pos,
                                        const std::string& text) const
{
	return new split_operation(
		m_first->transform_insert(pos, text),
		m_second->transform_insert(pos, text)
	);
}

obby::operation*
obby::split_operation::transform_delete(position pos, position len) const
{
	return new split_operation(
		m_first->transform_delete(pos, len),
		m_second->transform_delete(pos, len)
	);
}

void obby::split_operation::append_packet(net6::packet& pack) const
{
	pack << "split";
	m_first->append_packet(pack);
	m_second->append_packet(pack);
}
