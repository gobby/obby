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

#ifndef _OBBY_INSERT_OPERATION_HPP_
#define _OBBY_INSERT_OPERATION_HPP_

#include "line.hpp"
#include "operation.hpp"

namespace obby
{

/** insert_operation inserts an amount of text at a specified position in
 * the document.
 *
 * This is the base class for both reversed and non-reversed insert
 * operations that implements common functionality.
 */
template<typename Document, typename String>
class basic_insert_operation: public operation<Document>
{
public:
	typedef operation<Document> operation_type;
	typedef basic_insert_operation<Document, String>
		base_insert_operation_type;

	typedef typename operation_type::document_type document_type;
	typedef String string_type;

	basic_insert_operation(position pos,
	                       const String& text);

	/** Creates a copy of this operation.
	 */
	virtual operation_type* clone() const;

	/** Creates the reverse operation of this one.
	 * @param doc Document to receive additional information from.
	 */
	virtual operation_type* reverse(const document_type& doc) const;

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
protected:
	position m_pos;
	string_type m_text;

	// Has to be implemented by subclasses since basic_insert_operation
	// is abstract
	virtual base_insert_operation_type*
	construct(position pos,
	          const string_type& text) const = 0;
};

/** Operation that insert text at a position in the document.
 */
template<typename Document>
class insert_operation: public basic_insert_operation<Document, std::string>
{
public:
	typedef operation<Document> operation_type;
	typedef typename basic_insert_operation<Document, std::string>::
		base_insert_operation_type base_insert_operation_type;

	typedef typename operation_type::document_type document_type;
	typedef typename basic_insert_operation<Document, std::string>::
		string_type string_type;

	insert_operation(position pos,
	                 const string_type& text);

	insert_operation(const net6::packet& pack,
	                 unsigned int& index);

	virtual void apply(document_type& doc,
	                   const user* author) const;

	virtual void append_packet(net6::packet& pack) const;
protected:
	virtual base_insert_operation_type*
	construct(position pos,
	          const string_type& text) const;
};

/** @brief Operation that is the result of a reverse() call to delete_operation.
 *
 * A delete operation may have deleted text of multiple users, so this class
 * also stores the authors of the text to insert.
 *
 * TODO: Rename in reversed_insert_operation
 */
template<typename Document>
class reversible_insert_operation: public basic_insert_operation<Document, line>
{
public:
	typedef operation<Document> operation_type;
	typedef typename basic_insert_operation<Document, line>::
		base_insert_operation_type base_insert_operation_type;

	typedef typename operation_type::document_type document_type;
	typedef typename basic_insert_operation<Document, line>::string_type
		string_type;

	reversible_insert_operation(position pos,
	                            const string_type& text);

	reversible_insert_operation(const net6::packet& pack,
	                            unsigned int& index,
	                            const user_table& user_table);

	virtual void apply(document_type& doc,
	                   const user* author) const;

	virtual void append_packet(net6::packet& pack) const;
protected:
	virtual base_insert_operation_type*
	construct(position pos,
	          const string_type& text) const;
};

template<typename Document, typename String>
basic_insert_operation<Document, String>::
	basic_insert_operation(position pos,
                               const string_type& text):
	operation<Document>(), m_pos(pos), m_text(text)
{
}

template<typename Document, typename String>
typename basic_insert_operation<Document, String>::operation_type*
basic_insert_operation<Document, String>::clone() const
{
	return construct(m_pos, m_text);
}

template<typename Document, typename String>
typename basic_insert_operation<Document, String>::operation_type*
basic_insert_operation<Document, String>::
	reverse(const document_type& doc) const
{
	return new delete_operation<Document>(m_pos, m_text.length() );
}

template<typename Document, typename String>
typename basic_insert_operation<Document, String>::operation_type*
basic_insert_operation<Document, String>::
	transform(const operation_type& base_op) const
{
	return base_op.transform_insert(m_pos, m_text);
}

template<typename Document, typename String>
typename basic_insert_operation<Document, String>::operation_type*
basic_insert_operation<Document, String>::
	transform_insert(position pos,
                         const std::string& text) const
{
	if(m_pos < pos)
	{
		// Case 1
		return clone();
	}
	else if(m_pos == pos)
	{
		// Special case
		if(m_text < text)
		{
			return clone();
		}
		else
		{
			return construct(m_pos + m_text.length(), m_text);
		}
	}
	else
	{
		// Case 2
		return construct(
			m_pos + m_text.length(),
			m_text
		);
	}
}

template<typename Document, typename String>
typename basic_insert_operation<Document, String>::operation_type*
basic_insert_operation<Document, String>::transform_delete(position pos,
                                                           position len) const
{
	if(m_pos <= pos)
	{
		// Case 3
		return clone();
	}
	else if(m_pos > pos + len)
	{
		// Case 4
		return construct(m_pos - len, m_text);
	}
	else
	{
		// Case 5
		return construct(pos, m_text);
	}
}

template<typename Document>
insert_operation<Document>::insert_operation(position pos,
                                             const string_type& text):
	basic_insert_operation<Document, std::string>(pos, text)
{
}

template<typename Document>
insert_operation<Document>::insert_operation(const net6::packet& pack,
                                             unsigned int& index):
	basic_insert_operation<Document, std::string>(
		pack.get_param(index + 0).net6::parameter::as<int>(),
		pack.get_param(index + 1).net6::parameter::as<std::string>()
	)
{
	index += 2;
}

template<typename Document>
void insert_operation<Document>::apply(document_type& doc,
                                       const user* author) const
{
	doc.insert(
		basic_insert_operation<Document, std::string>::m_pos,
		basic_insert_operation<Document, std::string>::m_text,
		author
	);
}

template<typename Document>
void insert_operation<Document>::append_packet(net6::packet& pack) const
{
	pack << "ins" << basic_insert_operation<Document, std::string>::m_pos
	     << basic_insert_operation<Document, std::string>::m_text;
}

template<typename Document>
typename insert_operation<Document>::base_insert_operation_type*
insert_operation<Document>::construct(position pos,
                                      const string_type& text) const
{
	return new insert_operation<Document>(pos, text);
}

template<typename Document>
reversible_insert_operation<Document>::
	reversible_insert_operation(position pos,
	                            const string_type& text):
	basic_insert_operation<Document, line>(pos, text)
{
}

template<typename Document>
reversible_insert_operation<Document>::
	reversible_insert_operation(const net6::packet& pack,
	                            unsigned int& index,
	                            const user_table& user_table):
	basic_insert_operation<Document, line>(
		pack.get_param(index ++).net6::parameter::as<int>(),
		line(pack, index, user_table)
	)
{
}

template<typename Document>
void reversible_insert_operation<Document>::apply(document_type& doc,
                                                  const user* author) const
{
	doc.insert(
		basic_insert_operation<Document, line>::m_pos,
		basic_insert_operation<Document, line>::m_text
	);
}

template<typename Document>
void reversible_insert_operation<Document>::
	append_packet(net6::packet& pack) const
{
	pack << "revins" << basic_insert_operation<Document, line>::m_pos;
	basic_insert_operation<Document, line>::m_text.append_packet(pack);
}

template<typename Document>
typename reversible_insert_operation<Document>::base_insert_operation_type*
reversible_insert_operation<Document>::construct(position pos,
                                                 const string_type& text) const
{
	return new reversible_insert_operation<Document>(pos, text);
}

} // namespace obby

#endif // _OBBY_INSERT_OPERATION_HPP_
