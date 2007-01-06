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

#ifndef _OBBY_SERIALISE_PARSER_HPP_
#define _OBBY_SERIALISE_PARSER_HPP_

#include <string>
#include <iostream>
#include <net6/non_copyable.hpp>
#include "serialise/object.hpp"

namespace obby
{

namespace serialise
{

class parser : private net6::non_copyable
{
public:
	parser();

	void deserialise(
		const std::string& file
	);

	void deserialise(
		std::istream& stream
	);

	void serialise(
		const std::string& file
	) const;

	void serialise(
		std::ostream& stream
	) const;

	const std::string& get_type() const;

	void set_type(
		const std::string& type
	);

	const object& get_root() const;
	object& get_root();

protected:
	std::string m_type;
	object m_object;
};

} // namespace serialise

} // namespace obby

#endif // _OBBY_SERIALISE_PARSER_HPP_
