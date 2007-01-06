#include <iostream>
#include <stdexcept>
#include "serialise/parser.hpp"

namespace
{
	const std::string document =
		"!obby\n"
		"root\n"
		" child_1 foo=\"bar\"\n"
		" child_2 bar=\"foo\"\n"
		"  child_2_1 bar=\"foo\" foo=\"bar\"\n"
		"   child_2_1_1\n"
		"  child_2_2\n"
		" child_3\n"
		"  child_3_1";
}

int main() try
{
	obby::serialise::parser parser;
	parser.deserialise_memory(document);

	std::string parsed_document;
	parser.serialise_memory(parsed_document);

	if(parsed_document != document)
		throw std::logic_error("output differs from input");

	std::cout << "Serialisation test passed" << std::endl;
	return EXIT_SUCCESS;
}
catch(std::exception& e)
{
	std::cerr << "Serialisation test failed: " << e.what() << std::endl;
	return EXIT_FAILURE;
}
