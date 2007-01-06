/* libobby - Network text editing library
 * Copyright (C) 2005 0x539 dev group
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this program; if not, write to the Free
 * Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#ifndef _OBBY_RENDEZVOUS_HPP_
#define _OBBY_RENDEZVOUS_HPP_

#include <string>

#include <howl.h>

namespace obby
{

class rendezvous
{
public:
	rendezvous();
	~rendezvous();

	/** Publishes a record to the local rendezvous community */
	void publish(const std::string& name,
		     const std::string& type,
		     const std::string& domain,
		     unsigned int port);
	
	void discover(const std::string& type,
		      const std::string& domain);
	
protected:
	sw_discovery m_session;

private:
	static sw_result handle_publish_reply(sw_discovery discovery,
			sw_discovery_oid oid,
			sw_discovery_publish_status status,
			sw_opaque extra);
	static sw_result handle_browse_reply(sw_discovery discovery,
			sw_discovery_oid id,
			sw_discovery_browse_status status,
			sw_uint32 interface_index,
			sw_const_string name,
			sw_const_string type,
			sw_const_string domain,
			sw_opaque extra);
};

}

#endif // _OBBY_RENDEZVOUS_HPP_
