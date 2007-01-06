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

#ifndef _OBBY_SERIALISE_TOKEN_HPP_
#define _OBBY_SERIALISE_TOKEN_HPP_

#include <string>
#include <slist>

namespace obby::serialise
{

class token
{
public:
	enum type
	{
		TYPE_UNKNOWN,

		TYPE_INDENTATION,
		TYPE_EXCLAMATION,
		TYPE_IDENTIFIER,
		TYPE_STRING,
		TYPE_ASSIGNMENT
	};

	token(
		type type,
		const std::string& text,
		unsigned int line
	);

	type get_type() const;
	const std::string& get_text() const;
	unsigned int get_line() const;
private:
	type m_type;
	std::string m_text;
	unsigned int m_line;
};

class token_list
{
public:
	typedef std::slist<token> list_type;
	typedef list_type::const_iterator iterator;

	token_list();

	void serialise(std::string& string) const;
	void deserialise(const std::string& string);

	void add(
		token::type type,
		const std::string& text,
		unsigned int line
	);

	iterator begin() const;
	iterator end() const;

	/** Advances to the next token (as with ++ iter) but it throws an error
	 * if there is no next token (end of token list). This is used to
	 * determinate unexpected end of input while parsing the token list
	 * into objects and attributes.
	 */
	void next_token(
		iterator& iter
	) const;
private:
	list_type m_list;
	iterator m_last;
};

} // namespace obby::serialise

#endif // _OBBY_SERIALISE_TOKEN_HPP_
