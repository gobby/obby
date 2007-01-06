#include <iostream>
#include <stdexcept>
#include <gmpxx.h>
#include "rsa.hpp"

using namespace obby;

int main() try
{
	gmp_randclass rclass(GMP_RAND_ALG_LC, 16);
	rclass.seed(std::time(NULL) );
	std::pair<RSA::Key, RSA::Key> my(RSA::generate(rclass, 1024) );

	if(my.first.get_n() != my.second.get_n() )
		throw std::logic_error("key pair modules differ");

	if(my.second.apply(my.first.apply(1337) ) != 1337)
		throw std::logic_error("key pair does not match");

	if(RSA::decrypt(my.second, RSA::encrypt(my.first, "foobar")) !=
	   "foobar")
		throw std::logic_error("en/decryption of text fails");

	std::cout << "RSA test passed" << std::endl;
	return EXIT_SUCCESS;
}
catch(std::exception& e)
{
	std::cerr << "RSA test failed: " << e.what() << std::endl;
	return EXIT_FAILURE;
}
