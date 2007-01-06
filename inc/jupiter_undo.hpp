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

#ifndef _OBBY_JUPITER_UNDO_HPP_
#define _OBBY_JUPITER_UNDO_HPP_

#include <net6/non_copyable.hpp>
#include "ring.hpp"
#include "user.hpp"
#include "document.hpp"
#include "operation.hpp"

namespace obby
{

/** Undo manager for the jupiter class.
 *
 * TODO: IMPLEMENT ME!
 */
template<typename Document>
class jupiter_undo: private net6::non_copyable
{
public:
	typedef Document document_type;

	jupiter_undo(const document_type& doc);
	~jupiter_undo();

	/** Adds a new client to the undo manager.
	 */
	void client_add(const user& client);

	/** Removes a client from the undo manager.
	 */
	void client_remove(const user& client);

        /** Operation <em>op</em> has been performed locally by <em>from</em>.
         */
	void local_op(const operation& op, const user* from);

	/** Operation <em>op</em> has been performed remotely by <em>from</em>.
	 */
	void remote_op(const operation& op, const user* from);

	/** Returns TRUE if the user can undo its last operation.
	 */
	bool can_undo();

	/** Returns an operation that undoes the last operation of this user.
	 */
	std::auto_ptr<operation> undo();
protected:
	const document_type& m_doc;
};

template<typename Document>
jupiter_undo<Document>::jupiter_undo(const document_type& document):
	m_doc(document)
{
}

template<typename Document>
jupiter_undo<Document>::~jupiter_undo()
{
}

template<typename Document>
void jupiter_undo<Document>::client_add(const user& client)
{
}

template<typename Document>
void jupiter_undo<Document>::client_remove(const user& client)
{
}

template<typename Document>
void jupiter_undo<Document>::local_op(const operation& op, const user* from)
{
}

template<typename Document>
void jupiter_undo<Document>::remote_op(const operation& op, const user* from)
{
}

template<typename Document>
bool jupiter_undo<Document>::can_undo()
{
	return false;
}

template<typename Document>
std::auto_ptr<operation> jupiter_undo<Document>::undo()
{
	throw std::logic_error(
		"obby::jupiter_undo::undo:\n"
		"Not implemented!"
	);
}

} // namespace obby

#endif // _OBBY_JUPITER_UNDO_HPP_
