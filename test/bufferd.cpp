#include <iostream>
#include <sigc++/bind.h>

#include "record.hpp"
#include "insert_record.hpp"
#include "delete_record.hpp"
#include "document.hpp"
#include "server_buffer.hpp"

// TODO: rendezvous
// TODO: MDI output

class bufferd : public sigc::trackable
{
public:
	bufferd(int argc, char* argv[]);
	~bufferd();

	void run();
	void quit();
protected:
	void on_join(obby::user& user);
	void on_part(obby::user& user);

	void on_connect(net6::server::peer& peer);
	void on_disconnect(net6::server::peer& peer);

	void on_doc_insert(const obby::insert_record&, obby::document& doc);
	void on_doc_remove(const obby::delete_record&, obby::document& doc);

	obby::server_buffer m_buffer;
	bool m_quit;
};

bufferd::bufferd(int argc, char* argv[])
 : m_buffer(argc > 1 ? strtol(argv[1], NULL, 10) : 6522), m_quit(false)
{
	obby::document& doc = m_buffer.create_document();

	m_buffer.user_join_event().connect(
		sigc::mem_fun(*this, &bufferd::on_join) );
	m_buffer.user_part_event().connect(
		sigc::mem_fun(*this, &bufferd::on_part) );

	m_buffer.connect_event().connect(
		sigc::mem_fun(*this, &bufferd::on_connect) );
	m_buffer.disconnect_event().connect(
		sigc::mem_fun(*this, &bufferd::on_disconnect));
	
	doc.insert_event().connect(sigc::bind(
		sigc::mem_fun(*this, &bufferd::on_doc_insert), sigc::ref(doc)));
	doc.delete_event().connect(sigc::bind(
		sigc::mem_fun(*this, &bufferd::on_doc_remove), sigc::ref(doc)));
}

bufferd::~bufferd()
{
}

void bufferd::run()
{
	while(!m_quit)
		m_buffer.select();
}

void bufferd::quit()
{
	m_quit = true;
}

void bufferd::on_connect(net6::server::peer& peer)
{
	std::cout << peer.get_address().get_name() << " connected."
	          << std::endl;
}

void bufferd::on_disconnect(net6::server::peer& peer)
{
	std::cout << peer.get_address().get_name() << " disconnected."
	          << std::endl;
}

void bufferd::on_join(obby::user& user)
{
	std::cout << user.get_address().get_name() << " logged in as "
	          << user.get_name() << "." << std::endl;
}

void bufferd::on_part(obby::user& user)
{
	std::cout << user.get_name() << " has quit." << std::endl;
}

void bufferd::on_doc_insert(const obby::insert_record&, obby::document& doc)
{
	std::cout << "insert." << std::endl;
}

void bufferd::on_doc_remove(const obby::delete_record&, obby::document& doc)
{
	std::cout << "remove." << std::endl;
}

int main(int argc, char* argv[])
{
	bufferd buffer(argc, argv);
	buffer.run();

	return 0;
}

