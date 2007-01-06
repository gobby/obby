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

#ifndef _OBBY_JUPITER_CLIENT_HPP_
#define _OBBY_JUPITER_CLIENT_HPP_

#include <net6/non_copyable.hpp>
#include "operation.hpp"
#include "record.hpp"
#include "jupiter_algorithm.hpp"
#include "jupiter_undo.hpp"

namespace obby
{

/** Jupiter client implementation.
 */
template<typename Document>
class jupiter_client: private net6::non_copyable
{
public:
	typedef Document document_type;
	typedef jupiter_algorithm<document_type> algorithm_type;
	typedef jupiter_undo<document_type> undo_type;
	typedef operation<document_type> operation_type;
	typedef record<document_type> record_type;

	typedef sigc::signal<void, const record_type&, const user*>
		signal_record_type;

	/** Creates a new jupiter_client which uses the given document.
	 * Local and remote changes are applied to this document.
	 */
	jupiter_client(document_type& doc);

	/** Adds a new client to the jupiter algorithm.
	 */
	void client_add(const user& client);

	/** Removes a client from the jupiter algorithm.
	 */
	void client_remove(const user& client);

	/** Performs a local operation by the user <em>from</em>. local_event
	 * will be emitted with a resulting record that may be transmitted
	 * to the server.
	 */
	void local_op(const operation_type& op, const user* from);

	/** Performs a remote operation by the user <em>from</em>.
	 */
	void remote_op(const record_type& rec, const user* from);

	/** Undoes the last operation.
	 */
	void undo_op(const user* from);

	/** Signal which will be emitted when a record has to be transmitted to
	 * the server.
	 */
	signal_record_type record_event() const;

protected:
	algorithm_type m_algorithm;
	undo_type m_undo;

	document_type& m_document;
	signal_record_type m_signal_record;
};

template<typename Document>
jupiter_client<Document>::jupiter_client(document_type& doc):
	m_undo(doc), m_document(doc)
{
}

template<typename Document>
void jupiter_client<Document>::client_add(const user& client)
{
	m_undo.client_add(client);
}

template<typename Document>
void jupiter_client<Document>::client_remove(const user& client)
{
	m_undo.client_remove(client);
}

template<typename Document>
void jupiter_client<Document>::local_op(const operation_type& op,
                                        const user* from)
{
	op.apply(m_document, from);
	m_undo.local_op(op, from);
	std::auto_ptr<record_type> rec(m_algorithm.local_op(op) );
	m_signal_record.emit(*rec, from);
}

template<typename Document>
void jupiter_client<Document>::remote_op(const record_type& rec,
                                         const user* from)
{
	std::auto_ptr<operation_type> op(m_algorithm.remote_op(rec) );
	op->apply(m_document, from);
	m_undo.remote_op(*op, from);
}

template<typename Document>
void jupiter_client<Document>::undo_op(const user* from)
{
	std::auto_ptr<operation_type> op = m_undo.undo();
	op->apply(m_document, from);
	std::auto_ptr<record_type> rec(m_algorithm.local_op(*op) );
	m_signal_record.emit(*rec, from);
}

template<typename Document>
typename jupiter_client<Document>::signal_record_type
jupiter_client<Document>::record_event() const
{
	return m_signal_record;
}

} // namespace obby

#endif // _OBBY_JUPITER_CLIENT_HPP_
