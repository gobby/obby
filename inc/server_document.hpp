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

#ifndef _OBBY_SERVER_DOCUMENT_HPP_
#define _OBBY_SERVER_DOCUMENT_HPP_

#include <net6/server.hpp>
#include "document.hpp"

namespace obby
{

class server_document_info;
class server_buffer;

/** Document used by server_buffer. Usually you do not create or delete
 * documents yourself, the buffers manage them.
 */

class server_document : virtual public document
{
public: 
	/** Creates a new server document.
	 */
	server_document(const server_document_info& info, net6::server& server);
	virtual ~server_document();

	/** Returns the document info for this document.
	 */
	const server_document_info& get_info() const;

	/** Returns the buffer to which the document is assigned.
	 */
	const server_buffer& get_buffer() const;

	/** Inserts <em>text</em> at <em>pos</em> and synchronises it with
	 * the clients.
	 */
	virtual void insert(position pos, const std::string& text);

	/** Removes text from the given area and synchronises it with the
	 * clients.
	 */
	virtual void erase(position begin, position end);

	/** Applys a record to the document.
	 */
	void apply_record(const record& rec);

	/** Synchronizes the document to a user.
	 */
	void synchronise(const user& to);

protected:
	/** Inserts <em>text</em> at the given position and marks it as
	 * written by the user with the id <em>author_id</em>.
	 */
	void insert(position pos, const std::string& text,
	            unsigned int author_id);

	/** Erases text from <em>begin</em> to <em>end</em> and marks the
	 * changes as performed by the user with the id <em>author_id</em>.
	 */
	void erase(position begin, position end, unsigned int author_id);

	/** Forwards a given record to all the clients that are subscribed
	 * to this document.
	 */
	void forward_record(const record& rec) const;

	net6::server& m_server;
};

}

#endif // _OBBY_SERVER_DOCUMENT_HPP_
