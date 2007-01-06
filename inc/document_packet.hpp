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

#ifndef _OBBY_DOCUMENT_PACKET_HPP_
#define _OBBY_DOCUMENT_PACKET_HPP_

#include <net6/packet.hpp>

namespace obby
{

class document_info;

/** Packet which belongs to a specified document.
 */
class document_packet : public net6::packet
{
public:
	document_packet(const document_info& info, const std::string& command);
	document_packet(const net6::packet& base);

	const std::string& get_command() const;
	const net6::basic_parameter& get_param(unsigned int index) const;
	unsigned int get_param_count() const;
};

} // namespace obby

#endif // _OBBY_DOCUMENT_PACKET_HPP_
