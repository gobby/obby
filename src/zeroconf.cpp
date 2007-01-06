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

#include "zeroconf.hpp"

#include <stdexcept>
#include <sstream>

obby::zeroconf::zeroconf()
{
	if (sw_discovery_init(&m_session) != SW_OKAY)
		throw std::runtime_error("sw_discovery_init() failed.");
	if (sw_discovery_salt(m_session, &m_salt) != SW_OKAY)
		throw std::runtime_error("sw_discovery_salt() failed.");
}

obby::zeroconf::~zeroconf()
{
	for(std::vector<sw_discovery_oid>::iterator i = m_published.begin();
		i != m_published.end(); ++i)
	{
		sw_discovery_cancel(m_session, *i);
	}
	sw_discovery_fina(m_session);
}

void obby::zeroconf::publish(const std::string& name, unsigned int port)
{
	sw_discovery_oid oid;
	sw_result result;

	/* This publishes a record for other users of this library within the
	 * default domain (.local) */
	if((result = sw_discovery_publish(m_session, 0, name.c_str(),
		"_lobby._tcp.", NULL, NULL, port, NULL, 0,
	       	&zeroconf::handle_publish_reply,
		static_cast<sw_opaque>(this), &oid)) != SW_OKAY)
	{
		std::stringstream stream;
		stream << "sw_discovery_publish(...) failed: " << result;
		throw std::runtime_error(stream.str());
	}
	else
	{
		m_published.push_back(oid);
	}
}

void obby::zeroconf::discover()
{
	sw_discovery_oid oid;
	sw_result result;

	if ((result = sw_discovery_browse(m_session, 0, "_lobby._tcp", NULL,
		&zeroconf::handle_browse_reply,
		static_cast<sw_opaque>(this), &oid)) != SW_OKAY)
	{
		std::stringstream stream;
		stream << "discover failed: %d" << result;
		throw std::runtime_error(stream.str());
	}
}

void obby::zeroconf::select()
{
	sw_discovery_run(m_session);
}

void obby::zeroconf::select(unsigned int msecs)
{
	sw_ulong ms = msecs;
	sw_salt_step(m_salt, &ms);
}

obby::zeroconf::signal_discover_type
obby::zeroconf::discover_event() const
{
	return m_signal_discover;
}

obby::zeroconf::signal_leave_type
obby::zeroconf::leave_event() const
{
	return m_signal_leave;
}

sw_result obby::zeroconf::handle_publish_reply(sw_discovery discovery,
	sw_discovery_oid oid, sw_discovery_publish_status status,
	sw_opaque extra)
{
	if (status != SW_OKAY)
	{
		std::stringstream stream;
		stream << "publish failed: " << status;
		throw std::runtime_error(stream.str());
	}
	return SW_OKAY;
}

sw_result obby::zeroconf::handle_browse_reply(sw_discovery discovery,
	sw_discovery_oid oid, sw_discovery_browse_status status,
	sw_uint32 interface_index, sw_const_string name, sw_const_string type,
	sw_const_string domain, sw_opaque extra)
{
	sw_discovery session = static_cast<obby::zeroconf*>(extra)->m_session;
	
	switch(status)
	{
		case SW_DISCOVERY_BROWSE_INVALID:
		{
			throw std::runtime_error(
				"sw_discovery failed within the callback");
			break;
		}

		case SW_DISCOVERY_BROWSE_ADD_SERVICE:
		{
			sw_result result;
			if ((result = sw_discovery_resolve(session, 0, name,
				type, domain, &zeroconf::handle_resolve_reply,
				extra, &oid)) != SW_OKAY)
			{
				std::stringstream stream;
				stream << "resolve failed: " << result;
				throw std::runtime_error(stream.str());
			}
			break;
		}

		case SW_DISCOVERY_BROWSE_REMOVE_SERVICE:
		{
			static_cast<obby::zeroconf*>(
				extra)->leave_event().emit(name);
			break;
		}
	}
	
	return SW_OKAY;
}

sw_result obby::zeroconf::handle_resolve_reply(sw_discovery discovery,
	sw_discovery_oid oid, sw_uint32 interface_index, sw_const_string name,
	sw_const_string type, sw_const_string domain, sw_ipv4_address address,
	sw_port port, sw_octets text_record, sw_ulong text_record_len,
	sw_opaque extra)
{
	// At least newer revisions of OS X emit 0.0.0.0 as a second pseudo
	// entry when discovering the local host as the IPv6 address gets
	// parsed as a IPv4 address anywhere within Howl which does not yet
	// support service discovery on IPv6.
	uint32_t ip(sw_ipv4_address_saddr(address));
	if(ip != 0)
		static_cast<obby::zeroconf*>(extra)->discover_event().emit(
			name, net6::ipv4_address::create_from_address(
			ip, port));
	return SW_OKAY;
}

