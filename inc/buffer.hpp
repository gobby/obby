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

#ifndef _OBBY_BUFFER_HPP_
#define _OBBY_BUFFER_HPP_

#include <set>
#include <list>

#include <net6/main.hpp>
#include <net6/object.hpp>

#include "serialise/parser.hpp"
#include "common.hpp"
#include "format_string.hpp"
#include "user_table.hpp"
#include "document.hpp"
#include "chat.hpp"
#include "document_info.hpp"

namespace obby
{

extern const unsigned long PROTOCOL_VERSION;

/** Abstract base class for obby buffers. A buffer contains multiple documents
 * that are synchronised through many users and a user list.
 */
template<typename Document, typename Selector>
class basic_buffer: private net6::non_copyable, public sigc::trackable
{
public:
	typedef Document document_type;
	typedef Selector selector_type;
	typedef typename document_type::template_type document_template_type;

	// base_document_info_type is needed to support GCC-3.3 which does
	// not support covariant returns
	typedef basic_document_info<document_type, selector_type>
		base_document_info_type;

	typedef basic_document_info<document_type, selector_type>
		document_info_type;

	// Same as above for net type
	typedef net6::basic_object<selector_type> base_net_type;
	typedef net6::basic_object<selector_type> net_type;

	// Document list
	// TODO: Outsource this to document_list<document_info_type> class
	typedef std::list<document_info_type*> document_list;
	typedef typename document_list::size_type document_size_type;

	typedef ptr_iterator<
		document_info_type,
		document_list,
		typename document_list::const_iterator
	> document_iterator;

	// Signal types
	typedef sigc::signal<void, unsigned int>
		signal_sync_init_type;
	typedef sigc::signal<void>
		signal_sync_final_type;
	typedef sigc::signal<void, const user&>
		signal_user_join_type;
	typedef sigc::signal<void, const user&>
		signal_user_part_type;
	typedef sigc::signal<void, const user&>
		signal_user_colour_type;
	typedef sigc::signal<void, document_info_type&>
		signal_document_insert_type;
	typedef sigc::signal<void, document_info_type&>
		signal_document_rename_type;
	typedef sigc::signal<void, document_info_type&>
		signal_document_remove_type;

	basic_buffer();
	virtual ~basic_buffer();

	/** @brief Returns whether the session is open.
	 */
	virtual bool is_open() const;

	/** Returns the user table associated with the buffer.
	 */
	const user_table& get_user_table() const;

	/** Returns the obby::chat for this buffer.
	 */
	const chat& get_chat() const;

	/** Returns the selector of the underlaying net6 network object.
	 */
	selector_type& get_selector();

	/** Returns the selector of the underlaying net6 network object.
	 */
	const selector_type& get_selector() const;

	/** Serialises the complete obby session into <em>file</em>.
	 */
	void serialise(const std::string& file) const;

	/* Creates a new document with predefined content.
	 * signal_document_insert will be emitted if it has been created.
	 */
	virtual void document_create(const std::string& title,
	                             const std::string& encoding,
	                             const std::string& content) = 0;

	/** Removes an existing document. signal_document_remove will be
	 * emitted if the document has been removed.
	 */
	virtual void document_remove(base_document_info_type& doc) = 0;
	
	/** Looks for a document with the given ID which belongs to the user
	 * with the given owner ID. Note that we do not take a real user object
	 * here because the ID is enough and one might not have a user object
	 * to the corresponding ID. So a time-consuming lookup is obsolete.
	 */
	document_info_type* document_find(unsigned int owner_id,
	                                  unsigned int id) const;

	/** Returns the begin of the document list.
	 */
	document_iterator document_begin() const;

	/** Returns the end of the document list.
	 */
	document_iterator document_end() const;

	/** Returns the size of the document list.
	 */
	document_size_type document_count() const;

	/** Sends a global chat message to all users.
	 */
	virtual void send_message(const std::string& message) = 0;

	/** Checks if given colour components match an other
	 * already present one too closely.
	 * TODO: Move this function to user table?
	 */
	bool check_colour(const colour& colour,
	                  const user* ignore = NULL) const;

	/** @brief Returns the current document template that is used
	 * to instanciate a document.
	 */
	const document_template_type& get_document_template() const;

	/** Sets a new template that is used to instanciate a document.
	 */
	void set_document_template(const document_template_type& tmpl);

	/** @brief Looks for a free suffix in the buffer.
	 */
	unsigned int find_free_suffix(const std::string& for_title,
	                              const document_info_type* ignore) const;

	/** Signal which will be emitted when the initial syncrhonisation
	 * begins, thus if the client has logged in successfully.
	 */
	signal_sync_init_type sync_init_event() const;

	/** Signal which will be emitted when the initial synchronisation of
	 * the user list and the document list has been completed.
	 */
	signal_sync_final_type sync_final_event() const;

	/** Signal which will be emitted if a new user has joined the obby
	 * session.
	 */
	signal_user_join_type user_join_event() const;

	/** Signal which will be emitted if a user has quit.
	 */
	signal_user_part_type user_part_event() const;

	/** Signal which will be emitted if a user changes his colour.
	 */
	signal_user_colour_type user_colour_event() const;

	/** Signal which will be emitted when another participant in the
	 * obby session has created a new document.
	 */
	signal_document_insert_type document_insert_event() const;

	/** Signal which will be emitted when another participant in the
	 * obby session renames one document.
	 */
	signal_document_rename_type document_rename_event() const;

	/** Signal which will be emitted when another participant in the
	 * obby session has removed an existing document.
	 */
	signal_document_remove_type document_remove_event() const;

protected:
        /** Internal function to add a document to the buffer.
	 */
	void document_add(document_info_type& document);

	/** Internal function to delete a document from the buffer.
	 */
	void document_delete(document_info_type& document);

	/** Internal function to clear the whole document list.
	 */
	void document_clear();

	/** @brief Internal function to add a user to the buffer.
	 */
	void user_join(const user& user);

	/** @brief Internal function that clears up when a user has gone.
	 */
	void user_part(const user& user);

	/** @brief Closes the session.
	 */
	virtual void session_close();

	/** @brief Implementation of session_close() that does not call
	 * a base function.
	 */
	void session_close_impl();

	signal_sync_init_type m_signal_sync_init;
	signal_sync_final_type m_signal_sync_final;

	signal_user_join_type m_signal_user_join;
	signal_user_part_type m_signal_user_part;
	signal_user_colour_type m_signal_user_colour;

	signal_document_insert_type m_signal_document_insert;
	//signal_document_rename_type m_signal_document_rename;
	signal_document_remove_type m_signal_document_remove;

	net6::main m_netkit;
	std::auto_ptr<net_type> m_net;

	user_table m_user_table;
	chat m_chat;

	document_list m_docs;
	document_template_type m_document_template;
	unsigned int m_doc_counter;

	net6::gettext_package m_package;
};

typedef basic_buffer<obby::document, net6::selector> buffer;

template<typename Document, typename Selector>
basic_buffer<Document, Selector>::basic_buffer():
	m_chat(*this, 0xff),
	m_doc_counter(0), m_package(obby_package(), obby_localedir())
{
	// Initialize gettext
	init_gettext(m_package);
}

template<typename Document, typename Selector>
basic_buffer<Document, Selector>::~basic_buffer()
{
	document_clear();
}

template<typename Document, typename Selector>
bool basic_buffer<Document, Selector>::is_open() const
{
	return m_net.get() != NULL;
}

template<typename Document, typename Selector>
const user_table& basic_buffer<Document, Selector>::get_user_table() const
{
	return m_user_table;
}

template<typename Document, typename Selector>
const chat& basic_buffer<Document, Selector>::get_chat() const
{
	return m_chat;
}

template<typename Document, typename Selector>
typename basic_buffer<Document, Selector>::selector_type&
	basic_buffer<Document, Selector>::get_selector()
{
	if(m_net.get() == NULL)
	{
		throw std::logic_error(
			"obby::basic_buffer::get_selector:\n"
			"Net object not yet initialized"
		);
	}

	return m_net->get_selector();
}

template<typename Document, typename Selector>
const typename basic_buffer<Document, Selector>::selector_type&
	basic_buffer<Document, Selector>::get_selector() const
{
	if(m_net.get() == NULL)
	{
		throw std::logic_error(
			"obby::basic_buffer::get_selector:\n"
			"Net object not yet initialized"
		);
	}

	return m_net->get_selector();
}

template<typename Document, typename Selector>
void basic_buffer<Document, Selector>::
	serialise(const std::string& session) const
{
	serialise::parser parser;
	parser.set_type("obby");

	serialise::object& root = parser.get_root();
	root.set_name("session");
	root.add_attribute("version").set_value(obby_version() );

	serialise::object& user_table = root.add_child();
	user_table.set_name("user_table");
	m_user_table.serialise(user_table);

	serialise::object& chat = root.add_child();
	chat.set_name("chat");
	m_chat.serialise(chat);

	for(document_iterator iter = document_begin();
	    iter != document_end();
	    ++ iter)
	{
		// Do not serialise this document if we do not have its content
		try { iter->get_content(); } catch(...) { continue; }

		serialise::object& doc = root.add_child();
		doc.set_name("document");
		iter->serialise(doc);
	}

	parser.serialise(session);
}

template<typename Document, typename Selector>
typename basic_buffer<Document, Selector>::document_info_type*
basic_buffer<Document, Selector>::document_find(unsigned int owner_id,
                                                unsigned int id) const
{
	document_iterator iter;
	for(iter = m_docs.begin(); iter != m_docs.end(); ++ iter)
	{
		// Check document ID
		if(iter->get_id() != id) continue;
		// Check owner ID
		if(iter->get_owner_id() != owner_id) continue;
		// Found requested document
		return &(*iter);
	}

	return NULL;
}

template<typename Document, typename Selector>
bool basic_buffer<Document, Selector>::check_colour(const colour& colour,
                                                    const user* ignore) const
{
	for(user_table::iterator iter =
		m_user_table.begin(user::flags::CONNECTED, user::flags::NONE);
	    iter !=
		m_user_table.end(user::flags::CONNECTED, user::flags::NONE);
	    ++ iter)
	{
		// Ignore given user to ignore
		if(&(*iter) == ignore) continue;

		if(colour.similar_colour(iter->get_colour()) )
		{
			// Conflict
			return false;
		}
	}

	return true;
}

template<typename Document, typename Selector>
typename basic_buffer<Document, Selector>::document_iterator
basic_buffer<Document, Selector>::document_begin() const
{
	return static_cast<document_iterator>(m_docs.begin() );
}

template<typename Document, typename Selector>
typename basic_buffer<Document, Selector>::document_iterator
basic_buffer<Document, Selector>::document_end() const
{
	return static_cast<document_iterator>(m_docs.end() );
}

template<typename Document, typename Selector>
typename basic_buffer<Document, Selector>::document_size_type
basic_buffer<Document, Selector>::document_count() const
{
	return m_docs.size();
}

template<typename Document, typename Selector>
const typename basic_buffer<Document, Selector>::document_template_type&
basic_buffer<Document, Selector>::get_document_template() const
{
	return m_document_template;
}

template<typename Document, typename Selector>
void basic_buffer<Document, Selector>::
	set_document_template(const document_template_type& tmpl)
{
	m_document_template = tmpl;
}

template<typename Document, typename Selector>
unsigned int basic_buffer<Document, Selector>::
	find_free_suffix(const std::string& for_title,
	                 const document_info_type* ignore) const
{
	// Set that sorts suffixes in ascending order
	std::set<unsigned int> suffixes;

	// Put all suffixes into the set
	for(document_iterator it = m_docs.begin(); it != m_docs.end(); ++ it)
	{
		if(ignore == &(*it) )
			continue;

		if(it->get_title() == for_title)
			suffixes.insert(it->get_suffix() );
	}

	// Choose the lowest free one
	unsigned int prev_suffix = 0;
	for(std::set<unsigned int>::const_iterator iter = suffixes.begin();
	    iter != suffixes.end();
	    ++ iter)
	{
		if(*iter > prev_suffix + 1)
			break;
		else
			prev_suffix = *iter;
	}

	return prev_suffix + 1;
}

template<typename Document, typename Selector>
typename basic_buffer<Document, Selector>::signal_sync_init_type
basic_buffer<Document, Selector>::sync_init_event() const
{
	return m_signal_sync_init;
}

template<typename Document, typename Selector>
typename basic_buffer<Document, Selector>::signal_sync_final_type
basic_buffer<Document, Selector>::sync_final_event() const
{
	return m_signal_sync_final;
}

template<typename Document, typename Selector>
typename basic_buffer<Document, Selector>::signal_user_join_type
basic_buffer<Document, Selector>::user_join_event() const
{
	return m_signal_user_join;
}

template<typename Document, typename Selector>
typename basic_buffer<Document, Selector>::signal_user_part_type
basic_buffer<Document, Selector>::user_part_event() const
{
	return m_signal_user_part;
}

template<typename Document, typename Selector>
typename basic_buffer<Document, Selector>::signal_user_colour_type
basic_buffer<Document, Selector>::user_colour_event() const
{
	return m_signal_user_colour;
}

template<typename Document, typename Selector>
typename basic_buffer<Document, Selector>::signal_document_insert_type
basic_buffer<Document, Selector>::document_insert_event() const
{
	return m_signal_document_insert;
}

/*template<typename Document, typename Selector>
typename basic_buffer<Document, Selector>::signal_document_rename_type
basic_buffer<Document, Selector>::document_rename_event() const
{
	return m_signal_document_rename;
}*/

template<typename Document, typename Selector>
typename basic_buffer<Document, Selector>::signal_document_remove_type
basic_buffer<Document, Selector>::document_remove_event() const
{
	return m_signal_document_remove;
}

template<typename Document, typename Selector>
void basic_buffer<Document, Selector>::
	document_add(document_info_type& info)
{
	typedef typename document_info_type::user_iterator user_iterator;
	std::set<const obby::user*> users;

	// Remember all users initially subscribed
	for(user_iterator iter = info.user_begin();
	    iter != info.user_end();
	    ++ iter)
	{
		users.insert(&(*iter));
	}

	// Add new document into list
	m_docs.push_back(&info);
	// Emit document_insert signal
	m_signal_document_insert.emit(info);
	// Emit user_subscribe signal for each user that was initially
	// subscribed to the document. This does not include users that
	// have been subscribed by the document insert signal handler.
	for(user_iterator iter = info.user_begin();
	    iter != info.user_end();
	    ++ iter)
	{
		if(users.find(&(*iter)) != users.end())
			info.subscribe_event().emit(*iter);
	}
}

template<typename Document, typename Selector>
void basic_buffer<Document, Selector>::
	document_delete(document_info_type& info)
{
	// TODO: Emit user_unsubscribe signal for each user that was subscribed?
	// Emit document_remove signal
	m_signal_document_remove.emit(info);
	// Delete from list (TODO: Use std::set?)
	m_docs.erase(
		std::remove(m_docs.begin(), m_docs.end(), &info),
		m_docs.end()
	);

	// Delete document
	delete &info;
}

template<typename Document, typename Selector>
void basic_buffer<Document, Selector>::document_clear()
{
	// TODO: Emit document_remove signal for each document?
	typename document_list::iterator iter;
	for(iter = m_docs.begin(); iter != m_docs.end(); ++ iter)
		delete *iter;

	m_docs.clear();
}

template<typename Document, typename Selector>
void basic_buffer<Document, Selector>::user_join(const user& user)
{
	// User should have already been added to the user table (that creates
	// the user object).
	for(document_iterator iter = document_begin();
	    iter != document_end();
	    ++ iter)
	{
		iter->obby_user_join(user);
	}

	// TODO: Move signal emission to user_table::add_user.
	m_signal_user_join.emit(user);
}

template<typename Document, typename Selector>
void basic_buffer<Document, Selector>::user_part(const user& user)
{
	for(document_iterator iter = document_begin();
	    iter != document_end();
	    ++ iter)
	{
		iter->obby_user_part(user);
	}

	m_signal_user_part.emit(user);

	// TODO: Move signal emission to user_table::remove_user
	m_user_table.remove_user(user);
}

template<typename Document, typename Selector>
void basic_buffer<Document, Selector>::session_close()
{
	session_close_impl();
}

template<typename Document, typename Selector>
void basic_buffer<Document, Selector>::session_close_impl()
{
	for(document_iterator iter = document_begin();
	    iter != document_end();
	    ++ iter)
	{
		iter->obby_session_close();
	}

	m_net.reset(NULL);
}

} // namespace obby

#endif // _OBBY_BUFFER_HPP_
