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

#ifndef _OBBY_JUPITER_SERVER_HPP_
#define _OBBY_JUPITER_SERVER_HPP_

#include <map>
#include <net6/non_copyable.hpp>
#include "operation.hpp"
#include "record.hpp"
#include "jupiter_algorithm.hpp"
#include "jupiter_undo.hpp"

namespace obby
{

/** Jupiter server implementation.
 */
template<typename Document>
class jupiter_server: private net6::non_copyable
{
public:
	typedef Document document_type;
	typedef jupiter_algorithm<document_type> algorithm_type;
	typedef jupiter_undo<document_type> undo_type;

	typedef sigc::signal<void, const record&, const user&, const user*>
		signal_record_type;

	/** Creates a new jupiter_server which uses the given document.
	 * Local and remote changes are applied to this document.
	 */
	jupiter_server(document_type& doc);
	~jupiter_server();

	/** Adds a new client to the server.
	 */
	void client_add(const user& client);

	/** Removes a client from the server.
	 */
	void client_remove(const user& client);

	/** Performs a local operation by the user <em>from</em>. record_event
	 * will be emitted for each client with a corresponding
	 * record that may be transmitted to it.
	 */
	void local_op(const operation& op, const user* from);

	/** Performs a remote operation by the user <em>from</em>. record_event
	 * will be emitted for each client except <em>from</em> with a record
	 * that may be transmitted to it.
	 */
	void remote_op(const record& rec, const user* from);

	/** Undoes the last operation by the user <em>from</em>. record_event
	 * will be emitted for each client with a corresponding record that
	 * may be transmitted to it.
	 */
	void undo_op(const user* from);

	/** Signal which will be emitted when a local operation has been
	 * applied.
	 */
	signal_record_type record_event() const;
protected:
	typedef std::map<const user*, algorithm_type*> client_map;

	client_map m_clients;
	document_type& m_document;
	undo_type m_undo;

	signal_record_type m_signal_record;
};

template<typename Document>
jupiter_server<Document>::jupiter_server(document_type& doc):
	m_document(doc), m_undo(doc)
{
}

template<typename Document>
jupiter_server<Document>::~jupiter_server()
{
	for(typename client_map::iterator iter = m_clients.begin();
	    iter != m_clients.end();
	    ++ iter)
	{
		delete iter->second;
	}
}

template<typename Document>
void jupiter_server<Document>::client_add(const user& client)
{
	if(m_clients.find(&client) != m_clients.end() )
	{
		throw std::logic_error(
			"obby::jupiter_server::client_add:\n"
			"Client has already been added"
		);
	}

	m_clients[&client] = new algorithm_type;
}

template<typename Document>
void jupiter_server<Document>::client_remove(const user& client)
{
	typename client_map::iterator iter = m_clients.find(&client);
	if(iter == m_clients.end() )
	{
		throw std::logic_error(
			"obby::jupiter_server::client_remove:\n"
			"Client has not been added"
		);
	}

	delete iter->second;
	m_clients.erase(iter);
}

template<typename Document>
void jupiter_server<Document>::local_op(const operation& op, const user* from)
{
	op.apply(m_document, from);
	m_undo.local_op(op, from);

	for(typename client_map::iterator iter = m_clients.begin();
	    iter != m_clients.end();
	    ++ iter)
	{
		std::auto_ptr<record> rec = iter->second->local_op(op);
		m_signal_record.emit(*rec, *iter->first, from);
	}
}

template<typename Document>
void jupiter_server<Document>::remote_op(const record& rec, const user* from)
{
	typename client_map::iterator iter = m_clients.find(from);
	if(iter == m_clients.end() )
	{
		throw std::logic_error(
			"obby::jupiter_server::remote_op:\n"
			"Client has not been added"
		);
	}

	std::auto_ptr<operation> op = iter->second->remote_op(rec);
	op->apply(m_document, from);
	m_undo.remote_op(*op, from);

	for(iter = m_clients.begin(); iter != m_clients.end(); ++ iter)
	{
		if(iter->first != from)
		{
			std::auto_ptr<record> rec = iter->second->local_op(*op);
			m_signal_record.emit(*rec, *iter->first, from);
		}
	}
}

template<typename Document>
void jupiter_server<Document>::undo_op(const user* from)
{
	std::auto_ptr<operation> op = m_undo.undo();
	op->apply(m_document, from);

	for(typename client_map::iterator iter = m_clients.begin();
	    iter != m_clients.end();
	    ++ iter)
	{
		std::auto_ptr<record> rec = iter->second->local_op(*op);
		m_signal_record.emit(*rec, *iter->first, from);
	}
}

template<typename Document>
typename jupiter_server<Document>::signal_record_type
jupiter_server<Document>::record_event() const
{
	return m_signal_record;
}

} // namespace obby

#endif // _OBBY_JUPITER_SERVER_HPP_
