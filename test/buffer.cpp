#include <iostream>
#include <sigc++/bind.h>

#include "record.hpp"
#include "insert_record.hpp"
#include "delete_record.hpp"
#include "buffer.hpp"
#include "client_buffer.hpp"
#include "server_buffer.hpp"

int port = 6521;
bool quit = false;

#ifdef CLIENT_TEST
void client_join(net6::client::peer& peer, obby::client_buffer& buffer)
{
	std::cout << peer.get_name() << " has joined." << std::endl;
	if(peer.get_id() == buffer.get_self()->get_id() )
		buffer.insert(obby::position(0, 0), "l");
}

void client_part(net6::client::peer& peer)
{
	std::cout << peer.get_name() << " has quit." << std::endl;
}

void client_close()
{
	std::cout << "Connection lost." << std::endl;
	quit = true;
}

void client_login_failed(const std::string& reason)
{
	std::cout << reason << "." << std::endl;
	quit = true;
}

void client_insert(const obby::insert_record& record)
{
	std::cout << "Insert " << record.get_text() << " at " << record.get_position().get_line() << "/" << record.get_position().get_col() << std::endl;
}

void client_delete(const obby::delete_record& record)
{
	std::cout << "Delete from " << record.get_begin().get_line() << "/" << record.get_begin().get_col() << " to " << record.get_end().get_line() << "/" << record.get_end().get_col() << std::endl;
}

int client_main(int argc, char* argv[])
{
	if(argc < 3)
	{
		std::cerr << "Usage: " << argv[0] << " user host [port]" << std::endl;
		return -1;
	}

	if(argc > 3)
		port = strtol(argv[3], NULL, 10);

	obby::client_buffer buffer(argv[2], port);
	buffer.login(argv[1]);

	buffer.join_event().connect(sigc::bind(sigc::ptr_fun(&client_join), sigc::ref(buffer)) );
	buffer.part_event().connect(sigc::ptr_fun(&client_part) );
	buffer.close_event().connect(sigc::ptr_fun(&client_close) );
	buffer.login_failed_event().connect(sigc::ptr_fun(&client_login_failed));
	buffer.insert_event().connect(sigc::ptr_fun(&client_insert) );
	buffer.delete_event().connect(sigc::ptr_fun(&client_delete) );

	while(!quit)
		buffer.select();

	return 0;
}
#else

void server_join(net6::server::peer& peer)
{
	std::cout << "Connection from " << peer.get_address().get_name() << "." << std::endl;
}

void server_login(net6::server::peer& peer)
{
	std::cout << peer.get_address().get_name() << " logged in as " << peer.get_name() << "." << std::endl;
}

void server_part(net6::server::peer& peer)
{
	if(peer.is_logined() )
		std::cout << peer.get_name() << " has quit." << std::endl;
	else
		std::cout << peer.get_address().get_name() << "has quit." << std::endl;
}

void server_insert(const obby::insert_record& record)
{
	std::cout << "Insert " << record.get_text() << " at " << record.get_position().get_line() << "/" << record.get_position().get_col() << std::endl;
}

void server_delete(const obby::delete_record& record)
{
	std::cout << "Delete from " << record.get_begin().get_line() << "/" << record.get_begin().get_col() << " to " << record.get_end().get_line() << "/" << record.get_end().get_col() << std::endl;
}

int server_main(int argc, char* argv[])
{
	if(argc > 1)
		port = strtol(argv[1], NULL, 10);

	obby::server_buffer buffer(port);

	buffer.join_event().connect(sigc::ptr_fun(&server_join) );
	buffer.login_event().connect(sigc::ptr_fun(&server_login) );
	buffer.part_event().connect(sigc::ptr_fun(&server_part) );
	buffer.insert_event().connect(sigc::ptr_fun(&server_insert) );
	buffer.delete_event().connect(sigc::ptr_fun(&server_delete) );
	
	while(!quit)
		buffer.select();

	return 0;
}
#endif

int main(int argc, char* argv[]) try
{
#ifdef CLIENT_TEST
	client_main(argc, argv);
#else
	server_main(argc, argv);
#endif
}
catch(std::exception& e)
{
	std::cerr << e.what() << std::endl;
	return -1;
}

