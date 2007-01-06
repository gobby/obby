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
	 : m_content(format), m_argument(0) { }
	basic_format_string(const basic_format_string& other)
	 : m_content(other.m_content), m_argument(other.m_argument) { }
	~basic_format_string() { }

	basic_format_string& operator=(const string_type& format) {
		m_content = format;
		m_argument = 0;
		return *this;
	}
	basic_format_string& operator=(const basic_format_string& other) {
		m_content = other.m_content;
		m_argument = other.m_argument;
		return *this;
	}

	const string_type& str() const {
		return m_content;
	}

	// TODO: Some sort of escaping
	template<class value_type>
	basic_format_string& operator<<(const value_type& value) {
		stream_type value_stream;
		value_stream << value;
		string_type value_str = value_stream.str();

		stream_type find_stream;
		find_stream << "%" << (m_argument ++);
		string_type find_str = find_stream.str();
		
		typename string_type::size_type pos = 0;
		while( (pos = m_content.find(find_str, pos)) !=
		      string_type::npos) {
			m_content.replace(pos, find_str.length(), value_str);
			pos += value_str.length();
		}

		return *this;
	}
protected:
	string_type m_content;
	unsigned int m_argument;
};

typedef basic_format_string<std::string, std::stringstream> format_string;

}

#endif // _OBBY_FORMATSTRING_HPP_
