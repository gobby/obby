/* libobby - Network text editing library
 * Copyright (C) 2006 0x539 dev group
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

#ifndef _OBBY_JUPITER_ERROR_HPP_
#define _OBBY_JUPITER_ERROR_HPP_

#include <stdexcept>

namespace obby
{

/** @brief Class that reports error that the jupiter algorithm may produce
 * on corrupt input.
 *
 * This is not a logical error since the input may come from untrusted input
 * (such as through the network).
 */
class jupiter_error: public std::runtime_error
{
public:
	jupiter_error(const std::string& error_message);
};

} // namespace obby

#endif // _OBBY_JUPITER_ERROR_HPP_
