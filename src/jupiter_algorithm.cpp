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

#include "jupiter_algorithm.hpp"

obby::jupiter_algorithm::operation_wrapper::
	operation_wrapper(unsigned int count, const operation& op)
 : m_count(count), m_operation(op.clone() )
{
}

obby::jupiter_algorithm::operation_wrapper::
	operation_wrapper(unsigned int count, operation* op)
 : m_count(count), m_operation(op)
{
	if(op != NULL) return;

	throw std::logic_error(
		"obby::jupiter_algorithm::operation_wrapper::operation_wrapper"
	);
}

unsigned int obby::jupiter_algorithm::operation_wrapper::get_count() const
{
	return m_count;
}

const obby::operation&
obby::jupiter_algorithm::operation_wrapper::get_operation() const
{
	return *m_operation;
}

void obby::jupiter_algorithm::operation_wrapper::
	reset_operation(const operation& new_op)
{
	m_operation.reset(new_op.clone() );
}

void obby::jupiter_algorithm::operation_wrapper::
	reset_operation(operation* new_op)
{
	if(new_op != NULL)
	{
		m_operation.reset(new_op);
		return;
	}

	throw std::logic_error(
		"obby::jupiter_algorithm::operation_wrapper::reset_operation"
	);
}

obby::jupiter_algorithm::jupiter_algorithm()
 : m_time(0, 0)
{
}

obby::jupiter_algorithm::~jupiter_algorithm()
{
	for(ack_list::iterator iter = m_ack_list.begin();
	    iter != m_ack_list.end();
	    ++ iter)
	{
		delete *iter;
	}
}

std::auto_ptr<obby::record>
obby::jupiter_algorithm::local_op(const operation& op)
{
	// Generate request
	std::auto_ptr<record> rec(new record(m_time, op) );
	// Add to outgoing queue
	m_ack_list.push_back(new operation_wrapper(m_time.get_local(), op) );
	// Generated new message
	m_time.inc_local();
	// Return request
	return rec;
}

std::auto_ptr<obby::operation>
obby::jupiter_algorithm::remote_op(const record& rec)
{
	// Check preconditions before transforming
	check_preconditions(rec);
	// Discard acknowledged operations
	discard_operations(rec);
	// Transform operation
	std::auto_ptr<operation> op(transform(rec.get_operation()) );
	// Got new message
	m_time.inc_remote();
	// Return transformed operation
	return op;
}

void obby::jupiter_algorithm::discard_operations(const record& rec)
{
	for(ack_list::iterator iter = m_ack_list.begin();
	    iter != m_ack_list.end();)
	{
		// Already acknowledged?
		if( (*iter)->get_count() < rec.get_time().get_remote() )
		{
			// Delete operation
			delete *iter;
			// Remove from list
			iter = m_ack_list.erase(iter);
		}
		else
		{
			// Check next one
			// TODO: Newer operations won't be acknowledged either,
			// so break here?
			++ iter;
		}
	}

	// Verify sequence order
	if(rec.get_time().get_local() != m_time.get_remote() )
	{
		throw std::logic_error(
			"obby::jupiter_algorithm::discard_operations"
		);
	}
}

std::auto_ptr<obby::operation>
obby::jupiter_algorithm::transform(const operation& op) const
{
	std::auto_ptr<operation> new_op(op.clone() );

	for(ack_list::const_iterator iter = m_ack_list.begin();
	    iter != m_ack_list.end();
	    ++ iter)
	{
		// Get current operation in list
		const operation* existing_op = &(*iter)->get_operation();
		// Transform new operation against existing one
		operation* new_trans_op = existing_op->transform(*new_op);
		// Transform existing operation against new one
		operation* existing_trans_op = new_op->transform(*existing_op);
		// Replace existing operation by transformed one
		(*iter)->reset_operation(existing_trans_op);
		// Replace new operation by transformed one
		new_op.reset(new_trans_op);
	}

	return new_op;
}

void obby::jupiter_algorithm::check_preconditions(const record& rec) const
{
	// #1
	if(!m_ack_list.empty() &&
	   rec.get_time().get_remote() < m_ack_list.front()->get_count() )
	{
		throw std::logic_error(
			"obby::jupiter_algorithm::check_preconditions (#1)"
		);
	}

	// #2
	if(rec.get_time().get_remote() > m_time.get_local() )
	{
		throw std::logic_error(
			"obby::jupiter_algorithm::check_preconditions (#2)"
		);
	}

	// #3
	if(rec.get_time().get_local() != m_time.get_remote() )
	{
		throw std::logic_error(
			"obby::jupiter_algorithm::check_preconditions (#3)"
		);
	}
}
