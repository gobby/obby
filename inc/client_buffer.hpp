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

#ifndef _OBBY_CLIENT_BUFFER_HPP_
#define _OBBY_CLIENT_BUFFER_HPP_

#include <string>
#include <list>
#include <sigc++/signal.h>
#include <net6/client.hpp>
#include "record.hpp"
#include "insert_record.hpp"
#include "delete_record.hpp"
#include "buffer.hpp"

namespace obby
{

class client_buffer : public buffer, public sigc::trackable
{
public:
	typedef sigc::signal<void, const insert_record&> signal_insert_type;
	typedef sigc::signal<void, const delete_record&> signal_delete_type;
	typedef sigc::signal<void, net6::client::peer&> signal_join_type;
	typedef sigc::signal<void, net6::client::peer&> signal_part_type;
	typedef sigc::signal<void>                      signal_close_type;
	typedef sigc::signal<void, const std::string&> signal_login_failed_type;
	
	client_buffer(const std::string& hostname, unsigned int port);
	~client_buffer();
	
	void login(const std::string& name);
	net6::client::peer* get_self() const;

	void select();
	void select(unsigned int timeout);

	virtual void insert(const position& pos, const std::string& text);
	virtual void erase(const position& from, const position& to);

	signal_insert_type insert_event() const;
	signal_delete_type delete_event() const;
	signal_join_type join_event() const;
	signal_part_type part_event() const;
	signal_close_type close_event() const;
	signal_login_failed_type login_failed_event() const;

protected:
	void on_join(net6::client::peer& peer);
	void on_part(net6::client::peer& peer);
	void on_close();
	void on_data(const net6::packet& pack);
	void on_login_failed(const std::string& reason);

	std::list<record*> m_unsynced;
	net6::client m_connection;

	signal_insert_type m_signal_insert;
	signal_delete_type m_signal_delete;
	signal_join_type m_signal_join;
	signal_part_type m_signal_part;
	signal_close_type m_signal_close;
	signal_login_failed_type m_signal_login_failed;
};

}

#endif // _OBBY_CLIENT_BUFFER_HPP_
