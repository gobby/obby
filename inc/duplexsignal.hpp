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

#ifndef _OBBY_DUPLEXSIGNAL_HPP_
#define _OBBY_DUPLEXSIGNAL_HPP_

#include <sigc++/signal.h>

namespace obby
{

/* A duplex signal is a wrapper for two signals of the same type. It may be used
 * to emit a signal before _and_ after a certain operation took place.
 */
	
template<class SignalType>
class duplex_signal
{
public:
	duplex_signal() { }
	duplex_signal(const duplex_signal<SignalType>& other)
	 : m_sig_before(other.m_sig_before), m_sig_after(other.m_sig_after) { }
	~duplex_signal() { }

	SignalType before() { return m_sig_before; }
	SignalType after() { return m_sig_after; }

protected:
	SignalType m_sig_before;
	SignalType m_sig_after;
};

}

#endif // _OBBY_DUPLEXSIGNAL_HPP_

