/* libobby - Network text editing library
 * Copyright (C) 2005 0x539 dev group
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public
 * License as published by the Free Software Foundation; either
 i* version 2 of the License, or (at your option) any later version.
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

#include "document_info.hpp"

#if 0
#include "buffer.hpp"

obby::document_info::document_info(const user* owner, unsigned int id,
                                   const std::string& title)
 : m_owner(owner), m_id(id), m_title(title)
{
}

const obby::user* obby::document_info::get_owner() const
{
	return m_owner;
}

unsigned int obby::document_info::get_id() const
{
	return m_id;
}

const std::string& obby::document_info::get_title() const
{
	return m_title;
}

const obby::document& obby::document_info::get_content() const
{
	if(m_document.get() == NULL)
		throw std::logic_error("obby::document_info::get_content");

	return *m_document;
}

bool obby::document_info::is_subscribed(const user& user) const
{
	return std::find(
		m_users.begin(),
		m_users.end(),
		&user
	) != m_users.end();
}

obby::document_info::user_iterator obby::document_info::user_begin() const
{
	return m_users.begin();
}

obby::document_info::user_iterator obby::document_info::user_end() const
{
	return m_users.end();
}

obby::document_info::user_size_type obby::document_info::user_count() const
{
	return m_users.size();
}

obby::document_info::signal_rename_type
obby::document_info::rename_event() const
{
	return m_signal_rename;
}

obby::document_info::signal_subscribe_type
obby::document_info::subscribe_event() const
{
	return m_signal_subscribe;
}

obby::document_info::signal_unsubscribe_type
obby::document_info::unsubscribe_event() const
{
	return m_signal_unsubscribe;
}

void obby::document_info::obby_user_join(const user& user)
{
}

void obby::document_info::obby_user_part(const user& user)
{
	// User left the session: Unsubscribe from document
	user_unsubscribe(user);
}

void obby::document_info::user_subscribe(const user& user)
{
	// Add to list
	m_users.push_back(&user);
	// Emit subscription signal
	m_signal_subscribe.emit(user);
}

void obby::document_info::user_unsubscribe(const user& user)
{
	// Remove user from list
	m_users.erase(
		std::remove(m_users.begin(), m_users.end(), &user),
		m_users.end()
	);

	// Emit unsubscription signal
	m_signal_unsubscribe.emit(user);
}

void obby::document_info::rename_impl(const std::string& title)
{
	// Rename
	m_title = title;
	// Emit corresponding signal
	m_signal_rename.emit(title);
}
#endif
