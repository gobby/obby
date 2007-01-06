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

#ifndef _OBBY_CLIENT_DOCUMENT_INFO_HPP_
#define _OBBY_CLIENT_DOCUMENT_INFO_HPP_

#include <net6/client.hpp>
#include "format_string.hpp"
#include "no_operation.hpp"
#include "split_operation.hpp"
#include "insert_operation.hpp"
#include "delete_operation.hpp"
#include "record.hpp"
#include "jupiter_client.hpp"
#include "local_document_info.hpp"

namespace obby
{

template<typename Document, typename Selector>
class basic_client_buffer;

/** Information about a document that is provided without being subscribed to
 * a document.
 */
template<typename Document, typename Selector>
class basic_client_document_info:
	virtual public basic_local_document_info<Document, Selector>
{
public:
	typedef typename basic_local_document_info<Document, Selector>::
		document_type document_type;

	typedef basic_client_buffer<Document, Selector> buffer_type;
	typedef typename buffer_type::net_type net_type;
	typedef jupiter_client<Document> jupiter_type;
	typedef typename jupiter_type::record_type record_type;

	/** Constructor which does not automatically create an underlaying
	 * document.
	 */
	basic_client_document_info(const buffer_type& buffer,
	                           net_type& net,
	                           const user* owner,
	                           unsigned int id,
	                           const std::string& title);

	/** Constructor which allows to give initial content and as such creates
	 * an underlaying document assuming the local client just created the
	 * document.
	 */
	basic_client_document_info(const buffer_type& buffer,
	                           net_type& net,
	                           const user* owner,
	                           unsigned int id,
	                           const std::string& title,
	                           const std::string& content);

	/** Constructor which reads the document_info from a network packet
	 * that is received when the document list is initially synchronised.
	 */
	basic_client_document_info(const buffer_type& buffer,
	                           net_type& net,
	                           const net6::packet& init_pack);

	/** Inserts the given text at the given position into the document.
	 */
	virtual void insert(position pos, const std::string& text);

	/** Erases the given range from the document.
	 */
	virtual void erase(position pos, position len);

	/** Sends a rename request for the document.
	 */
	virtual void rename(const std::string& new_title);

	/** Sends a subscribe request for the local user. If the subscribe
	 * request succeeded, the subscribe_event will be emitted.
	 */
	virtual void subscribe();

	/** Unsubscribes the local user from this document. signal_unsubscribe
	 * will be emitted if the request has been accepted.
	 */
	virtual void unsubscribe();

	/** Called by the buffer if a network event occured that belongs to the
	 * document.
	 */
	virtual void on_net_packet(const document_packet& pack);

#if 0
	/** Called by the client buffer when user synchronisation begins.
	 * This clears all the currently subscribed users.
	 * TODO: Make another function here. Maybe something like
	 * on_net_sync(const net6::packet& pack)
	 */
	virtual void obby_sync_init();

	/** Adds a user to the list of subscribed users. This function is
	 * called by the buffer while synchronising the document list.
	 * TODO: Merge with TODO item above
	 */
	virtual void obby_sync_subscribe(const user& user);

	/** Called by the buffer if the local user created a document. The
	 * document content is assigned immediately in this case because
	 * the clients does not wait for server acknowledgement to show the
	 * new document without delay.
	 * TODO: Make something other with this...
	 */
	virtual void obby_local_init(const std::string& initial_content);
#endif

protected:
	/** Subscribes a user to this document.
	 */
	virtual void user_subscribe(const user& user);

	/** Unsubscribes a user from this document.
	 */
	virtual void user_unsubscribe(const user& user);

	/** Executes a packet.
	 */
	bool execute_packet(const document_packet& pack);

	/** Rename command.
	 */
	virtual void on_net_rename(const document_packet& pack);

	/** Record command: Change in the document.
	 */
	virtual void on_net_record(const document_packet& pack);

	/** Synchronisation initialisation command.
	 */
	virtual void on_net_sync_init(const document_packet& pack);

	/** Synchronisation of a line of the document.
	 */
	virtual void on_net_sync_line(const document_packet& pack);

	/** User subscription command.
	 */
	virtual void on_net_subscribe(const document_packet& pack);

	/** User unsubscription.
	 */
	virtual void on_net_unsubscribe(const document_packet& pack);

	/** Callback from jupiter implementation with record of local operation
	 * that has to be sent to the server.
	 */
	virtual void on_jupiter_record(const record_type& rec,
	                               const user* from);

	std::auto_ptr<jupiter_type> m_jupiter;

public:
	/** Returns the buffer to which this document_info belongs.
	 */
	const buffer_type& get_buffer() const;

protected:
	/** Returns the underlaying net6 obejct.
	 */
	net_type& get_net6();

	/** Returns the underlaying net6 obejct.
	 */
	const net_type& get_net6() const;
};

template<typename Document, typename Selector>
basic_client_document_info<Document, Selector>::
	basic_client_document_info(const buffer_type& buffer,
                                   net_type& net,
	                           const user* owner,
	                           unsigned int id,
	                           const std::string& title):
	basic_document_info<Document, Selector>(
		buffer,
		net,
		owner,
		id,
		title
	),
	basic_local_document_info<Document, Selector>(
		buffer,
		net,
		owner,
		id,
		title
	)
{
	// If we created this document, the constructor with initial content
	// should be called.
	if(owner == &buffer.get_self() )
	{
		throw std::logic_error(
			"obby::basic_client_document_info::"
			"basic_client_document_info:\n"
			"Owner of document info without initial content is self"
		);
	}

	// Implictly subscribe owner
	if(owner != NULL)
		user_subscribe(*owner);
}

template<typename Document, typename Selector>
basic_client_document_info<Document, Selector>::
	basic_client_document_info(const buffer_type& buffer,
	                           net_type& net,
	                           const user* owner,
	                           unsigned int id,
	                           const std::string& title,
	                           const std::string& content):
	basic_document_info<Document, Selector>(
		buffer,
		net,
		owner,
		id,
		title
	),
	basic_local_document_info<Document, Selector>(
		buffer,
		net,
		owner,
		id,
		title
	)
{
	// content is provided, so we should have created this document
	if(owner != &buffer.get_self() )
	{
		throw std::logic_error(
			"obby::basic_client_document_info::"
			"basic_client_document_info:\n"
			"Owner of document info with initial content is "
			"not self"
		);
	}

	// Assign document, initialise content
	basic_document_info<Document, Selector>::assign_document();
	basic_document_info<Document, Selector>::
		m_document->insert(0, content, NULL);

	// Subscribe owner
	user_subscribe(*owner);
}

template<typename Document, typename Selector>
basic_client_document_info<Document, Selector>::
	basic_client_document_info(const buffer_type& buffer,
	                           net_type& net,
	                           const net6::packet& init_pack):
	// TODO: Find a way to only extract the data once out of the packet
	basic_document_info<Document, Selector>(
		buffer,
		net,
		init_pack.get_param(0).net6::parameter::as<const user*>(
			::serialise::hex_context<const user*>(
				buffer.get_user_table()
			)
		),
		init_pack.get_param(1).net6::parameter::as<unsigned int>(),
		init_pack.get_param(2).net6::parameter::as<std::string>()
	),
	basic_local_document_info<Document, Selector>(
		buffer,
		net,
		init_pack.get_param(0).net6::parameter::as<const user*>(
			::serialise::hex_context<const user*>(
				buffer.get_user_table()
			)
		),
		init_pack.get_param(1).net6::parameter::as<unsigned int>(),
		init_pack.get_param(2).net6::parameter::as<std::string>()
	)
{
	// Load initially subscribed users
	for(unsigned int i = 3; i < init_pack.get_param_count(); ++ i)
	{
		// Get user
		const user* cur_user =
			init_pack.get_param(i).net6::parameter::as<const user*>(
				::serialise::hex_context<const user*>(
					buffer.get_user_table()
				)
			);

		// Must not be local user (who just joined the session and now
		// synchronises the document list)
		if(cur_user == &buffer.get_self() )
		{
			throw std::logic_error(
				"obby::basic_client_document_info::"
				"basic_client_document_info:\n"
				"Local user is in subscription list of "
				"initially synchronised document list"
			);
		}

		// Subscribe it
		user_subscribe(*cur_user);
	}
}

template<typename Document, typename Selector>
void basic_client_document_info<Document, Selector>::
	insert(position pos,
	       const std::string& text)
{
	if(m_jupiter.get() == NULL)
	{
		throw std::logic_error(
			"obby::basic_client_document_info::insert:\n"
			"Local user is not subscribed"
		);
	}

	insert_operation<document_type> op(pos, text);
	m_jupiter->local_op(op, &get_buffer().get_self() );
}

template<typename Document, typename Selector>
void basic_client_document_info<Document, Selector>::
	erase(position pos,
	      position len)
{
	if(m_jupiter.get() == NULL)
	{
		throw std::logic_error(
			"obby::basic_client_document_info::erase:\n"
			"Local user is not subscribed"
		);
	}

	delete_operation<document_type> op(pos, len);
	m_jupiter->local_op(op, &get_buffer().get_self() );
}

template<typename Document, typename Selector>
void basic_client_document_info<Document, Selector>::
	rename(const std::string& new_title)
{
	document_packet pack(*this, "rename");
	pack << new_title;
	get_net6().send(pack);
}

template<typename Document, typename Selector>
void basic_client_document_info<Document, Selector>::subscribe()
{
	// Already subscribed
	if(m_jupiter.get() != NULL)
	{
		throw std::logic_error(
			"obby::basic_client_document_info::subscribe:\n"
			"Local user is already subscribed"
		);
	}

	// Send request
	document_packet pack(*this, "subscribe");
	get_net6().send(pack);
}

template<typename Document, typename Selector>
void basic_client_document_info<Document, Selector>::unsubscribe()
{
	// Not subscribed?
	if(m_jupiter.get() == NULL)
	{
		throw std::logic_error(
			"obby::basic_client_document_info::unsubscribe:\n"
			"Local user is not subscribed"
		);
	}

	// Send request
	document_packet pack(*this, "unsubscribe");
	get_net6().send(pack);
}

template<typename Document, typename Selector>
void basic_client_document_info<Document, Selector>::
	on_net_packet(const document_packet& pack)
{
	if(!execute_packet(pack) )
	{
		throw net6::bad_value(
			"Unexpected command: " + pack.get_command()
		);
	}
}

#if 0
template<typename Document, typename Selector>
void basic_client_document_info<Document, Selector>::obby_sync_init()
{
	// Document gets synced now, all subscribed users will be transmitted.
	basic_document_info<Document, Selector>::m_users.clear();
}

template<typename Document, typename Selector>
void basic_client_document_info<Document, Selector>::
	obby_sync_subscribe(const user& user)
{
	// Subscribe user to info
	// TODO: Why did we call the base method here?
	// - armin, 10-31-2005
	//basic_document_info<Dorument, Selector>::user_subscribe(user);

	user_subscribe(user);
}

template<typename Document, typename Selector>
void basic_client_document_info<Document, Selector>::
	obby_local_init(const std::string& content)
{
	// Assign local document before subscribing
	basic_document_info<Document, Selector>::assign_document();
	// Subscribe local user
	user_subscribe(get_buffer().get_self() );
	// Add initial content
	basic_document_info<Document, Selector>::m_document->insert(
		0,
		content,
		NULL
	);
}
#endif

template<typename Document, typename Selector>
void basic_client_document_info<Document, Selector>::
	user_subscribe(const user& user)
{
	// Call base function
	basic_document_info<Document, Selector>::user_subscribe(user);

	// Add client to jupiter algo if we are subscribed
	if(m_jupiter.get() != NULL)
		m_jupiter->client_add(user);

	// Local user subscription
	if(&get_buffer().get_self() == &user)
	{
		// Note that the document must be there at this point because
		// the whole document synchronisation process should have been
		// performed before we subscribed to a document.
		if(basic_document_info<Document, Selector>::
			m_document.get() == NULL)
		{
			throw std::logic_error(
				"obby::basic_client_document_info::"
				"user_subscribe:\n"
				"Document content has not yet been synced"
			);
		}

		// Create jupiter algorithm to merge changes
		m_jupiter.reset(
			new jupiter_type(
				*basic_document_info<Document, Selector>::
					m_document
			)
		);

		// Add existing clients
		for(typename basic_document_info<Document, Selector>::
			user_iterator iter =
			basic_document_info<Document, Selector>::user_begin();
		    iter != basic_document_info<Document, Selector>::user_end();
		    ++ iter)
		{
			m_jupiter->client_add(*iter);
		}

		m_jupiter->record_event().connect(
			sigc::mem_fun(
				*this,
				&basic_client_document_info::on_jupiter_record
			)
		);
	}
}

template<typename Document, typename Selector>
void basic_client_document_info<Document, Selector>::
	user_unsubscribe(const user& user)
{
	// Remove user from jupiter if we are subscribed
	if(m_jupiter.get() != NULL)
		m_jupiter->client_remove(user);

	// Call base function
	basic_document_info<Document, Selector>::user_unsubscribe(user);

	// Local unsubscription
	if(&get_buffer().get_self() == &user)
	{
		// Release document if the local user unsubscribed
		basic_document_info<Document, Selector>::release_document();
		// Release jupiter algorithm
		m_jupiter.reset(NULL);
	}
}

template<typename Document, typename Selector>
bool basic_client_document_info<Document, Selector>::
	execute_packet(const document_packet& pack)
{
	// TODO: std::map<> with command to function
	if(pack.get_command() == "rename")
		{ on_net_rename(pack); return true; }

	if(pack.get_command() == "record")
		{ on_net_record(pack); return true; }

	if(pack.get_command() == "sync_init")
		{ on_net_sync_init(pack); return true; }

	if(pack.get_command() == "sync_line")
		{ on_net_sync_line(pack); return true; }

	if(pack.get_command() == "subscribe")
		{ on_net_subscribe(pack); return true; }

	if(pack.get_command() == "unsubscribe")
		{ on_net_unsubscribe(pack); return true; }

	return false;
}

template<typename Document, typename Selector>
void basic_client_document_info<Document, Selector>::
	on_net_rename(const document_packet& pack)
{
	// First parameter is the user who changed the title
	const std::string& new_title =
		pack.get_param(1).net6::parameter::as<std::string>();

	// Rename document
	basic_document_info<Document, Selector>::document_rename(new_title);
}

template<typename Document, typename Selector>
void basic_client_document_info<Document, Selector>::
	on_net_record(const document_packet& pack)
{
	// Not subscribed?
	if(m_jupiter.get() == NULL)
	{
		format_string str(
			"Got record without being subscribed to document "
			"%0%/%1%"
		);

		str << basic_document_info<Document, Selector>::get_owner_id()
		    << basic_document_info<Document, Selector>::get_id();

		throw net6::bad_value(str.str() );
	}

	// Get author of record
	const user* author = pack.get_param(0).net6::parameter::as<const user*>(
		::serialise::hex_context<const user*>(
			get_buffer().get_user_table()
		)
	);

	// Extract record from packet (TODO: virtualness for document_packet,
	// would allow to remove "+ 2" here)
	unsigned int index = 1 + 2;
	record_type rec(
		pack,
		index,
		basic_document_info<Document, Selector>::
			m_buffer.get_user_table()
	);

	// Apply remote operation
	m_jupiter->remote_op(rec, author);
}

template<typename Document, typename Selector>
void basic_client_document_info<Document, Selector>::
	on_net_sync_init(const document_packet& pack)
{
	// Subscribed?
	if(basic_document_info<Document, Selector>::m_document.get() != NULL)
	{
		format_string str(
			"Got sync_init for subscribed document %0%/%1%"
		);

		str << basic_document_info<Document, Selector>::get_owner_id()
		    << basic_document_info<Document, Selector>::get_id();

		throw net6::bad_value(str.str() );
	}

	// Assign empty document
	basic_document_info<Document, Selector>::assign_document();
	// Clear all lines, the document will be synced line-wise
	// TODO: Get at least rid of this function call
	basic_document_info<Document, Selector>::m_document->clear_lines();
}

template<typename Document, typename Selector>
void basic_client_document_info<Document, Selector>::
	on_net_sync_line(const document_packet& pack)
{
	// No document assigned or subscribed?
	if(basic_document_info<Document, Selector>::m_document.get() == NULL)
	{
		format_string str(
			"Got sync_line without sync_init for document %0%/%1%"
		);

		str << basic_document_info<Document, Selector>::get_owner_id()
		    << basic_document_info<Document, Selector>::get_id();

		throw net6::bad_value(str.str() );
	}

	// Add line to document
	unsigned int index = 2;
	basic_document_info<Document, Selector>::m_document->add_line(
		line(pack, index, get_buffer().get_user_table())
	);
}

template<typename Document, typename Selector>
void basic_client_document_info<Document, Selector>::
	on_net_subscribe(const document_packet& pack)
{
	const user* new_user =
		pack.get_param(0).net6::parameter::as<const user*>(
			::serialise::hex_context<const user*>(
				get_buffer().get_user_table()
			)
		);

	user_subscribe(*new_user);
}

template<typename Document, typename Selector>
void basic_client_document_info<Document, Selector>::
	on_net_unsubscribe(const document_packet& pack)
{
	const user* old_user =
		pack.get_param(0).net6::parameter::as<const user*>(
			::serialise::hex_context<const user*>(
				get_buffer().get_user_table()
			)
		);

	user_unsubscribe(*old_user);
}

template<typename Document, typename Selector>
void basic_client_document_info<Document, Selector>::
	on_jupiter_record(const record_type& rec,
	                  const user* from)
{
	// Build packet with record
	document_packet pack(*this, "record");
	rec.append_packet(pack);
	// Send to server
	get_net6().send(pack);
}

template<typename Document, typename Selector>
const typename basic_client_document_info<Document, Selector>::buffer_type&
basic_client_document_info<Document, Selector>::get_buffer() const
{
	return dynamic_cast<const buffer_type&>(
		basic_local_document_info<Document, Selector>::get_buffer()
	);
}

template<typename Document, typename Selector>
typename basic_client_document_info<Document, Selector>::net_type&
basic_client_document_info<Document, Selector>::get_net6()
{
	return dynamic_cast<net_type&>(
		basic_local_document_info<Document, Selector>::get_net6()
	);
}

template<typename Document, typename Selector>
const typename basic_client_document_info<Document, Selector>::net_type&
basic_client_document_info<Document, Selector>::get_net6() const
{
	return dynamic_cast<const net_type&>(
		basic_local_document_info<Document, Selector>::get_net6()
	);
}

} // namespace obby

#endif // _OBBY_CLIENT_DOCUMENT_INFO_HPP_
