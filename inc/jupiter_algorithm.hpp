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

#ifndef _OBBY_JUPITER_ALGORITHM_HPP_
#define _OBBY_JUPITER_ALGORITHM_HPP_

#include <net6/non_copyable.hpp>
#include "operation.hpp"
#include "record.hpp"

namespace obby
{

/** Implementation of the Jupiter algorithm.
 */
class jupiter_algorithm : private net6::non_copyable
{
public:
	jupiter_algorithm();
	~jupiter_algorithm();

	/** Returns a record the given local operation.
	 */
	std::auto_ptr<record> local_op(const operation& op);

	/** Returns a transformed operation after a remote host sent
	 * the given record.
	 */
	std::auto_ptr<operation> remote_op(const record& rec);
protected:
	/** Helper class that stores an operation with the current local
	 * operation count.
	 */
	class operation_wrapper : private net6::non_copyable
	{
	public:
		/** Cosntructor taking a copy from op.
		 */
		operation_wrapper(unsigned int count, const operation& op);

		/** Constructor taking the ownership of op.
		 */
		operation_wrapper(unsigned int count, operation* op);

		/** Returns the local operation count of this operation.
		 */
		unsigned int get_count() const;

		/** Returns the wrapped operation.
		 */
		const operation& get_operation() const;

		/** Replaces the wrapped operation by the copy of another one.
		 */
		void reset_operation(const operation& new_op);

		/** Replaces the wrapped operation by another one.
		 */
		void reset_operation(operation* new_op);
	protected:
		unsigned int m_count;
		std::auto_ptr<operation> m_operation;
	};

	typedef std::list<operation_wrapper*> ack_list;

	/** Discard from the remote site acknowledged operations.
	 */
	void discard_operations(const record& rec);

	/** Transform the given operation by the local ones that have not
	 * been acknowledged by the remote site.
	 */
	std::auto_ptr<operation> transform(const operation& op) const;

	/** Checks preconditions that have to be fulfilled before transforming.
	 */
	void check_preconditions(const record& rec) const;

protected:
	vector_time m_time;
	ack_list m_ack_list;
};

} // namespace obby

#endif // _OBBY_JUPITER_ALGORITHM_HPP_
