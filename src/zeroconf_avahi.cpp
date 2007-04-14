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
#include <iostream>
#include <assert.h>

#include <avahi-client/client.h>
#include <avahi-client/lookup.h>
#include <avahi-client/publish.h>
#include <avahi-common/simple-watch.h>
#include <avahi-common/error.h>

#include "zeroconf_avahi.hpp"
#include "common.hpp"

namespace obby
{

zeroconf_avahi::zeroconf_avahi()
 : m_client(NULL), m_simple_poll(NULL), m_sb(NULL), m_group(NULL)
{
	int error;

	m_simple_poll = avahi_simple_poll_new();

	m_client = avahi_client_new(avahi_simple_poll_get(m_simple_poll),
		static_cast<AvahiClientFlags>(0),
		&zeroconf_avahi::avahi_client_callback, this, &error);
	if(!m_client)
	{
		std::stringstream stream;
		stream << "Failed to create client: " << avahi_strerror(error);
		throw std::runtime_error(stream.str() );
	}
}

zeroconf_avahi::zeroconf_avahi(AvahiPoll* poll):
	m_client(NULL), m_simple_poll(NULL), m_sb(NULL), m_group(NULL)
{
	int error;

	m_client = avahi_client_new(poll, static_cast<AvahiClientFlags>(0),
		&zeroconf_avahi::avahi_client_callback, this, &error);

	if(!m_client)
	{
		std::stringstream stream;
		stream << "Failed to create client: " << avahi_strerror(error);
		throw std::runtime_error(stream.str() );
	}
}

zeroconf_avahi::~zeroconf_avahi()
{
	if(m_group)
		avahi_entry_group_free(m_group);
	if(m_sb)
		avahi_service_browser_free(m_sb);
	if(m_client)
		avahi_client_free(m_client);
	if(m_simple_poll)
		avahi_simple_poll_free(m_simple_poll);
}

void zeroconf_avahi::publish(const std::string& name, unsigned int port)
{
	std::cout << name << port << std::endl;

	if(m_group == NULL)
	{
		m_group = avahi_entry_group_new(
			m_client,
			&zeroconf_avahi::avahi_entry_group_callback,
			this
			);
	}

	std::string version = "version=";
	version += obby_version();
	avahi_entry_group_add_service(
		m_group,
		AVAHI_IF_UNSPEC,
		AVAHI_PROTO_INET,
		static_cast<AvahiPublishFlags>(0),
		name.c_str(),
		"_lobby._tcp",
		NULL,
		NULL,
		port,
		version.c_str(),
		NULL
		);

	avahi_entry_group_commit(m_group);
}

void zeroconf_avahi::unpublish(const std::string& name)
{
	throw std::runtime_error("Not yet implemented");
}

void zeroconf_avahi::unpublish_all()
{
	if(m_group != NULL)
	{
		avahi_entry_group_free(m_group);
		m_group = NULL;
	}
}

void zeroconf_avahi::discover()
{
	std::cout << "discover" << std::endl;
	/* If there is already a service browser present we need to
	 * replace it to receive all already emitted servers.
	 */
	if(m_sb)
	{
		avahi_service_browser_free(m_sb);
		m_sb = NULL;
	}
	std::cout << "discover2" << std::endl;

	m_sb = avahi_service_browser_new(
		m_client,
		AVAHI_IF_UNSPEC,
		AVAHI_PROTO_INET,
		"_lobby._tcp",
		NULL,
		static_cast<AvahiLookupFlags>(0),
		&zeroconf_avahi::avahi_browse_callback,
		this
		);

	std::cout << "discover3 " << m_sb << std::endl;
	if(!m_sb)
	{
		std::stringstream stream;
		stream << "Failed to create service browser: "
		       << avahi_strerror(avahi_client_errno(m_client));
		throw std::runtime_error(stream.str() );
	}
}

void zeroconf_avahi::select()
{
	assert(m_simple_poll != NULL);
	avahi_simple_poll_loop(m_simple_poll);
}

void zeroconf_avahi::select(unsigned int msecs)
{
	assert(m_simple_poll != NULL);

	/* Work around Avahi bug */
	if(msecs == 0) msecs = 1;
	avahi_simple_poll_iterate(m_simple_poll, msecs);
}

void zeroconf_avahi::avahi_client_callback(AvahiClient* client,
	AvahiClientState state, void* userdata)
{
	std::cout << (state == AVAHI_CLIENT_S_RUNNING) << std::endl;
	switch(state)
	{
	case AVAHI_CLIENT_FAILURE:
		throw std::runtime_error("Avahi failed to connect");
	}
}

void zeroconf_avahi::avahi_browse_callback(AvahiServiceBrowser* sb,
	AvahiIfIndex interface, AvahiProtocol protocol, AvahiBrowserEvent event,
	const char* name, const char* type, const char* domain,
	AvahiLookupResultFlags flags, void* userdata)
{
	std::cout << "browse!" << std::endl;
	zeroconf_avahi* obj = static_cast<zeroconf_avahi*>(userdata);
	AvahiClient* cl = obj->m_client;

	switch(event)
	{
	case AVAHI_BROWSER_FAILURE:
		std::cerr << "(Browser) " << avahi_strerror(
			avahi_client_errno(avahi_service_browser_get_client(sb))
			) << std::endl;
		return;

	case AVAHI_BROWSER_NEW:
		std::cout << "AVAHI_BROWSER_NEW " << name << std::endl;
		if(!avahi_service_resolver_new(cl, interface, protocol, name,
			type, domain, AVAHI_PROTO_INET,
			static_cast<AvahiLookupFlags>(0),
			&zeroconf_avahi::avahi_resolve_callback,
			userdata))
		{
			std::stringstream stream;
			stream << "Failed to resolve service '" << name << "': "
			       << avahi_strerror(avahi_client_errno(cl));
			throw std::runtime_error(stream.str() );
		}
		break;
	case AVAHI_BROWSER_REMOVE:
		std::cout << "AVAHI_BROWSER_REMOVE " << name << std::endl;
		obj->leave_event().emit(name);
		break;
	
	case AVAHI_BROWSER_ALL_FOR_NOW:
	case AVAHI_BROWSER_CACHE_EXHAUSTED:
		std::cout << "(Browser) "
			<< (event == AVAHI_BROWSER_CACHE_EXHAUSTED ?
				"CACHE_EXHAUSTED" : "ALL_FOR_NOW")
			<< std::endl;
		break;
	default: 
		std::cout << "uncaught " << std::endl;
	}
}

void zeroconf_avahi::avahi_resolve_callback(AvahiServiceResolver* r,
	AvahiIfIndex interface, AvahiProtocol protocol,
	AvahiResolverEvent event, const char* name, const char* type,
	const char* domain, const char* hostname, const AvahiAddress* addr,
	uint16_t port, AvahiStringList* txt, AvahiLookupResultFlags flags,
	void* userdata)
{
	switch(event)
	{
	case AVAHI_RESOLVER_FOUND:
		std::cout << "New Service: " << name << "(" << port << ")" << std::endl; 
		static_cast<zeroconf_avahi*>(userdata)->discover_event().emit(
			name, net6::ipv4_address::create_from_address(
			addr->data.ipv4.address, port));
	}

	/* Clean up */
	avahi_service_resolver_free(r);
}

void zeroconf_avahi::avahi_entry_group_callback(AvahiEntryGroup* g,
	AvahiEntryGroupState state, void* userdata)
{
}

}

