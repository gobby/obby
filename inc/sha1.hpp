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

#ifndef _OBBY_SHA1_HPP_
#define _OBBY_SHA1_HPP_

#include <string>

namespace obby
{

class SHA1 {
public:
	typedef size_t size_type;

	SHA1();
	SHA1(const uint8_t* data, size_type len);
	SHA1(const std::string& input);
	~SHA1();

	void append(const unit8_t* data, size_type len);
	void append(const std::string& input);

	void operator<<(const std::string& input);
	std::string final();

	static std::string hash(const std::string& input);
};

}

#endif // _OBBY_SHA1_HPP_

