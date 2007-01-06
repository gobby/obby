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

#ifndef _OBBY_SERIALISE_ATTRIBUTE_HPP_
#define _OBBY_SERIALISE_ATTRIBUTE_HPP_

#include <string>
#include <sstream>
#include <net6/serialise.hpp>
#include "../format_string.hpp"
#include "error.hpp"
#include "token.hpp"

namespace obby
{

namespace serialise
{

/** Attribute for an object. An attribute has a name and a corresponding value.
 */
class attribute
{
public:
	/** Creates a new attribute with a value of the given data type that
	 * is serialised using the given context.
	 */
	template<typename data_type>
	attribute(const std::string& name,
	          const data_type& value,
	          const ::serialise::context<data_type>& ctx =
	          ::serialise::context<data_type>());

	/** Creates a new attribute with the serialised value given.
	 */
	attribute(const std::string& name = "Unnamed",
	          const std::string& value = "Unassigned");

	/** Serialises the attribute to a list of tokens.
	 */
	void serialise(token_list& tokens) const;

	/** Deserialises the attribute from a list of tokens.
	 */
	void deserialise(const token_list& tokens,
	                 token_list::iterator& iter);

	/** Changes the value of the attribute by serialising the value to
	 * a string using <em>ctx</em> as context.
	 */
	template<typename data_type>
	void set_value(const data_type& value,
	               const ::serialise::context<data_type>& ctx =
	               ::serialise::context<data_type>());

	/** Changes the value of the attribute.
	 */
	void set_value(const std::string& value);

	/** Returns the serialised value of the attribute.
	 */
	const std::string& get_value() const;

	/** Returns the name of the attribute.
	 */
	const std::string& get_name() const;

	/** Returns the line where the attribute occured in the source file, if
	 * the attribute was deserialised from a token list.
	 */
	unsigned int get_line() const;

	/** Deserialises the value of the attribute to <em>data_type</em> using
	 * the context <em>ctx</em>.
	 */
	template<typename data_type>
	data_type as(const ::serialise::context<data_type>& ctx =
	             ::serialise::context<data_type>()) const;
private:
	std::string m_name;
	::serialise::data m_value;
	unsigned int m_line;
};

template<typename data_type>
attribute::attribute(const std::string& name,
	             const data_type& value,
	             const ::serialise::context<data_type>& ctx):
	m_name(name), m_value(value, ctx), m_line(0)
{
}

template<typename data_type>
void attribute::set_value(const data_type& value,
                          const ::serialise::context<data_type>& ctx)
{
	m_value = ::serialise::data(value, ctx);
}

template<typename data_type>
data_type attribute::as(const ::serialise::context<data_type>& ctx) const
{
	try
	{
		return m_value.as<data_type>(ctx);
	}
	catch(::serialise::conversion_error& e)
	{
		// TODO: Localise this. This would depend on a call to _
		// in a header file!
		format_string str("Attribute '%0%' has unexpected type");
		str << m_name;
		throw error(str.str(), m_line);
	}
}

} // namespace serialise

} // namespace obby

#endif // _OBBY_SERIALISE_ATTRIBUTE_HPP_
