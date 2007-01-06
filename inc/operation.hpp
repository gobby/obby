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

#ifndef _OBBY_OPERATION_HPP_
#define _OBBY_OPERATION_HPP_

#include <net6/non_copyable.hpp>
#include "document.hpp"

namespace obby
{

/** An operation describes a change in the document.
 */
class operation : private net6::non_copyable
{
public:
	operation();
	virtual ~operation();

	/** Creates a copy of this operation.
	 */
	virtual operation* clone() const = 0;

	/** Creates the reverse operation of this one.
	 * @param doc Document to receive additional information from.
	 */
	virtual operation* reverse(const document& doc) const = 0;

	/** Applies this operation to a document.
	 * @param doc Document to apply this operation to.
	 * @param author User who performed this operation.
	 */
	virtual void apply(document& doc, const user* author) const = 0;

	/** Transforms <em>base_op</em> against this operation.
	 */
	virtual operation* transform(const operation& base_op) const = 0;

	/** Includes the effect of the given insertion into this operation.
	 */
	virtual operation* transform_insert(position pos,
	                                    const std::string& text) const = 0;

	/** Includes the effect of the given deletion into this operation.
	 */
	virtual operation* transform_delete(position pos,
	                                    position len) const = 0;

	/** Appends this operation to the given packet.
	 */
	virtual void append_packet(net6::packet& pack) const = 0;

	/** Reads an operation from the given packet.
	 * @param pack Packet to read from.
	 * @param index From which parameter to read at.
	 * @param user_table User table were to read potential user
	 * information from.
	 */
	static std::auto_ptr<operation>
		from_packet(const net6::packet& pack,
	                    unsigned int& index,
	                    const user_table& user_table);
protected:
};

} // namespace obby

#endif // _OBBY_OPERATION_HPP_

