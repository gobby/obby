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

#include "document_packet.hpp"

obby::document_packet::document_packet(const net6::packet& base)
 : net6::packet(base)
{
	// Verify command
	if(base.get_command() != "obby_document")
	{
		throw std::logic_error(
			"obby::document_packet::document_packet"
		);
	}

	// Must have at least two parameters: document and command
	if(base.get_param_count() < 2)
	{
		throw std::logic_error(
			"obby::document_packet::document_packet"
		);
	}
}

const std::string& obby::document_packet::get_command() const
{
	// Real command is obby_document, first parameter is document
	return net6::packet::get_param(1).serialised();
}

const net6::parameter&
obby::document_packet::get_param(unsigned int index) const
{
	return net6::packet::get_param(index + 2);
}

unsigned int obby::document_packet::get_param_count() const
{
	return net6::packet::get_param_count() - 2;
}

