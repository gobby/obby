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

#include <stdexcept>
#include <sstream>

#include "zeroconf.hpp"

#include "config.hpp"
#if defined(WITH_HOWL)
# include <howl.h>
#elif defined(WITH_AVAHI)
# include <avahi-client/client.h>
#else
# error This file cannot be compiled without neither Howl nor Avahi.
#endif

namespace obby
{

zeroconf_base::zeroconf_base()
{
}

zeroconf_base::signal_discover_type zeroconf_base::discover_event() const
{
	return m_signal_discover;
}

zeroconf_base::signal_leave_type zeroconf_base::leave_event() const
{
	return m_signal_leave;
}

#if defined(WITH_HOWL)
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

zeroconf_howl::zeroconf_howl()
{
	if (sw_discovery_init(&m_session) != SW_OKAY)
		throw std::runtime_error("sw_discovery_init() failed.");
	if (sw_discovery_salt(m_session, &m_salt) != SW_OKAY)
		throw std::runtime_error("sw_discovery_salt() failed.");
}

zeroconf_howl::~zeroconf_howl()
{
	unpublish_all();
	sw_discovery_fina(m_session);
}

void zeroconf_howl::publish(const std::string& name, unsigned int port)
{
	sw_discovery_oid oid;
	sw_result result;

	/* This publishes a record for other users of this library within the
	 * default domain (.local) */
	if((result = sw_discovery_publish(m_session, 0, name.c_str(),
		"_lobby._tcp.", NULL, NULL, port, NULL, 0,
	       	&zeroconf_howl::handle_publish_reply,
		static_cast<sw_opaque>(this), &oid)) != SW_OKAY)
	{
		std::stringstream stream;
		stream << "sw_discovery_publish(...) failed: " << result;
		throw std::runtime_error(stream.str());
	}
	else
	{
		m_published[name] = oid;
	}
}

void zeroconf_howl::unpublish(const std::string& name)
{
	if(!m_published[name])
	{
		std::stringstream stream;
		stream << "unpublish not possible for \"" << name << "\"";
		throw std::runtime_error(stream.str());
	}
	else
	{
		sw_discovery_cancel(m_session, m_published[name]);
	}
	m_published.erase(name);
}

void zeroconf_howl::unpublish_all()
{
	std::map<std::string, sw_discovery_oid>::iterator i;
	for(i = m_published.begin(); i != m_published.end(); ++i)
		sw_discovery_cancel(m_session, i->second);
	m_published.clear();
}

void zeroconf_howl::discover()
{
	sw_discovery_oid oid;
	sw_result result;

	if ((result = sw_discovery_browse(m_session, 0, "_lobby._tcp", NULL,
		&zeroconf_howl::handle_browse_reply,
		static_cast<sw_opaque>(this), &oid)) != SW_OKAY)
	{
		std::stringstream stream;
		stream << "discover failed: %d" << result;
		throw std::runtime_error(stream.str());
	}
}

void zeroconf_howl::select()
{
	sw_discovery_run(m_session);
}

void zeroconf_howl::select(unsigned int msecs)
{
	sw_ulong ms = msecs;
	sw_salt_step(m_salt, &ms);
}

sw_result zeroconf_howl::handle_publish_reply(sw_discovery discovery,
	sw_discovery_oid oid, sw_discovery_publish_status status,
	sw_opaque extra)
{
	if (status != SW_OKAY)
	{
		std::stringstream stream;
		stream << "publish failed: %d" << status;
		throw std::runtime_error(stream.str());
	}
	return SW_OKAY;
}

sw_result zeroconf_howl::handle_browse_reply(sw_discovery discovery,
	sw_discovery_oid oid, sw_discovery_browse_status status,
	sw_uint32 interface_index, sw_const_string name, sw_const_string type,
	sw_const_string domain, sw_opaque extra)
{
	sw_discovery session = static_cast<zeroconf_howl*>(extra)->m_session;

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
				type, domain,
				&zeroconf_howl::handle_resolve_reply,
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
			static_cast<zeroconf_howl*>(
				extra)->leave_event().emit(name);
			break;
		}
	}

	return SW_OKAY;
}

sw_result zeroconf_howl::handle_resolve_reply(sw_discovery discovery,
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
		static_cast<zeroconf_howl*>(extra)->discover_event().emit(
			name, net6::ipv4_address::create_from_address(
			ip, port));
	return SW_OKAY;
}

#elif defined(WITH_AVAHI)
class zeroconf_avahi : public zeroconf_base
{
public:
	zeroconf_avahi();

private:
	AvahiClient* m_client;
};
#endif

zeroconf::zeroconf()
{
#if defined(WITH_HOWL)
	m_delegate.reset(new zeroconf_howl() );
#elif defined(WITH_AVAHI)
	m_delegate.reset(new zeroconf_avahi() );
#endif
}

zeroconf::~zeroconf()
{
}

void zeroconf::publish(const std::string& name, unsigned int port)
{
	m_delegate->publish(name, port);
}

void zeroconf::unpublish(const std::string& name)
{
	m_delegate->unpublish(name);
}

void zeroconf::unpublish_all()
{
	m_delegate->unpublish_all();
}

void zeroconf::discover()
{
	m_delegate->discover();
}

void zeroconf::select()
{
	m_delegate->select();
}

void zeroconf::select(unsigned int msecs)
{
	m_delegate->select(msecs);
}

zeroconf::signal_discover_type zeroconf::discover_event() const
{
	return m_delegate->discover_event();
}

zeroconf::signal_leave_type zeroconf::leave_event() const
{
	return m_delegate->leave_event();
}

}

