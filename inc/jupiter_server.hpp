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
class jupiter_server: private net6::non_copyable
{
public:
	typedef sigc::signal<void, const record&, const user&, const user*>
		signal_record_type;

	/** Creates a new jupiter_server which uses the given document.
	 * Local and remote changes are applied to this document.
	 */
	jupiter_server(document& doc);
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
	typedef std::map<const user*, jupiter_algorithm*> client_map;

	client_map m_clients;
	document& m_document;
	jupiter_undo m_undo;

	signal_record_type m_signal_record;
};

} // namespace obby

#endif // _OBBY_JUPITER_SERVER_HPP_
