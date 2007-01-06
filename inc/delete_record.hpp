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

#ifndef _OBBY_DELETE_RECORD_HPP_
#define _OBBY_DELETE_RECORD_HPP_

#include <string>
#include "position.hpp"
#include "record.hpp"

namespace obby
{

class document;

class delete_record : public record
{
public:
	delete_record(position pos, const std::string& text,
	              document& doc, const user* from,
	              unsigned int revision);
	delete_record(position pos, const std::string& text,
	              document& doc, const user* from,
	              unsigned int revision, unsigned int id);
	~delete_record();

	virtual record* clone() const;

	virtual void apply() const;
	virtual void apply(record& rec) const;
	virtual net6::packet to_packet() const;
	virtual record* reverse();

	virtual void on_insert(position pos, const std::string& text);
	virtual void on_delete(position from, position to);

	position get_begin() const;
	position get_end() const;
	const std::string& get_text() const;

	virtual std::string inspect() const;
protected:
	position m_pos;
	std::string m_text;
};

}

#endif // _OBBY_DELETE_RECORD_HPP_
