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

#include <stdexcept>
#include "sha1.hpp"

namespace
{
	template<typename T>
	inline T circular_shift(T bits, T word)
	{
		return (word << bits) | (word >> (32 - bits) );
	}

	inline char hexnum(int num)
	{
		if(num < 10)
			return '0' + num;
		else
			return 'a' + (num - 10);
	}
}

obby::SHA1::SHA1()
{
	init();
}

obby::SHA1::SHA1(const uint8_t* data, size_type len)
{
	init();
	append(data, len);
}

obby::SHA1::SHA1(const std::string& input)
{
	init();
	append(input);
}

void obby::SHA1::init()
{
	m_length_low = 0;
	m_length_high = 0;
	m_message_block_index = 0;

	m_intermediate_hash[0] = 0x67452301;
	m_intermediate_hash[1] = 0xefcdab89;
	m_intermediate_hash[2] = 0x98badcfe;
	m_intermediate_hash[3] = 0x10325476;
	m_intermediate_hash[4] = 0xc3d2e1f0;
}

obby::SHA1::~SHA1()
{
}

void obby::SHA1::append(const uint8_t* data, size_type len)
{
	while(len--)
	{
		m_message_block[m_message_block_index ++] = (*data & 0xff);

		// Count message length
		m_length_low += 8;
		if(m_length_low == 0)
		{
			++ m_length_high;
			if(m_length_high == 0)
				throw std::runtime_error("Message too long");
		}

		// Process each message block every 512 bit
		if(m_message_block_index == 64)
			process_message_block();

		++ data;
	}
}

void obby::SHA1::append(const std::string& input)
{
	append(reinterpret_cast<const uint8_t*>(input.c_str()), input.length());
}

void obby::SHA1::operator<<(const std::string& input)
{
	append(reinterpret_cast<const uint8_t*>(input.c_str()), input.length());
}

std::string obby::SHA1::final()
{
	uint8_t digest;
	std::string str;
	str.resize(40);

	pad_message_block();
	for(size_type i = 0; i < 20; ++ i)
	{
		digest = m_intermediate_hash[i >> 2] >> 8 * (3 - (i & 0x03));
		str[i * 2] = hexnum(digest >> 4);
		str[i * 2 + 1] = hexnum(digest & 0xf);
	}

	return str;
}

void obby::SHA1::process_message_block()
{
	const uint32_t K[] = {
		0x5a827999,
		0x6ed9eba1,
		0x8f1bbcdc,
		0xca62c1d6
	};

	size_t t;
	uint32_t temp;
	uint32_t w[80];
	uint32_t a, b, c, d, e;

	for(t = 0; t < 16; ++ t)
	{
		w[t]  = m_message_block[4 * t + 0] << 24;
		w[t] |= m_message_block[4 * t + 1] << 16;
		w[t] |= m_message_block[4 * t + 2] << 8;
		w[t] |= m_message_block[4 * t + 3];
	}

	for(t = 16; t < 80; ++ t)
	{
		w[t] = circular_shift<uint32_t>(
			1, w[t-3] ^ w[t-8] ^ w[t-14] ^ w[t-16]);
	}

	a = m_intermediate_hash[0];
	b = m_intermediate_hash[1];
	c = m_intermediate_hash[2];
	d = m_intermediate_hash[3];
	e = m_intermediate_hash[4];

	for(t = 0; t < 20; ++ t)
	{
		temp = circular_shift<uint32_t>(5, a) +
			((b & c) | ((~b) & d)) + e + w[t] + K[0];
		e = d;
		d = c;
		c = circular_shift<uint32_t>(30, b);
		b = a;
		a = temp;
	}

	for(t = 20; t < 40; ++ t)
	{
		temp = circular_shift<uint32_t>(5, a) +
			(b ^ c ^ d) + e + w[t] + K[1];
		e = d;
		d = c;
		c = circular_shift<uint32_t>(30, b);
		b = a;
		a = temp;
	}

	for(t = 40; t < 60; ++ t)
	{
		temp = circular_shift<uint32_t>(5, a) +
			((b & c) | (b & d) | (c & d)) + e + w[t] + K[2];
		e = d;
		d = c;
		c = circular_shift<uint32_t>(30, b);
		b = a;
		a = temp;
	}

	for(t = 60; t < 80; ++ t)
	{
		temp = circular_shift<uint32_t>(5, a) +
			(b ^ c ^ d) + e + w[t] + K[3];
		e = d;
		d = c;
		c = circular_shift<uint32_t>(30, b);
		b = a;
		a = temp;
	}

	m_intermediate_hash[0] += a;
	m_intermediate_hash[1] += b;
	m_intermediate_hash[2] += c;
	m_intermediate_hash[3] += d;
	m_intermediate_hash[4] += e;

	m_message_block_index = 0;
}

void obby::SHA1::pad_message_block()
{
	if(m_message_block_index > 55)
	{
		m_message_block[m_message_block_index ++] = 0x80;
		while(m_message_block_index < 64)
			m_message_block[m_message_block_index ++] = 0;

		process_message_block();
		
		while(m_message_block_index < 56)
			m_message_block[m_message_block_index ++] = 0;
	}
	else
	{
		m_message_block[m_message_block_index ++] = 0x80;
		while(m_message_block_index < 56)
			m_message_block[m_message_block_index ++] = 0;
	}

	m_message_block[56] = (m_length_high >> 24) & 0xff;
	m_message_block[57] = (m_length_high >> 16) & 0xff;
	m_message_block[58] = (m_length_high >>  8) & 0xff;
	m_message_block[59] = (m_length_high      ) & 0xff;
	m_message_block[60] = (m_length_low  >> 24) & 0xff;
	m_message_block[61] = (m_length_low  >> 16) & 0xff;
	m_message_block[62] = (m_length_low  >>  8) & 0xff;
	m_message_block[63] = (m_length_low       ) & 0xff;

	process_message_block();
}

std::string obby::SHA1::hash(const std::string& input)
{
	SHA1 ctx(input);
	return ctx.final();
}

std::string obby::SHA1::hash(const uint8_t* data, size_type len)
{
	SHA1 ctx(data, len);
	return ctx.final();
}
