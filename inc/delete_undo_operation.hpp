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

#ifndef _OBBY_DELETE_UNDO_OPERATION_HPP_
#define _OBBY_DELETE_UNDO_OPERATION_HPP_

#include "operation.hpp"
#include "line.hpp"

namespace obby
{

/** delete_undo_operation deletes an amount of text at a specified position in
 * the document and remembers the string and the authors of the respective
 * substrings.
 */
class delete_undo_operation : public operation
{
public:
	/** Standard constructor with no original operation assigned.
	 */
	delete_undo_operation(position pos, const line& text);

	/** Constructor taking a given operation as the original one.
	 */
	delete_undo_operation(position pos, const line& text,
	                      const operation& original);

	/** Creates a copy of this operation.
	 */
	virtual operation* clone() const;

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
	delete_undo_operation(position pos, const line& text,
	                      original_operation* original);

	position m_pos;
	line m_text;
};

} // namespace obby

#endif // _OBBY_DELETE_UNDO_OPERATION_HPP_
