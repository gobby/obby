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

#ifndef _OBBY_SERIALISE_PARSER_HPP_
#define _OBBY_SERIALISE_PARSER_HPP_

#include <vector>
#include <slist>
#include "serialise/token.hpp"
#include "serialise/attribute.hpp"

namespace obby::serialise
{

class object
{
public:
	typedef std::map<std::string, attribute> attribute_map;
	class attribute_iterator : public attribute_map::iterator
	{
	public:
		typedef attribute value_type;
		typedef attribute_map::iterator base_iterator;

		attribute_iterator(
			const base_iterator& base
		);

		value_type& operator*();
		const value_type& operator*() const;

		value_type* operator->();
		const value_type* operator->() const;
	};

	typedef std::slist<object>::const_iterator child_iterator;

	object(
		object* parent = NULL
	);

	void serialise(
		token_list& tokens
	) const;

	void deserialise(
		const token_list& tokens,
		token_list::iterator& iter
	);

	object& add_child();

	const std::string& get_name() const;

	void set_name(
		const std::string& name
	);

	attribute& add_attribute(
		const std::string& name
	);

	attribute* get_attriubte(
		const std::string& name
	);

	const attribute* get_attribute(
		const std::string& name
	) const;

	attribute_iterator attributes_begin() const;
	attribute_iterator attributes_end() const;

	child_iterator children_begin() const;
	child_iterator children_end() const;

	unsigned int get_line() const;

	unsigned int get_indentation() const;

protected:
	object* m_parent;
	std::string m_name;
	attribute_map m_attributes;
	std::slist<object> m_children;
	unsigned int m_line;

private:
	std::slist<object>::iterator m_last_child;
};

} // namespace obby::serialise

#endif // _OBBY_SERIALISE_PARSER_HPP_
