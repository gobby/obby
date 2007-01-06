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

#include <cassert>
#include "buffer.hpp"

obby::buffer::buffer()
 : m_history(), m_revision(0)
{
}

obby::buffer::~buffer()
{
	std::list<record*>::iterator iter;
	for(iter = m_history.begin(); iter != m_history.end(); ++ iter)
		delete *iter;

	std::list<user*>::iterator user_i;
	for(user_i = m_userlist.begin(); user_i != m_userlist.end(); ++ user_i)
		delete *user_i;
}

const std::string& obby::buffer::get_whole_buffer() const
{
	return m_buffer;
}

std::string obby::buffer::get_sub_buffer(position from, position to) const
{
	assert(to >= from);
	return m_buffer.substr(from, to - from);
}

void obby::buffer::insert_nosync(position pos, const std::string& text)
{
	m_buffer.insert(pos, text);
	std::string::size_type new_pos = 0, new_prev = 0;
	std::vector<position>::iterator iter;
	for(iter = m_lines.begin(); iter != m_lines.end(); ++ iter)
	{
		if(new_pos != std::string::npos && pos <= *iter)
		{
			while((new_pos = text.find('\n', new_pos)) !=
			       std::string::npos)
			{
				iter = m_lines.insert(iter, pos + (new_pos++) );
				++ iter;
			}
		}

		if(pos <= *iter)
			*iter += text.length();
	}

	if(new_pos != std::string::npos)
		while((new_pos = text.find('\n', new_pos)) != std::string::npos)
			m_lines.push_back(pos + (new_pos++) );
}

void obby::buffer::erase_nosync(position from, position to)
{
	assert(to >= from);
	m_buffer.erase(from, to - from);

	std::vector<position>::iterator iter;
	for(iter = m_lines.begin(); iter != m_lines.end(); )
	{
		if(*iter >= from && *iter < to)
			iter = m_lines.erase(iter);
		else
		{
			if(*iter >= to)
				*iter -= (to - from);

			++ iter;
		}
	}
}

obby::buffer::signal_insert_type obby::buffer::insert_event() const
{
	return m_signal_insert;
}

obby::buffer::signal_delete_type obby::buffer::delete_event() const
{
	return m_signal_delete;
}

std::string obby::buffer::get_line(unsigned int index) const
{
	assert(index <= m_lines.size() );

	position from = (index == 0) ? (0) : (m_lines[index - 1] + 1);
	position to = (index == m_lines.size() ) ? (m_buffer.length() )
	                                         : (m_lines[index]);
	return m_buffer.substr(from, to - from);
}

unsigned int obby::buffer::get_line_count() const
{
	return m_lines.size() + 1;
}

obby::user* obby::buffer::find(unsigned int id) const
{
	std::list<user*>::const_iterator iter;
	for(iter = m_userlist.begin(); iter != m_userlist.end(); ++ iter)
		if( (*iter)->get_id() == id)
			return *iter;
	return NULL;
}

obby::user* obby::buffer::find(const std::string& name) const
{
	std::list<user*>::const_iterator iter;
	for(iter = m_userlist.begin(); iter != m_userlist.end(); ++ iter)
		if( (*iter)->get_name() == name)
			return *iter;
	return NULL;
}

obby::position obby::buffer::coord_to_position(unsigned int x,
                                               unsigned int y) const
{
	assert(y <= m_lines.size() );
	if(y == 0) return x;

	return m_lines[y - 1] + x + 1;
}

void obby::buffer::position_to_coord(position pos, unsigned int& x,
                                     unsigned int& y) const
{
	assert(pos <= m_buffer.length() );

	y = 0;

	std::vector<position>::const_iterator iter;
	for(iter = m_lines.begin(); iter != m_lines.end(); ++ iter)
	{
		if(*iter < pos)
			++ y;
		else
			break;
	}

	if(iter == m_lines.begin() )
		x = pos;
	else
		x = pos - (*(--iter) ) - 1;
}

obby::user* obby::buffer::add_user(net6::peer& peer, int red, int green,
                                   int blue)
{
	user* new_user = new user(peer, red, green, blue);
	m_userlist.push_back(new_user);
	return new_user;
}

