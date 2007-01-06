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

#include "command.hpp"

namespace
{
	void unescape(std::string& result)
	{
		std::string::size_type pos = 0;
		while((pos = result.find('\\', pos)) != std::string::npos)
		{
			// get_text_param ensures that there is no backslash
			// at the end of the string
			switch(result[pos + 1])
			{
			case 'n':
				result.replace(pos, 2, 1, '\n');
				break;
			case '\'':
			case '\"':
			case '\\':
				result.erase(pos, 1);
				break;
			default:
				throw std::logic_error(
					"obby::command.cpp::unescape:\n"
					"Encountered invalid escape sequence"
				);
			}

			++ pos;
		}
	}

	std::string::size_type get_next_param(const std::string& list,
	                                      std::string::size_type pos,
	                                      std::string& result)
	{
		std::string::size_type i = pos;

		while(i < list.length() && isspace(list[i]) )
			++ i;

		if(i == list.length() )
			return std::string::npos;

		char str_char = '\0';
		if(list[i] == '\"' || list[i] == '\'')
		{
			str_char = list[i];
			++ i;
		}

		pos = i;
		bool escape_flag = false;

		for(; i < list.length(); ++ i)
		{
			if(escape_flag)
			{
				escape_flag = false;
			}
			else
			{
				if(list[i] == '\\')
					escape_flag = true;

				if(str_char == '\0' && isspace(list[i]) )
					break;

				if(str_char != '\0' && list[i] == str_char)
					break;
			}
		}

		if(escape_flag)
		{
			throw std::logic_error(
				"obby::command.cpp::get_next_param:\n"
				"Escaping backslash at end of line"
			);
		}

		if(i == list.length() && str_char != '\0')
		{
			throw std::logic_error(
				"obby::command.cpp::get_next_param:\n"
				"String not closed"
			);
		}

		if(str_char != '\0')
		{
			result = list.substr(pos, i - pos);
			pos = i + 1;
		}
		else
		{
			result = list.substr(pos, i - pos - 1);
			pos = i;
		}

		unescape(result);
		return pos;
	}
}

obby::command_query::command_query(const std::string& command,
                                   const std::string& paramlist):
	m_command(command), m_paramlist(paramlist)
{
}

obby::command_query::command_query(const net6::packet& pack,
                                   unsigned int& index):
	m_command(pack.get_param(index).as<std::string>() ),
	m_paramlist(pack.get_param(index + 1).as<std::string>() )
{
	index += 2;
}

const std::string& obby::command_query::get_command() const
{
	return m_command;
}

const std::string& obby::command_query::get_paramlist() const
{
	return m_paramlist;
}

void obby::command_query::append_packet(net6::packet& pack) const
{
	pack << m_command << m_paramlist;
}

obby::command_result::command_result():
	m_type(NO_REPLY)
{
}

obby::command_result::command_result(type type, const std::string& reply):
	m_type(type), m_reply(reply)
{
	if(type != REPLY && !reply.empty() )
	{
		throw std::logic_error(
			"obby::command_result::command_result:\n"
			"Result type is not reply, but reply string "
			"is nonempty"
		);
	}
}

// Reply is only packed when type == REPLY
obby::command_result::command_result(const net6::packet& pack,
                                     unsigned int& index):
	m_type(static_cast<type>(pack.get_param(index).as<unsigned int>()) ),
	m_reply(
		(m_type != REPLY) ?
		("") :
		pack.get_param(index + 1).as<std::string>()
	)
{
	++ index;
	if(m_type == REPLY) ++ index;
}

obby::command_result::type obby::command_result::get_type() const
{
	return m_type;
}

const std::string& obby::command_result::get_reply() const
{
	return m_reply;
}

void obby::command_result::append_packet(net6::packet& pack) const
{
	pack << static_cast<unsigned int>(m_type);
	if(m_type == REPLY) pack << m_reply;
}

obby::command_paramlist::command_paramlist(const std::string& list)
{
	std::string param;
	std::string::size_type pos = 0;
	while((pos = get_next_param(list, pos, param)) != std::string::npos)
		m_params.push_back(param);
}

const std::string& obby::command_paramlist::value(unsigned int index) const
{
	return m_params.at(index);
}

obby::command_map::command_map()
{
	add_command(
		"help",
		_("Shows all available commands"),
		sigc::mem_fun(*this, &command_map::on_help)
	);
}

void obby::command_map::add_command(const std::string& name,
                                    const std::string& desc,
                                    const slot_type& func)
{
	if(m_map.get() == NULL) m_map.reset(new map_type);

	map_type::const_iterator iter = m_map->find(name);
	if(iter != m_map->end() )
	{
		throw std::logic_error(
			"obby::command_map::add_command:\n"
			"Command exists already"
		);
	}

	command comm = { name, desc, func };
	(*m_map)[name] = comm;
}

obby::command_result obby::command_map::
	exec_command(const user& from,
                     const command_query& query) const
{
	if(m_map.get() == NULL) return command_result::NOT_FOUND;
	map_type::const_iterator iter = m_map->find(query.get_command() );
	if(iter == m_map->end() ) return command_result::NOT_FOUND;
	return iter->second.func(from, query.get_paramlist());
}

obby::command_result obby::command_map::on_help(const user& from,
                                                const std::string& paramlist)
{
	std::string reply;

	for(map_type::const_iterator iter = m_map->begin();
	    iter != m_map->end();
	    ++ iter)
	{
		reply += iter->second.name;
		reply += ' ';
		reply += iter->second.desc;
		reply += '\n';
	}

	return command_result(command_result::REPLY, reply);
}

obby::command_queue::command_queue():
	m_map(new map_type)
{
	result_event("help").connect(
		sigc::mem_fun(*this, &command_queue::on_help)
	);
}

void obby::command_queue::query(const command_query& query)
{
	m_commands.push(query);
}

void obby::command_queue::result(const command_result& result)
{
	if(m_commands.empty() )
	{
		throw std::logic_error(
			"obby::command_queue::reply:\n"
			"No query in command queue"
		);
	}

	command_query query = m_commands.front();
	m_commands.pop();

	if(result.get_type() == command_result::NOT_FOUND)
		m_signal_query_failed.emit(query);
	else
		(*m_map)[query.get_command()].emit(query, result);
}

void obby::command_queue::clear()
{
	while(!m_commands.empty() )
		m_commands.pop();
}

obby::command_queue::signal_result_type obby::command_queue::
	result_event(const std::string& command) const
{
	return (*m_map)[command];
}

obby::command_queue::signal_query_failed_type obby::command_queue::
	query_failed_event() const
{
	return m_signal_query_failed;
}

obby::command_queue::signal_help_type obby::command_queue::help_event() const
{
	return m_signal_help;
}

void obby::command_queue::on_help(const command_query& query,
                                  const command_result& result)
{
	const std::string& reply = result.get_reply();
	std::string::size_type pos = 0, prev = 0;

	while( (pos = reply.find('\n', pos)) != std::string::npos)
	{
		std::string line = reply.substr(prev, pos - prev);
		std::string::size_type sep = line.find(' ');
		if(sep == std::string::npos) continue;

		m_signal_help.emit(line.substr(0, sep), line.substr(sep + 1));
		prev = ++ pos;
	}
}
