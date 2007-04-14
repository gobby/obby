/* libobby - Network text editing library
 * Copyright (C) 2005-2006 0x539 dev group
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

#ifndef _OBBY_ZEROCONF_HPP_
#define _OBBY_ZEROCONF_HPP_

#include <string>
#include <map>
#include <memory>

#include <sigc++/signal.h>
#include <net6/non_copyable.hpp>
#include <net6/address.hpp>

namespace obby
{

class zeroconf_base : private net6::non_copyable
{
public:
	typedef sigc::signal<void, const std::string&, 
		const net6::ipv4_address&> signal_discover_type;
	typedef sigc::signal<void, const std::string&> signal_leave_type;

	virtual void publish(const std::string& name, unsigned int port) = 0;
	virtual void unpublish(const std::string& name) = 0;
	virtual void unpublish_all() = 0;
	virtual void discover() = 0;
	virtual void select() = 0;
	virtual void select(unsigned int msecs) = 0;

	virtual signal_discover_type discover_event() const;
	virtual signal_leave_type leave_event() const;

	// TODO: Provide API to allow application programmers to handle
	// errors while publishing/discovering. This requires at least
	// additional virtual methods which brak ABI.

protected:
	zeroconf_base();
	// TODO: Get a virtual destructor as soon as we can break ABI.
	//virtual ~zeroconf_base() {}

private:
	signal_discover_type m_signal_discover;
	signal_leave_type m_signal_leave;
};

class zeroconf : public zeroconf_base
{
public:
	zeroconf();
	~zeroconf();

	/** Publishes a record to other users of this library within the
	 * default domain (.local). It uses the service identifier
	 * _lobby._tcp. <em>name</em> is the value which should be displayed
	 * when other users are discovering this record. */
	virtual void publish(const std::string& name, unsigned int port);

	/** Unpublishes a record.
	 */
	virtual void unpublish(const std::string& name);

	/** Unpublishes all records.
	 */
	virtual void unpublish_all();

	/** Discovers other users in the local network within the default
	 * domain (.local). It searches for participants with the service
	 * identifier set to _lobby._tcp. It emits a signal when a new user
	 * is found, handing over the name, the ip and the port of the
	 * participant. */
	virtual void discover();

	/** Process all zeroconf events. This procedure does not return,
	 * so it should be used in an own thread. */
	virtual void select();

	/** Process all available Zeroconf events in a timeframe of
	 * <em>msecs</em> milliseconds. A value of 0 will prevent the command
	 * from blocking the caller. */
	virtual void select(unsigned int msecs);

	/** Signal which will be emitted when a new server is found on the
	 * network. */
	virtual signal_discover_type discover_event() const;

	/** Signal which will be emitted when a formerly present server left
	 * the network. */
	virtual signal_leave_type leave_event() const;

protected:
	std::auto_ptr<zeroconf_base> m_delegate;
};

}

#endif // _OBBY_ZEROCONF_HPP_

