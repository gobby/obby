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

obby::no_operation::no_operation()
 : operation()
{
}

obby::no_operation::no_operation(const net6::packet& pack, unsigned int& index)
 : operation()
{
	// Nothing to read
}

obby::operation* obby::no_operation::clone() const
{
	return new no_operation;
}

obby::operation* obby::no_operation::reverse(const document& doc) const
{
	return new no_operation;
}

void obby::no_operation::apply(document& doc, const user* author) const
{
}

obby::operation* obby::no_operation::transform(const operation& base_op) const
{
	// Nothing to do
	return base_op.clone();
}

obby::operation*
obby::no_operation::transform_insert(position pos,
                                     const std::string& text) const
{
	// Nothing happens to no-op
	return clone();
}

obby::operation*
obby::no_operation::transform_delete(position pos, position len) const
{
	// Nothing happens to no-op
	return clone();
}

void obby::no_operation::append_packet(net6::packet& pack) const
{
	pack << "noop";
	// Nothing to append
}


