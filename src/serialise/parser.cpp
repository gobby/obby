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

#include <fstream>
#include "gettext.hpp"
#include "format_string.hpp"
#include "serialise/error.hpp"
#include "serialise/parser.hpp"

obby::serialise::parser::parser()
{
}

void obby::serialise::parser::deserialise(
	const std::string& file
)
{
	std::ifstream in(file.c_str() );

	if(!in)
	{
		obby::format_string str(_(
			"Could not open file '%0%' for reading"
		) );

		str << file;
		throw error(str.str(), 0);
	}

	deserialise(in);
}

void obby::serialise::parser::deserialise(
	std::istream& stream
)
{
	const unsigned int bufsize = 1024;
	char readbuf[bufsize];
	std::string result;

	// Initial preallocation
	result.reserve(bufsize << 3);
	while(stream)
	{
		// More preallocation, if the following 1024 byte exceed the
		// current capacity
		if(result.capacity() < result.length() + bufsize)
			result.reserve(result.capacity() * 2);

		// Read data, append to resulting string
		stream.read(readbuf, bufsize);
		result.append(readbuf, stream.gcount() );
	}

	token_list list;
	list.deserialise(result);
	token_list::iterator iter = list.begin();

	// Get initial '!'
	if(iter->get_type() != token::TYPE_EXCLAMATION)
	{
		throw error(
			_("Expected initial exclamation mark"),
			iter->get_line()
		);
	}
	list.next_token(iter);

	// Read document type
	if(iter->get_type() != token::TYPE_IDENTIFIER)
	{
		throw error(
			_("Expected document type after '!'"),
			iter->get_line()
		);
	}
	m_type = iter->get_text();
	list.next_token(iter);

	// Get indentation for following object
	if(iter->get_type() != token::TYPE_INDENTATION)
	{
		throw error(
			_("Expected newline after document type"),
			iter->get_line()
		);
	}

	// Must be top-level (indentation == 0)
	if(iter->get_text().length() > 0)
	{
		throw error(
			_("Expected top-level object after document type"),
			iter->get_line()
		);
	}
	list.next_token(iter);

	// Root object should follow
	if(iter->get_type() != token::TYPE_IDENTIFIER)
	{
		throw error(
			_("Expected root object after document type"),
			iter->get_line()
		);
	}

	m_object.deserialise(list, iter);

	// Must be EOF now
	if(iter != list.end() )
	{
		throw error(
			_("Expected end of input"),
			iter->get_line()
		);
	}
}

void obby::serialise::parser::serialise(
	const std::string& file
) const
{
	std::ofstream out(file.c_str() );
	if(!out)
	{
		obby::format_string str(_(
			"Could not open file '%0%' for writing"
		) );

		str << file;
	}

	serialise(out);
}

void obby::serialise::parser::serialise(
	std::ostream& stream
) const
{
	// Empty list
	token_list list;

	// Add document type
	list.add(token::TYPE_EXCLAMATION, "!", 0);
	list.add(token::TYPE_IDENTIFIER, m_type, 0);

	// Add top-level indentation
	list.add(token::TYPE_INDENTATION, "", 0);

	// Serialise root object with its children
	m_object.serialise(list);

	// Get string
	std::string result;
	list.serialise(result);

	// Write it into the file
	stream << result;
	stream.flush();
}

const std::string& obby::serialise::parser::get_type() const
{
	return m_type;
}

void obby::serialise::parser::set_type(
	const std::string& type
)
{
	m_type = type;
}

const obby::serialise::object& obby::serialise::parser::get_root() const
{
	return m_object;
}

obby::serialise::object& obby::serialise::parser::get_root()
{
	return m_object;
}
