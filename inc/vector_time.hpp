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

#ifndef _OBBY_VECTOR_TIME_HPP_
#define _OBBY_VECTOR_TIME_HPP_

namespace obby
{

/** Vector time counting both local and remote operation count.
 */
template<typename time_type>
class basic_vector_time
{
public:
	basic_vector_time(time_type local, time_type remote);

	bool operator==(const basic_vector_time& other);
	bool operator!=(const basic_vector_time& other);

	time_type get_local() const;
	time_type get_remote() const;

	void inc_local();
	void inc_remote();

protected:
	time_type m_local;
	time_type m_remote;
};

typedef basic_vector_time<unsigned int> vector_time;

template<typename time_type>
basic_vector_time<time_type>::
	basic_vector_time(time_type local, time_type remote)
 : m_local(local), m_remote(remote)
{
}

template<typename time_type>
bool basic_vector_time<time_type>::operator==(const basic_vector_time& other)
{
	return m_local == other.m_local && m_remote == other.m_remote;
}

template<typename time_type>
bool basic_vector_time<time_type>::operator!=(const basic_vector_time& other)
{
	return m_local != other.m_local || m_remote != other.m_remote;
}

template<typename time_type>
time_type basic_vector_time<time_type>::get_local() const
{
	return m_local;
}

template<typename time_type>
time_type basic_vector_time<time_type>::get_remote() const
{
	return m_remote;
}

template<typename time_type>
void basic_vector_time<time_type>::inc_local()
{
	++ m_local;
}

template<typename time_type>
void basic_vector_time<time_type>::inc_remote()
{
	++ m_remote;
}

} // namespace obby

#endif // _OBBY_VECTOR_TIME_HPP_
