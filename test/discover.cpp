#include "rendezvous.hpp"

#include <iostream>

using namespace std;
using namespace obby;

void on_discover(const string& name, const string& ip,
	unsigned int port)
{
	cout << std::time(0) << " join: \"" << name << "\" (" << ip
	       	<< ":" << port << ")" << endl;
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

