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

#ifndef _OBBY_RSA_HPP_
#define _OBBY_RSA_HPP_

#include <string>
#include <gmpxx.h>

namespace obby
{

namespace RSA
{
	class Key
	{
	public:
		Key(const mpz_class& n, const mpz_class& k);
		Key(const Key& other);
		~Key();

		Key& operator=(const Key& other);

		const std::string& get_id() const;
		const mpz_class& get_n() const;
		const mpz_class& get_k() const;

		/** Applies the key components using the RSA algorithm to a
		 * number to generate a new encrypted one.
		 */
		mpz_class apply(const mpz_class& num) const;

	private:
		mpz_class m_n, m_k;
		std::string m_id;
	};

	/** Generates a public/private key pair with a given bit count.
	 * <em>rclass</em> is a GMP randomness context. The function
	 * returns the public key as the first, the private key as the
	 * second element in the pair.
	 */
	std::pair<Key, Key> generate(gmp_randclass& rclass, unsigned int bits);

	/** Encrypts a string with a given key to another, binary-safe
	 * string representation.
	 */
	std::string encrypt(const Key& key, const std::string& msg);

	/** Decrypts a string with a given key to the original one,
	 * if the key matches.
	 */
	std::string decrypt(const Key& key, const std::string& msg);
};

}

#endif // _OBBY_RSA_HPP_

