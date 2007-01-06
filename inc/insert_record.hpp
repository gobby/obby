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

#ifndef _OBBY_INSERT_RECORD_HPP_
#define _OBBY_INSERT_RECORD_HPP_

#include <string>
#include "position.hpp"
#include "record.hpp"

namespace obby
{

class insert_record : public record
{
public:
	insert_record(const position& pos, const std::string& text,
	              unsigned int revision, unsigned int from);
	insert_record(const position& pos, const std::string& text,
	              unsigned int revision, unsigned int from,
	              unsigned int id);
	~insert_record();

	virtual void apply(buffer& buf);
	virtual bool is_valid() const;

	virtual void on_insert(const position& pos, const std::string& text);
	virtual void on_delete(const position& from, const position& to);
protected:
	position pos;
	std::string text;
};

}

#endif // _OBBY_INSERT_RECORD_HPP_
