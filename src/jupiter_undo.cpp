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
	m_doc(doc)
{
	m_opmap[NULL] = new operation_ring(100);
}

obby::jupiter_undo::~jupiter_undo()
{
	for(operation_map::iterator map_iter = m_opmap.begin();
	    map_iter != m_opmap.end();
	    ++ map_iter)
	{
		operation_ring* ring = map_iter->second;
		for(operation_ring::iterator op_iter = ring->begin();
		    op_iter != ring->end();
		    ++ op_iter)
		{
			delete *op_iter;
		}

		delete ring;
	}
}

void obby::jupiter_undo::client_add(const user& client)
{
	m_opmap[&client] = new operation_ring(MAX_UNDO);
}

void obby::jupiter_undo::client_remove(const user& client)
{
	operation_map::iterator map_iter = m_opmap.find(&client);
	if(map_iter == m_opmap.end() )
		throw std::logic_error("obby::jupiter_undo::client_remove");

	operation_ring* ring = map_iter->second;
	for(operation_ring::iterator ring_iter = ring->begin();
	    ring_iter != ring->end();
	    ++ ring_iter)
	{
		delete *ring_iter;
	}

	m_opmap.erase(map_iter);
	delete ring;
}

void obby::jupiter_undo::perform_op(const operation& op, const user* from)
{
	operation_map::iterator map_iter = m_opmap.find(from);
	if(map_iter == m_opmap.end() )
		throw std::logic_error("obby::jupiter_undo::perform_op");

	// Add reverse operation to undo ring
	operation_ring* ring = map_iter->second;
	ring->push_back(op.reverse(m_doc) );
	transform_undo_map(op, from);
}

bool obby::jupiter_undo::can_undo(const user* user)
{
	operation_map::iterator map_iter = m_opmap.find(user);
	if(map_iter == m_opmap.end() )
		throw std::logic_error("obby::jupiter_undo::can_undo");

	return !map_iter->second->empty();
}

std::auto_ptr<obby::operation> obby::jupiter_undo::undo(const user* user)
{
	operation_map::iterator map_iter = m_opmap.find(user);
	if(map_iter == m_opmap.end() )
		throw std::logic_error("obby::jupiter_undo::undo");

	// Get last operation from undo ring, transform others.
	operation_ring* ring = map_iter->second;
	std::auto_ptr<operation> op(ring->back() );
	ring->pop_back();
	transform_undo_map(*op, user);

	return op;
}

void obby::jupiter_undo::transform_undo_map(const operation& op,
                                            const user* from)
{
	// Transform new operation against operations of other clients
	for(operation_map::iterator map_iter = m_opmap.begin();
	    map_iter != m_opmap.end();
	    ++ map_iter)
	{
		// Ignore original author
		if(map_iter->first == from) continue;

		operation_ring* ring = map_iter->second;
		for(operation_ring::iterator ring_iter = ring->begin();
		    ring_iter != ring->end();
		    ++ ring_iter)
		{
			(*ring_iter) = op.transform(**ring_iter);
		}
	}
}
