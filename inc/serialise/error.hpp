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

#ifndef _OBBY_SERIALISE_ERROR_HPP_
#define _OBBY_SERIALISE_ERROR_HPP_

#include <stdexcept>

namespace obby::serialise
{

/** Error with obby serialisation. Mostly, incorrect input causes such errors.
 */
class error : public std::runtime_error
{
public:
	error(
		const std::string& msg,
		unsigned int line
	);

	unsigned int get_line() const;

protected:
	unsigned int m_line;
};

} // namespace obby::serialise

#endif // _OBBY_SERIALISE_ERROR_HPP_
