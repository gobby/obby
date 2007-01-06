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

#ifndef _OBBY_RING_HPP_
#define _OBBY_RING_HPP_

#include <list>
#include <stdexcept>

namespace obby
{

/** STL style ring container class. A ring container is a container with a
 * fixed size that allows to insert more elements as the container is able to
 * hold. Older elements will be removed in this case.
 */
template<typename value_type>
class ring
{
public:
	typedef std::list<value_type> container_type;
	typedef typename container_type::size_type size_type;
	typedef typename container_type::iterator iterator;
	typedef typename container_type::const_iterator const_iterator;

	/** Creates a ring with <em>n</em> elements before wrapping around.
	 */
	ring(size_type n);

	/** Adds a new element to the ring. If the ring is full, the oldest
	 * element will be overridden.
	 */
	void push_back(const value_type& val);

	/** Removes the last (newest) element from the ring. The ring will
	 * shrink after this and the next call to push_back will never overwrite
	 * an element.
	 */
	void pop_back();

	/** Returns the first (oldest) element in the ring.
	 */
	const value_type& front() const;

	/** Returns the last (newest) element in the ring.
	 */
	const value_type& back() const;

	/** Checks if the ring contains any elements. If not, calls to
	 * pop_back, front or back will not succeed.
	 */
	bool empty() const;

	/** Clears all contents within the ring.
	 */
	void clear();

	/** Returns an STL-like iterator pointing at the first (oldest) element
	 * in the ring.
	 */
	iterator begin();

	/** Returns an STL-like iterator pointing <strong>after</strong> the
	 * last (newest) element in the ring.
	 */
	iterator end();

	/** Returns an STL-like const_iterator pointing at the first
	 * (oldest) element in the ring.
	 */
	const_iterator begin() const;

	/** Returns an STL-like const_iterator pointing <strong>after</strong>
	 * the last (newest) element in the ring.
	 */
	const_iterator end() const;

private:
	container_type m_elems;
	size_type m_n;
};

template<typename value_type>
ring<value_type>::ring(size_type n):
	m_n(n)
{
}

template<typename value_type>
void ring<value_type>::push_back(const value_type& val)
{
	m_elems.push_back(val);
	while(m_elems.size() > m_n)
		m_elems.pop_front();
}

template<typename value_type>
void ring<value_type>::pop_back()
{
	m_elems.pop_back();
}

template<typename value_type>
const value_type& ring<value_type>::front() const
{
	return m_elems.front();
}

template<typename value_type>
const value_type& ring<value_type>::back() const
{
	return m_elems.back();
}

template<typename value_type>
bool ring<value_type>::empty() const
{
	return m_elems.empty();
}

template<typename value_type>
void ring<value_type>::clear()
{
	m_elems.clear();
}

template<typename value_type>
typename ring<value_type>::const_iterator ring<value_type>::begin() const
{
	return m_elems.begin();
}

template<typename value_type>
typename ring<value_type>::const_iterator ring<value_type>::end() const
{
	return m_elems.end();
}

template<typename value_type>
typename ring<value_type>::iterator ring<value_type>::begin()
{
	return m_elems.begin();
}

template<typename value_type>
typename ring<value_type>::iterator ring<value_type>::end()
{
	return m_elems.end();
}

} // namespace obby

#endif // _OBBY_RING_HPP_
