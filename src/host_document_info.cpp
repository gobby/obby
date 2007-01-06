/* libobby - Network text editing library
 * Copyright (C) 2005 0x539 dev group
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public
 * License as published by the Free Software Foundation; either
 i* version 2 of the License, or (at your option) any later version.
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

#include "host_document_info.hpp"
#include "host_buffer.hpp"

obby::host_document_info::host_document_info(const basic_host_buffer<net6::selector>& buf,
                                             net6::host& host,
                                             const user* owner,
                                             unsigned int id,
                                             const std::string& title)
 : document_info(buf, owner, id, title),
   local_document_info(buf, owner, id, title),
   server_document_info(buf, host, owner, id, title, true)
{
	// Server documents infos have always a document assigned.
	assign_document();
}

obby::host_document_info::host_document_info(const basic_host_buffer<net6::selector>& buf,
                                             net6::host& host,
                                             const user* owner,
                                             unsigned int id,
                                             const std::string& title,
                                             bool noassign)
 : document_info(buf, owner, id, title),
   local_document_info(buf, owner, id, title),
   server_document_info(buf, host, owner, id, title)
{
}

obby::host_document_info::~host_document_info()
{
}

const obby::host_buffer& obby::host_document_info::get_buffer() const
{
	return dynamic_cast<const obby::host_buffer&>(m_buffer);
}

obby::host_document* obby::host_document_info::get_document()
{
	return dynamic_cast<obby::host_document*>(m_document);
}

const obby::host_document* obby::host_document_info::get_document() const
{
	return dynamic_cast<const obby::host_document*>(m_document);
}

void obby::host_document_info::rename(const std::string& new_title)
{
	// Rename with local user's ID instead of server ID 0.
	rename_impl(new_title, &get_buffer().get_self() );
}

void obby::host_document_info::subscribe()
{
	// Subscribe the local user.
	subscribe_user(get_buffer().get_self() );
}

void obby::host_document_info::unsubscribe()
{
	// Unsubscribe the local user.
	unsubscribe_user(get_buffer().get_self() );
}

void obby::host_document_info::assign_document()
{
	m_document = new host_document(
		*this, dynamic_cast<net6::host&>(m_server)
	);
}

