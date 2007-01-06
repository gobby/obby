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

#include "rsa.hpp"

// TODO: mpz_class("ffffffffffffffff", 16) as const static member
obby::RSA::Key::Key(const mpz_class& n, const mpz_class& k)
 : m_n(n), m_k(k),
   m_id(mpz_class(n & mpz_class("ffffffffffffffff", 16)).get_str(16))
{
}

obby::RSA::Key::Key(const Key& other)
 : m_n(other.m_n), m_k(other.m_k), m_id(other.m_id)
{
}

obby::RSA::Key::~Key()
{
}

obby::RSA::Key& obby::RSA::Key::operator=(const Key& other)
{
	m_n = other.m_n;
	m_k = other.m_k;
	m_id = other.m_id;
	return *this;
}

const std::string& obby::RSA::Key::get_id() const
{
	return m_id;
}

const mpz_class& obby::RSA::Key::get_n() const
{
	return m_n;
}

const mpz_class& obby::RSA::Key::get_k() const
{
	return m_k;
}

mpz_class obby::RSA::Key::apply(const mpz_class& num) const
{
	mpz_class t;
	mpz_powm(t.get_mpz_t(), num.get_mpz_t(), m_k.get_mpz_t(),
	         m_n.get_mpz_t());
	return t;
}

namespace
{
	inline void encrypt_num_to_str(const obby::RSA::Key& key,
	                               const mpz_class& num, std::string& res)
	{
		res += key.apply(num).get_str(36);
	}

	inline void decrypt_num_to_str(const obby::RSA::Key& key,
		                       const mpz_class& num, std::string& res)
	{
		// TODO: Mit Armin klaeren, ob das wirklich stimmt...
		// Testcare, ob das nicht den String rueckwaerts gibt oder so.
		mpz_class t(key.apply(num) );
		do
		{
			res += mpz_class(t & mpz_class(0xff) ).get_ui();
			t >>= 8;
		} while(t != 0);
	}
}

std::pair<obby::RSA::Key, obby::RSA::Key> obby::RSA::generate(unsigned int bits)
{
	/* TODO... IMPLEMENT THIS */
	/* There is still room for optimisations... */
}

std::string obby::RSA::encrypt(const Key& key, const std::string& msg)
{
	mpz_class t;
	std::string result;
	for(std::string::size_type i = 0; i < msg.length(); ++i)
	{
		// Perhaps we could optimise here by checking the bit
		// count of both numbers. [TODO]
		t <<= 8;
		t |= mpz_class(msg[i]);

		// When the number gets bigger than the RSA modulus, we
		// need to split it up, otherwise the algorithm fails.
		if(t >= key.get_n() )
		{
			t >>= 8;
			encrypt_num_to_str(key, t, result);
			result += '|'; // Separation of the values
			t = msg[i];
		}
	}
	encrypt_num_to_str(key, t, result);
	return result;
}

std::string obby::RSA::decrypt(const Key& key, const std::string& msg)
{
	std::string result;
	std::string::size_type pos = 0, prev = 0;

	// Split at separations
	while((pos = msg.find('|') ) != std::string::npos)
	{
		// Decrypt number between
		decrypt_num_to_str(
			key,
		       	mpz_class(msg.substr(prev, pos - prev), 36),
		       	result
		);
	}

	// Decrypt last part
	decrypt_num_to_str(key, mpz_class(msg.substr(prev), 36), result);
	return result;
}

