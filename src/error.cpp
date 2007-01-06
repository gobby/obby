/* libobby - Network text editing library
 * Copyright (C) 2005 0x539 dev group
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public
 * License as published by the Free Software Foundation; either
 i* version 2 of the License, or (at your option) any later version.
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

// Login error codes
const obby::login::error obby::login::ERROR_COLOR_IN_USE =
	net6::login::ERROR_MAX + 1;
const obby::login::error obby::login::ERROR_MAX =
	net6::login::ERROR_MAX + 1;

std::string obby::login::errstring(error err)
{
	if(err == ERROR_COLOR_IN_USE)
		return "Color is already in use";
	else
		return net6::login::errstring(err);
}
