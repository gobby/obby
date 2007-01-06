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
template<typename Document>
class delete_operation: public operation<Document>
{
public:
	typedef operation<Document> operation_type;
	typedef typename operation<Document>::document_type document_type;

	delete_operation(position pos, position len);

	/** Reads a delete_operation from the given network packet.
	 */
	delete_operation(const net6::packet& pack, unsigned int& index);

	/** Creates a copy of this operation.
	 */
	virtual operation_type* clone() const;

	/** Creates the reverse operation of this one.
	 * @param doc Document to receive additional information from.
	 */
	virtual operation_type* reverse(const document_type& doc) const;

	/** Applies this operation to a document.
	 */
	virtual void apply(document_type& doc, const user* author) const;

	/** Transforms <em>base_op</em> against this operation.
	 */
	virtual operation_type* transform(const operation_type& base_op) const;

	/** Includes the effect of the given insertion into this operation.
	 */
	virtual operation_type* transform_insert(position pos,
	                                         const std::string& text) const;

	/** Includes the effect of the given deletion into this operation.
	 */
	virtual operation_type* transform_delete(position pos,
	                                         position len) const;

	/** Appends the operation to the given packet.
	 */
	virtual void append_packet(net6::packet& pack) const;
protected:
	position m_pos;
	position m_len;
};

template<typename Document>
delete_operation<Document>::delete_operation(position pos,
                                             position len):
	operation<Document>(), m_pos(pos), m_len(len)
{
}

template<typename Document>
delete_operation<Document>::delete_operation(const net6::packet& pack,
                                             unsigned int& index):
	operation<Document>(),
	m_pos(pack.get_param(index + 0).net6::parameter::as<int>() ),
	m_len(pack.get_param(index + 1).net6::parameter::as<int>() )
{
	index += 2;
}

template<typename Document>
typename delete_operation<Document>::operation_type*
delete_operation<Document>::clone() const
{
	return new delete_operation<Document>(m_pos, m_len);
}

template<typename Document>
typename delete_operation<Document>::operation_type*
delete_operation<Document>::reverse(const document_type& doc) const
{
	return new reversible_insert_operation<Document>(
		m_pos,
		doc.get_slice(m_pos, m_len)
	);
}

template<typename Document>
void delete_operation<Document>::apply(document_type& doc,
                                       const user* author) const
{
	doc.erase(m_pos, m_len);
}

template<typename Document>
typename delete_operation<Document>::operation_type*
delete_operation<Document>::transform(const operation_type& base_op) const
{
	return base_op.transform_delete(m_pos, m_len);
}

template<typename Document>
typename delete_operation<Document>::operation_type*
delete_operation<Document>::transform_insert(position pos,
                                             const std::string& text) const
{
	if(m_pos + m_len < pos)
	{
		// Case 6
		return clone();
	}
	else if(pos <= m_pos)
	{
		// Case 7
		return new delete_operation<Document>(
			m_pos + text.length(),
			m_len
		);
	}
	else
	{
		// Case 8
		return new split_operation<Document>(
			std::auto_ptr<operation_type>(
				new delete_operation<Document>(
					m_pos,
					pos - m_pos
				)
			),
			std::auto_ptr<operation_type>(
				new delete_operation<Document>(
					pos + text.length(),
					m_len - (pos - m_pos)
				)
			)
		);
	}
}

template<typename Document>
typename delete_operation<Document>::operation_type*
delete_operation<Document>::transform_delete(position pos,
                                             position len) const
{
	if(m_pos + m_len < pos)
	{
		// Case 9
		return clone();
	}
	else if(m_pos >= pos + len)
	{
		// Case 10
		return new delete_operation<Document>(m_pos - len, m_len);
	}
	else if(pos <= m_pos && pos + len >= m_pos + m_len)
	{
		// Case 11
		return new no_operation<Document>;
	}
	else if(pos <= m_pos && pos + len < m_pos + m_len)
	{
		// Case 12
		return new delete_operation<Document>(
			pos,
			m_len - (pos + len - m_pos)
		);
	}
	else if(pos > m_pos && pos + len >= m_pos + m_len)
	{
		// Case 13
		return new delete_operation<Document>(
			m_pos,
			pos - m_pos
		);
	}
	else
	{
		// Case 14
		return new delete_operation<Document>(
			m_pos,
			m_len - len
		);
	}
}

template<typename Document>
void delete_operation<Document>::append_packet(net6::packet& pack) const
{
	pack << "del" << m_pos << m_len;
}

} // namespace obby

#endif // _OBBY_DELETE_OPERATION_HPP_

