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

#include "zeroconf.hpp"

#include "config.hpp"
#if defined(WITH_BONJOUR)
# include "zeroconf_bonjour.hpp"
#elif defined(WITH_HOWL)
# include "zeroconf_howl.hpp"
#elif defined(WITH_AVAHI)
# include "zeroconf_avahi.hpp"
#else
# error This file cannot be compiled without neither Bonjour, Howl nor Avahi.
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

zeroconf_base::signal_discover6_type zeroconf_base::discover6_event() const
{
	return m_signal_discover6;
}

zeroconf_base::signal_leave_type zeroconf_base::leave_event() const
{
	return m_signal_leave;
}

zeroconf::zeroconf()
{
#if defined(WITH_BONJOUR)
	m_delegate.reset(new zeroconf_bonjour() );
#elif defined(WITH_HOWL)
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

zeroconf::signal_discover6_type zeroconf::discover6_event() const
{
	return m_delegate->discover6_event();
}

}

