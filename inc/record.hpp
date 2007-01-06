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
class record : private net6::non_copyable
{
public:
	/** Creates a record that takes a copy of the given operation.
	 */
	record(const vector_time& timestamp, const operation& op);

	/** Creates a record that takes ownership of the given operation.
	 */
	record(const vector_time& timestamp, operation* op);

	/** Returns the operation of the record.
	 */
	const operation& get_operation() const;

	/** Returns the timestamp of this record.
	 */
	const vector_time& get_time() const;

protected:
	vector_time m_timestamp;
	std::auto_ptr<operation> m_operation;
};

}

#endif // _OBBY_RECORD_HPP_
