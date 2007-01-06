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

#include "gettext.hpp"
#include "format_string.hpp"
#include "serialise/error.hpp"
#include "serialise/attribute.hpp"

obby::serialise::attribute::attribute(
	const std::string& name,
	const std::string& value
) :
	m_name(name), m_value(value)
{
}

void obby::serialise::attribute::serialise(
	token_list& tokens
) const
{
	tokens.add(token::TYPE_IDENTIFIER, m_name, 0);
	tokens.add(token::TYPE_ASSIGNMENT, "=", 0);
	tokens.add(token::TYPE_STRING, m_value, 0);
}

void obby::serialise::attribute::deserialise(
	const token_list& tokens,
	token_list::iterator& iter
)
{
	// Get attribute name from identifier's name
	m_name = iter->get_text();
	tokens.next_token(iter);

	// Next token must be assignment
	if(iter->get_type() != token::TYPE_ASSIGNMENT)
	{
		obby::format_string str(_("Expected '=' after %0%") );
		str << m_name;
		throw error(str.str(), iter->get_line() );
	}

	tokens.next_token(iter);

	// Read attribute value
	if(iter->get_type() != token::TYPE_STRING)
	{
		obby::format_string str(_(
			"Expected string literal as value for attribute '%0%'"
		) );

		str << m_name;
		throw error(str.str(), iter->get_line() );
	}
	m_value = iter->get_text();

	tokens.next_token(iter);
}

void obby::serialise::attribute::set_value(
	const std::string& value
)
{
	m_value = value;
}

const std::string& obby::serialise::attribute::get_value() const
{
	return m_value;
}

const std::string& obby::serialise::attribute::get_name() const
{
	return m_name;
}

