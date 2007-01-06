#include <iostream>
#include <cassert>
#include <gmpxx.h>
#include "rsa.hpp"

using namespace obby;

int main()
{
	gmp_randclass rclass(GMP_RAND_ALG_LC, 16);
	rclass.seed(std::time(NULL) );
	std::pair<RSA::Key, RSA::Key> my(RSA::generate(rclass, 1024) );
	assert(my.first.get_n() == my.second.get_n() );
	std::cout << "n      : " << my.first.get_n() << std::endl;
	std::cout << "pub    : " << my.first.get_k() << std::endl;
	std::cout << "priv   : " << my.second.get_k() << std::endl;
	std::cout << std::endl;
	std::cout << "apply  : " << my.first.apply(1337) << std::endl;
	std::cout << "unapply: " << my.second.apply(my.first.apply(1337) ) << std::endl;
	std::cout << std::endl;
	std::cout << "encrypt: " << RSA::encrypt(my.first, "foobar") << std::endl;
	std::cout << "decrypt: " << RSA::decrypt(my.second, RSA::encrypt(my.first, "foobar") ) << std::endl;
}

