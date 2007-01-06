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

#ifndef _OBBY_LOCAL_DOCUMENT_HPP_
#define _OBBY_LOCAL_DOCUMENT_HPP_

#include "document.hpp"

namespace obby
{

class local_document_info;

template<typename selector_type>
class basic_local_buffer;

/** Abstract base class for obby documents that have a local user.
 */
class local_document : virtual public document
{
public:
	local_document(const local_document_info& info);
	//local_document(const void** foo, const local_document_info& bar);
	virtual ~local_document();

	/** Returns the document info for this document.
	 */
	const local_document_info& get_info() const;

	/** Returns the buffer that is associated to this document.
	 */
	const basic_local_buffer<net6::selector>& get_buffer() const;

protected:
};

}

#endif // _OBBY_LOCAL_DOCUMENT_HPP_
