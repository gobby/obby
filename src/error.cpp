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

#include "error.hpp"
#include "gettext.hpp"

// Login error codes
const obby::login::error obby::login::ERROR_COLOR_IN_USE =
	net6::login::ERROR_MAX + 1;
const obby::login::error obby::login::ERROR_WRONG_GLOBAL_PASSWORD =
	net6::login::ERROR_MAX + 2;
const obby::login::error obby::login::ERROR_WRONG_USER_PASSWORD =
	net6::login::ERROR_MAX + 3;
const obby::login::error obby::login::ERROR_PROTOCOL_VERSION_MISMATCH =
	net6::login::ERROR_MAX + 4;
const obby::login::error obby::login::ERROR_MAX =
	net6::login::ERROR_MAX + 0xff;

std::string obby::login::errstring(error err)
{
	if(err == ERROR_COLOR_IN_USE)
		return _("Colour is already in use");
	if(err == ERROR_WRONG_GLOBAL_PASSWORD)
		return _("Wrong session password");
	if(err == ERROR_WRONG_USER_PASSWORD)
		return _("Wrong user password");
	if(err == ERROR_PROTOCOL_VERSION_MISMATCH)
		return _("Protocol version mismatch");
	else
		return net6::login::errstring(err);
}
