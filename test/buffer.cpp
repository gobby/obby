#include <iostream>
#include <sigc++/bind.h>

#include "record.hpp"
#include "insert_record.hpp"
#include "delete_record.hpp"
#include "document.hpp"
#include "client_buffer.hpp"

// TODO: Zeroconf
// TODO: MDI output

class buffer : public sigc::trackable
{
public:
	buffer(int argc, char* argv[]);
	~buffer();

	void run();
	void quit();

protected:
	void on_join(obby::user& user);
	void on_part(obby::user& user);
	void on_sync();
	void on_close();

	void on_insert_document(obby::document& doc);
	void on_remove_document(obby::document& doc);

	void on_doc_insert(const obby::insert_record&, obby::document& doc);
	void on_doc_remove(const obby::delete_record&, obby::document& doc);

	obby::client_buffer m_buffer;
	bool m_quit;
};

buffer::buffer(int argc, char* argv[])
 : m_buffer(argv[2], argc > 3 ? strtol(argv[3], NULL, 10) : 6522), m_quit(false)
{
	srand(time(NULL) );
	m_buffer.login(argv[1], rand() % 256, rand() % 256, rand() % 256);

	m_buffer.user_join_event().connect(
		sigc::mem_fun(*this, &buffer::on_join) );
	m_buffer.user_part_event().connect(
		sigc::mem_fun(*this, &buffer::on_part) );
	m_buffer.sync_event().connect(
		sigc::mem_fun(*this, &buffer::on_sync) );
	m_buffer.close_event().connect(
		sigc::mem_fun(*this, &buffer::on_close) );

	m_buffer.insert_document_event().connect(
		sigc::mem_fun(*this, &buffer::on_insert_document) );
	m_buffer.remove_document_event().connect(
		sigc::mem_fun(*this, &buffer::on_remove_document) );
}

buffer::~buffer()
{
}

void buffer::run()
{
	while(!m_quit)
		m_buffer.select();
}

void buffer::quit()
{
	m_quit = true;
}

void buffer::on_join(obby::user& user)
{
	std::cout << user.get_name() << " joined." << std::endl;
}

void buffer::on_part(obby::user& user)
{
	std::cout << user.get_name() << " has quit." << std::endl;
}

void buffer::on_sync()
{
	obby::buffer::document_iterator iter;
	for(iter = m_buffer.document_begin(); iter != m_buffer.document_end(); ++ iter)
		on_insert_document(*iter);
}

void buffer::on_close()
{
	std::cout << "Connection lost" << std::endl;
	quit();
}

void buffer::on_insert_document(obby::document& doc)
{
	std::cout << "Created document (ID = " << doc.get_id() << ", ";
	std::cout << "TITLE = \"" << doc.get_title() << "\"):" << std::endl;
	std::cout << doc.get_whole_buffer() << std::endl;
	std::cout << "----------------------" << std::endl;

	doc.insert_event().connect(sigc::bind(
		sigc::mem_fun(*this, &buffer::on_doc_insert),
		sigc::ref(doc)
	));

	doc.delete_event().connect(sigc::bind(
		sigc::mem_fun(*this, &buffer::on_doc_remove),
		sigc::ref(doc)
	));

//	doc.insert(obby::position(0, 0), "A");
//	doc.erase(obby::position(0, 0), obby::position(0, 1) );
//	doc.insert(obby::position(0, 0), "B");

//	doc.insert(0, "\n");
//	doc.insert(1, "C");
//	doc.erase(0, 1);
//
	doc.insert(0, "A");
	doc.insert(1, "B");

/*	doc.insert(obby::position(0, 0), "A");
	doc.insert(obby::position(0, 1), "B");
	doc.erase(obby::position(0, 0), obby::position(0, 1) );
	doc.erase(obby::position(0, 0), obby::position(0, 1) );*/
}

void buffer::on_remove_document(obby::document& doc)
{
	std::cout << "Removed document " << doc.get_id() << std::endl;
}

void buffer::on_doc_insert(const obby::insert_record& rec, obby::document& doc)
{
	std::cout << "Insert " << rec.get_text() << " at " << rec.get_position() << ":" << std::endl;
	std::cout << doc.get_whole_buffer() << std::endl;
	std::cout << "----------------------" << std::endl;
}

void buffer::on_doc_remove(const obby::delete_record& rec, obby::document& doc)
{
	std::cout << "Delete " << rec.get_text() << " at " << rec.get_begin() << ":" << std::endl;
	std::cout << doc.get_whole_buffer() << std::endl;
	std::cout << "----------------------" << std::endl;
}

int main(int argc, char* argv[]) try
{
	if(argc < 3)
	{
		std::cout << argv[0] << " username host [port]" << std::endl;
	}
	else
	{
		buffer buf(argc, argv);
		buf.run();
	}

	return 0;
}
catch(std::exception& e)
{
	std::cerr << e.what() << std::endl;
	return 1;
}
