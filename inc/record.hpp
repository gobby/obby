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

#ifndef _OBBY_RECORD_HPP_
#define _OBBY_RECORD_HPP_

#include <string>
#include <net6/non_copyable.hpp>
#include <net6/packet.hpp>
#include "position.hpp"

namespace obby
{

class document;

class record : private net6::non_copyable
{
public:
	record(unsigned int document, unsigned int revision, unsigned int from);
	record(unsigned int document, unsigned int revision, unsigned int from,
	       unsigned int id);
	~record();

	virtual record* clone() const = 0;
	
	virtual void apply(document& doc) const = 0;
	virtual void apply(record& rec) const = 0;
	virtual net6::packet to_packet() const = 0;
	virtual record* reverse() = 0;

	bool is_valid() const;

	unsigned int get_id() const;
	unsigned int get_document() const;
	unsigned int get_from() const;
	unsigned int get_revision() const;

	void set_from(unsigned int from);
	void set_revision(unsigned int revision);

	virtual void on_insert(position pos, const std::string& text) = 0;
	virtual void on_delete(position from, position to) = 0;

	static record* from_packet(const net6::packet& pack);

#ifndef NDEBUG
	virtual std::string inspect() const = 0;
#endif
protected:
	void invalidate();

	unsigned int m_id;
	unsigned int m_document;
	unsigned int m_revision;
	unsigned int m_from;
	bool m_valid;

	static unsigned int m_counter; // id counter to create unique ids
};

}

#endif // _OBBY_RECORD_HPP_
