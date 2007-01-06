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

#include <cassert>
#include "record.hpp"
#include "insert_record.hpp"
#include "delete_record.hpp"
#include "document_info.hpp"

unsigned int obby::record::m_counter = 0;

obby::record::record(document& doc, const user* from, unsigned int revision)
 : m_id(++ m_counter), m_document(&doc), m_revision(revision),
   m_user(from), m_valid(true)
{
}

obby::record::record(document& doc, const user* from, unsigned int revision,
                     unsigned int id)
 : m_id(id), m_document(&doc), m_revision(revision), m_user(from),
   m_valid(true)
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

const obby::document& obby::record::get_document() const
{
	return *m_document;
}

const obby::user* obby::record::get_user() const
{
	return m_user;
}

unsigned int obby::record::get_revision() const
{
	return m_revision;
}

void obby::record::set_user(const user* new_user)
{
	m_user = new_user;
}

void obby::record::set_revision(unsigned int revision)
{
	m_revision = revision;
}

void obby::record::invalidate()
{
	m_valid = false;
}

obby::record* obby::record::from_packet(const net6::packet& pack)
{
	// param 0 is document
	// param 1 is "record"
	const std::string& operation = pack.get_param(2).as<std::string>();
	unsigned int id = pack.get_param(3).as<int>();
	document_info* info = pack.get_param(0).as<document_info*>();
	unsigned int revision = pack.get_param(4).as<int>();
	const user* from = pack.get_param(5).as<user*>();
	document* doc = info->get_document();

	if(operation == "insert")
	{
		unsigned int pos = pack.get_param(6).as<int>();
		const std::string& text = pack.get_param(7).as<std::string>();
		
		return new insert_record(pos, text, *doc, from, revision, id);
	}
	else if(operation == "delete")
	{
		unsigned int pos = pack.get_param(6).as<int>();
		const std::string& text = pack.get_param(7).as<std::string>();

		return new delete_record(pos, text, *doc, from, revision, id);
	}
	else
	{
		return NULL;
	}
}

