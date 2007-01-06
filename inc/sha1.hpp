/* libobby - Network text editing library
 * Copyright (C) 2005 0x539 dev group
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this program; if not, write to the Free
 * Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#ifndef _OBBY_SHA1_HPP_
#define _OBBY_SHA1_HPP_

#include <string>
#include <inttypes.h>

namespace obby
{

/** SHA1 hash algorithm according to RFC 3174.
 */

class SHA1 {
public:
	typedef size_t size_type;

	/** Creates an empty SHA1 context.
	 */
	SHA1();

	/** Create SHA1 context with initial content.
	 */
	SHA1(const uint8_t* data, size_type len);

	/** Create SHA1 context with initial string context.
	 */
	SHA1(const std::string& input);
	~SHA1();

	/** Appends data to be hashed.
	 */
	void append(const uint8_t* data, size_type len);

	/** Appends string data to be hashed.
	 */
	void append(const std::string& input);

	/** Alias for append().
	 */
	void operator<<(const std::string& input);

	/** Finalises this SHA1 context and returns the SHA1 digest string
	 * of the data that has previousley added using append().
	 */
	std::string final();

	/** Returns the SHA1 hash sum of the given string.
	 */
	static std::string hash(const std::string& input);

	/** Returns the SHA1 hash sum of the given data.
	 */
	static std::string hash(const uint8_t* data, size_type len);
private:
	void init();

	void process_message_block();
	void pad_message_block();

	uint32_t m_intermediate_hash[5];
	uint32_t m_length_low, m_length_high;

	int_least16_t m_message_block_index;
	uint8_t m_message_block[64];
};

}

#endif // _OBBY_SHA1_HPP_

