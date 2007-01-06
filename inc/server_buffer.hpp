/* libobby - Network text editing library
 * Copyright (C) 2005 0x539 dev group
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this program; if not, write to the Free
 * Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#ifndef _OBBY_SERVER_BUFFER_HPP_
#define _OBBY_SERVER_BUFFER_HPP_

#include <sigc++/signal.h>
#include <net6/server.hpp>
#include "insert_record.hpp"
#include "delete_record.hpp"
#include "buffer.hpp"

namespace obby
{

class server_buffer : public buffer, public sigc::trackable
{
public: 
	typedef sigc::signal<void, const insert_record&> signal_insert_type;
	typedef sigc::signal<void, const delete_record&> signal_delete_type;
	typedef sigc::signal<void, net6::server::peer&> signal_join_type;
	typedef sigc::signal<void, net6::server::peer&> signal_login_type;
	typedef sigc::signal<void, net6::server::peer&> signal_part_type;

	server_buffer(unsigned int port);
	~server_buffer();

	void select();
	void select(unsigned int timeout);

	signal_insert_type insert_event() const;
	signal_delete_type delete_event() const;
	signal_join_type join_event() const;
	signal_login_type login_event() const;
	signal_part_type part_event() const;

protected:
	void on_join(net6::server::peer& peer);
	void on_login(net6::server::peer& peer);
	void on_part(net6::server::peer& peer);
	void on_data(const net6::packet& pack, net6::server::peer& from);

	unsigned int m_counter;
	net6::server m_server;

	signal_insert_type m_signal_insert;
	signal_delete_type m_signal_delete;
	signal_join_type m_signal_join;
	signal_login_type m_signal_login;
	signal_part_type m_signal_part;
};

}

#endif // _OBBY_SERVER_BUFFER_HPP_
