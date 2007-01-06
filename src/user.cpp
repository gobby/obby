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

#include "format_string.hpp"
#include "user.hpp"
#include "user_table.hpp"

const obby::user::flags obby::user::flags::NONE = obby::user::flags(0x00000000);
const obby::user::flags obby::user::flags::CONNECTED = obby::user::flags(0x00000001);

const obby::user::privileges obby::user::privileges::NONE = obby::user::privileges(0x00000000);
const obby::user::privileges obby::user::privileges::CREATE_DOCUMENT = obby::user::privileges(0x00000001);

obby::user::user(unsigned int id,
                 const net6::user& user6,
                 const colour& colour):
	m_user6(&user6), m_id(id), m_name(user6.get_name() ),
	m_colour(colour), m_flags(flags::CONNECTED),
	m_privs(privileges::NONE)
{
}

obby::user::user(unsigned int id,
                 const std::string& name,
		 const colour& colour):
	m_user6(NULL), m_id(id), m_name(name), m_colour(colour),
	m_flags(flags::NONE), m_privs(privileges::NONE)
{
}

obby::user::user(const serialise::object& obj):
	m_flags(flags::NONE), m_privs(privileges::NONE)
{
	const serialise::attribute& id_attr =
		obj.get_required_attribute("id");
	const serialise::attribute& name_attr =
		obj.get_required_attribute("name");
	const serialise::attribute& colour_attr =
		obj.get_required_attribute("colour");

	m_user6 = NULL;
	m_id = id_attr.as<unsigned int>();
	m_name = name_attr.as<std::string>();
	m_colour = colour_attr.as<obby::colour>();

	m_privs = privileges::NONE;
}

void obby::user::serialise(serialise::object& obj) const
{
	obj.add_attribute("id").set_value(m_id);
	obj.add_attribute("name").set_value(m_name);
	obj.add_attribute("colour").set_value(m_colour);
}

void obby::user::release_net6()
{
	// User must be already connected
	if(~get_flags() & flags::CONNECTED)
		throw std::logic_error("obby::user::release_net6");

	m_user6 = NULL;
	remove_flags(flags::CONNECTED);
}

void obby::user::assign_net6(const net6::user& user6,
                             const colour& colour)
{
	// User must not be already connected
	if(get_flags() & flags::CONNECTED)
		throw std::logic_error("obby::user::assign_net6");

	// Name must be the same
	if(m_name != user6.get_name() )
		throw std::logic_error("obby::user::assign_net6");

	m_user6 = &user6;
	m_colour = colour;

	add_flags(flags::CONNECTED);
}

const net6::user& obby::user::get_net6() const
{
	if(m_user6 == NULL)
		throw std::logic_error("obby::user::get_net6");

	return *m_user6;
}

const std::string& obby::user::get_name() const
{
	return m_name;
}

const net6::address& obby::user::get_address() const
{
	if(m_user6 == NULL)
		throw std::logic_error("obby::user::get_address");

	return m_user6->get_connection().get_remote_address();
}

unsigned int obby::user::get_id() const
{
	return m_id;
}

const obby::colour& obby::user::get_colour() const
{
	return m_colour;
}

const std::string& obby::user::get_password() const
{
	return m_password;
}

obby::user::flags obby::user::get_flags() const
{
	return m_flags;
}

void obby::user::set_colour(const colour& colour)
{
	m_colour = colour;
}

void obby::user::set_password(const std::string& password)
{
	m_password = password;
}

void obby::user::add_flags(flags new_flags)
{
	m_flags |= new_flags;
}

void obby::user::remove_flags(flags old_flags)
{
	m_flags &= ~old_flags;
}

const obby::user* serialise::user_table_find(const obby::user_table& user_table,
                                             unsigned int index)
{
	return user_table.find(
		index,
		obby::user::flags::NONE,
		obby::user::flags::NONE
	);
}

serialise::default_context_from<const obby::user*>::
	default_context_from(const obby::user_table& user_table):
	user_context_from<const obby::user*>(user_table)
{
}

serialise::hex_context_from<const obby::user*>::
	hex_context_from(const obby::user_table& user_table):
	user_hex_context_from<const obby::user*>(user_table)
{
}
