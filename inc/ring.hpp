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

#include <vector>
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
	typedef std::size_t size_type;
	typedef std::vector<value_type> container_type;

	template<typename base_iterator>
	class basic_iterator
	{
	public:
		basic_iterator(const ring& container,
		               const base_iterator& base_iter);

		basic_iterator& operator++();
		basic_iterator operator++(int);

		basic_iterator& operator--();
		basic_iterator operator--(int);

		bool operator==(const base_iterator& rhs) const;
		bool operator!=(const base_iterator& lhs) const;

		bool operator==(const basic_iterator& rhs) const;
		bool operator!=(const basic_iterator& lhs) const;
	protected:
		const ring& m_ring;
		base_iterator m_iter;
	};

	class const_iterator:
		public basic_iterator<typename container_type::const_iterator>
	{
	public:
		typedef typename container_type::const_iterator base_iterator;

		const_iterator(const ring& container,
		               const base_iterator& base_iter);

		const value_type& operator*() const;
		const value_type* operator->() const;
	};

	class iterator:
		public basic_iterator<typename container_type::iterator>
	{
	public:
		typedef typename container_type::iterator base_iterator;

		iterator(const ring& container,
		         const base_iterator& base_iter);

		value_type& operator*();
		value_type* operator->();

		operator const_iterator() const;
	};

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
	typename container_type::iterator m_begin;
	typename container_type::iterator m_end;
};

template<typename value_type> template<typename base_iterator>
ring<value_type>::basic_iterator<base_iterator>::
	basic_iterator(const ring& container,
	               const base_iterator& base_iter):
	m_ring(container), m_iter(base_iter)
{
}

template<typename value_type> template<typename base_iterator>
ring<value_type>::basic_iterator<base_iterator>&
ring<value_type>::basic_iterator<base_iterator>::operator++()
{
	if(m_iter == m_ring.m_end)
	{
		m_iter = m_ring.m_elems.end();
	}
	else
	{
		++ m_iter;
		if(m_iter == m_ring.m_elems.end() )
			m_iter = m_ring.m_elems.begin();
	}

	return *this;
}

template<typename value_type> template<typename base_iterator>
ring<value_type>::basic_iterator<base_iterator>
ring<value_type>::basic_iterator<base_iterator>::operator++(int)
{
	const_iterator temp(*this);
	operator++();
	return temp;
}

template<typename value_type> template<typename base_iterator>
ring<value_type>::basic_iterator<base_iterator>&
ring<value_type>::basic_iterator<base_iterator>::operator--()
{
	if(m_iter == m_ring.m_elems.end() )
	{
		m_iter = m_ring.m_end;
	}
	else
	{
		if(m_iter == m_ring.m_elems.begin() )
			m_iter = m_ring.m_elems.end();

		-- m_iter;
	}

	return *this;
}

template<typename value_type> template<typename base_iterator>
ring<value_type>::basic_iterator<base_iterator>
ring<value_type>::basic_iterator<base_iterator>::operator--(int)
{
	const_iterator temp(*this);
	operator--();
	return temp;
}

template<typename value_type> template<typename base_iterator>
bool ring<value_type>::basic_iterator<base_iterator>::
	operator==(const base_iterator& rhs) const
{
	return m_iter == rhs;
}

template<typename value_type> template<typename base_iterator>
bool ring<value_type>::basic_iterator<base_iterator>::
	operator!=(const base_iterator& rhs) const
{
	return m_iter != rhs;
}

template<typename value_type> template<typename base_iterator>
bool ring<value_type>::basic_iterator<base_iterator>::
	operator==(const basic_iterator& rhs) const
{
	return m_iter == rhs.m_iter;
}

template<typename value_type> template<typename base_iterator>
bool ring<value_type>::basic_iterator<base_iterator>::
	operator!=(const basic_iterator& rhs) const
{
	return m_iter != rhs.m_iter;
}

template<typename value_type>
ring<value_type>::const_iterator::
	const_iterator(const ring<value_type>& container,
	               const base_iterator& base_iter):
	basic_iterator<base_iterator>(container, base_iter)
{
}                                                

template<typename value_type>
const value_type& ring<value_type>::const_iterator::operator*() const
{
	return basic_iterator<base_iterator>::m_iter.operator*();
}

template<typename value_type>
const value_type* ring<value_type>::const_iterator::operator->() const
{
	return basic_iterator<base_iterator>::m_iter.operator->();
}

template<typename value_type>
ring<value_type>::iterator::iterator(const ring<value_type>& container,
                                     const base_iterator& base_iter):
	basic_iterator<base_iterator>(container, base_iter)
{
}                             

template<typename value_type>
value_type& ring<value_type>::iterator::operator*()
{
	return basic_iterator<base_iterator>::m_iter.operator*();
}

template<typename value_type>
value_type* ring<value_type>::iterator::operator->()
{
	return basic_iterator<base_iterator>::m_iter.operator->();
}

template<typename value_type>
ring<value_type>::iterator::operator typename ring<value_type>::const_iterator() const
{
	return const_iterator(
		basic_iterator<base_iterator>::m_ring,
		basic_iterator<base_iterator>::m_iter
	);
}

template<typename value_type>
ring<value_type>::ring(size_type n):
	m_elems(n), m_begin(m_elems.end() ), m_end(m_elems.end() )
{
}

template<typename value_type>
void ring<value_type>::push_back(const value_type& val)
{
	if(m_end == m_elems.end() )
	{
		// No elements, begin ring with first
		m_begin = m_end = m_elems.begin();
	}
	else
	{
		// Advance
		++ m_end;
		if(m_end == m_elems.end() ) m_end = m_elems.begin();
	}

	// Overwrite last
	if(m_end == m_begin)
	{
		++ m_begin;
		if(m_begin == m_elems.end() )
			m_begin = m_elems.begin();
	}

        *m_end = val;
}

template<typename value_type>
void ring<value_type>::pop_back()
{
	// No elements
	if(m_end == m_elems.end() )
		throw std::logic_error("obby::ring::pop_back");

	// Removed last element
	if(m_end == m_begin)
	{
		m_begin = m_end = m_elems.end();
	}
	else
	{
		// Move iterator backwards, the element left out will be
		// overwritten if it reused
		if(m_end == m_elems.begin() )
			m_end = m_elems.end();

		-- m_end;
	}
}

template<typename value_type>
const value_type& ring<value_type>::front() const
{
	if(m_begin == m_elems.end() )
		throw std::logic_error("obby::ring::front");

	return *m_begin;
}

template<typename value_type>
const value_type& ring<value_type>::back() const
{
	if(m_end == m_elems.end() )
		throw std::logic_error("obby::ring::back");

	return *m_end;
}

template<typename value_type>
bool ring<value_type>::empty() const
{
	return m_end == m_elems.end();
}

template<typename value_type>
typename ring<value_type>::const_iterator ring<value_type>::begin() const
{
	return const_iterator(*this, m_elems.begin() );
}

template<typename value_type>
typename ring<value_type>::const_iterator ring<value_type>::end() const
{
	return const_iterator(*this, m_elems.end() );
}

template<typename value_type>
typename ring<value_type>::iterator ring<value_type>::begin()
{
	return iterator(*this, m_elems.begin() );
}

template<typename value_type>
typename ring<value_type>::iterator ring<value_type>::end()
{
	return iterator(*this, m_elems.end() );
}

} // namespace obby

#endif // _OBBY_RING_HPP_
