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

#ifndef _OBBY_LOCAL_BUFFER_HPP_
#define _OBBY_LOCAL_BUFFER_HPP_

#include "buffer.hpp"
#include "local_document_info.hpp"

namespace obby
{

/** A local_buffer is a buffer object with a local user.
 */
	
class local_buffer : virtual public buffer
{
public: 
	local_buffer();
	virtual ~local_buffer();

	/** Returns the local user.
	 */
	virtual user& get_self() = 0;

	/** Returns the local user.
	 */
	virtual const user& get_self() const = 0;

	/** Returns the name of the local user. This method is overwritten
	 * by client_buffer to provide access to the user's name even if the
	 * login process has not completed.
	 */
	virtual const std::string& get_name() const;

	/** Looks for a document with the given ID which belongs to the user
	 * with the given owner ID. Note that we do not take a real user object
	 * here because the ID is enough and one might not have a user object
	 * to the corresponding ID. So a time-consuming lookup is obsolete.
	 */
	local_document_info* find_document(unsigned int owner_id,
	                                   unsigned int id) const;

protected:
        /** Adds a new document with the given title to the buffer.
	 */
	virtual document_info& add_document_info(const user* owner,
	                                         unsigned int id,
	                                         const std::string& title) = 0;
};

}

#endif // _OBBY_LOCAL_BUFFER_HPP_
