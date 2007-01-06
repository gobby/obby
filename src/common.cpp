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

#include "common.hpp"
#include "config.hpp"

#ifdef ENABLE_NLS
namespace
{
	// obby gettext catalog
	net6::gettext_package* local_package = NULL;
}
#endif

extern "C"
{

const char* obby_version()
{
	return PACKAGE_VERSION;
}

const char* obby_codename()
{
	return "Faust";
}

const char* obby_package()
{
	return PACKAGE;
}

#ifdef LOCALEDIR
const char* obby_localedir()
{
	return LOCALEDIR;
}
#endif

#if defined(WITH_HOWL) || defined(WITH_AVAHI)
/* This is an entry point for which external scripts could check. */
void obby_has_zeroconf()
{
	return;
}
#endif
}

void obby::init_gettext(net6::gettext_package& package)
{
#ifdef ENABLE_NLS
	local_package = &package;
#endif
}

const char* obby::_(const char* msgid)
{
#ifdef ENABLE_NLS
	return local_package->gettext(msgid);
#else
	return msgid;
#endif
}
