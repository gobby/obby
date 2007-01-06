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

#ifndef _OBBY_DELETE_OPERATION_HPP_
#define _OBBY_DELETE_OPERATION_HPP_

#include "operation.hpp"

namespace obby
{

/** delete_operation deletes text from a document.
 */
class delete_operation: public operation
{
public:
	/** Standard constructor with no original operation assigned.
	 */
	delete_operation(position pos, position len);

	/** Constructor taking a given operation as the original one.
	 */
	delete_operation(position pos, position len,
	                 const operation& original);

	/** Reads a delete_operation from the given network packet.
	 */
	delete_operation(const net6::packet& pack, unsigned int& index);

	/** Creates a copy of this operation.
	 */
	virtual operation* clone() const;

	/** Creates the reverse operation of this one.
	 * @param doc Document to receive additional information from.
	 */
	virtual operation* reverse(const document& doc) const;

	/** Applies this operation to a document.
	 */
	virtual void apply(document& doc, const user* author) const;

	/** Transforms <em>base_op</em> against this operation.
	 */
	virtual operation* transform(const operation& base_op) const;

	/** Includes the effect of the given insertion into this operation.
	 */
	virtual operation* transform_insert(position pos,
	                                    const std::string& text) const;

	/** Includes the effect of the given deletion into this operation.
	 */
	virtual operation* transform_delete(position pos,
	                                    position len) const;

	/** Appends the operation to the given packet.
	 */
	virtual void append_packet(net6::packet& pack) const;
protected:
	/** Constructor taking an original_operation struct.
	 */
	delete_operation(position pos, position len,
	                 original_operation* original);

	position m_pos;
	position m_len;
};

} // namespace obby

#endif // _OBBY_DELETE_OPERATION_HPP_

