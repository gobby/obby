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

#include <avahi-client/client.h>
#include <avahi-client/lookup.h>
#include <avahi-client/publish.h>
#include <avahi-common/simple-watch.h>
#include <avahi-common/timeval.h>

#include <sigc++/adaptors/bind.h>

#include <net6/socket.hpp>
#include <map>
#include <set>

#include "zeroconf.hpp"

namespace obby
{

class zeroconf_avahi : public zeroconf_base
{
public:
	zeroconf_avahi();

	/** Uses a custom avahi poll. Note that select() does not work with
	 * a custom poll. */
	zeroconf_avahi(AvahiPoll* poll);
	~zeroconf_avahi();

	virtual void publish(const std::string& name, unsigned int port);
	virtual void unpublish(const std::string& name);
	virtual void unpublish_all();
	virtual void discover();
	virtual void select();
	virtual void select(unsigned int msecs);

private:
	AvahiClient* m_client;
	AvahiSimplePoll* m_simple_poll;
	AvahiServiceBrowser* m_sb;
	AvahiEntryGroup* m_group;

	static void avahi_client_callback(AvahiClient* client,
			AvahiClientState state,
			void* userdata);
	static void avahi_browse_callback(AvahiServiceBrowser* sb,
			AvahiIfIndex interface,
			AvahiProtocol protocol,
			AvahiBrowserEvent event,
			const char* name,
			const char* type,
			const char* domain,
			AvahiLookupResultFlags flags,
			void* userdata);
	static void avahi_resolve_callback(AvahiServiceResolver* r,
			AvahiIfIndex interface,
			AvahiProtocol protocol,
			AvahiResolverEvent event,
			const char* name,
			const char* type,
			const char* domain,
			const char* hostname,
			const AvahiAddress* addr,
			uint16_t port,
			AvahiStringList* txt,
			AvahiLookupResultFlags flags,
			void* userdata);
	static void avahi_entry_group_callback(AvahiEntryGroup* g,
			AvahiEntryGroupState state,
			void* userdata);
};

namespace avahi_select
{

class fd_socket: public net6::socket {
public:
  fd_socket(int fd): net6::socket(fd) {}
  ~fd_socket() { invalidate(); }
};

class timeout_socket: public net6::socket {
public:
  timeout_socket(): net6::socket(-1) {}
};

template<typename Selector>
class poll
{
private:
  struct watch {
    watch(poll* poll, int fd): socket_(fd), poll_(poll) {}
    fd_socket socket_;
    AvahiWatchEvent occured_event_;
    AvahiWatchCallback callback_;
    void* userdata_;
    poll* poll_;
  };

  struct timeout {
    timeout(poll* poll): poll_(poll) {}

    timeout_socket socket_;
    poll* poll_;
  };

  static net6::io_condition avahi_to_obby(AvahiWatchEvent event)
  {
    net6::io_condition cond = net6::IO_NONE;
    if(event & AVAHI_WATCH_IN) cond |= net6::IO_INCOMING;
    if(event & AVAHI_WATCH_OUT) cond |= net6::IO_OUTGOING;
    if(event & (AVAHI_WATCH_ERR | AVAHI_WATCH_HUP)) cond |= net6::IO_ERROR;
    return cond;
  }

  static AvahiWatchEvent obby_to_avahi(net6::io_condition event)
  {
    AvahiWatchEvent cond = static_cast<AvahiWatchEvent>(0);
    if(event & net6::IO_INCOMING) cond |= AVAHI_WATCH_IN;
    if(event & net6::IO_OUTGOING) cond |= AVAHI_WATCH_OUT;
    if(event & net6::IO_ERROR) cond |= AVAHI_WATCH_ERR;
    return cond;
  }

  static AvahiWatch* watch_new_s(const AvahiPoll* api, int fd, AvahiWatchEvent event, AvahiWatchCallback callback, void* userdata) {
    return reinterpret_cast<watch*>(static_cast<poll*>(api->userdata)->watch_new(fd, event, callback, userdata));
  }

  static void watch_update_s(AvahiWatch* w, AvahiWatchEvent event) {
    reinterpret_cast<watch*>(w)->poll_->watch_update(reinterpret_cast<watch*>(w), event);
  }

  static AvahiWatchEvent watch_get_events_s(AvahiWatch* w) {
    return reinterpret_cast<watch*>(w)->poll_->watch_get_events(reinterpret_cast<watch*>(w));
  }

  static void watch_free_s(AvahiWatch* w) {
    reinterpret_cast<watch*>(w)->poll_->watch_free(reinterpret_cast<watch*>(w));
  }

  static AvahiTimeout* timeout_new_s(const AvahiPoll* api, const struct timeval* tv, AvahiTimeoutCallback callback, void* userdata) {
    return reinterpret_cast<AvahiTimeout*>(static_cast<poll*>(api->userdata)->timeout_new(tv, callback, userdata));
  }

  static void timeout_update_s(AvahiTimeout* t, const struct timeval* tv) {
    reinterpret_cast<timeout*>(t)->poll_->timeout_update(reinterpret_cast<timeout*>(t), tv);
  }

  static void timeout_free_s(AvahiTimeout* t) {
    reinterpret_cast<timeout*>(t)->poll_->timeout_free(reinterpret_cast<timeout*>(t));
  }

  void on_io(net6::io_condition condition, watch* w) {
    w->occured_event = obby_to_avahi(condition);
    w->callback_(reinterpret_cast<AvahiWatch*>(w), w->socket_.cobj(), w->occured_event, w->userdata);
  }

  void on_timeout(net6::io_condition condition, timeout* t) {
    if(condition != net6::IO_TIMEOUT)
      throw std::logic_error("on_timeout: condition != TIMEOUT");
    t->callback_(reinterpret_cast<AvahiTimeout*>(t), t->userdata);
  }

  watch* watch_new(int fd, AvahiWatchEvent event, AvahiWatchCallback callback, void* userdata)
  {
    if(m_watches.find(fd) != m_watches.end())
      throw std::logic_error("watch_new: FD is already watched");

    std::auto_ptr<watch> w(new watch(fd));
    w->occured_event_ = static_cast<AvahiWatchEvent>(0);
    w->callback_ = callback;
    w->userdata_ = userdata;
    w->socket_.signal_io().connect(sigc::bind(sigc::mem_fun(*this, &poll<Selector>::on_io), w));
    m_selector.set(w->fd_socket_, avahi_to_obby(event));
    m_watches[fd] = w.get();
    return w.release();
  }

  void watch_update(watch* w, AvahiWatchEvent event)
  {
    if(m_watches.find(w->fd_socket_.cobj()) == m_watches.end())
      throw std::logic_error("watch_update: FD is not watched");

    m_selector.set(w->fd_socket_, avahi_to_obby(event));
  }

  AvahiWatchEvent watch_get_events(watch* w)
  {
    return w->occured_event_;
  }

  void watch_free(watch* w)
  {
    m_watches.erase(w->socket_.cobj());
    delete w;
  }

  timeout* timeout_new(const timeval* tv, AvahiTimeoutCallback callback, void* userdata) {
    AvahiUsec sec = avahi_age(tv);
    if(sec > 0) sec = 0;
    sec = -sec/1000;

    std::auto_ptr<timeout> t(new timeout(this));
    t->callback_ = callback;
    t->userdata_ = userdata;
    t->socket_.signal_io().connect(sigc::bind(sigc::mem_fun(*this, &poll<Selector>::on_timeout), t));
    m_selector.set(t->socket_, net6::IO_TIMEOUT);
    m_selector.set_timeout(t->socket_, sec);
    m_timeouts.insert(t);
    return reinterpret_cast<timeout*>(t.release());
  }

  void timeout_update(timeout* t, const struct timeval* tv)
  {
    AvahiUsec sec = avahi_age(tv);
    if(sec > 0) sec = 0;
    sec = -sec/1000;

    m_selector.set(t->socket_, net6::IO_TIMEOUT);
    m_selector.set_timeout(t->socket_, sec);
  }

  void timeout_free(timeout* t)
  {
    m_timeouts.erase(t);
    delete t;
  }

public:
  poll(Selector& selector): m_selector(selector) {
    m_poll.userdata = this;
    m_poll.watch_new = &poll<Selector>::watch_new_s;
    m_poll.watch_update = &poll<Selector>::watch_update_s;
    m_poll.watch_get_events = &poll<Selector>::watch_get_events_s;
    m_poll.watch_free = &poll<Selector>::watch_free_s;
    m_poll.timeout_new = &poll<Selector>::timeout_new_s;
    m_poll.timeout_update = &poll<Selector>::timeout_update_s;
    m_poll.timeout_free = &poll<Selector>::timeout_free_s;
  }

  ~poll() {
    if(!m_watches.empty())
      std::cerr << "~avahi_select::poll: Watches not empty!" << std::endl;

    if(!m_timeouts.empty())
      std::cerr << "~avahi_select::poll: Timeouts not empty!" << std::endl;
  }

  AvahiPoll* get_poll() { return &m_poll; }

protected:
  AvahiPoll m_poll;
  Selector m_selector;

  std::map<int, watch*> m_watches;
  std::set<timeout*> m_timeouts;
};

}

}

