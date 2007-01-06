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

#ifndef _OBBY_NO_OPERATION_HPP_
#define _OBBY_NO_OPERATION_HPP_

#include "operation.hpp"

namespace obby
{

/** no_operation is an operation that does nothing. This is a helper class
 * that may be created if a delete_operation gets transformed against a
 * delete_operation which fully overlaps the range of the first operation. In
 * this case, the effect of the first operation is fully handled by the second
 * one and nothing has to be done anymore.
 */
class no_operation : public operation
{
public:
	/** Standard constructor with no original operation assigned.
	 */
	no_operation();

	/** Constructor taking a given operation as the original one.
	 */
	no_operation(const operation& original);

	/** Creates a copy of this operation.
	 */
	virtual operation* clone() const;

	/** Applies this operation to a document. Since this is a no_operation,
	 * nothing will be done.
	 */
	virtual void apply(document& doc, const user* author) const;

	/** Transforms <em>base_op</em> against this operation. Since this is
	 * a no_operation, nothing will be transformed.
	 */
	virtual operation* transform(const operation& base_op) const;

	/** Includes the effect of the given insertion into this operation.
	 * Since this is a no_operation, nothing will be done.
	 */
	virtual operation* transform_insert(position pos,
	                                    const std::string& text) const;

	/** Includes the effect of the given deletion into this operation.
	 * Since this is a no_operation, nothing will be done.
	 */
	virtual operation* transform_delete(position pos,
	                                    position len) const;
protected:
	/** Constructor taking an original_operation struct.
	 */
	no_operation(original_operation* original);
};

} // namespace obby

#endif // _OBBY_NO_OPERATION_HPP_
