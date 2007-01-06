#include <unistd.h>
#include <iostream>
#include <string>
#include <stdexcept>
#include <sigc++/bind.h>
#include "insert_record.hpp"
#include "delete_record.hpp"
#include "document.hpp"
#include "client_buffer.hpp"

void insert_n(const obby::insert_record& record, obby::document& doc, int n)
{
	std::cout << "Client " << n << ": " << record.inspect() << ": " << doc.get_whole_buffer() << std::endl;
}

void erase_n(const obby::delete_record& record, obby::document& doc, int n)
{
	std::cout << "Client " << n << ": " << record.inspect() << ": " << doc.get_whole_buffer() << std::endl;
}

void sync_1(obby::client_buffer& client1)
{
	obby::document& doc = *client1.document_begin();
	
	doc.insert_event().connect(sigc::bind(
		sigc::ptr_fun(&insert_n), sigc::ref(doc), 1) );
	doc.delete_event().connect(sigc::bind(
		sigc::ptr_fun(&erase_n), sigc::ref(doc), 1) );
	
	doc.insert(4, "H");
}

void sync_2(obby::client_buffer& client2)
{
	obby::document& doc = *client2.document_begin();
	
	doc.insert_event().connect(sigc::bind(
		sigc::ptr_fun(&insert_n), sigc::ref(doc), 2) );
	doc.delete_event().connect(sigc::bind(
		sigc::ptr_fun(&erase_n), sigc::ref(doc), 2) );
	
	doc.erase(3, 4);
}

int main(int argc, char* argv[]) try
{
	int port = 6522;
	if(argc > 1)
		port = strtol(argv[1], NULL, 10);

	obby::client_buffer client1("localhost", port);
	obby::client_buffer client2("localhost", port);

	client1.login("ck", 100, 100, 100);
	client2.login("phil", 200, 200, 200);

	client1.sync_event().connect(
		sigc::bind(sigc::ptr_fun(&sync_1), sigc::ref(client1) ));
	client2.sync_event().connect(
		sigc::bind(sigc::ptr_fun(&sync_2), sigc::ref(client2) ));

	while(true)
	{
		client1.select(0);
		client2.select(0);
		usleep(100000);
	}

}
catch(std::exception& e)
{
	std::cerr << e.what() << std::endl;
}
