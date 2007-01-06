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

obby::user::user(unsigned int id, const net6::user& user6,
                 int red, int green, int blue):
	m_user6(&user6), m_id(id), m_name(user6.get_name() ),
	m_red(red), m_green(green), m_blue(blue), m_flags(flags::CONNECTED),
	m_privs(privileges::NONE)
{
}

obby::user::user(unsigned int id, const std::string& name, int red, int green,
                 int blue)
 : m_user6(NULL), m_id(id), m_name(name), m_red(red), m_green(green),
   m_blue(blue), m_flags(flags::NONE), m_privs(privileges::NONE)
{
}

obby::user::user(const serialise::object& obj):
	m_flags(flags::NONE), m_privs(privileges::NONE)
{
	const serialise::attribute& id_attr =
		obj.get_required_attribute("id");
	const serialise::attribute& name_attr =
		obj.get_required_attribute("name");

	// TODO: Replace this by a obby::colour class before releasing 0.3.0!
	const serialise::attribute& red_attr =
		obj.get_required_attribute("red");
	const serialise::attribute& green_attr =
		obj.get_required_attribute("green");
	const serialise::attribute& blue_attr =
		obj.get_required_attribute("blue");

	m_user6 = NULL;
	m_id = id_attr.as<unsigned int>();
	m_name = name_attr.as<std::string>();
	m_red = red_attr.as<unsigned int>();
	m_green = green_attr.as<unsigned int>();
	m_blue = blue_attr.as<unsigned int>();

	m_privs = privileges::NONE;
}

void obby::user::serialise(serialise::object& obj) const
{
	obj.add_attribute("id").set_value(m_id);
	obj.add_attribute("name").set_value(m_name);
	obj.add_attribute("red").set_value(m_red);
	obj.add_attribute("green").set_value(m_green);
	obj.add_attribute("blue").set_value(m_blue);
}

void obby::user::release_net6()
{
	// User must be already connected
	if(~get_flags() & flags::CONNECTED)
		throw std::logic_error("obby::user::release_net6");

	m_user6 = NULL;
	remove_flags(flags::CONNECTED);
}

void obby::user::assign_net6(const net6::user& user6, int red, int green,
                             int blue)
{
	// User must not be already connected
	if(get_flags() & flags::CONNECTED)
		throw std::logic_error("obby::user::assign_net6");

	// Name must be the same
	if(m_name != user6.get_name() )
		throw std::logic_error("obby::user::assign_net6");

	m_user6 = &user6;
	m_red = red;
	m_green = green;
	m_blue = blue;

	add_flags(flags::CONNECTED);
}

const net6::user& obby::user::get_net6() const
{
	if(m_user6 == NULL)
		// TODO: Own error class?
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
		// TODO: Own error class?
		throw std::logic_error("obby::user::get_address");

	return m_user6->get_connection().get_remote_address();
}

unsigned int obby::user::get_id() const
{
	return m_id;
}

int obby::user::get_red() const
{
	return m_red;
}

int obby::user::get_green() const
{
	return m_green;
}

int obby::user::get_blue() const
{
	return m_blue;
}

const std::string& obby::user::get_token() const
{
	return m_token;
}

const std::string& obby::user::get_password() const
{
	return m_password;
}

obby::user::flags obby::user::get_flags() const
{
	return m_flags;
}

void obby::user::set_colour(int red, int green, int blue)
{
	m_red = red;
	m_green = green;
	m_blue = blue;
}

void obby::user::set_token(const std::string& token)
{
	m_token = token;
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

serialise::context<obby::user*>::context():
	m_user_table(NULL)
{
}

serialise::context<obby::user*>::context(const obby::user_table& user_table):
	m_user_table(&user_table)
{
}

std::string
serialise::context<obby::user*>::to_string(const obby::user* from) const
{
	std::stringstream stream;
	stream << ( (from != NULL) ? (from->get_id()) : (0) );
	return stream.str();
}

const obby::user*
serialise::context<obby::user*>::from_string(const std::string& string) const
{
	// We need a user table to lookup the user ID
	if(m_user_table == NULL)
		throw conversion_error("User table required");

	// Extract user ID from stream
	unsigned int user_id;
	std::stringstream stream(string);
	stream >> user_id;

	// Not a valid ID?
	if(stream.bad() )
		throw conversion_error("User ID must be an integer");

	// "No user"
	if(user_id == 0) return NULL;

	// Find user
	const obby::user* user = m_user_table->find(user_id);
	if(user == NULL)
	{
		// Not in user table
		obby::format_string str("User ID %0% does not exist");
		str << user_id;
		throw conversion_error(str.str() );
	}

	// Deserialisation successful
	return user;
}
