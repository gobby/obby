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

#ifndef _OBBY_DELETE_RECORD_HPP_
#define _OBBY_DELETE_RECORD_HPP_

#include <string>
#include "position.hpp"
#include "record.hpp"

namespace obby
{

class delete_record : public record
{
public:
	delete_record(const position& begin, const position& end,
	              unsigned int revision, unsigned int from);
	delete_record(const position& begin, const position& end,
	              unsigned int revision, unsigned int from,
	              unsigned int id);
	~delete_record();

	virtual void apply(buffer& buf);
	virtual net6::packet to_packet();
	virtual record* reverse(const buffer& buf);

	virtual void on_insert(const position& pos, const std::string& text);
	virtual void on_delete(const position& from, const position& to);

	const position& get_begin() const;
	const position& get_end() const;
protected:
	position m_from;
	position m_to;
};

}

#endif // _OBBY_DELETE_RECORD_HPP_
