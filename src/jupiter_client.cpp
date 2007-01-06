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

#include "jupiter_client.hpp"

obby::jupiter_client::jupiter_client(document& doc)
 : m_document(doc)
{
}

void obby::jupiter_client::local_op(const operation& op, const user* from)
{
	// Apply operation locally
	op.apply(m_document, from);
	// Generate record
	std::auto_ptr<record> rec(m_algorithm.local_op(op) );
	// Emit local signal
	m_signal_local.emit(*rec, from);
}

void obby::jupiter_client::remote_op(const record& rec, const user* from)
{
	// Transform
	std::auto_ptr<operation> op(m_algorithm.remote_op(rec) );
	// Apply to document
	op->apply(m_document, from);
}

obby::jupiter_client::signal_local_type
obby::jupiter_client::local_event() const
{
	return m_signal_local;
}
