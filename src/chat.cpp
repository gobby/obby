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

#include "format_string.hpp"
#include "common.hpp"
#include "chat.hpp"

namespace
{
	// Discards old messages
	void discard_messages(std::list<obby::chat::message*>& list,
	                      unsigned int max)
	{
		while(list.size() > max)
		{
			delete list.front();
			list.pop_front();
		}
	}
}

obby::chat::message::message(const std::string& text,
                             std::time_t timestamp):
	m_text(text), m_timestamp(timestamp)
{
}

obby::chat::message::message(const serialise::object& obj,
                             const user_table& user_table):
	m_text(obj.get_required_attribute("text").get_value() ),
	m_timestamp(obj.get_required_attribute("timestamp").as<std::time_t>() )
{
}

obby::chat::message::~message()
{
}

void obby::chat::message::serialise(serialise::object& obj) const
{
	obj.add_attribute("text").set_value(m_text);
	obj.add_attribute("timestamp").set_value(m_timestamp);
}

const std::string& obby::chat::message::get_text() const
{
	return m_text;
}

std::time_t obby::chat::message::get_timestamp() const
{
	return m_timestamp;
}

std::string obby::chat::message::format_timestamp(const char* format) const
{
	std::size_t alloc_size = 64;
	char* buf = static_cast<char*>(std::malloc(alloc_size) );

	std::tm* tm = std::localtime(&m_timestamp);

	for(;;)
	{
		std::size_t retval = std::strftime(buf, alloc_size, format, tm);
		if(retval == 0 || retval == alloc_size)
		{
			buf = static_cast<char*>(
				std::realloc(buf, alloc_size *= 2)
			);
		}
		else
			break;
	}

	std::string result = buf;
	std::free(buf);
	return result;
}

obby::chat::user_message::user_message(const std::string& text,
                                       std::time_t timestamp,
                                       const user& from):
	message(text, timestamp), m_user(from)
{
}

obby::chat::user_message::user_message(const serialise::object& obj,
                                       const user_table& user_table):
	message(obj, user_table),
	m_user(
		*obj.get_required_attribute("user").as<const user*>(user_table)
	)
{
}

void obby::chat::user_message::serialise(serialise::object& obj) const
{
	message::serialise(obj);
	obj.add_attribute("user").set_value(&m_user);
}

const obby::user& obby::chat::user_message::get_user() const
{
	return m_user;
}

std::string obby::chat::user_message::repr() const
{
	format_string str("<%0%> %1%");
	str << m_user.get_name() << m_text;
	return str.str();
}

obby::chat::emote_message::emote_message(const std::string& text,
                                         std::time_t timestamp,
                                         const user& from):
	user_message(text, timestamp, from)
{
}

obby::chat::emote_message::emote_message(const serialise::object& obj,
                                         const user_table& user_table):
	user_message(obj, user_table)
{
}

std::string obby::chat::emote_message::repr() const
{
	format_string str(" * %0% %1%");
	str << m_user.get_name() << m_text;
	return str.str();
}

obby::chat::server_message::server_message(const std::string& text,
                                           std::time_t timestamp):
	message(text, timestamp)
{
}

obby::chat::server_message::server_message(const serialise::object& obj,
                                           const user_table& user_table):
	message(obj, user_table)
{
}

std::string obby::chat::server_message::repr() const
{
	return m_text;
}

obby::chat::system_message::system_message(const std::string& text,
                                           std::time_t timestamp):
	message(text, timestamp)
{
}

obby::chat::system_message::system_message(const serialise::object& obj,
                                           const user_table& user_table):
	message(obj, user_table)
{
}

std::string obby::chat::system_message::repr() const
{
	return m_text;
}

/*obby::chat::chat(unsigned int max_messages):
	m_max_messages(max_messages)
{
}*/

obby::chat::~chat()
{
	clear();
}

void obby::chat::serialise(serialise::object& obj) const
{
	for(message_iterator iter = message_begin();
	    iter != message_end();
	    ++ iter)
	{
		message* msg = &(*iter);
		serialise::object& child = obj.add_child();

		// Set child name according to message type
		// TODO: virtual call that gets this string
		if(dynamic_cast<emote_message*>(msg) != NULL)
			child.set_name("emote_message");
		else if(dynamic_cast<user_message*>(msg) != NULL)
			child.set_name("user_message");
		else if(dynamic_cast<server_message*>(msg) != NULL)
			child.set_name("server_message");
		else if(dynamic_cast<system_message*>(msg) != NULL)
			child.set_name("system_message");
		else
			throw std::logic_error("obby::chat::serialise");

		iter->serialise(child);
	}
}

void obby::chat::deserialise(const serialise::object& obj,
                             const user_table& user_table)
{
	clear();

	for(serialise::object::child_iterator iter = obj.children_begin();
	    iter != obj.children_end();
	    ++ iter)
	{
		if(iter->get_name() == "emote_message")
		{
			add_message(new emote_message(*iter, user_table) );
		}
		else if(iter->get_name() == "user_message")
		{
			add_message(new user_message(*iter, user_table) );
		}
		else if(iter->get_name() == "server_message")
		{
			add_message(new server_message(*iter, user_table) );
		}
		else if(iter->get_name() == "system_message")
		{
			add_message(new system_message(*iter, user_table) );
		}
		else
		{
			// TODO: Unexpected child error
			format_string str(_("Unexpected child node: '%0%'") );
			str << iter->get_name();
			throw serialise::error(str.str(), iter->get_line() );
		}
	}

	add_message(new system_message(_("Restored session"), std::time(NULL)));
}

void obby::chat::clear()
{
	for(std::list<message*>::iterator iter = m_messages.begin();
	    iter != m_messages.end();
	    ++ iter)
	{
		delete *iter;
	}

	m_messages.clear();
}

void obby::chat::add_user_message(const std::string& text,
                                  const user& from)
{
	add_message(new user_message(text, std::time(NULL), from) );
}

void obby::chat::add_emote_message(const std::string& text,
                                   const user& from)
{
	add_message(new emote_message(text, std::time(NULL), from) );
}

void obby::chat::add_server_message(const std::string& text)
{
	add_message(new server_message(text, std::time(NULL)) );
}

obby::chat::message_iterator obby::chat::message_begin() const
{
	return m_messages.begin();
}

obby::chat::message_iterator obby::chat::message_end() const
{
	return m_messages.end();
}

obby::chat::signal_message_type obby::chat::message_event() const
{
	return m_signal_message;
}

void obby::chat::add_message(message* msg)
{
	m_messages.push_back(msg);
	discard_messages(m_messages, m_max_messages);
	m_signal_message.emit(*msg);
}

void obby::chat::on_sync_init(unsigned int)
{
	m_user_join_conn.block();
	m_user_part_conn.block();
	m_document_insert_conn.block();
}

void obby::chat::on_sync_final()
{
	m_user_join_conn.unblock();
	m_user_part_conn.unblock();
	m_document_insert_conn.unblock();
}

void obby::chat::on_user_join(const user& user)
{
	if(~user.get_flags() & user::flags::CONNECTED)
		return;

	obby::format_string str(_("%0% has joined") );
	str << user.get_name();
	add_message(new system_message(str.str(), std::time(NULL)) );
}

void obby::chat::on_user_part(const user& user)
{
	obby::format_string str(_("%0% has left") );
	str << user.get_name();
	add_message(new system_message(str.str(), std::time(NULL)) );
}
