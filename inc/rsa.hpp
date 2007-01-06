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
		Key();
		Key(const mpz_class& n, const mpz_class& k);
		Key(const Key& other);
		~Key();

		Key& operator =(const Key& other);

		/** Checks if the key is empty.
		 */
		bool empty() const;

		/** Checks if the key is valid, e.g. !empty().
		 */
		operator bool() const;

		/** Alias for empty().
		 */
		bool operator !() const;

		/** Compares two keys for equalness.
		 */
		bool operator ==(const Key& other);
		bool operator !=(const Key& other);

		/** Returns a unique ID (Fingerprint) for this key.
		 */
		const std::string& get_id() const;

		/** Returns the modulus of the key.
		 */
		const mpz_class& get_n() const;

		/** Returns the factor (e or d) of the key.
		 */
		const mpz_class& get_k() const;

		void set_n(const mpz_class& n);
		void set_k(const mpz_class& k);

		/** Applies the key components using the RSA algorithm to a
		 * number to generate a new encrypted one.
		 */
		mpz_class apply(const mpz_class& num) const;

	private:
		/** 2 ** 64.
		 */
		static const mpz_class _2e64;

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

