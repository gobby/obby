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

#ifndef _OBBY_SERVER_BUFFER_HPP_
#define _OBBY_SERVER_BUFFER_HPP_

#include "serialise/error.hpp"
#include "serialise/parser.hpp"
#include "common.hpp"
#include "error.hpp"
#include "command.hpp"
#include "buffer.hpp"
#include "server_document_info.hpp"

namespace obby
{

/** Buffer that serves as (dedicated) server. It listens for incoming
 * connections from client_buffers and synchronises their changes.
 */
template<typename Document, typename Selector>
class basic_server_buffer:
	virtual public basic_buffer<Document, Selector>
{
public: 
	// Document info
	typedef typename basic_buffer<Document, Selector>::
		base_document_info_type base_document_info_type;

	typedef basic_server_document_info<Document, Selector>
		document_info_type;

	// Network
	typedef typename basic_buffer<Document, Selector>::
		base_net_type base_net_type;

	typedef net6::basic_server<Selector> net_type;

	// Signal
	typedef sigc::signal<void, const net6::user&> signal_connect_type;
	typedef sigc::signal<void, const net6::user&> signal_disconnect_type;

	/** Default constructor.
	 */
	basic_server_buffer();

	/** Opens the server on the given port.
	 */
	virtual void open(unsigned int port);

	/** Opens the server on the given port and resumes the obby session
	 * stored in the given file.
	 *
	 * Note that the session deserialisation is not atomic. So, if an
	 * error occured while loading the session, the session is partly
	 * loaded (but the server is closed) and the old session is gone.
	 *
	 * Keep an old buffer while trying to open a new one if you would like
	 * to keep old data on deserialisation error.
	 */
	virtual void open(const std::string& session, unsigned int port);

	/** Closes the obby session.
	 */
	virtual void close();

	/** Changes the global password for this session.
	 */
	void set_global_password(const std::string& password);
	
	/** Creates a new document with predefined content.
	 * signal_document_insert will be emitted and may be used to access
	 * the resulting obby::document_info.
	 */
	virtual void document_create(const std::string& title,
	                             const std::string& encoding,
	                             const std::string& content = "");

	/** Removes an existing document.
	 */
	virtual void document_remove(base_document_info_type& doc);

	/** Looks for a document with the given ID which belongs to the user
	 * with the given owner ID. Note that we do not take a real user object
	 * here because the ID is enough and one might not have a user object
	 * to the corresponding ID. So a time-consuming lookup is obsolete.
	 */
	document_info_type* document_find(unsigned int owner_id,
	                                  unsigned int id) const;

	/** Sends a global message to all users.
	 */
	virtual void send_message(const std::string& message);

	/** Sends a message to the given user.
	 */
	virtual void send_message(const std::string& message, const user& to);

	/** @brief Sets whether keepalives are sent to clients.
	 *
	 * With this option enabled, the server sends keepalive packets to
	 * client if the connection is idle to make sure that the
	 * connection has not gone away.
	 */
	void set_enable_keepalives(bool enable);

	/** @brief Provides access to the server's command map.
	 */
	command_map& get_command_map();

	/** @brief Provides access to the server's command map.
	 */
	const command_map& get_command_map() const;

	/** Signal which will be emitted if a new client has connected.
	 */
	signal_connect_type connect_event() const;

	/** Signal which will be emitted if a connected client has quit.
	 */
	signal_disconnect_type disconnect_event() const;

protected:
	/** Registers net6 signal handlers. May be used by derived classes
	 * which override the server_buffer constructor.
	 */
	void register_signal_handlers();

        /** Creates a new document info object according to the type of buffer.
	 */
	virtual base_document_info_type*
	new_document_info(const user* owner,
	                  unsigned int id,
	                  const std::string& title,
	                  const std::string& encoding,
	                  const std::string& content);

	/** Creates a new document info deserialised from a serialisation
	 * object according to the type of buffer.
	 */
	virtual base_document_info_type*
	new_document_info(const serialise::object& obj);

	/** Creates the underlaying net6 network object corresponding to the
	 * buffer's type.
	 */
	virtual base_net_type* new_net();

	/** Internal function to create a document with the given owner.
	 */
	void document_create_impl(const obby::user* owner,
	                          unsigned int id,
	                          const std::string& title,
	                          const std::string& encoding,
	                          const std::string& content);

	/** Internal function send a message to all users that comes from
	 * the user <em>writer</em>.
	 */
	void send_message_impl(const std::string& message,
	                       const obby::user* writer);

	/** Internal function send a message to a given user that comes from
	 * the user <em>writer</em>.
	 */
	void send_message_impl(const std::string& message,
	                       const obby::user* writer,
			       const obby::user& to);

	/** Changes the colour of the given user to the new provided
	 * colour and relays the fact to the other users.
	 */
	void user_colour_impl(const obby::user& user,
	                      const colour& colour);

	/** net6 signal handlers.
	 */
	void on_connect(const net6::user& user6);
	void on_disconnect(const net6::user& user6);
	void on_join(const net6::user& user6);
	void on_part(const net6::user& user6);
	bool on_auth(const net6::user& user6,
	             const net6::packet& pack,
	             net6::login::error& error);
	void on_login(const net6::user& user6,
	              const net6::packet& pack);
	void on_extend(const net6::user& user6,
	               net6::packet& pack);
	void on_data(const net6::user& from,
	             const net6::packet& pack);

	/** Executes a network packet.
	 */
	bool execute_packet(const net6::packet& pack, const user& from);

	/** Document commands.
	 */
	virtual void on_net_document_create(const net6::packet& pack,
	                                    const user& from);
	virtual void on_net_document_remove(const net6::packet& pack,
	                                    const user& from);

	/** Messaging commands.
	 */
	virtual void on_net_message(const net6::packet& pack, const user& from);

	/** User commands.
	 */
	virtual void on_net_user_password(const net6::packet& pack,
	                                  const user& from);
	virtual void on_net_user_colour(const net6::packet& pack,
	                                const user& from);

	/** Forwarding commands.
	 */
	virtual void on_net_document(const net6::packet& pack,
	                             const user& from);

	/** Command commands ;-)
	 */
	virtual void on_net_command_query(const net6::packet& pack,
	                                  const user& from);

	/** Commands.
	 */
	command_result on_command_emote(const user& from,
	                                const std::string& paramlist);

	/** @brief Closes a session.
	 */
	virtual void session_close();

	/** @brief Implementation of session_close() that does not call
	 * a base function.
	 */
	void session_close_impl();

	/** Global session password. Only people who know this password are
	 * allowed to join the session.
	 */
	std::string m_global_password;

	bool m_enable_keepalives;

	signal_connect_type m_signal_connect;
	signal_disconnect_type m_signal_disconnect;

	command_map m_command_map;
private:
	void reopen_impl(unsigned int port);

	/** This function provides access to the underlaying net6::basic_server
	 * object.
	 */
	net_type& net6_server();

	/** This fucntion provides access to the underlaying net6::basic_server
	 * object.
	 */
	const net_type& net6_server() const;
};

typedef basic_server_buffer<obby::document, net6::selector> server_buffer;

template<typename Document, typename Selector>
basic_server_buffer<Document, Selector>::
		basic_server_buffer():
	basic_buffer<Document, Selector>(),
	m_enable_keepalives(false)
{
	// Note that the command description is translated on server side.
	// We cannot just send a number or something that the client converts
	// to localised text since the client does not know the available
	// commands (and neither their descriptions).
	m_command_map.add_command(
		"me",
		_("Sends an action to the chat."),
		sigc::mem_fun(*this, &basic_server_buffer::on_command_emote)
	);
}

template<typename Document, typename Selector>
void basic_server_buffer<Document, Selector>::reopen_impl(unsigned int port)
{
	if(IPV6_ENABLED)
	{
		// Prefer IPv6 when compiled with IPv6 support,
		// fallback to IPv4
		try
		{
			net6_server().reopen(port, true);
		}
		catch(net6::error& e)
		{
			net6_server().reopen(port, false);
		}
	}
	else
	{
		net6_server().reopen(port, false);
	}
}

template<typename Document, typename Selector>
void basic_server_buffer<Document, Selector>::open(unsigned int port)
{
	if(basic_buffer<Document, Selector>::is_open() )
	{
		throw std::logic_error(
			"obby::basic_server_buffer::open:\n"
			"Server is already open"
		);
	}

	basic_buffer<Document, Selector>::m_net.reset(new_net());
	register_signal_handlers();

	reopen_impl(port);

	// Clear previous documents and users
	basic_buffer<Document, Selector>::document_clear();
	basic_buffer<Document, Selector>::m_user_table.clear();

	basic_buffer<Document, Selector>::m_signal_sync_init.emit(0);
	basic_buffer<Document, Selector>::m_signal_sync_final.emit();
}

template<typename Document, typename Selector>
void basic_server_buffer<Document, Selector>::open(const std::string& session,
                                                   unsigned int port)
{
	// TODO: Close server when session deserialisation failed
	if(basic_buffer<Document, Selector>::is_open() )
	{
		throw std::logic_error(
			"obby::basic_server_buffer::open:\n"
			"Server is already open"
		);
	}

	// Open server
	basic_buffer<Document, Selector>::m_net.reset(new_net());
	register_signal_handlers();

	reopen_impl(port);

	// Deserialise file
	serialise::parser parser;
	parser.deserialise(session);

	// Clear previous documents and users
	basic_buffer<Document, Selector>::document_clear();
	basic_buffer<Document, Selector>::m_user_table.clear();

	basic_buffer<Document, Selector>::m_signal_sync_init.emit(0);

	if(parser.get_type() != "obby")
		throw serialise::error(_("File is not an obby document"), 1);

	// Get root object, verify that it is an obby session
	serialise::object& root = parser.get_root();
	if(root.get_name() != "session")
	{
		throw serialise::error(
			_("File is not a stored obby session"),
			root.get_line()
		);
	}

	serialise::attribute& version_attr =
		root.get_required_attribute("version");

	// TODO: Check version for incompatibilites
	// TODO: Block higher version files
	// Check children
	for(serialise::object::child_iterator iter = root.children_begin();
	    iter != root.children_end();
	    ++ iter)
	{
		if(iter->get_name() == "user_table")
		{
			// Stored user table
			basic_buffer<Document, Selector>::
				m_user_table.deserialise(
					*iter
				);
		}
		else if(iter->get_name() == "chat")
		{
			// Stored chat history
			basic_buffer<Document, Selector>::m_chat.deserialise(
				*iter,
				basic_buffer<Document, Selector>::m_user_table
			);
		}
		else if(iter->get_name() == "document")
		{
			// Stored document, load it
			base_document_info_type* info =
				new_document_info(*iter);
			// Add to list
			basic_buffer<Document, Selector>::document_add(*info);
		}
		else
		{
			// Unexpected child
			// TODO: unexpected_child_error
			format_string str(_("Unexpected child node: '%0%'") );
			str << iter->get_name();
			throw serialise::error(str.str(), iter->get_line() );
		}
	}

	basic_buffer<Document, Selector>::m_signal_sync_final.emit();
}

template<typename Document, typename Selector>
void basic_server_buffer<Document, Selector>::close()
{
	if(!basic_buffer<Document, Selector>::is_open() )
	{
		throw std::logic_error(
			"obby::basic_server_buffer::close:\n"
			"Server is not open"
		);
	}

	// Virtual call - host buffer overwrites this to keep local
	// user subscribed
	session_close();
}

template<typename Document, typename Selector>
void basic_server_buffer<Document, Selector>::
	set_global_password(const std::string& password)
{
	m_global_password = password;
}

template<typename Document, typename Selector>
void basic_server_buffer<Document, Selector>::
	document_create(const std::string& title,
	                const std::string& encoding,
	                const std::string& content)
{
	// TODO: m_doc_counter should not be declared in basic_buffer
	// but client_buffer / server_buffer separately
	unsigned int id = ++ basic_buffer<Document, Selector>::m_doc_counter;

	// Create the document with the special owner NULL which means that
	// this document was created by the server.
	document_create_impl(NULL, id, title, encoding, content);
}

template<typename Document, typename Selector>
void basic_server_buffer<Document, Selector>::
	document_remove(base_document_info_type& info)
{
	if(basic_buffer<Document, Selector>::is_open() )
	{
		// Emit unsubscribe signal for all users that were
		// subscribed to this document
		// TODO: Do this in document_delete
		for(typename document_info_type::user_iterator user_iter =
			info.user_begin();
		    user_iter != info.user_end();
		    ++ user_iter)
		{
			info.unsubscribe_event().emit(*user_iter);
		}

		// Tell other clients about removal
		net6::packet remove_pack("obby_document_remove");
		remove_pack << &info;
		net6_server().send(remove_pack);
	}

	// Delete document
	basic_buffer<Document, Selector>::document_delete(info);
}

template<typename Document, typename Selector>
typename basic_server_buffer<Document, Selector>::document_info_type*
basic_server_buffer<Document, Selector>::
	document_find(unsigned int owner_id,
	              unsigned int id) const
{
	return dynamic_cast<document_info_type*>(
		basic_buffer<Document, Selector>::document_find(owner_id, id)
	);
}

template<typename Document, typename Selector>
void basic_server_buffer<Document, Selector>::
	send_message(const std::string& message)
{
	// send_message_impl relays the message to the clients
	send_message_impl(message, NULL);
}

template<typename Document, typename Selector>
void basic_server_buffer<Document, Selector>::
	send_message(const std::string& message, const user& to)
{
	// send_message_impl relays the message to the clients
	send_message_impl(message, NULL, to);
}

template<typename Document, typename Selector>
void basic_server_buffer<Document, Selector>::
	set_enable_keepalives(bool enable)
{
	if(m_enable_keepalives == enable) return;

	m_enable_keepalives = enable;
	user_table& table = this->m_user_table;

	for(user_table::iterator iter =
		table.begin(user::flags::CONNECTED, user::flags::NONE);
	    iter != table.end(user::flags::CONNECTED, user::flags::NONE);
	    ++ iter)
	{
		// Dirty hack for host to work (local user...). We need
		// user::flags::DIRECT_CONNECTION and user::flags::LOCAL!
		try
		{
			iter->get_net6().set_enable_keepalives(enable);
		} catch(...) {}
	}
}

template<typename Document, typename Selector>
command_map& basic_server_buffer<Document, Selector>::get_command_map()
{
	return m_command_map;
}

template<typename Document, typename Selector>
const command_map& basic_server_buffer<Document, Selector>::
	get_command_map() const
{
	return m_command_map;
}

template<typename Document, typename Selector>
typename basic_server_buffer<Document, Selector>::signal_connect_type
basic_server_buffer<Document, Selector>::connect_event() const
{
	return m_signal_connect;
}

template<typename Document, typename Selector>
typename basic_server_buffer<Document, Selector>::signal_disconnect_type
basic_server_buffer<Document, Selector>::disconnect_event() const
{
	return m_signal_disconnect;
}

template<typename Document, typename Selector>
void basic_server_buffer<Document, Selector>::
	document_create_impl(const user* owner,
	                     unsigned int id,
	                     const std::string& title,
	                     const std::string& encoding,
                             const std::string& content)
{
	if(!basic_buffer<Document, Selector>::is_open() )
	{
		throw std::logic_error(
			"obby::basic_server_buffer::document_create_impl:\n"
			"Document creation while server is closed"
		);
	}

	// No need to ignore since the document is not yet in
	// the document list
	unsigned int suffix =
		basic_buffer<Document, Selector>::find_free_suffix(title, NULL);

	// Create new document
	base_document_info_type* info =
		new_document_info(owner, id, title, encoding, content);

	// Add to buffer
	basic_buffer<Document, Selector>::document_add(*info);

	// Publish the new document to the users. Note that we do not have
	// to send the document's initial content because no user is currently
	// subscribed, and the content is synced with the subscription.
	net6::packet pack("obby_document_create");
	pack << owner << id << title << suffix << encoding;

	// TODO: send_to_all_except function or something
	// or, better: user_table.send() with given flags.
	for(user_table::iterator iter = basic_buffer<Document, Selector>::
		m_user_table.begin(user::flags::CONNECTED, user::flags::NONE);
	    iter != basic_buffer<Document, Selector>::
		m_user_table.end(user::flags::CONNECTED, user::flags::NONE);
	    ++ iter)
	{
		// The owner already knows about the document.
		if(&(*iter) != owner)
			net6_server().send(pack, iter->get_net6() );
	}
}

template<typename Document, typename Selector>
void basic_server_buffer<Document, Selector>::
	send_message_impl(const std::string& message, const user* writer, const user& to)
{
	if(basic_buffer<Document, Selector>::is_open())
	{
		net6::packet message_pack("obby_message");
		message_pack << writer << message;
		net6_server().send(message_pack, to.get_net6());
	}

	if(writer == NULL)
	{
		basic_buffer<Document, Selector>::m_chat.add_server_message(
			message
		);
	}
	else
	{
		basic_buffer<Document, Selector>::m_chat.add_user_message(
			message,
			*writer
		);
	}
}

template<typename Document, typename Selector>
void basic_server_buffer<Document, Selector>::
	send_message_impl(const std::string& message, const user* writer)
{
	if(basic_buffer<Document, Selector>::is_open() )
	{
		net6::packet message_pack("obby_message");
		message_pack << writer << message;
		net6_server().send(message_pack);
	}

	if(writer == NULL)
	{
		basic_buffer<Document, Selector>::m_chat.add_server_message(
			message
		);
	}
	else
	{
		basic_buffer<Document, Selector>::m_chat.add_user_message(
			message,
			*writer
		);
	}
}

template<typename Document, typename Selector>
void basic_server_buffer<Document, Selector>::
	user_colour_impl(const obby::user& user,
	                 const colour& colour)
{
	// TODO: user_colour_impl should check for color conflicts - should it?
	basic_buffer<Document, Selector>::
		m_user_table.set_user_colour(user, colour);
	// TODO: user::set_colour should emit this signal
	basic_buffer<Document, Selector>::m_signal_user_colour.emit(user);

	if(basic_buffer<Document, Selector>::is_open() )
	{
		net6::packet colour_pack("obby_user_colour");
		colour_pack << &user << colour;
		net6_server().send(colour_pack);
	}
}

template<typename Document, typename Selector>
void basic_server_buffer<Document, Selector>::
	on_connect(const net6::user& user6)
{
	// Send our protocol version.
	net6::packet welcome_pack("obby_welcome");
	welcome_pack << PROTOCOL_VERSION;

	net6_server().send(welcome_pack, user6);

	// Request encryption after welcome packet.
	net6_server().request_encryption(user6);

	user6.set_enable_keepalives(m_enable_keepalives);

	// User connected
	m_signal_connect.emit(user6);
}

template<typename Document, typename Selector>
void basic_server_buffer<Document, Selector>::
	on_disconnect(const net6::user& user6)
{
	m_signal_disconnect.emit(user6);
}

template<typename Document, typename Selector>
void basic_server_buffer<Document, Selector>::on_join(const net6::user& user6)
{
	// Find user in list
	const user* new_user =
		basic_buffer<Document, Selector>::m_user_table.find(
			user6,
			user::flags::CONNECTED,
			user::flags::NONE
		);

	// Should not happen
	if(new_user == NULL)
	{
		format_string str("User %0% is not connected");
		str << user6.get_id();
		throw net6::bad_value(str.str() );
	}

	// Synchronise non-connected users.
	for(user_table::iterator iter = basic_buffer<Document, Selector>::
		m_user_table.begin(user::flags::NONE, user::flags::CONNECTED);
	    iter != basic_buffer<Document, Selector>::
		m_user_table.end(user::flags::NONE, user::flags::CONNECTED);
	    ++ iter)
	{
		net6::packet user_pack("obby_sync_usertable_user");
		user_pack << iter->get_id() << iter->get_name()
		          << iter->get_colour();
		net6_server().send(user_pack, user6);
	}

	// Synchronise the document list
	for(typename basic_buffer<Document, Selector>::document_iterator iter =
		basic_buffer<Document, Selector>::document_begin();
	   iter != basic_buffer<Document, Selector>::document_end();
	   ++ iter)
	{
		// Setup document packet
		net6::packet document_pack("obby_sync_doclist_document");

		// TODO: Creation of this packet should be a method of
		// server_document_info
		document_pack << iter->get_owner() << iter->get_id()
		              << iter->get_title() << iter->get_suffix()
		              << iter->get_encoding();

		// Add users that are subscribed
		for(typename document_info_type::user_iterator user_iter =
			iter->user_begin();
		    user_iter != iter->user_end();
		    ++ user_iter)
		{
			document_pack << &(*user_iter);
		}

		net6_server().send(document_pack, user6);
	}

	// Done with synchronising
	net6::packet final_pack("obby_sync_final");
	net6_server().send(final_pack, user6);

	// Forward join message to documents
	// TODO: Let the documents connect to signal_user_join
	for(typename basic_buffer<Document, Selector>::document_iterator iter =
		basic_buffer<Document, Selector>::document_begin();
	   iter != basic_buffer<Document, Selector>::document_end();
	   ++ iter)
	{
		iter->obby_user_join(*new_user);
	}

	// User joined successfully; emit user_join signal
	// TODO: Move to user_table::add_user
	basic_buffer<Document, Selector>::m_signal_user_join.emit(*new_user);
}

template<typename Document, typename Selector>
void basic_server_buffer<Document, Selector>::on_part(const net6::user& user6)
{
	// Find obby::user object for given net6::user
	const user* cur_user =
		basic_buffer<Document, Selector>::m_user_table.find(
			user6,
			user::flags::CONNECTED,
			user::flags::NONE
		);

	// Should not happen
	if(cur_user == NULL)
	{
		format_string str("User %0% is not connected");
		str << user6.get_id();
		throw net6::bad_value(str.str() );
	}

	// Forward part message to documents
	// TODO: Let the documents connect to signal_user_part
	for(typename basic_buffer<Document, Selector>::document_iterator iter =
		basic_buffer<Document, Selector>::document_begin();
	   iter != basic_buffer<Document, Selector>::document_end();
	   ++ iter)
	{
		iter->obby_user_part(*cur_user);
	}

	// Emit part signal, remove user from list
	// TODO: Move part signal emission to remove_user
	basic_buffer<Document, Selector>::m_signal_user_part.emit(*cur_user);
	basic_buffer<Document, Selector>::m_user_table.remove_user(*cur_user);
}

template<typename Document, typename Selector>
bool basic_server_buffer<Document, Selector>::
	on_auth(const net6::user& user6,
	        const net6::packet& pack,
	        net6::login::error& error)
{
	// Do not allow joins from clients whose connection is not encrypted
	if(!user6.is_encrypted() )
	{
		error = login::ERROR_NOT_ENCRYPTED;
		return false;
	}

	const std::string name =
		pack.get_param(0).net6::parameter::as<std::string>();
	colour colour =
		pack.get_param(1).net6::parameter::as<obby::colour>();

	// Get global and user password, if given
	std::string global_password, user_password;
	if(pack.get_param_count() > 2)
		global_password =
			pack.get_param(2).net6::parameter::as<std::string>();

	if(pack.get_param_count() > 3)
		user_password =
			pack.get_param(3).net6::parameter::as<std::string>();

	// Check colour
	if(!basic_buffer<Document, Selector>::check_colour(colour) )
	{
		error = login::ERROR_COLOUR_IN_USE;
		return false;
	}

	// Check global password
	if(!m_global_password.empty() )
	{
		if(global_password != m_global_password)
		{
			error = login::ERROR_WRONG_GLOBAL_PASSWORD;
			return false;
		}
	}

	// Search user in user table
	const obby::user* user =
		basic_buffer<Document, Selector>::m_user_table.find(
			name,
			user::flags::NONE,
			user::flags::CONNECTED
		);

	// Compare user password
	if(user && !user->get_password().empty() )
	{
		if(user_password != user->get_password() )
		{
			error = login::ERROR_WRONG_USER_PASSWORD;
			return false;
		}
	}

	// Auth OK
	return true;
}

template<typename Document, typename Selector>
void basic_server_buffer<Document, Selector>::
	on_login(const net6::user& user6,
	         const net6::packet& pack)
{
	// Get color
	colour colour =
		pack.get_param(1).net6::parameter::as<obby::colour>();

	// Choose free user ID (note that this is another ID as the net6
	// user ID because this ID must remain valid over multiple sessions).
	unsigned int user_id =
		basic_buffer<Document, Selector>::m_user_table.find_free_id();

	// Insert into user list
	const user* new_user =
		basic_buffer<Document, Selector>::m_user_table.add_user(
			user_id, user6, colour
		);

	// Send initial sync packet; this is here in on_login() for it to
	// happen before net6 syncs its users.

	// Calculate number of required sync packets (one for each
	// non-connected user, one for each document in the list).
	unsigned int sync_n = basic_buffer<Document, Selector>::
		m_user_table.count(user::flags::NONE, user::flags::NONE);
	sync_n += basic_buffer<Document, Selector>::document_count();

	// Send initial sync packet with this number, the client may then show
	// a progressbar or something.
	net6::packet init_pack("obby_sync_init");
	init_pack << sync_n;
	net6_server().send(init_pack, user6);
}

template<typename Document, typename Selector>
void basic_server_buffer<Document, Selector>::on_extend(const net6::user& user6,
                                                        net6::packet& pack)
{
	// Find corresponding user in list
	const user* cur_user =
		basic_buffer<Document, Selector>::m_user_table.find(
			user6,
			user::flags::CONNECTED,
			user::flags::NONE
		);

	// Should not happen
	if(cur_user == NULL)
	{
		format_string str("User %0% is not connected");
		str << user6.get_id();
		throw net6::bad_value(str.str() );
	}

	// Extend user-join packet with colour and obby-ID
	pack << cur_user->get_id() << cur_user->get_colour();
}

template<typename Document, typename Selector>
void basic_server_buffer<Document, Selector>::on_data(const net6::user& user6,
                                                      const net6::packet& pack)
{
	// Get obby::user from net6::user
	const user* from_user =
		basic_buffer<Document, Selector>::m_user_table.find(
			user6,
			user::flags::CONNECTED,
			user::flags::NONE
		);

	// Should not happen
	if(from_user == NULL)
	{
		format_string str("User %0% is not connected");
		str << user6.get_id();
		throw net6::bad_value(str.str() );
	}

	// Execute packet
	if(!execute_packet(pack, *from_user) )
	{
		throw net6::bad_value(
			"Unexpected command: " + pack.get_command()
		);
	}
}

template<typename Document, typename Selector>
bool basic_server_buffer<Document, Selector>::
	execute_packet(const net6::packet& pack, const user& from)
{
	try
	{
		// TODO: std::map<> mapping command to function
		if(pack.get_command() == "obby_document_create")
			{ on_net_document_create(pack, from); return true; }

		if(pack.get_command() == "obby_document_remove")
			{ on_net_document_remove(pack, from); return true; }

		if(pack.get_command() == "obby_message")
			{ on_net_message(pack, from); return true; }

		if(pack.get_command() == "obby_user_password")
			{ on_net_user_password(pack, from); return true; }

		if(pack.get_command() == "obby_user_colour")
			{ on_net_user_colour(pack, from); return true; }

		if(pack.get_command() == "obby_document")
			{ on_net_document(pack, from); return true; }

		if(pack.get_command() == "obby_command_query")
			{ on_net_command_query(pack, from); return true; }

		return false;
	}
	catch(std::logic_error& e)
	{
		// Print a meaningful error message to stderr why
		// the offending user was dropped.
		std::cerr << "obby logic error caught in connection to "
		          << from.get_name() << ": " << e.what() << std::endl;

		// Packet caused a logic error. Not good. Drop the client.
		throw net6::bad_packet(e.what());
	}
}

template<typename Document, typename Selector>
void basic_server_buffer<Document, Selector>::
	on_net_document_create(const net6::packet& pack,
	                       const user& from)
{
	unsigned int id =
		pack.get_param(0).net6::parameter::as<unsigned int>();
	const std::string title =
		pack.get_param(1).net6::parameter::as<std::string>();
	const std::string encoding =
		pack.get_param(2).net6::parameter::as<std::string>();
	const std::string content =
		pack.get_param(3).net6::parameter::as<std::string>();

	document_create_impl(&from, id, title, encoding, content);
}

template<typename Document, typename Selector>
void basic_server_buffer<Document, Selector>::
	on_net_document_remove(const net6::packet& pack,
	                       const user& from)
{
	// Get document to remove
	document_info_type& doc = dynamic_cast<document_info_type&>(
		*pack.get_param(0).net6::parameter::as<
			base_document_info_type*
		>(::serialise::hex_context_from<base_document_info_type*>(
			*this
		))
	);

	// TODO: AUTH

	// Remove it
	document_remove(doc);
}

template<typename Document, typename Selector>
void basic_server_buffer<Document, Selector>::
	on_net_message(const net6::packet& pack,
	               const user& from)
{
	const std::string message =
		pack.get_param(0).net6::parameter::as<std::string>();

	send_message_impl(message, &from);
}

template<typename Document, typename Selector>
void basic_server_buffer<Document, Selector>::
	on_net_user_password(const net6::packet& pack,
	                     const user& from)
{
	// Set password for this user
	basic_buffer<Document, Selector>::m_user_table.set_user_password(
		from,
		pack.get_param(0).net6::parameter::as<std::string>()
	);
}

template<typename Document, typename Selector>
void basic_server_buffer<Document, Selector>::
	on_net_user_colour(const net6::packet& pack,
	                   const user& from)
{
	colour colour = pack.get_param(0).net6::parameter::as<obby::colour>();

	// Check new user colour for conflicts
	if(!basic_buffer<Document, Selector>::check_colour(colour, &from) )
	{
		net6::packet reply_pack("obby_user_colour_failed");
		net6_server().send(reply_pack, from.get_net6() );
	}
	else
	{
		user_colour_impl(from, colour);
	}
}

template<typename Document, typename Selector>
void basic_server_buffer<Document, Selector>::
	on_net_document(const net6::packet& pack,
	                const user& from)
{
	document_info_type& info = dynamic_cast<document_info_type&>(
		*pack.get_param(0).net6::parameter::as<
			base_document_info_type*
		>(::serialise::hex_context_from<base_document_info_type*>(
			*this
		))
	);

	// TODO: Rename this function. Think about providing a signal that may
	// be emitted.
	info.on_net_packet(document_packet(pack), from);
}

template<typename Document, typename Selector>
void basic_server_buffer<Document, Selector>::
	on_net_command_query(const net6::packet& pack,
	                     const user& from)
{
	unsigned int index = 0;
	command_query query(pack, index);

	command_result result = m_command_map.exec_command(from, query);

	net6::packet reply_pack("obby_command_result");
	result.append_packet(reply_pack);

	net6_server().send(reply_pack, from.get_net6() );
}

template<typename Document, typename Selector>
command_result basic_server_buffer<Document, Selector>::
	on_command_emote(const user& from,
	                 const std::string& paramlist)
{
	user_table& table = this->m_user_table;

	net6::packet pack("obby_emote_message");
	pack << &from << paramlist;

	for(user_table::iterator iter =
		table.begin(user::flags::CONNECTED, user::flags::NONE);
	    iter != table.end(user::flags::CONNECTED, user::flags::NONE);
	    ++ iter)
	{
		// command_result will be sent to sender automatically, so
		// there is no need to send emote message packet extra
		if(&(*iter) == &from) continue;

		net6_server().send(pack, iter->get_net6() );
	}

	basic_buffer<Document, Selector>::m_chat.add_emote_message(
		paramlist,
		from
	);

	return command_result(command_result::NO_REPLY);
}

template<typename Document, typename Selector>
void basic_server_buffer<Document, Selector>::session_close()
{
	session_close_impl();
	basic_buffer<Document, Selector>::session_close_impl();
}

template<typename Document, typename Selector>
void basic_server_buffer<Document, Selector>::session_close_impl()
{
	// Session is closed, so all users have quit
	user_table& table = this->m_user_table;

	for(user_table::iterator iter =
		table.begin(user::flags::CONNECTED, user::flags::NONE);
	    iter != table.end(user::flags::CONNECTED, user::flags::NONE);
	    ++ iter)
	{
		// This call also unsubscribes the user from all documents
		basic_buffer<Document, Selector>::user_part(*iter);
	}
}

template<typename Document, typename Selector>
void basic_server_buffer<Document, Selector>::register_signal_handlers()
{
	net6_server().connect_event().connect(
		sigc::mem_fun(*this, &basic_server_buffer::on_connect) );
	net6_server().disconnect_event().connect(
		sigc::mem_fun(*this, &basic_server_buffer::on_disconnect) );
	net6_server().join_event().connect(
		sigc::mem_fun(*this, &basic_server_buffer::on_join) );
	net6_server().part_event().connect(
		sigc::mem_fun(*this, &basic_server_buffer::on_part) );
	net6_server().login_auth_event().connect(
		sigc::mem_fun(*this, &basic_server_buffer::on_auth) );
	net6_server().login_event().connect(
		sigc::mem_fun(*this, &basic_server_buffer::on_login) );
	net6_server().login_extend_event().connect(
		sigc::mem_fun(*this, &basic_server_buffer::on_extend) );
	net6_server().data_event().connect(
		sigc::mem_fun(*this, &basic_server_buffer::on_data) );
}

template<typename Document, typename Selector>
typename basic_server_buffer<Document, Selector>::base_document_info_type*
basic_server_buffer<Document, Selector>::
	new_document_info(const user* owner,
	                  unsigned int id,
                          const std::string& title,
	                  const std::string& encoding,
	                  const std::string& content)
{
	// Create server_document_info, according to server_buffer
	return new document_info_type(
		*this, net6_server(), owner, id, title, encoding, content
	);
}

template<typename Document, typename Selector>
typename basic_server_buffer<Document, Selector>::base_document_info_type*
basic_server_buffer<Document, Selector>::
	new_document_info(const serialise::object& obj)
{
	// Create server_document_info, according to server_buffer
	return new document_info_type(*this, net6_server(), obj);
}

template<typename Document, typename Selector>
typename basic_server_buffer<Document, Selector>::base_net_type*
basic_server_buffer<Document, Selector>::new_net()
{
	return new net_type();
}

template<typename Document, typename Selector>
typename basic_server_buffer<Document, Selector>::net_type&
basic_server_buffer<Document, Selector>::net6_server()
{
	return dynamic_cast<net_type&>(
		*basic_buffer<Document, Selector>::m_net
	);
}

template<typename Document, typename Selector>
const typename basic_server_buffer<Document, Selector>::net_type&
basic_server_buffer<Document, Selector>::net6_server() const
{
	return dynamic_cast<const net_type&>(
		*basic_buffer<Document, Selector>::m_net
	);
}

} // namespace obby

#endif // _OBBY_SERVER_BUFFER_HPP_
