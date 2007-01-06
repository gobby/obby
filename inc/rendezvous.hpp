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

#include <sigc++/signal.h>
#include <howl.h>

namespace obby
{

class rendezvous
{
public:
	typedef sigc::signal<void, const std::string&, const std::string&,
		unsigned int> signal_discover_type;

	rendezvous();
	~rendezvous();

	/** Publishes a record to other users of this library within the
	 * default domain (.local). It uses the service identifier
	 * _lobby._tcp. <em>name</em> is the value which should be displayed
	 * when other users are discovering this record. */
	void publish(const std::string& name, unsigned int port);

	/** Discovers other users in the local network within the default
	 * domain (.local). It searches for participants with the service
	 * identifier set to _lobby._tcp. It emits a signal when a new user
	 * is found, handing over the name, the ip and the port of the
	 * participant. */
	void discover();

	void select();
	void select(unsigned int msecs);
	
	signal_discover_type discover_event() const;

protected:
	sw_discovery m_session;
	sw_salt m_salt;
	
	signal_discover_type m_signal_discover;

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
	static sw_result handle_resolve_reply(sw_discovery discovery,
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

#endif // _OBBY_RENDEZVOUS_HPP_
