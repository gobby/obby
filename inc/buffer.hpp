/* libobby - Network text editing library
 * Copyright (C) 2005 0x539 dev group
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this program; if not, write to the Free
 * Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#ifndef _OBBY_BUFFER_HPP_
#define _OBBY_BUFFER_HPP_

#include <string>
#include <list>
#include <vector>
#include "position.hpp"
#include "record.hpp"

namespace obby
{

class buffer
{
public:
	buffer();
	~buffer();

	std::string get_sub_buffer(const position& from, const position& to);

	void insert(const position& pos, const std::string& text) = 0;
	void erase(const position& from, const position& to) = 0;

	void insert_nosync(const position& pos, const std::string& text);
	void erase_nosync(const position& from, const position& to);

protected:
	void insert_lines(unsigned int pos, unsigned int count);
	void erase_lines(unsigned int pos, unsigned int count);

	std::list<record*> m_history;
	unsigned int m_revision;
	std::vector<std::string> m_lines;
};

}

#endif // _OBBY_BUFFER_HPP_
