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
#include "serialise/token.hpp"

namespace
{
	using namespace obby::serialise;
	typedef std::string::const_iterator string_iterator;

	void escape(
		std::string& src
	)
	{
		std::string::size_type pos = 0;
		while( (pos = src.find_first_of("\n\t\\\"", pos)) != std::string::npos)
		{
			std::string replace_with;

			switch(src[pos])
			{
			case '\n':
				replace_with = "\\n";
				break;
			case '\t':
				replace_with = "\\t";
				break;
			case '\\':
				replace_with = "\\\\";
				break;
			case '\"':
				replace_with = "\\\"";
				break;
			}

			src.replace(pos, 1, replace_with);
			pos += replace_with.length();
		}
	}

	void unescape(
		std::string& src,
		unsigned int src_line
	)
	{
		std::string::size_type pos = 0;
		while( (pos = src.find('\\', pos)) != std::string::npos)
		{
			char replace_with;

			// \\ cannot be at end of string - terminating " would
			// have been escaped
			switch(src[pos + 1])
			{
			case 'n':
				replace_with = '\n';
				break;
			case '\\':
				replace_with = '\\';
				break;
			case 't':
				replace_with = '\t';
				break;
			case '\"':
				replace_with = '\"';
				break;
			default:
				obby::format_string str(
					_("Unexpected escape sequence: \\%0%")
				);

				str << src[pos + 1];
				throw error(str.str(), src_line);
			}

			src.replace(pos, 2, 1, replace_with);
			++ pos;
		}
	}

	void tokenise_identifier(
		token_list& list,
		const std::string& src,
		string_iterator& iter,
		unsigned int& line
	)
	{
		// Read to next non-alphanumerical character
		string_iterator orig = iter ++;
		for(; iter != src.end(); ++ iter)
			if(!isalnum(*iter) && *iter != '_')
				break;

		list.add(token::TYPE_IDENTIFIER, std::string(orig, iter), line);
	}

	void tokenise_comment(
		token_list& list,
		const std::string& src,
		string_iterator& iter,
		unsigned int& line
	)
	{
		// Ignore rest of line
		for(++ iter; iter != src.end(); ++ iter)
			if(*iter == '\n')
				break;
	}

	void tokenise_string(
		token_list& list,
		const std::string& src,
		string_iterator& iter,
		unsigned int& line
	)
	{
		string_iterator orig = ++ iter;
		unsigned int orig_line = line;
		bool escaped = false;

		for(; iter != src.end(); ++ iter)
		{
			// Line counting
			if(*iter == '\n')
				++ line;

			// Is this character escaped?
			if(!escaped)
			{
				if(*iter == '\\')
					escaped = true;
				else if(*iter == '\"')
					break;
			}
			else
			{
				// This one was escaped, process normally with
				// next one
				escaped = false;
			}
		}

		// Unexpected end of input
		if(iter == src.end() )
			throw error(_("String not closed"), orig_line);

		// Add unescaped string literal
		std::string unescaped(orig, iter);
		unescape(unescaped, orig_line);
		list.add(token::TYPE_STRING, unescaped, orig_line);

		// Proceed after string termination ('\"')
		++ iter;
	}

	void tokenise_indentation(
		token_list& list,
		const std::string& src,
		string_iterator& iter,
		unsigned int& line
	)
	{
		// Proceed to next char that is not a space character
		string_iterator orig = iter;
		for(; iter != src.end(); ++ iter)
			if(!isspace(*iter) || *iter == '\n')
				break;

		// Add indentation if this is not an empty line
		if(*iter != '\n' && *iter != '\0' && iter != src.end() )
		{
			list.add(
				token::TYPE_INDENTATION,
				std::string(orig, iter),
				line
			);
		}
	}

	void tokenise(
		token_list& list,
		const std::string& src
	)
	{
		unsigned int line = 1;
		for(string_iterator iter = src.begin();
		    iter != src.end();)
		{
			// Nullbyte identifies end of string
			if(*iter == '\0')
				break;

			if(*iter == '\n')
			{
				// Line counting
				++ line;

				// Parse it
				++ iter;
				tokenise_indentation(
					list, src, iter, line
				);

				continue;
			}

			// String literal
			if(*iter == '\"')
			{
				tokenise_string(list, src, iter, line);
				continue;
			}

			// Comment
			if(*iter == '#')
			{
				tokenise_comment(list, src, iter, line);
				continue;
			}

			// Identifier
			if(isalnum(*iter) || *iter == '_')
			{
				tokenise_identifier(list, src, iter, line);
				continue;
			}

			// Ignore whitespace
			if(isspace(*iter) )
			{
				++ iter;
				continue;
			}

			// Special character?
			token::type type = token::TYPE_UNKNOWN;
			switch(*iter)
			{
			case '!':
				type = token::TYPE_EXCLAMATION;
				break;
			case '=':
				type = token::TYPE_ASSIGNMENT;
				break;
			}

			if(type == token::TYPE_UNKNOWN)
			{
				obby::format_string str(
					_("Unexpected token: '%0%'")
				);

				str << *iter;
				throw error(str.str(), line);
			}

			list.add(type, std::string(1, *iter), line);

			// Go on with next character
			++ iter;
		}
	}

	void detokenise(const token_list& list, std::string& target)
	{
		bool line_begin = true;
		std::string escaped_string;

		for(token_list::iterator iter = list.begin();
		    iter != list.end();
		    ++ iter)
		{
			switch(iter->get_type() )
			{
			case token::TYPE_INDENTATION:
				target.append("\n" + iter->get_text() );
				line_begin = true;
				break;
			case token::TYPE_STRING:
				escaped_string = iter->get_text();
				escape(escaped_string);

				target.append("\"");
				target.append(escaped_string);
				target.append("\"");

				line_begin = false;
				break;
			case token::TYPE_IDENTIFIER:
				if(!line_begin)
					target.append(" ");
				// Fallthrough
			default:
				target.append(iter->get_text() );
				if(iter->get_type() != token::TYPE_EXCLAMATION)
					line_begin = false;
				break;
			}
		}
	}
}

obby::serialise::token::token(
	type type,
	const std::string& text,
	unsigned int line
) :
	m_type(type), m_text(text), m_line(line)
{
}

obby::serialise::token::type obby::serialise::token::get_type() const
{
	return m_type;
}

const std::string& obby::serialise::token::get_text() const
{
	return m_text;
}

unsigned int obby::serialise::token::get_line() const
{
	return m_line;
}

obby::serialise::token_list::token_list()
{
}

void obby::serialise::token_list::serialise(
	std::string& string
) const
{
	detokenise(*this, string);
}

void obby::serialise::token_list::deserialise(
	const std::string& string
)
{
	tokenise(*this, string);
}

void obby::serialise::token_list::add(
	token::type type,
	const std::string& text,
	unsigned int line
)
{
	m_list.push_back(token(type, text, line) );
}

obby::serialise::token_list::iterator obby::serialise::token_list::begin() const
{
	return m_list.begin();
}

obby::serialise::token_list::iterator obby::serialise::token_list::end() const
{
	return m_list.end();
}

void obby::serialise::token_list::next_token(
	iterator& iter
) const
{
	unsigned int orig_line = iter->get_line();
	if(++ iter == m_list.end() )
		throw error(_("Unexpected end of input"), orig_line);
}

