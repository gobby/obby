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

#ifndef _OBBY_RECORD_HPP_
#define _OBBY_RECORD_HPP_

#include <net6/non_copyable.hpp>
#include "vector_time.hpp"
#include "operation.hpp"

namespace obby
{

/** vector_time attached to an operation. The record may be sent through the
 * network. jupiter_algorithm is then able to transform the operation against
 * own operations the original sender did not know about.
 */
template<typename Document>
class record: private net6::non_copyable
{
public:
	typedef operation<Document> operation_type;

	/** Creates a record that takes a copy of the given operation.
	 */
	record(const vector_time& timestamp, const operation_type& op);

	/** Creates a record that takes ownership of the given operation.
	 */
	record(const vector_time& timestamp, std::auto_ptr<operation_type> op);

	/** Reads the record from the given packet, beginning at the parameter
	 * <em>index</em>. After the call, <em>index</em> points to the next
	 * parameter in the packet.
	 */
	record(const net6::packet& pack,
	       unsigned int& index,
	       const user_table& user_table);

	/** Returns the operation of the record.
	 */
	const operation_type& get_operation() const;

	/** Returns the timestamp of this record.
	 */
	const vector_time& get_time() const;

	/** Appends the record to a packet.
	 */
	void append_packet(net6::packet& pack) const;

protected:
	vector_time m_timestamp;
	std::auto_ptr<operation_type> m_operation;
};

template<typename Document>
record<Document>::record(const vector_time& timestamp,
                         const operation_type& op):
	m_timestamp(timestamp), m_operation(op.clone() )
{
}

template<typename Document>
record<Document>::record(const vector_time& timestamp,
                         std::auto_ptr<operation_type> op):
	m_timestamp(timestamp), m_operation(op.clone() )
{
}

template<typename Document>
record<Document>::record(const net6::packet& pack,
                         unsigned int& index,
                         const user_table& user_table):
	m_timestamp(
		pack.get_param(index).net6::parameter::as<int>(),
		pack.get_param(index + 1).net6::parameter::as<int>()
	),
	m_operation(NULL)
{
	index += 2;
	m_operation = operation_type::from_packet(pack, index, user_table);
}

template<typename Document>
const typename record<Document>::operation_type&
record<Document>::get_operation() const
{
	return *m_operation;
}

template<typename Document>
const vector_time& record<Document>::get_time() const
{
	return m_timestamp;
}

template<typename Document>
void record<Document>::append_packet(net6::packet& pack) const
{
	pack << m_timestamp.get_local() << m_timestamp.get_remote();
	m_operation->append_packet(pack);
}

} // namespace obby

#endif // _OBBY_RECORD_HPP_
