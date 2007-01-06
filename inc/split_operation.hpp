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

#ifndef _OBBY_SPLIT_OPERATION_HPP_
#define _OBBY_SPLIT_OPERATION_HPP_

#include "operation.hpp"

namespace obby
{

/** split_operation is a wrapper around two other operations. If an
 * insert_operation occurs in the range of a delete_operation, the
 * delete_operation has to be splitted into two delete_operations.
 */
class split_operation : public operation
{
public:
	/** Constructor taking a copy of both given operations.
	 */
	split_operation(const operation& first, const operation& second);

	/** Constructor taking the ownership of both operations instead of
	 * making a copy.
	 */
	split_operation(operation* first, operation* second);

	/** Constructor taking a given operation as the original one.
	 */
	split_operation(const operation& first, const operation& second,
	                const operation& original);

	/** Constructor taking the ownership of both operations instead of
	 * making a copy and the original one.
	 */
	split_operation(operation* first, operation* second,
	                const operation& original);

	/** Creates a copy of this operation.
	 */
	virtual operation* clone() const;

	/** Applies this operation to a document. The split_operation just
	 * forwards this request to both wrapped operations.
	 */
	virtual void apply(document& doc, const user* author) const;

	/** Transforms <em>base_op</em> against this operation. This function
	 * transforms base_op against both wrapped operations.
	 */
	virtual operation* transform(const operation& base_op) const;

	/** Includes the effect of the given insertion into this operation.
	 * Both wrapped operations will be transformed.
	 */
	virtual operation* transform_insert(position pos,
	                                    const std::string& text) const;

	/** Includes the effect of the given deletion into this operation.
	 * Both wrapped operations will be transformed.
	 */
	virtual operation* transform_delete(position pos,
	                                    position len) const;
protected:
	/** Constructor taking an original_operation struct.
	 */
	split_operation(const operation& first, const operation& second,
	                original_operation* original);

	/** Constructor taking the ownership of both operations instead of
	 * making a copy and an original_operation struct.
	 */
	split_operation(operation* first, operation* second,
	                original_operation* original);

	std::auto_ptr<operation> m_first;
	std::auto_ptr<operation> m_second;
};

} // namespace obby

#endif // _OBBY_SPLIT_OPERATION_HPP_
