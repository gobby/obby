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

#include "jupiter_undo.hpp"

const unsigned int obby::jupiter_undo::MAX_UNDO = 0x7f;

obby::jupiter_undo::jupiter_undo(const obby::document& doc):
	m_doc(doc), m_opring(MAX_UNDO)
{
}

obby::jupiter_undo::~jupiter_undo()
{
	for(operation_ring::iterator op_iter = m_opring.begin();
	    op_iter != m_opring.end();
	    ++ op_iter)
	{
		delete *op_iter;
	}
}

void obby::jupiter_undo::client_add(const user& client)
{
}

void obby::jupiter_undo::client_remove(const user& client)
{
}

void obby::jupiter_undo::local_op(const operation& op, const user* from)
{
	m_opring.push_back(op.reverse(m_doc) );
	transform_undo_ring(op);
}

void obby::jupiter_undo::remote_op(const operation& op, const user* from)
{
	// No need to check, transform in all cases
	transform_undo_ring(op);
}

bool obby::jupiter_undo::can_undo()
{
	return !m_opring.empty();
}

std::auto_ptr<obby::operation> obby::jupiter_undo::undo()
{
	// Get last operation from undo ring, transform others.
	std::auto_ptr<operation> op(m_opring.back() );
	m_opring.pop_back();
	transform_undo_ring(*op);

	return op;
}

void obby::jupiter_undo::transform_undo_ring(const operation& op)
{
	for(operation_ring::iterator op_iter = m_opring.begin();
	    op_iter != m_opring.end();
	    ++ op_iter)
	{
		(*op_iter) = op.transform(**op_iter);
	}
}
