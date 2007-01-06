#include "rendezvous.hpp"

#include <iostream>

using namespace std;
using namespace obby;
using namespace net6;
// ^ - kranker Troll :(

void on_discover(const string& name, const ipv4_address& addr)
{
	cout << std::time(0) << " join: \"" << name << "\" ("
		<< addr.get_name() << ":" << addr.get_port() << ")" << endl;
}

void on_leave(const string& name)
{
	cout << std::time(0) << " leave: \"" << name << "\"" << endl;
}

int main()
{
	rendezvous rendezvous;

	rendezvous.discover_event().connect(sigc::ptr_fun(&on_discover));
	rendezvous.leave_event().connect(sigc::ptr_fun(&on_leave));

	rendezvous.discover();
	
	while(true)
		rendezvous.select();
}

