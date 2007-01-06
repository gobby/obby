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

#ifndef _OBBY_SERIALISE_ATTRIBUTE_HPP_
#define _OBBY_SERIALISE_ATTRIBUTE_HPP_

#include <string>
#include "serialise/token.hpp"

namespace obby::serialise
{

class attribute
{
public:
	attribute(
		const std::string& name,
		const std::string& value = ""
	);

	void serialise(
		token_list& tokens
	) const;

	void deserialise(
		const token_list& tokens,
		token_list::iterator& iter
	);

	void set_value(
		const std::string& value
	);

	const std::string& get_value();

	const std::string& get_name();
private:
	std::string m_name;
	std::string m_value;
};

} // namespace obby::serialise

#endif // _OBBY_SERIALISE_ATTRIBUTE_HPP_
