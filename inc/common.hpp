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

#ifndef _OBBY_COMMON_HPP_
#define _OBBY_COMMON_HPP_

#include "net6/gettext_package.hpp"

extern "C" {

const char* obby_version();
const char* obby_codename();
const char* obby_package();
const char* obby_localedir();

}

/**
 * @brief Foo!
 */
namespace obby
{

/** Initialises gettext for usage with libobby. The constructor of the main
 * libobby objects (basic_buffer and derivates) call this function.
 */
void init_gettext(net6::gettext_package& package);

/** Translates a message of the libobby catalog.
 */
const char* _(const char* msgid);

}

#endif // _OBBY_COMMON_HPP_

