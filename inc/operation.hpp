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

#ifndef _OBBY_OPERATION_HPP_
#define _OBBY_OPERATION_HPP_

#include <net6/non_copyable.hpp>
#include <net6/packet.hpp>
#include "position.hpp"
#include "user.hpp"

namespace obby
{

/** An operation describes a change in the document.
 */
template<typename Document>
class operation: private net6::non_copyable
{
public:
	typedef Document document_type;

	/** Creates a copy of this operation.
	 */
	virtual operation* clone() const = 0;

	/** Creates the reverse operation of this one.
	 * @param doc Document to receive additional information from.
	 */
	virtual operation* reverse(const document_type& doc) const = 0;

	/** Applies this operation to a document.
	 * @param doc Document to apply this operation to.
	 * @param author User who performed this operation.
	 */
	virtual void apply(document_type& doc, const user* author) const = 0;

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

template<typename Document>
class no_operation;

template<typename Document>
class split_operation;

template<typename Document>
class insert_operation;

template<typename Document>
class delete_operation;

template<typename Document>
class reversible_insert_operation;

template<typename Document>
std::auto_ptr<operation<Document> >
operation<Document>::from_packet(const net6::packet& pack,
                                 unsigned int& index,
                                 const user_table& user_table)
{
	const std::string& type = pack.get_param(index ++).net6::parameter::as<std::string>();
	std::auto_ptr<operation<Document> > op;

	if(type == "ins")
	{
		op.reset(new insert_operation<Document>(pack, index) );
	}
	else if(type == "del")
	{
		op.reset(new delete_operation<Document>(pack, index) );
	}
	else if(type == "split")
	{
		op.reset(
			new split_operation<Document>(
				pack,
				index,
				user_table)
		);
	}
	else if(type == "noop")
	{
		op.reset(new no_operation<Document>(pack, index) );
	}
/*	else if(type == "revins")
	{
		op.reset(
			new reversible_insert_operation<Document>(
				pack,
				index,
				user_table
			)
		);
	}*/
	else
	{
		throw net6::bad_value("Unexpected record type: " + type);
	}

	return op;
}

} // namespace obby

#endif // _OBBY_OPERATION_HPP_

