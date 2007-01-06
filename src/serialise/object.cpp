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

#include "common.hpp"
#include "format_string.hpp"
#include "serialise/error.hpp"
#include "serialise/object.hpp"

obby::serialise::object::attribute_iterator::attribute_iterator(
	const base_iterator& base
) :
	base_iterator(base)
{
}
/*
obby::serialise::object::attribute_iterator::value_type&
obby::serialise::object::attribute_iterator::operator*()
{
	return base_iterator::operator->()->second;
}
*/
const obby::serialise::object::attribute_iterator::value_type&
obby::serialise::object::attribute_iterator::operator*() const
{
	return base_iterator::operator->()->second;
}
/*
obby::serialise::object::attribute_iterator::value_type*
obby::serialise::object::attribute_iterator::operator->()
{
	return &base_iterator::operator->()->second;
}
*/
const obby::serialise::object::attribute_iterator::value_type*
obby::serialise::object::attribute_iterator::operator->() const
{
	return &base_iterator::operator->()->second;
}

obby::serialise::object::object(
	const object* parent
) :
	m_parent(parent), m_name(), m_line(0)
{
}

void obby::serialise::object::serialise(
	token_list& tokens
) const
{
	// Find indentation level
	unsigned int indentation_deep = get_indentation();

	// Add object name
	tokens.add(token::TYPE_IDENTIFIER, m_name, 0);

	// Add attributes
	for(attribute_iterator iter = attributes_begin();
	    iter != attributes_end();
	    ++ iter)
	{
		iter->serialise(tokens);
	}

	// Add children
	for(child_iterator iter = children_begin();
	    iter != children_end();
	    ++ iter)
	{
		// Indent child to our indentation + 1
		tokens.add(
			token::TYPE_INDENTATION,
			std::string(indentation_deep + 1, ' '),
			0
		);

		// Serialise it
		iter->serialise(tokens);
	}
}

void obby::serialise::object::deserialise(
	const token_list& tokens,
	token_list::iterator& iter
)
{
	unsigned int indentation_deep = get_indentation();

	// Object name
	m_name = iter->get_text();

	// Line of object declaration
	m_line = iter->get_line();
	++ iter;

	// Expect any number of attributes
	while(iter != tokens.end() &&
	      iter->get_type() == token::TYPE_IDENTIFIER)
		m_attributes[iter->get_text()].deserialise(tokens, iter);

	// Indentation (for child objects)
	while(iter != tokens.end() &&
	      iter->get_type() == token::TYPE_INDENTATION)
	{
		// Get indentation of this child
		unsigned int child_indentation = iter->get_text().length();
		// Belongs to some super-object
		if(child_indentation <= indentation_deep) break;

		// Is a child
		tokens.next_token(iter);

		// Next token must be identifier (object name)
		if(iter->get_type() == token::TYPE_IDENTIFIER)
		{
			if(indentation_deep != child_indentation - 1)
			{
				throw error(
					_(
						"Child object's indentation "
						"must be parent's plus one"
					),
					iter->get_line()
				);
			}

			object& child = add_child();
			child.deserialise(tokens, iter);
		}
		else
		{
			throw error(
				_("Expected child object after indentation"),
				iter->get_line()
			);
		}
	}

	// An object _must_ follow even if it is not a child of us but one of
	// our parent (or grandparent...).
	if(iter != tokens.end() && iter->get_type() != token::TYPE_INDENTATION)
	{
		obby::format_string str(
			_("Expected child object instead of '%0%'")
		);
		str << iter->get_text();
		throw error(str.str(), iter->get_line() );
	}
}

const obby::serialise::object* obby::serialise::object::get_parent() const
{
	return m_parent;
}

obby::serialise::object& obby::serialise::object::add_child()
{
	m_children.push_back(object(this) );
	return m_children.back();
}

const std::string& obby::serialise::object::get_name() const
{
	return m_name;
}

void obby::serialise::object::set_name(
	const std::string& name
)
{
	m_name = name;
}

obby::serialise::attribute& obby::serialise::object::add_attribute(
	const std::string& name
)
{
	return m_attributes.insert(
		std::make_pair(name, attribute(name))
	).first->second;
}

obby::serialise::attribute*
obby::serialise::object::get_attribute(const std::string& name)
{
	attribute_map::iterator iter = m_attributes.find(name);
	if(iter == m_attributes.end() ) return NULL;
	return &iter->second;
}

const obby::serialise::attribute*
obby::serialise::object::get_attribute(const std::string& name) const
{
	attribute_map::const_iterator iter = m_attributes.find(name);
	if(iter == m_attributes.end() ) return NULL;
	return &iter->second;
}

obby::serialise::attribute&
obby::serialise::object::get_required_attribute(const std::string& name)
{
	attribute_map::iterator iter = m_attributes.find(name);
	if(iter == m_attributes.end() )
	{
		format_string str(_("Object '%0%' requires attribute '%1%'") );
		str << m_name << name;
		throw error(str.str(), m_line);
	}

	return iter->second;
}

const obby::serialise::attribute&
obby::serialise::object::get_required_attribute(const std::string& name) const
{
	attribute_map::const_iterator iter = m_attributes.find(name);
	if(iter == m_attributes.end() )
	{
		format_string str(_("Object '%0%' requires attribute '%1%'") );
		str << m_name << name;
		throw error(str.str(), m_line);
	}

	return iter->second;
}

obby::serialise::object::attribute_iterator
obby::serialise::object::attributes_begin() const
{
	return attribute_iterator(m_attributes.begin() );
}

obby::serialise::object::attribute_iterator
obby::serialise::object::attributes_end() const
{
	return attribute_iterator(m_attributes.end() );
}

obby::serialise::object::child_iterator
obby::serialise::object::children_begin() const
{
	return m_children.begin();
}

obby::serialise::object::child_iterator
obby::serialise::object::children_end() const
{
	return m_children.end();
}

unsigned int obby::serialise::object::get_line() const
{
	return m_line;
}

unsigned int obby::serialise::object::get_indentation() const
{
	unsigned int indentation_deep = 0;
	const object* parent = m_parent;

	// Find indentation level
	while(parent != NULL)
	{
		++ indentation_deep;
		parent = parent->get_parent();
	}

	return indentation_deep;
}
