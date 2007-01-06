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

#include <cassert>
#include <net6/server.hpp>
#include "user.hpp"
#include "document_info.hpp"

obby::user::user(net6::peer& peer, int red, int green, int blue)
 : m_peer(&peer), m_id(peer.get_id() ), m_name(peer.get_name() ), m_red(red),
   m_green(green), m_blue(blue), m_flags(CONNECTED)
{
}

obby::user::user(unsigned int id, const std::string& name, int red, int green,
                 int blue)
 : m_peer(NULL), m_id(id), m_name(name), m_red(red), m_green(green),
   m_blue(blue), m_flags(NONE)
{
}

obby::user::~user()
{
}

void obby::user::release_peer()
{
	m_peer = NULL;
	remove_flags(CONNECTED);
}

void obby::user::assign_peer(net6::peer& peer, int red, int green, int blue)
{
	// User must not be already connected
	assert(~get_flags() & CONNECTED);

	// Name must be the same
	assert(m_name == peer.get_name() );

	m_peer = &peer;
	m_red = red;
	m_green = green;
	m_blue = blue;

	add_flags(CONNECTED);
}

net6::peer* obby::user::get_peer() const
{
	return m_peer;
}

const std::string& obby::user::get_name() const
{
	return m_name;
}

const net6::address& obby::user::get_address() const
{
	return static_cast<const net6::server::peer*>(m_peer)->get_address();
}

unsigned int obby::user::get_id() const
{
	return m_id;
}

int obby::user::get_red() const
{
	return m_red;
}

int obby::user::get_green() const
{
	return m_green;
}

int obby::user::get_blue() const
{
	return m_blue;
}

const std::string& obby::user::get_password() const
{
	return m_password;
}

obby::user::flags obby::user::get_flags() const
{
	return m_flags;
}

void obby::user::set_password(const std::string& password)
{
	m_password = password;
}

void obby::user::add_flags(flags new_flags)
{
	m_flags |= new_flags;
}

void obby::user::remove_flags(flags old_flags)
{
	m_flags &= ~old_flags;
}

#if 0
void obby::user::subscribe(document_info& document)
{
	m_documents.push_back(&document);
}

void obby::user::unsubscribe(document_info& document)
{
	m_documents.erase(
		std::remove(m_documents.begin(), m_documents.end(), &document),
		m_documents.end()
	);
}

obby::document_info* obby::user::find_document(unsigned int id) const
{
	for(document_iterator i = document_begin(); i != document_end(); ++ i)
		if(i->get_id() == id)
			return &(*i);
	return NULL;
}

bool obby::user::is_subscribed(const document_info& document) const
{
	return find_document(document.get_id() ) != NULL;
}

obby::user::document_iterator obby::user::document_begin() const
{
	return m_documents.begin();
}

obby::user::document_iterator obby::user::document_end() const
{
	return m_documents.end();
}
#endif
