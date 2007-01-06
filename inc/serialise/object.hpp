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

#ifndef _OBBY_SERIALISE_OBJECT_HPP_
#define _OBBY_SERIALISE_OBJECT_HPP_

#include <map>
#include <list>
#include "token.hpp"
#include "attribute.hpp"

namespace obby
{

namespace serialise
{

class object
{
public:
	typedef std::map<std::string, attribute> attribute_map;
	class attribute_iterator : public attribute_map::const_iterator
	{
	public:
		typedef attribute value_type;
		typedef attribute_map::const_iterator base_iterator;

		attribute_iterator(
			const base_iterator& base
		);

		//value_type& operator*();
		const value_type& operator*() const;

		//value_type* operator->();
		const value_type* operator->() const;
	};

	typedef std::list<object>::const_iterator child_iterator;

	object(
		const object* parent = NULL
	);

	void serialise(
		token_list& tokens
	) const;

	void deserialise(
		const token_list& tokens,
		token_list::iterator& iter
	);

	const object* get_parent() const;

	object& add_child();

	const std::string& get_name() const;

	void set_name(
		const std::string& name
	);

	attribute& add_attribute(
		const std::string& name
	);

	attribute* get_attribute(const std::string& name);
	const attribute* get_attribute(const std::string& name) const;

	/** Throws a serialise::error if the attribute is not defined.
	 */
	attribute& get_required_attribute(const std::string& name);

	/** Throws a serialise::error if the attribute is not defined.
	 */
	const attribute& get_required_attribute(const std::string& name) const;

	attribute_iterator attributes_begin() const;
	attribute_iterator attributes_end() const;

	child_iterator children_begin() const;
	child_iterator children_end() const;

	unsigned int get_line() const;

	unsigned int get_indentation() const;

protected:
	const object* m_parent;
	std::string m_name;
	attribute_map m_attributes;
	std::list<object> m_children;
	unsigned int m_line;
};

} // namespace serialise

} // namespace obby

#endif // _OBBY_SERIALISE_OBJECT_HPP_
