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

#ifndef _OBBY_PTR_ITERATOR_HPP_
#define _OBBY_PTR_ITERATOR_HPP_

namespace obby
{

/* An iterator type to iterate through a container containing pointers of the
 * given data type but retrieving references to them.
 */
template<typename type, typename container, typename base_iterator>
class ptr_iterator : public base_iterator
{
public:
	ptr_iterator()
	 : base_iterator() { }
	ptr_iterator(const base_iterator& iter)
	 : base_iterator(iter) { }

	ptr_iterator& operator=(const base_iterator& iter) {
		return static_cast<ptr_iterator&>(
			base_iterator::operator=(iter)
		);
	}

	type& operator*() {
		return *base_iterator::operator*();
	}
	const type& operator*() const {
		return *base_iterator::operator*();
	}

	type* operator->() {
		return *base_iterator::operator->();
	}
	const type* operator->() const {
		return *base_iterator::operator->();
	}
};

}

#endif // _OBBY_PTR_ITERATOR_HPP_
