#include <iostream>
#include <stdexcept>
#include "ring.hpp"

int main() try
{
	const std::size_t COUNT = 6;

	obby::ring<unsigned int> ring(COUNT);
	if(!ring.empty() )
		throw std::logic_error("Initial ring is not empty");

	// Fill first half
	for(std::size_t i = 0; i < COUNT / 2; ++ i)
		ring.push_back(i * i);

	unsigned int loop_count = 0;
	for(obby::ring<unsigned int>::const_iterator iter = ring.begin();
	    iter != ring.end();
	    ++ iter)
	{
		if(*iter != (loop_count * loop_count) )
		{
			throw std::logic_error(
				"First half of ring could not be filled "
				"correctly"
			);
		}

		++ loop_count;
	}

	if(loop_count != COUNT / 2)
	{
		throw std::logic_error(
			"Ring contains not expected amount of elements"
		);
	}

	// Fill second half
	for(std::size_t i = 0; i < COUNT - (COUNT/2); ++ i)
		ring.push_back( (i + COUNT/2) * (i + COUNT/2) );

	loop_count = 0;
	for(obby::ring<unsigned int>::const_iterator iter = ring.begin();
	    iter != ring.end();
	    ++ iter)
	{
		if(*iter != (loop_count * loop_count) )
		{
			throw std::logic_error(
				"Second half of ring could not be filled "
				"correctly"
			);
		}

		++ loop_count;
	}
	
	if(loop_count != COUNT)
	{
		throw std::logic_error(
			"Ring contains not expected amount of elements"
		);
	}

	// Test first/last value
	if(ring.front() != 0)
		throw std::logic_error("First value is incorrect");
	if(ring.back() != (COUNT-1)*(COUNT-1) )
		throw std::logic_error("Last value is incorrect");

	// Overwrite
	for(std::size_t i = 0; i < COUNT; ++ i)
		ring.push_back( (COUNT + i) * (COUNT + i) );

	loop_count = 0;
	for(obby::ring<unsigned int>::const_iterator iter = ring.begin();
	    iter != ring.end();
	    ++ iter)
	{
		if(*iter != ( (COUNT+loop_count)*(COUNT+loop_count)) )
		{
			throw std::logic_error(
				"Overwrite operation failed"
			);
		}

		++ loop_count;
	}

	loop_count = 0;
	while(!ring.empty() )
	{
		ring.pop_back();
		++ loop_count;
	}

	if(loop_count != COUNT)
		throw std::logic_error("Failed clearing ring");

	std::cout << "Ring test passed" << std::endl;
	return EXIT_SUCCESS;
}
catch(std::exception& e)
{
	std::cerr << "Ring test failed: " << e.what() << std::endl;
	return EXIT_FAILURE;
}
