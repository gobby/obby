#include "zeroconf.hpp"

#include <iostream>

using namespace std;
using namespace obby;
using namespace net6;

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
	zeroconf zeroconf;

	zeroconf.discover_event().connect(sigc::ptr_fun(&on_discover));
	zeroconf.leave_event().connect(sigc::ptr_fun(&on_leave));

	zeroconf.discover();
	
	while(true)
		zeroconf.select();
}

