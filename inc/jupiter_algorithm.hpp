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

#ifndef _OBBY_JUPITER_ALGORITHM_HPP_
#define _OBBY_JUPITER_ALGORITHM_HPP_

#include <net6/non_copyable.hpp>
#include "jupiter_error.hpp"
#include "operation.hpp"
#include "record.hpp"

namespace obby
{

/** Implementation of the Jupiter algorithm.
 */
template<typename Document>
class jupiter_algorithm: private net6::non_copyable
{
public:
	typedef Document document_type;
	typedef operation<document_type> operation_type;
	typedef record<document_type> record_type;

	jupiter_algorithm();
	~jupiter_algorithm();

	/** Returns a record the given local operation.
	 */
	std::auto_ptr<record_type> local_op(const operation_type& op);

	/** Returns a transformed operation after a remote host sent
	 * the given record.
	 */
	std::auto_ptr<operation_type> remote_op(const record_type& rec);
protected:
	/** Helper class that stores an operation with the current local
	 * operation count.
	 */
	class operation_storage: private net6::non_copyable
	{
	public:
		/** Constructor taking a copy from op.
		 */
		operation_storage(unsigned int count,
		                  const operation_type& op);

		/** Constructor taking the ownership of op.
		 */
		operation_storage(unsigned int count,
		                  std::auto_ptr<operation_type> op);

		/** Returns the local operation count of this operation.
		 */
		unsigned int get_count() const;

		/** Returns the wrapped operation.
		 */
		const operation_type& get_operation() const;

		/** Replaces the wrapped operation by the copy of another one.
		 */
		void reset_operation(const operation_type& new_op);

		/** Replaces the wrapped operation by another one.
		 */
		void reset_operation(std::auto_ptr<operation_type> new_op);
	protected:
		unsigned int m_count;
		std::auto_ptr<operation_type> m_operation;
	};

	/** Discard from the remote site acknowledged operations.
	 */
	void discard_operations(const record_type& rec);

	/** Transform the given operation by the local ones that have not
	 * been acknowledged by the remote site.
	 */
	std::auto_ptr<operation_type> transform(const operation_type& op) const;

	/** Checks preconditions that have to be fulfilled before transforming.
	 */
	void check_preconditions(const record_type& rec) const;

protected:
	typedef std::list<operation_storage*> ack_list_type;

	vector_time m_time;
	ack_list_type m_ack_list;
};

template<typename Document>
jupiter_algorithm<Document>::operation_storage::
	operation_storage(unsigned int count,
	                  const operation_type& op):
	m_count(count), m_operation(op.clone() )
{
}

template<typename Document>
jupiter_algorithm<Document>::operation_storage::
	operation_storage(unsigned int count,
	                  std::auto_ptr<operation_type> op):
	m_count(count), m_operation(op)
{
}

template<typename Document>
unsigned int jupiter_algorithm<Document>::operation_storage::get_count() const
{
	return m_count;
}

template<typename Document>
const typename jupiter_algorithm<Document>::operation_type&
jupiter_algorithm<Document>::operation_storage::get_operation() const
{
	return *m_operation;
}

template<typename Document>
void jupiter_algorithm<Document>::operation_storage::
	reset_operation(const operation_type& new_op)
{
	m_operation.reset(new_op.clone() );
}

template<typename Document>
void jupiter_algorithm<Document>::operation_storage::
	reset_operation(std::auto_ptr<operation_type> new_op)
{
	m_operation = new_op;
}

template<typename Document>
jupiter_algorithm<Document>::jupiter_algorithm():
	m_time(0, 0)
{
}

template<typename Document>
jupiter_algorithm<Document>::~jupiter_algorithm()
{
	for(typename ack_list_type::iterator iter = m_ack_list.begin();
	    iter != m_ack_list.end();
	    ++ iter)
	{
		delete *iter;
	}
}

template<typename Document>
std::auto_ptr<typename jupiter_algorithm<Document>::record_type>
jupiter_algorithm<Document>::local_op(const operation_type& op)
{
	std::auto_ptr<record_type> rec(new record_type(m_time, op) );
	m_ack_list.push_back(new operation_storage(m_time.get_local(), op) );
	m_time.inc_local();
	return rec;
}

template<typename Document>
std::auto_ptr<typename jupiter_algorithm<Document>::operation_type>
jupiter_algorithm<Document>::remote_op(const record_type& rec)
{
	check_preconditions(rec);
	discard_operations(rec);
	std::auto_ptr<operation_type> op(transform(rec.get_operation()) );
	m_time.inc_remote();
	return op;
}

template<typename Document>
void jupiter_algorithm<Document>::discard_operations(const record_type& rec)
{
	typename ack_list_type::iterator iter;
	for(iter = m_ack_list.begin(); iter != m_ack_list.end(); ++ iter)
	{
		if( (*iter)->get_count() < rec.get_time().get_remote() )
			delete *iter;
		else
			break;
	}

	m_ack_list.erase(m_ack_list.begin(), iter);

	// Verify sequence order (TCP should ensure this, if noone sends
	// corrupt packets).
	if(rec.get_time().get_local() != m_time.get_remote() )
	{
		throw jupiter_error(
			"Sequence order mismatch: Incoming record's local "
			"time does not match own remote time"
		);
	}
}

template<typename Document>
std::auto_ptr<typename jupiter_algorithm<Document>::operation_type>
jupiter_algorithm<Document>::transform(const operation_type& op) const
{
	std::auto_ptr<operation_type> new_op(op.clone() );

	for(typename ack_list_type::const_iterator iter = m_ack_list.begin();
	    iter != m_ack_list.end();
	    ++ iter)
	{
		const operation_type* existing_op =
			&(*iter)->get_operation();
		operation_type* new_trans_op =
			existing_op->transform(*new_op);
		operation_type* existing_trans_op =
			new_op->transform(*existing_op);

		(*iter)->reset_operation(
			std::auto_ptr<operation_type>(existing_trans_op)
		);

		new_op.reset(new_trans_op);
	}

	return new_op;
}

template<typename Document>
void obby::jupiter_algorithm<Document>::
	check_preconditions(const record_type& rec) const
{
	if(!m_ack_list.empty() &&
	   rec.get_time().get_remote() < m_ack_list.front()->get_count() )
	{
		throw jupiter_error(
			"Transformation precondition failed: Incoming remote "
			"time is lower than oldest time in ack list"
		);
	}

	if(rec.get_time().get_remote() > m_time.get_local() )
	{
		throw jupiter_error(
			"Transformation precondition failed: Incoming remote "
			"time is greater than own local time"
		);
	}

	if(rec.get_time().get_local() != m_time.get_remote() )
	{
		throw jupiter_error(
			"Transformation precondition failed: Incoming local "
			"time does not match own remote time"
		);
	}
}

} // namespace obby

#endif // _OBBY_JUPITER_ALGORITHM_HPP_
