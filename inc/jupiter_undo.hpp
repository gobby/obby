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
 */
class jupiter_undo: private net6::non_copyable
{
public:
	static const unsigned int MAX_UNDO;

	jupiter_undo(const document& doc);
	~jupiter_undo();

	/** Adds a new client to the undo manager.
	 */
	void client_add(const user& client);

	/** Removes a client from the undo manager.
	 */
	void client_remove(const user& client);

        /** Operation op has been performed by <em>from</em>.
         */
	void perform_op(const operation& op, const user* from);

	/** Returns TRUE if the user can undo its last operation.
	 */
	bool can_undo(const user* user);

	/** Returns an operation that undoes the last operation of this user.
	 */
	std::auto_ptr<operation> undo(const user* user);
protected:
	typedef ring<operation*> operation_ring;
	typedef std::map<const user*, operation_ring*> operation_map;

	/** Internal function to transform the complete undo map against a
	 * single operation. TODO: Optimize this function!
	 */
	void transform_undo_map(const operation& op, const user* from);

	const document& m_doc;
	operation_map m_opmap;
};

} // namespace obby

#endif // _OBBY_JUPITER_UNDO_HPP_

