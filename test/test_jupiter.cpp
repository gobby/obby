#include <cstdlib>
#include <cerrno>

#include <string>
#include <fstream>
#include <iostream>
#include <vector>
#include <list>

#include <sigc++/sigc++.h>

#include "jupiter_client.hpp"
#include "jupiter_server.hpp"
#include "operation.hpp"
#include "insert_operation.hpp"
#include "delete_operation.hpp"
#include "split_operation.hpp"
#include "no_operation.hpp"
#include "document.hpp"

typedef obby::operation<obby::document> operation;
typedef obby::record<obby::document> record;
typedef obby::jupiter_server<obby::document> jupiter_server;
typedef obby::jupiter_client<obby::document> jupiter_client;

struct record_wrapper { record* rec; const obby::user* from; };

std::vector<std::string> split_line(const std::string& line,
                                    const std::string& separator)
{
	std::vector<std::string> result;
	std::string::size_type pos = 0, prev = 0;
	while( (pos = line.find(separator, pos)) != std::string::npos)
	{
		result.push_back(line.substr(prev, pos - prev) );

		pos += separator.length();
		prev = pos;
	}
	result.push_back(line.substr(prev) );
	return result;
}

void server_record(const record& rec,
                   const obby::user& to,
                   const obby::user* from,
                   std::vector<std::list<record_wrapper> >& recs)
{
	record_wrapper wrapper = {
		new record(rec.get_time(), rec.get_operation()),
		from
	};

	// TODO: Discard from?
	recs[to.get_id() - 1].push_back(wrapper);
}

void client_record(const record& rec,
                   const obby::user* from,
		   std::list<record_wrapper>& recs)
{
	record_wrapper wrapper = { 
		new record(rec.get_time(), rec.get_operation()),
		from
	};

	recs.push_back(wrapper);
}

void test(const std::string& line)
{
	const obby::user* users[] = {
		new obby::user(1, "user1", obby::colour(5,  5,  5) ),
		new obby::user(2, "user2", obby::colour(25, 25, 25) ),
		new obby::user(3, "user3", obby::colour(45, 45, 45) )
	};

	std::vector<std::string> fs = split_line(line, "|");
	if(fs.size() != 3) throw std::runtime_error("Expected 3 sections");

	std::string init = fs[0];
	std::string result = fs[2];

	const unsigned int clients = sizeof(users) / sizeof(users[0]);

	obby::document::template_type templ;

	obby::document serv_doc(templ);
	std::vector<obby::document*> client_doc;
	client_doc.resize(clients);
	
	serv_doc.insert(0, init, NULL);
	for(unsigned int i = 0; i < client_doc.size(); ++ i)
	{
		client_doc[i] = new obby::document(templ);
		client_doc[i]->insert(0, init, NULL);
	}

	jupiter_server server(serv_doc);

	std::vector<jupiter_client*> client_algos;
	client_algos.resize(clients, NULL);

	for(unsigned int i = 0; i < client_algos.size(); ++ i)
	{
		client_algos[i] = new jupiter_client(*client_doc[i]);
		server.client_add(*users[i]);
	}

	std::list<record_wrapper> serv_rec;
	std::vector<std::list<record_wrapper> > client_rec;
	client_rec.resize(clients);

	server.record_event().connect(
		sigc::bind(
			sigc::ptr_fun(&server_record),
			sigc::ref(client_rec)
		)
	);

	for(unsigned int i = 0; i < clients; ++ i)
	{
		client_algos[i]->record_event().connect(
			sigc::bind(
				sigc::ptr_fun(&client_record),
				sigc::ref(serv_rec)
			)
		);
	}

	std::vector<std::string> ops = split_line(fs[1], ",");
	for(unsigned int i = 0; i < ops.size(); ++ i)
	{
		if(ops[i].empty() ) continue;

		std::vector<std::string> vs = split_line(ops[i], "->");
		if(vs.size() != 2)
			throw std::runtime_error("Expected Site->op");

		unsigned long site = strtoul(vs[0].c_str(), NULL, 0);
		if(site > clients)
		{
			throw std::runtime_error(
				"Site must be between 0 and client count"
			);
		}

		std::vector<std::string> op = split_line(vs[1], "(");
		if(op.size() != 2)
			throw std::runtime_error("Expected op(desc)");

		if(op[0] != "ins" && op[0] != "del")
		{
			throw std::runtime_error(
				"Unsupported operation: " + op[0]
			);
		}

		std::auto_ptr<operation> new_op;
		if(op[0] == "ins")
		{
			std::vector<std::string> desc = split_line(op[1], "@");
			if(desc.size() != 2)
			{
				throw std::runtime_error(
					"Expected ins(text@position"
				);
			}

			errno = 0;

			std::string text = desc[0];
			obby::position pos = strtoul(desc[1].c_str(), NULL, 0);

			if(errno != 0)
			{
				throw std::runtime_error(
					"Expected numerical position"
				);
			}

			new_op.reset(
				new obby::insert_operation<obby::document>(
					pos,
					text
				)
			);
		}
		else
		{
			std::vector<std::string> desc = split_line(op[1], "-");
			if(desc.size() != 2)
			{
				throw std::runtime_error(
					"Expected del(from-to)"
				);
			}

			errno = 0;

			obby::position from = strtoul(desc[0].c_str(), NULL, 0);
			if(errno != 0)
			{
				throw std::runtime_error(
					"Expected numerical position"
				);
			}

			obby::position to = strtoul(desc[1].c_str(), NULL, 0);
			if(errno != 0)
			{
				throw std::runtime_error(
					"Expected numerical position"
				);
			}

			new_op.reset(
				new obby::delete_operation<obby::document>(
					from,
					to-from
				)
			);
		}

		if(site == 0)
		{
			server.local_op(*new_op, NULL);
		}
		else
		{
			client_algos[site - 1]->local_op(
				*new_op,
				users[site - 1]
			);
		}
	}

	for(std::list<record_wrapper>::iterator iter = serv_rec.begin();
	    iter != serv_rec.end(); ++ iter)
	{
		server.remote_op(*iter->rec, iter->from);
	}

	for(unsigned int i = 0; i < clients; ++ i)
	{
		for(std::list<record_wrapper>::iterator iter =
			client_rec[i].begin();
		    iter != client_rec[i].end(); ++ iter)
		{
			client_algos[i]->remote_op(*iter->rec, iter->from);
		}
	}

	if(serv_doc.get_text() != result)
	{
		throw std::runtime_error(
			"Server document \"" + serv_doc.get_text() + "\" "
			"differs from expected result \"" + result + "\""
		);
	}

	for(unsigned int i = 0; i < clients; ++ i)
	{
		if(client_doc[i]->get_text() != result)
		{
			throw std::runtime_error(
				"Client document \"" +
				client_doc[i]->get_text() + "\" from client " +
				users[i]->get_name() + " differs from expected "
				"result \"" + result + "\""
			);
		}
	}

	// TODO: Delete allocated memory
}

int main(int argc, char* argv[])
{
	std::cout << argv[0] << std::endl;
	std::cout << "Program to test the Jupiter implementation" << std::endl;
	std::cout << "See the file \"base_file\" for how to build tests" << std::endl;
	std::cout << std::endl;

	std::string filename = "base_file";

	// Support testing from a build directory (make distcheck).
	const char* base_path = getenv("srcdir");
	if(base_path != NULL) {
		filename = std::string(base_path) + "/" + filename;
	}

	if(argc >= 2) filename = argv[1];

	std::ifstream file(filename.c_str());
	if(!file)
	{
		std::cerr << "Could not open \"" << filename << "\"" << std::endl;
		return EXIT_FAILURE;
	}

	std::string line;
	unsigned int count = 0;
	unsigned int success = 0;
	unsigned int line_num = 0;

	bool result = true;
	while(std::getline(file, line) )
	{
		++ line_num;
		if(line.empty() )
			continue;
		if(line[0] == '#')
			continue;

		++ count;
		std::cout << "Test " << count << "(" << line_num << "): ";

		try
		{
			test(line);
		}
		catch(std::exception& e)
		{
			std::cout << e.what() << std::endl;
			result = false;
			continue;
		}

		std::cout << "passed!" << std::endl;
		++ success;
	}

	return result ? EXIT_SUCCESS : EXIT_FAILURE;
}
