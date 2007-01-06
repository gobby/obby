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
#include <net6/non_copyable.hpp>
#include <net6/address.hpp>

namespace obby
{

class rendezvous : private net6::non_copyable
{
public:
	typedef sigc::signal<void, const std::string&, 
		const net6::ipv4_address&> signal_discover_type;
	typedef sigc::signal<void, const std::string&> signal_leave_type;
	
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

	/** Process all rendezvous events. This procedure does not return,
	 * so it should be used in an own thread. */
	void select();

	/** Process all available rendezvous events in a timeframe of
	 * <em>msecs</em> milliseconds. A value of 0 will prevent the command
	 * from blocking the caller. */
	void select(unsigned int msecs);
	
	signal_discover_type discover_event() const;
	signal_leave_type leave_event() const;
	
protected:
	sw_discovery m_session;
	sw_salt m_salt;
	
	signal_discover_type m_signal_discover;
	signal_leave_type m_signal_leave;

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
