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

#ifndef _OBBY_LOCAL_BUFFER_HPP_
#define _OBBY_LOCAL_BUFFER_HPP_

#include "buffer.hpp"
#include "local_document_info.hpp"

namespace obby
{

/** A local_buffer is a buffer object with a local user.
 */
template<typename Document, typename Selector>
class basic_local_buffer:
	virtual public basic_buffer<Document, Selector>
{
public: 
	typedef typename basic_buffer<Document, Selector>::
		base_document_info_type base_document_info_type;

	typedef basic_local_document_info<Document, Selector>
		document_info_type;

	typedef typename basic_buffer<Document, Selector>::base_net_type
		base_net_type;

	typedef net6::basic_local<Selector> net_type;

	typedef sigc::signal<void>
		signal_user_colour_failed_type;

	/** Standard constructor.
	 */
	basic_local_buffer();

	/** Returns the local user.
	 */
	virtual const user& get_self() const = 0;

	/** Returns the name of the local user. This method is overwritten
	 * by basic_client_buffer to provide access to the user's name even if
	 * the login process has not completed.
	 */
	virtual const std::string& get_name() const;

	/** Looks for a document with the given ID which belongs to the user
	 * with the given owner ID. Note that we do not take a real user object
	 * here because the ID is enough and one might not have a user object
	 * to the corresponding ID. So a time-consuming lookup is obsolete.
	 */
	document_info_type* document_find(unsigned int owner_id,
	                                  unsigned int id) const;

	/** Sets a new colour for the local user and propagates this change
	 * to the others.
	 */
	virtual void set_colour(const colour& colour) = 0;

	/** Signal which will be emitted if a user colour changecd failed at
	 * the server.
	 */
	signal_user_colour_failed_type user_colour_failed_event() const;

protected:
	signal_user_colour_failed_type m_signal_user_colour_failed;
};

typedef basic_local_buffer<obby::document, net6::selector> local_buffer;

template<typename Document, typename Selector>
basic_local_buffer<Document, Selector>::basic_local_buffer():
	basic_buffer<Document, Selector>()
{
}

template<typename Document, typename Selector>
const std::string& basic_local_buffer<Document, Selector>::get_name() const
{
	return get_self().get_name();
}

template<typename Document, typename Selector>
typename basic_local_buffer<Document, Selector>::document_info_type*
basic_local_buffer<Document, Selector>::document_find(unsigned int owner_id,
                                                      unsigned int id) const
{
	return dynamic_cast<document_info_type*>(
		basic_buffer<Document, Selector>::document_find(owner_id, id)
	);
}

template<typename Document, typename Selector>
typename basic_local_buffer<Document, Selector>::signal_user_colour_failed_type
basic_local_buffer<Document, Selector>::user_colour_failed_event() const
{
	return m_signal_user_colour_failed;
}

} // namespace obby

#endif // _OBBY_LOCAL_BUFFER_HPP_
