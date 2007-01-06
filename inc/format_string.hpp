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

#ifndef _OBBY_FORMATSTRING_HPP_
#define _OBBY_FORMATSTRING_HPP_

#include <string>
#include <vector>
#include <sstream>
#include <sigc++/signal.h>

namespace obby
{

/** Format string that may be used for type-safe printf-like formatting.
 */

template<typename string_type, typename stream_type>
class basic_format_string
{
public:
	basic_format_string(const string_type& format) 
	 : m_content(format) { }
	basic_format_string(const basic_format_string& other)
	 : m_content(other.m_content), m_arguments(other.m_arguments) { }
	~basic_format_string() { }

	basic_format_string& operator=(const string_type& format) {
		m_content = format;
		m_arguments.clear();
		return *this;
	}
	basic_format_string& operator=(const basic_format_string& other) {
		m_content = other.m_content;
		m_arguments = other.m_arguments;
		return *this;
	}

	string_type str() const {
		// Copy content 
		string_type content(m_content);

		// Replace place holders
		typename string_type::size_type pos = 0, end = 0;
		while( (pos = content.find('%', pos)) != string_type::npos)
		{
			// Got first % char, look for second one.
			end = content.find('%', pos + 1);
			if(end == std::string::npos)
				break;

			// %% -> % in string
			if(pos + 1 == end)
			{
				content.erase(++ pos, 1);
				continue;
			}

			// Convert text in between to int
			int argnum = strtol(content.c_str() + pos+1, NULL, 10);
			const string_type& arg = m_arguments[argnum];
			content.replace(pos, end - pos + 1, arg);
			pos += arg.length();
		}

		// Return new string
		return content;
	}

	template<class value_type>
	basic_format_string& operator<<(const value_type& value) {
		stream_type value_stream;
		value_stream << value;
		m_arguments.push_back(value_stream.str() );
		return *this;
	}

protected:
	string_type m_content;
	std::vector<string_type> m_arguments;
};

typedef basic_format_string<std::string, std::stringstream> format_string;

}

#endif // _OBBY_FORMATSTRING_HPP_
