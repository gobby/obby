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

#include <cstdlib>
#include <sstream>
#include "colour.hpp"

obby::colour::colour()
{
}

obby::colour::colour(unsigned int red, unsigned int green, unsigned int blue):
	m_red(red), m_green(green), m_blue(blue)
{
}

unsigned int obby::colour::get_red() const
{
	return m_red;
}

unsigned int obby::colour::get_green() const
{
	return m_green;
}

unsigned int obby::colour::get_blue() const
{
	return m_blue;
}

bool obby::colour::similar_colour(const colour& colour) const
{
	// TODO: Convert to HSV for better checking
	return abs(m_red - colour.m_red) +
	       abs(m_green - colour.m_green) +
	       abs(m_blue - colour.m_blue) < 32;
}

std::string serialise::default_context_to<obby::colour>::
	to_string(const obby::colour& from) const
{
	unsigned int val =
		(from.get_red()   << 16) |
		(from.get_green() <<  8) |
		(from.get_blue()  <<  0);

	std::stringstream stream;
	stream << std::hex << val;
	return stream.str();
}

obby::colour serialise::default_context_from<obby::colour>::
	from_string(const std::string& string) const
{
	unsigned int val;
	std::stringstream stream(string);
	stream >> std::hex >> val;

	return obby::colour( (val >> 16) & 0xff, (val >> 8) & 0xff, val & 0xff);
}
