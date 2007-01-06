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

#ifndef _OBBY_HOST_DOCUMENT_INFO_HPP_
#define _OBBY_HOST_DOCUMENT_INFO_HPP_

#include <net6/host.hpp>
#include "host_document.hpp"
#include "local_document_info.hpp"
#include "server_document_info.hpp"

namespace obby
{

template<typename selector_type>
class basic_host_buffer;

/** Information about a document that is provided without being subscribed to
 * a document.
 */

class host_document_info : public local_document_info,
                           public server_document_info
{
public:
	host_document_info(const basic_host_buffer<net6::selector>& buf, net6::host& host,
	                   const user* owner, unsigned int id,
	                   const std::string& title);
	~host_document_info();

	/** Returns the buffer to which the document is assigned.
	 */
	const basic_host_buffer<net6::selector>& get_buffer() const;

	/** Returns the document for this info, if one is assigned.
	 */
	host_document* get_document();

	/** Returns the document for this info, if one is assigned.
	 */
	const host_document* get_document() const;

	/** Renames the current document.
	 */
	virtual void rename(const std::string& new_title);

	/** Sends a subscribe request for the local user. If the subscribe
	 * request succeeded, the subscribe_event will be emitted.
	 */
	virtual void subscribe();

	/** Unsubscribes the local user from this document. signal_unsubscribe
	 * will be emitted if the request has been accepted.
	 */
	virtual void unsubscribe();

protected:
	/** Protected constructor that may be used by derived classes.
	 * server_document_infos do always assign their document immediately
	 * because they need it to share changes between clients. This
	 * constructor does not assign such a document, so a derived class
	 * is able to call this constructor and then its own assign_document
	 * function to create its own document. For example, the
	 * server_document_info creates a server_document in assign_document,
	 * but the host_document_info needs to create a host_document.
	 */
	host_document_info(const basic_host_buffer<net6::selector>& buf, net6::host& host,
	                   const user* owner, unsigned int id,
			   const std::string& title, bool noassign);

	/** Assigns a document to the document info.
	 */
	virtual void assign_document();
};

}

#endif // _OBBY_HOST_DOCUMENT_INFO_HPP_
