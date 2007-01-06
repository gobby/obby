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

#ifndef _OBBY_COLOUR_HPP_
#define _OBBY_COLOUR_HPP_

#include <net6/serialise.hpp>

namespace obby
{

/** Simple class to store a colour composed by red, green and blue components
 * ranging from 0 to 255.
 */
class colour
{
public:
	/** Default constructor, leaves an uninitialised colour.
	 */
	colour();

	/** Composes a new colour.
	 */
	colour(unsigned int red, unsigned int green, unsigned int blue);

	/** Returns the red colour component.
	 */
	unsigned int get_red() const;

	/** Returns the green colour component.
	 */
	unsigned int get_green() const;

	/** Returns the blue colour component.
	 */
	unsigned int get_blue() const;

	/** Returns TRUE if <em>colour</em> is similar to this colour.
	 */
	bool similar_colour(const colour& colour) const;
protected:
	unsigned int m_red;
	unsigned int m_green;
	unsigned int m_blue;
};

} // namespace obby

namespace serialise
{

/** Normal context prints colour in hexadecimal format because RRGGBB format
 * is quite common and also human-readable.
 */
template<>
class context<obby::colour>
{
public:
	virtual std::string to_string(const obby::colour& from) const;
	virtual obby::colour from_string(const std::string& string) const;
};

} // namespace serialise

#endif // _OBBY_COLOUR_HPP_
