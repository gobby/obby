/* libobby - Network text editing library
 * Copyright (C) 2005-2006 0x539 dev group
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

#include <howl.h>
#include "zeroconf.hpp"

namespace obby
{

/* Concrete Zeroconf implementation using the Howl library */
class zeroconf_howl : public zeroconf_base
{
public:
	zeroconf_howl();
	~zeroconf_howl();

	virtual void publish(const std::string& name, unsigned int port);
	virtual void unpublish(const std::string& name);
	virtual void unpublish_all();
	virtual void discover();
	virtual void select();
	virtual void select(unsigned int msecs);

private:
	std::map<std::string, sw_discovery_oid> m_published;

	sw_discovery m_session;
	sw_salt m_salt;

	static sw_result HOWL_API handle_publish_reply(sw_discovery discovery,
			sw_discovery_oid oid,
			sw_discovery_publish_status status,
			sw_opaque extra);
	static sw_result HOWL_API handle_browse_reply(sw_discovery discovery,
			sw_discovery_oid id,
			sw_discovery_browse_status status,
			sw_uint32 interface_index,
			sw_const_string name,
			sw_const_string type,
			sw_const_string domain,
			sw_opaque extra);
	static sw_result HOWL_API handle_resolve_reply(sw_discovery discovery,
			sw_discovery_oid oid,
			sw_uint32 interface_index,
			sw_const_string name,
			sw_const_string type,
			sw_const_string domain,
			sw_ipv4_address address,
			sw_port port,
			sw_octets text_record,
			sw_ulong text_record_len,
			sw_opaque extra);
};

}

