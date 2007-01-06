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

#ifndef _OBBY_SPLIT_OPERATION_HPP_
#define _OBBY_SPLIT_OPERATION_HPP_

#include "operation.hpp"

namespace obby
{

/** split_operation is a wrapper around two other operations. If an
 * insert_operation occurs in the range of a delete_operation, the
 * delete_operation has to be splitted into two delete_operations.
 */
template<typename Document>
class split_operation: public operation<Document>
{
public:
	typedef operation<Document> operation_type;
	typedef typename operation_type::document_type document_type;

	/** Constructor taking a copy of both given operations.
	 */
	split_operation(const operation_type& first,
	                const operation_type& second);

	/** Constructor taking the ownership of both operations instead of
	 * making a copy.
	 */
	split_operation(std::auto_ptr<operation_type> first,
	                std::auto_ptr<operation_type> second);

	/** Reads a split_operation from the given network packet.
	 */
	split_operation(const net6::packet& pack,
	                unsigned int& index,
	                const user_table& user_table);

	/** Creates a copy of this operation.
	 */
	virtual operation_type* clone() const;

	/** Creates the reverse operation of this one.
	 * @param doc Document to receive additional information from.
	 */
	virtual operation_type* reverse(const document_type& doc) const;

	/** Applies this operation to a document. The split_operation just
	 * forwards this request to both wrapped operations.
	 */
	virtual void apply(document_type& doc, const user* author) const;

	/** Transforms <em>base_op</em> against this operation. This function
	 * transforms base_op against both wrapped operations.
	 */
	virtual operation_type* transform(const operation_type& base_op) const;

	/** Includes the effect of the given insertion into this operation.
	 * Both wrapped operations will be transformed.
	 */
	virtual operation_type* transform_insert(position pos,
	                                         const std::string& text) const;

	/** Includes the effect of the given deletion into this operation.
	 * Both wrapped operations will be transformed.
	 */
	virtual operation_type* transform_delete(position pos,
	                                         position len) const;

	/** Appends the operation to the given packet.
	 */
	virtual void append_packet(net6::packet& pack) const;
protected:
	std::auto_ptr<operation_type> m_first;
	std::auto_ptr<operation_type> m_second;
};

template<typename Document>
split_operation<Document>::split_operation(const operation_type& first,
                                           const operation_type& second):
	operation<Document>(), m_first(first.clone() ),
	m_second(second.clone() )
{
}

template<typename Document>
split_operation<Document>::
	split_operation(std::auto_ptr<operation_type> first,
                        std::auto_ptr<operation_type> second):
	operation<Document>(), m_first(first), m_second(second)
{
}

template<typename Document>
split_operation<Document>::split_operation(const net6::packet& pack,
                                           unsigned int& index,
                                           const user_table& user_table):
	operation<Document>(),
	m_first(
		operation<Document>::from_packet(
			pack,
			index,
			user_table
		).release()
	),
	m_second(operation<Document>::from_packet(
			pack,
			index,
			user_table
		).release()
	)
{
}

template<typename Document>
typename split_operation<Document>::operation_type*
split_operation<Document>::clone() const
{
	return new split_operation<Document>(*m_first, *m_second);
}

template<typename Document>
typename split_operation<Document>::operation_type*
split_operation<Document>::reverse(const document_type& doc) const
{
	return new split_operation<Document>(
		std::auto_ptr<operation_type>(m_first->reverse(doc) ),
		std::auto_ptr<operation_type>(m_second->reverse(doc) )
	);
}

template<typename Document>
void split_operation<Document>::apply(document_type& doc,
                                      const user* author) const
{
	m_first->apply(doc, author);

	// Transform second operation because first has just been applied
	std::auto_ptr<operation_type> second(m_first->transform(*m_second) );
	second->apply(doc, author);
}

template<typename Document>
typename split_operation<Document>::operation_type*
split_operation<Document>::transform(const operation_type& base_op) const
{
	std::auto_ptr<operation_type> op1(m_second->transform(base_op) );
	return m_first->transform(*op1);
}

template<typename Document>
typename split_operation<Document>::operation_type*
split_operation<Document>::transform_insert(position pos,
                                            const std::string& text) const
{
	return new split_operation<Document>(
		std::auto_ptr<operation_type>(
			m_first->transform_insert(pos, text)
		),
		std::auto_ptr<operation_type>(
			m_second->transform_insert(pos, text)
		)
	);
}

template<typename Document>
typename split_operation<Document>::operation_type*
split_operation<Document>::transform_delete(position pos,
                                            position len) const
{
	return new split_operation<Document>(
		std::auto_ptr<operation_type>(
			m_first->transform_delete(pos, len)
		),
		std::auto_ptr<operation_type>(
			m_second->transform_delete(pos, len)
		)
	);
}

template<typename Document>
void split_operation<Document>::append_packet(net6::packet& pack) const
{
	pack << "split";
	m_first->append_packet(pack);
	m_second->append_packet(pack);
}

} // namespace obby

#endif // _OBBY_SPLIT_OPERATION_HPP_
