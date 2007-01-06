#include <iostream>

#define protected public
#include "line.hpp"

void inspect(const obby::line& line)
{
	std::cout << "Content: " << static_cast<const std::string&>(line) << std::endl;
	std::cout << "Author count: " << line.m_authors.size() << std::endl;

	for(unsigned int i = 0; i < line.m_authors.size(); ++ i)
	{
		std::cout << "Author " << i << ":" << std::endl;
		std::cout << "\tAuthor: " << line.m_authors[i].author << std::endl;
		std::cout << "\tPosition: " << line.m_authors[i].position << std::endl;
	}
	std::cout << std::endl;
}

int main()
{
	// Three test users
	obby::user_table::user user1(1, "foo", 0x00, 0x00, 0x00);
	obby::user_table::user user2(2, "bar", 0x7f, 0x7f, 0x7f);
	obby::user_table::user user3(3, "baz", 0xff, 0xff, 0xff);

	obby::line line1;
	obby::line line2;

	// Expecting [1]foo
	line1.insert(0, "foo", user1);
	assert(line1.m_authors.size() == 1);
	assert(line1.m_authors[0].author == &user1);
	assert(line1.m_authors[0].position == 0);

	// Expecting [1]fo[2]bar[1]o
	line1.insert(2, "bar", user2);
	assert(line1.m_authors.size() == 3);
	assert(line1.m_authors[0].author == &user1);
	assert(line1.m_authors[0].position == 0);
	assert(line1.m_authors[1].author == &user2);
	assert(line1.m_authors[1].position == 2);
	assert(line1.m_authors[2].author == &user1);
	assert(line1.m_authors[2].position == 5);

	// Expecting [3]foobar
	line2.insert(0, "f", user3);
	line2.insert(1, "oo", user3);
	line2.insert(3, "b", user3);
	line2.insert(4, "ar", user3);
	assert(line2.m_authors.size() == 1);
	assert(line2.m_authors[0].author == &user3);
	assert(line2.m_authors[0].position == 0);

	// Expecting [3]foobar[1]fo[2]bar[1]o
	line1.insert(0, line2);
	assert(line1.m_authors.size() == 4);
	assert(line1.m_authors[0].author == &user3);
	assert(line1.m_authors[0].position == 0);
	assert(line1.m_authors[1].author == &user1);
	assert(line1.m_authors[1].position == 6);
	assert(line1.m_authors[2].author == &user2);
	assert(line1.m_authors[2].position == 8);
	assert(line1.m_authors[3].author == &user1);
	assert(line1.m_authors[3].position == 11);

	// Expecting [1]fo[2]bar[1]o
	line1.erase(0, 6);
	assert(line1.m_authors.size() == 3);
	assert(line1.m_authors[0].author == &user1);
	assert(line1.m_authors[0].position == 0);
	assert(line1.m_authors[1].author == &user2);
	assert(line1.m_authors[1].position == 2);
	assert(line1.m_authors[2].author == &user1);
	assert(line1.m_authors[2].position == 5);

	// Expecting [1]fo[2]bar[1]o[3]foobar
	line1.insert(6, line2);
	assert(line1.m_authors.size() == 4);
	assert(line1.m_authors[0].author == &user1);
	assert(line1.m_authors[0].position == 0);
	assert(line1.m_authors[1].author == &user2);
	assert(line1.m_authors[1].position == 2);
	assert(line1.m_authors[2].author == &user1);
	assert(line1.m_authors[2].position == 5);
	assert(line1.m_authors[3].author == &user3);
	assert(line1.m_authors[3].position == 6);

	// Expecting [2]ar[1]o[3]foobar
	line1.erase(0, 3);
	assert(line1.m_authors.size() == 3);
	assert(line1.m_authors[0].author == &user2);
	assert(line1.m_authors[0].position == 0);
	assert(line1.m_authors[1].author == &user1);
	assert(line1.m_authors[1].position == 2);
	assert(line1.m_authors[2].author == &user3);
	assert(line1.m_authors[2].position == 3);

	// Expecting [2]ar[3]foobar
	line1.erase(2, 1);
	assert(line1.m_authors.size() == 2);
	assert(line1.m_authors[0].author == &user2);
	assert(line1.m_authors[0].position == 0);
	assert(line1.m_authors[1].author == &user3);
	assert(line1.m_authors[1].position == 2);

	return 0;
}

