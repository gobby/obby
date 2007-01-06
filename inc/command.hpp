/* libobby - Network text editing library
 * Copyright (C) 2005, 2006 0x539 dev group
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

#ifndef _OBBY_COMMAND_HPP_
#define _OBBY_COMMAND_HPP_

#include <map>
#include <list>
#include <queue>
#include <memory>

#include <net6/packet.hpp>

#include "user.hpp"

namespace obby
{

// TODO: Put all this in a separate namespace?

template<typename data_type>
class command_context_from: public ::serialise::default_context_from<data_type>
{
};

/** @brief Command query that may be sent to a server.
 */
class command_query
{
public:
	/** @brief Creates a new query with a command and an optional
	 * parameter list.
	 */
	command_query(const std::string& command,
	              const std::string& paramlist);

	/** @brief Reads a command query from a network packet.
	 */
	command_query(const net6::packet& pack,
	              unsigned int& index);

	/** @brief Returns the command of this query.
	 */
	const std::string& get_command() const;

	/** @brief Returns the parameter list for this query.
	 */
	const std::string& get_paramlist() const;

	/** @brief Appends the query to the given packet.
	 */
	void append_packet(net6::packet& pack) const;
protected:
	std::string m_command;
	std::string m_paramlist;
};

/** @brief Result of command execution.
 */
class command_result
{
public:
	/** @brief Type of result.
	 */
	enum type {
		/** Command has not been found.
		 */
		NOT_FOUND,

		/** Dummy for commands without reply.
		 */
		NO_REPLY,

		/** Command has a reply.
		 */
		REPLY
	};

	/** @brief Standard constructor required by sigc::slot.
	 */
	command_result();

	/** @brief Creates a command result.
	 */
	command_result(type type, const std::string& reply = "");

	/** @brief Reads a command result from a network packet.
	 */
	command_result(const net6::packet& pack, unsigned int& index);

	/** @brief Gets the type of result.
	 */
	type get_type() const;

	/** @brief Returns the reply from the server.
	 */
	const std::string& get_reply() const;

	/** @brief Appends the result to packet.
	 */
	void append_packet(net6::packet& pack) const;
protected:
	type m_type;
	std::string m_reply;
};

/** @brief Class handling a parameter list.
 */
class command_paramlist
{
public:
	typedef std::vector<std::string>::size_type size_type;

	/** @brief Creates a parameter list from its string representation.
	 */
	command_paramlist(const std::string& list);

	/** @brief Returns the amount of parameters.
	 */
	size_type count() const;

	/** @brief Accesses the <em>index</em>th element and tries to
	 * convert it to the given type.
	 */
	template<typename data_type>
	data_type get(unsigned int index,
	              const ::serialise::context_base_from<data_type>& context =
	              command_context_from<data_type>() )
	{
		return context.from_string(m_params.at(index) );
	}

	/** @brief Returns the raw string value of the given parameter.
	 */
	const std::string& value(unsigned int index) const;
protected:
	std::vector<std::string> m_params;
};

/** @brief Command map for command execution.
 */
class command_map
{
public:
	typedef sigc::slot<command_result, const user&, const std::string&>
		slot_type;

	command_map();

	/** @brief Adds a new command to the map.
	 */
	void add_command(const std::string& name,
	                 const std::string& desc,
	                 const slot_type& func);

	/** @brief Executes a command query sent by the given user and
	 * returns an appropriate result.
	 */
	command_result exec_command(const user& from,
	                            const command_query& query) const;
protected:
	command_result on_help(const user& from, const std::string& paramlist);

	struct command
	{
		std::string name;
		std::string desc;
		slot_type func;
	};

	typedef std::map<std::string, command> map_type;

	std::auto_ptr<map_type> m_map;
};

/** Command execution queue.
 *
 * Stores commands that have been sent to be executed and waits for results.
 */
class command_queue
{
public:
	typedef sigc::signal<void, const command_query&, const command_result&>
		signal_result_type;

	typedef sigc::signal<void, const command_query&>
		signal_query_failed_type;

	typedef sigc::signal<void, const std::string&, const std::string&>
		signal_help_type;

	command_queue();

	/** @brief Waits for a result for the given query.
	 */
	void query(const command_query& query);

	/** @brief Replies a query in the queue.
	 */
	void result(const command_result& result);

	/** @brief Clears the command queue.
	 */
	void clear();

	/** @brief Signal that is emitted when a result for a previously
	 * sent command is available.
	 */
	signal_result_type result_event(const std::string& command) const;

	/** @brief Emitted when a query failed because the server did
	 * not know the command.
	 */
	signal_query_failed_type query_failed_event() const;

	/** @brief Emitted for each help entry when a help command has
	 * been queried.
	 */
	signal_help_type help_event() const;

protected:
	void on_help(const command_query& query, const command_result& result);

	typedef std::map<std::string, signal_result_type> map_type;

	std::auto_ptr<map_type> m_map;
	std::queue<command_query> m_commands;
	signal_query_failed_type m_signal_query_failed;
	signal_help_type m_signal_help;
};

} // namespace obby

#endif // _OBBY_COMMAND_HPP_
