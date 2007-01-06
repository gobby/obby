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

#ifndef _OBBY_LOCAL_DOCUMENT_INFO_HPP_
#define _OBBY_LOCAL_DOCUMENT_INFO_HPP_

#include "local_document.hpp"
#include "document_info.hpp"

namespace obby
{

class local_buffer;

/** Information about a document that is provided without being subscribed to
 * a document.
 */

class local_document_info : virtual public document_info
{
public:
	local_document_info(const local_buffer& buf, const user* owner,
	                    unsigned int id, const std::string& title);
	~local_document_info();

	/** Returns the buffer associated with the document.
	 */
	const local_buffer& get_buffer() const;

	/** Returns the document for this info, if one is assigned.
	 */
	local_document* get_document();

	/** Returns the document for this info, if one is assigned.
	 */
	const local_document* get_document() const;

	/** Sends a subscribe request for the local user. If the subscribe
	 * request succeeded, the subscribe_event will be emitted.
	 */
	virtual void subscribe() = 0;

	/** Unsubscribes the local user from this document. signal_unsubscribe
	 * will be emitted if the request has been accepted.
	 */
	virtual void unsubscribe() = 0;

protected:
	/** Assigns a document to the document info.
	 */
	virtual void assign_document() = 0;
};

}

#endif // _OBBY_DOCUMENT_INFO_HPP_
