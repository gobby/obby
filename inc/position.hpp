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

#ifndef _OBBY_POSITION_HPP_
#define _OBBY_POSITION_HPP_

namespace obby
{

class position
{
public:
	position();
	position(unsigned int line, unsigned int col);
	position(const position& other);
	~position();
	
	position& operator=(const position& other);

	unsigned int get_line() const;
	unsigned int get_col() const;

	void move_to(unsigned int line, unsigned int col);
	void move_by(int lines, int cols);
private:
	unsigned int m_line;
	unsigned int m_col;
};

}

#endif // _OBBY_POSITION_HPP_

