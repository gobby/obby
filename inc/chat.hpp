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

#ifndef _OBBY_CHAT_HPP_
#define _OBBY_CHAT_HPP_

#include <ctime>
#include <list>
#include <sigc++/signal.h>
#include <sigc++/connection.h>
#include "user.hpp"
#include "user_table.hpp"
#include "document_info.hpp"

namespace obby
{

/** Handles obby chat messages.
 */
class chat
{
public:
	/** Base class for chat messages.
	 */
	class message: private net6::non_copyable
	{
	public:
		message(const std::string& text,
		        std::time_t timestamp);
		message(const serialise::object& obj,
		        const user_table& user_table);
		virtual ~message();

		virtual void serialise(serialise::object& obj) const;

		const std::string& get_text() const;
		std::time_t get_timestamp() const;

		std::string format_timestamp(const char* format) const;

		virtual std::string repr() const = 0;
	protected:
		std::string m_text;
		std::time_t m_timestamp;
	};

	/** Message sent by a user.
	 */
	class user_message: public message
	{
	public:
		user_message(const std::string& text,
		             std::time_t timestamp,
		             const user& from);
		user_message(const serialise::object& obj,
		             const user_table& user_table);

		virtual void serialise(serialise::object& obj) const;

		const user& get_user() const;
		virtual std::string repr() const;
	protected:
		const user& m_user;
	};

	/** Message sent by server.
	 */
	class server_message: public message
	{
	public:
		server_message(const std::string& text,
		               std::time_t timestamp);
		server_message(const serialise::object& obj,
		               const user_table& user_table);

		virtual std::string repr() const;
	};

	/** Base system notice message.
	 */
	class system_message: public message
	{
	public:
		system_message(const std::string& text,
		               std::time_t timestamp);
		system_message(const serialise::object& obj,
		               const user_table& user_table);

		virtual std::string repr() const;
	};

	typedef ptr_iterator<
		message,
		std::list<message*>,
		std::list<message*>::const_iterator
	> message_iterator;

	typedef sigc::signal<void, const message&>
		signal_message_type;

	/** Chat constructor.
	 * @param buffer Buffer owning this chat.
	 * TODO: Just take user_table and connect to user_table's signals
	 * @param max_messages How many messages to store before deleting
	 * old ones.
	 */
	template<typename buffer_type>
	chat(const buffer_type& buffer, unsigned int max_messages):
		m_max_messages(max_messages)
	{
		buffer.sync_init_event().connect(
			sigc::mem_fun(*this, &chat::on_sync_init) );
		buffer.sync_final_event().connect(
			sigc::mem_fun(*this, &chat::on_sync_final) );
		m_user_join_conn = buffer.user_join_event().connect(
			sigc::mem_fun(*this, &chat::on_user_join) );
		m_user_part_conn = buffer.user_part_event().connect(
			sigc::mem_fun(*this, &chat::on_user_part) );
		m_document_insert_conn = buffer.document_insert_event().connect(
			sigc::mem_fun(*this, &chat::on_document_insert) );
	}

	~chat();

	/** Serialises the chat history to a serialisation object.
	 * @param obj Object to serialise to.
	 */
	void serialise(serialise::object& obj) const;

	/** Deserialises a chat history from a serialisation object.
	 * @param obj Object to deserialise from.
	 * @param user_table user_table where to read user information from.
	 */
	void deserialise(const serialise::object& obj,
	                 const user_table& user_table);

	/** Clears the whole history.
	 */
	void clear();

	/** Adds a new user message to the history.
	 */
	void add_user_message(const std::string& text,
	                      const user& from);

	/** Adds a new server message to the history.
	 */
	void add_server_message(const std::string& text);

	/** Returns an iterator pointing at the beginning of the chat history.
	 */
	message_iterator message_begin() const;

	/** Returns an iterator pointing at the end of the chat history.
	 */
	message_iterator message_end() const;

	/** Signal that will be emitted if a user message arrived.
	 */
	signal_message_type message_event() const;
protected:
	void add_message(message* msg);

	void on_sync_init(unsigned int);
	void on_sync_final();

	void on_user_join(const user& user);
	void on_user_part(const user& user);
	void on_document_insert(document_info& document);

	unsigned int m_max_messages;
	std::list<message*> m_messages;

	signal_message_type m_signal_message;

	sigc::connection m_user_join_conn;
	sigc::connection m_user_part_conn;
	sigc::connection m_document_insert_conn;
};

} // namespace obby

#endif // _OBBY_CHAT_HPP_

