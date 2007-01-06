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

#include <avahi-client/client.h>
#include <avahi-client/lookup.h>
#include <avahi-client/publish.h>
#include <avahi-common/simple-watch.h>

#include "zeroconf.hpp"

namespace obby
{

class zeroconf_avahi : public zeroconf_base
{
public:
	zeroconf_avahi();
	~zeroconf_avahi();

	virtual void publish(const std::string& name, unsigned int port);
	virtual void unpublish(const std::string& name);
	virtual void unpublish_all();
	virtual void discover();
	virtual void select();
	virtual void select(unsigned int msecs);

private:
	AvahiClient* m_client;
	AvahiSimplePoll* m_poll;
	AvahiServiceBrowser* m_sb;
	AvahiEntryGroup* m_group;

	static void avahi_client_callback(AvahiClient* client,
			AvahiClientState state,
			void* userdata);
	static void avahi_browse_callback(AvahiServiceBrowser* sb,
			AvahiIfIndex interface,
			AvahiProtocol protocol,
			AvahiBrowserEvent event,
			const char* name,
			const char* type,
			const char* domain,
			AvahiLookupResultFlags flags,
			void* userdata);
	static void avahi_resolve_callback(AvahiServiceResolver* r,
			AvahiIfIndex interface,
			AvahiProtocol protocol,
			AvahiResolverEvent event,
			const char* name,
			const char* type,
			const char* domain,
			const char* hostname,
			const AvahiAddress* addr,
			uint16_t port,
			AvahiStringList* txt,
			AvahiLookupResultFlags flags,
			void* userdata);
	static void avahi_entry_group_callback(AvahiEntryGroup* g,
			AvahiEntryGroupState state,
			void* userdata);
};

}

