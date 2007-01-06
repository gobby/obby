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

#include "record.hpp"
#include "insert_record.hpp"
#include "delete_record.hpp"

unsigned int obby::record::m_counter = 0;

obby::record::record(unsigned int revision, unsigned int from)
 : m_id(++ m_counter), m_revision(revision), m_from(from), m_valid(true)
{
}

obby::record::record(unsigned int revision, unsigned int from, unsigned int id)
 : m_id(id), m_revision(revision), m_from(from), m_valid(true)
{
}

obby::record::~record()
{
}

bool obby::record::is_valid() const
{
	return m_valid;
}

unsigned int obby::record::get_id() const
{
	return m_id;
}

unsigned int obby::record::get_from() const
{
	return m_from;
}

unsigned int obby::record::get_revision() const
{
	return m_revision;
}

void obby::record::invalidate()
{
	m_valid = false;
}

obby::record* obby::record::from_packet(const net6::packet& pack)
{
	if(pack.get_param_count() < 4) return NULL;
	if(pack.get_param(0).get_type() != net6::packet::param::STRING)
		return NULL;
	if(pack.get_param(1).get_type() != net6::packet::param::INT)
		return NULL;
	if(pack.get_param(2).get_type() != net6::packet::param::INT)
		return NULL;
	if(pack.get_param(3).get_type() != net6::packet::param::INT)
		return NULL;

	const std::string& operation = pack.get_param(0).as_string();
	unsigned int id = pack.get_param(1).as_int();
	unsigned int revision = pack.get_param(2).as_int();
	unsigned int from = pack.get_param(3).as_int();

	if(operation == "insert")
	{
		if(pack.get_param_count() < 7) return NULL;
		if(pack.get_param(4).get_type() != net6::packet::param::INT)
			return NULL;
		if(pack.get_param(5).get_type() != net6::packet::param::INT)
			return NULL;
		if(pack.get_param(6).get_type() != net6::packet::param::STRING)
			return NULL;

		unsigned int line = pack.get_param(4).as_int();
		unsigned int col = pack.get_param(5).as_int();
		const std::string& text = pack.get_param(6).as_string();
		
		return new insert_record(position(line, col),
		                         text, revision, from, id);
	}
	else if(operation == "delete")
	{
		if(pack.get_param_count() < 8) return NULL;
		if(pack.get_param(4).get_type() != net6::packet::param::STRING)
			return NULL;
		if(pack.get_param(5).get_type() != net6::packet::param::STRING)
			return NULL;
		if(pack.get_param(6).get_type() != net6::packet::param::STRING)
			return NULL;
		if(pack.get_param(7).get_type() != net6::packet::param::STRING)
			return NULL;

		unsigned int from_line = pack.get_param(4).as_int();
		unsigned int from_col = pack.get_param(5).as_int();
		unsigned int to_line = pack.get_param(6).as_int();
		unsigned int to_col = pack.get_param(7).as_int();

		return new delete_record(position(from_line, from_col),
		                         position(to_line, to_col),
		                         revision, from, id);
	}
	else
	{
		assert(false);
	}
}

