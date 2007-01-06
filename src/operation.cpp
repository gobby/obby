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

#include "operation.hpp"

#if 0
#include "no_operation.hpp"
#include "split_operation.hpp"
#include "insert_operation.hpp"
#include "delete_operation.hpp"
#include "reversible_insert_operation.hpp"

obby::operation::operation()
{
}

obby::operation::~operation()
{
}

std::auto_ptr<obby::operation>
obby::operation::from_packet(const net6::packet& pack,
                             unsigned int& index,
                             const user_table& user_table)
{
	// Get type
	const std::string& type = pack.get_param(index ++).as<std::string>();
	std::auto_ptr<operation> op;

	// Generate operation according to type
	if(type == "ins")
	{
		op.reset(new insert_operation(pack, index) );
	}
	else if(type == "del")
	{
		op.reset(new delete_operation(pack, index) );
	}
	else if(type == "split")
	{
		op.reset(new split_operation(pack, index, user_table) );
	}
	else if(type == "noop")
	{
		op.reset(new no_operation(pack, index) );
	}
	else if(type == "revins")
	{
		op.reset(
			new reversible_insert_operation(
				pack,
				index,
				user_table
			)
		);
	}
	else
	{
		throw net6::bad_value("Unexpected record type: " + type);
	}

	return op;
}
#endif
