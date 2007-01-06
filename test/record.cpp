#include <iostream>

#include "record.hpp"
#include "insert_record.hpp"
#include "delete_record.hpp"

unsigned int count = 0;

std::ostream& operator<<(std::ostream& out, const obby::insert_record& record)
{
	out << record.get_text() << "@" << record.get_position();
}

std::ostream& operator<<(std::ostream& out, const obby::delete_record& record)
{
	out << record.get_begin() << "-" << record.get_end();
}

bool ins_ins(const std::string& message, const obby::position& pos,
             const obby::position& ins_pos, const std::string& insert,
             const obby::position& target)
{
	++ count;
	std::cout << count << ": " << message << std::endl;
	obby::insert_record record(pos, "foobar", 0, 0);
	record.on_insert(ins_pos, insert);
	if(record.get_position() != target)
		std::cout << "failed. got " << record.get_position() << ", but expected " << target << std::endl;
	return record.get_position() == target;
}

bool ins_del(const std::string& message, const obby::position& pos,
             const obby::position& begin, const obby::position& end,
	     const obby::position& target)
{
	++ count;
	std::cout << count << ": " << message << std::endl;
	obby::insert_record record(pos, "foobar", 0, 0);
	record.on_delete(begin, end);
	if(record.get_position() != target)
		std::cout << "failed. got " << record.get_position() << ", but expected " << target << std::endl;
	return record.get_position() == target;
}

int main()
{
	unsigned int success = 0;
	
	if(ins_ins(
		"text in front of insert_record...",
	        5, 2, "hallo", 10
	)) ++ success;

	if(ins_ins(
		"text at insert_record...",
		5, 5, "moin", 9
	)) ++ success;

	if(ins_ins(
		"text after insert_record...",
		5, 7, "moin", 5
	)) ++ success;

	std::cout << success << " of " << count << " tests passed successfully" << std::endl;
	return 0;
}

