#include <iostream>

#define protected public
#include "text.hpp"
#undef protected

#define ARRAY_SIZE(array) (sizeof(array) / sizeof(array[0]))

using namespace obby;

namespace
{
	class desc_error: public std::logic_error
	{
	public:
		desc_error(const std::string& error_message):
			std::logic_error(error_message) {}
	};

	const user* USERS[] = {
		new user(1, "pi", obby::colour(255, 255, 0) ),
		new user(2, "pa", obby::colour(255, 255, 0) ),
		new user(3, "po", obby::colour(255, 255, 0) )
	};

	text make_text_from_desc(const std::string& desc)
	{
		text my_text;

		if(desc.empty() )
			return my_text;

		std::string::size_type pos = 0;
		const user* cur_user = NULL;

		std::string::size_type prev = 0;
		while( (pos = desc.find('[', pos)) != std::string::npos)
		{
			if(pos > prev)
			{
				if(cur_user == NULL)
					throw desc_error("Unusered chunk");

				my_text.m_chunks.push_back(
					new text::chunk(
						desc.substr(prev, pos - prev),
						cur_user
					)
				);
			}

			text::size_type close = desc.find(']', pos);
			if(close == std::string::npos)
				throw std::logic_error("Missing ']'");

			cur_user = USERS[desc[pos + 1] - '1'];
			pos = prev = close + 1;
		}

		if(cur_user == NULL)
			throw desc_error("Unusered chunk");

		my_text.m_chunks.push_back(
			new text::chunk(desc.substr(prev), cur_user)
		);

		return my_text;
	}

	std::string make_desc_from_text(const text& txt)
	{
		std::string str;

		for(text::chunk_iterator iter = txt.chunk_begin();
		    iter != txt.chunk_end();
		    ++ iter)
		{
			str += "[";
			str += iter->get_author()->get_id() + '0';
			str += "]";
			str += iter->get_text();
		}

		return str;
	}

	struct insert_test {
		static const char* NAME;

		const char* first;
		const text::size_type pos;
		const char* second;
		const char* expected;

		text perform() const
		{
			text base(make_text_from_desc(first) );
			base.insert(pos, make_text_from_desc(second) );
			return base;
		}
	};

	struct substr_test {
		static const char* NAME;

		const char* str;
		const text::size_type pos;
		const text::size_type len;
		const char* expected;

		text perform() const
		{
			text base(make_text_from_desc(str) );
			return base.substr(pos, len);
		}
	};

	struct erase_test {
		static const char* NAME;

		const char* str;
		const text::size_type pos;
		const text::size_type len;
		const char* expected;

		text perform() const
		{
			text base(make_text_from_desc(str) );
			base.erase(pos, len);
			return base;
		}
	};

	struct append_test {
		static const char* NAME;

		const char* str;
		const char* app;
		const char* expected;

		text perform() const
		{
			text base(make_text_from_desc(str) );
			base.append(make_text_from_desc(app) );
			return base;
		}
	};

	struct prepend_test {
		static const char* NAME;

		const char* str;
		const char* pre;
		const char* expected;

		text perform() const
		{
			text base(make_text_from_desc(str) );
			base.prepend(make_text_from_desc(pre) );
			return base;
		}
	};

	const char* insert_test::NAME = "insert";
	const char* substr_test::NAME = "substr";
	const char* erase_test::NAME = "erase";
	const char* append_test::NAME = "append";
	const char* prepend_test::NAME = "prepend";

	insert_test INSERT_TESTS[] = {
		{ "", 0, "[1]bar", "[1]bar" },
		{ "", 1, "[1]bar", NULL },
		{ "[1]foo", 3, "[1]bar", "[1]foobar" },
		{ "[1]foo", 0, "[1]bar", "[1]barfoo" },
		{ "[1]for", 2, "[1]oba", "[1]foobar" },
		{ "[1]foo", 0, "[2]bar", "[2]bar[1]foo" },
		{ "[1]foo", 4, "[1]bar", NULL },
		{ "[1]gnah", 3, "[2]gnah", "[1]gna[2]gnah[1]h" },
		{ "[1]gnah", 4, "[2]gnah", "[1]gnah[2]gnah" },
		{ "[1]b[2]a", 1, "[1]foo", "[1]bfoo[2]a" },
		{ "[1]b[2]a", 1, "[2]foo", "[1]b[2]fooa" },
		{ "[1]b[2]a", 1, "[1]f[2]oo", "[1]bf[2]ooa" },
		{ "[1]b[2]a", 1, "[2]f[1]oo", "[1]b[2]f[1]oo[2]a" },
		{ "[1]b[2]a", 1, "[1]f[2]o[1]o", "[1]bf[2]o[1]o[2]a" },
		{ "[1]b[2]a", 1, "[2]f[1]o[2]o", "[1]b[2]f[1]o[2]oa" },
		{ "[1]Die Frage[2] ist halt,[3] [2]ob[1] das so", 11, "[3]n", "[1]Die Frage[2] i[3]n[2]st halt,[3] [2]ob[1] das so" }
	};

	substr_test SUBSTR_TESTS[] = {
		{ "", 0, 0, "" },
		{ "[1]bar", 0, 3, "[1]bar" },
		{ "[1]bar", 0, 1, "[1]b" },
		{ "[1]bar", 0, 0, "" },
		{ "[1]b[2]a[1]r", 0, 3, "[1]b[2]a[1]r" },
		{ "[1]b[2]a[1]r", 0, 2, "[1]b[2]a" },
		{ "[1]b[2]a[1]r", 1, 1, "[2]a" },
		{ "[1]foo[2]bar", 0, 3, "[1]foo" },
		{ "[1]foo[2]bar", 3, 3, "[2]bar" },
		{ "[1]foo[2]bar", 1, 3, "[1]oo[2]b" },
		{ "[1]foo[2]bar", 2, 3, "[1]o[2]ba" },
		{ "[1]foo[2]bar", 0, 4, "[1]foo[2]b" },
		{ "[1]foo[2]bar", 1, 4, "[1]oo[2]ba" },
		{ "[1]foo[2]bar", 2, 4, "[1]o[2]bar" },
		{ "[1]foo[2]bar", 1, 2, "[1]oo" },
		{ "[1]foo[2]bar", 2, 2, "[1]o[2]b" },
		{ "[1]foo[2]bar", 3, 2, "[2]ba" },
		{ "[1]foo[2]bar", 5, 2, NULL },
		{ "[1]foo[2]bar", 2, text::npos, "[1]o[2]bar" },
		{ "[1]foo[2]bar", 3, text::npos, "[2]bar" }
	};

	erase_test ERASE_TESTS[] = {
		{ "", 0, 0, "" },
		{ "", 1, 0, NULL },
		{ "", 0, 1, NULL },
		{ "[1]foo", 0, 1, "[1]oo" },
		{ "[1]foo", 1, 1, "[1]fo" },
		{ "[1]foo", 1, 2, "[1]f" },
		{ "[1]foo", 0, 3, "" },
		{ "[1]foo[2]bar", 0, 1, "[1]oo[2]bar" },
		{ "[1]foo[2]bar", 1, 1, "[1]fo[2]bar" },
		{ "[1]foo[2]bar", 2, 1, "[1]fo[2]bar" },
		{ "[1]foo[2]bar", 3, 1, "[1]foo[2]ar" },
		{ "[1]foo[2]bar", 4, 1, "[1]foo[2]br" },
		{ "[1]foo[2]bar", 5, 1, "[1]foo[2]ba" },
		{ "[1]foo[2]bar", 0, 3, "[2]bar" },
		{ "[1]foo[2]bar", 1, 3, "[1]f[2]ar" },
		{ "[1]foo[2]bar", 2, 3, "[1]fo[2]r" },
		{ "[1]foo[2]bar", 3, 3, "[1]foo" },
		{ "[1]foo[2]bar[1]baz", 2, 3, "[1]fo[2]r[1]baz" },
		{ "[1]foo[2]bar[1]baz", 3, 3, "[1]foobaz" },
		{ "[1]foo[2]bar[1]baz", 4, 3, "[1]foo[2]b[1]az" },
		{ "[1]foo[2]b[3]az[1]o", 2, 5, "[1]fo" },
		{ "[1]foo[2]bar[3]baz[2]qux[3]fo[2]gneh[1]grah", 0, 10, "[2]ux[3]fo[2]gneh[1]grah" }
	};

	append_test APPEND_TESTS[] = {
		{ "", "", "" },
		{ "", "[1]bar", "[1]bar" },
		{ "", "[1]bar[2]foo", "[1]bar[2]foo" },
		{ "[1]foo[2]bar", "[1]bar[2]foo", "[1]foo[2]bar[1]bar[2]foo" },
		{ "[1]foo[2]bar", "[2]bar[1]foo", "[1]foo[2]barbar[1]foo" }
	};

	prepend_test PREPEND_TESTS[] = {
		{ "", "", "" },
		{ "", "[1]bar", "[1]bar" },
		{ "", "[1]bar[2]foo", "[1]bar[2]foo" },
		{ "[1]foo[2]bar", "[1]bar[2]foo", "[1]bar[2]foo[1]foo[2]bar" },
		{ "[1]foo[2]bar", "[2]bar[1]foo", "[2]bar[1]foofoo[2]bar" }
	};

	bool compare_text(const text& txt1, const text& txt2)
	{
		text::chunk_iterator iter1 = txt1.chunk_begin();
		text::chunk_iterator iter2 = txt2.chunk_begin();

		while(iter1 != txt1.chunk_end() && iter2 != txt2.chunk_end() )
		{
			if(iter1->get_author() != iter2->get_author() )
				return false;
			if(iter1->get_text() != iter2->get_text() )
				return false;

			++ iter1; ++ iter2;
		}

		bool match =
			(iter1 == txt1.chunk_end() &&
			 iter2 == txt2.chunk_end());

		if(match && (txt1 != txt2) )
		{
			throw std::logic_error(
				"compare_text is equal, but txt1 == txt2 "
				"not!\n" + make_desc_from_text(txt1) + " == " +
				make_desc_from_text(txt2)
			);
		}

		return match;
	}

	template<typename Test>
	void test_generic(const Test& test)
	{
		try
		{
			text result = test.perform();

			if(test.expected == NULL)
			{
				throw std::logic_error(
					std::string(Test::NAME) + " should "
					"fail, but it has not"
				);
			}

			text exp(make_text_from_desc(test.expected) );
			if(compare_text(result, exp) == false)
			{
				throw std::logic_error(
					"Result does not match expectation:\n"
					"Expected " + make_desc_from_text(exp) +
					", got " + make_desc_from_text(result)
				);
			}
		}
		catch(desc_error& e)
		{
			throw e;
		}
		catch(std::logic_error& e)
		{
			if(test.expected != NULL)
				throw e;
		}
	}

	template<typename Test>
	void test_suite(const Test* tests, std::size_t num)
	{
		for(std::size_t i = 0; i < num; ++ i)
		{
			try
			{
				test_generic(tests[i]);
			}
			catch(std::exception& e)
			{
				std::cerr << Test::NAME << " test #" << (i + 1)
				          << " failed:\n" << e.what()
				          << "\n" << std::endl;

				continue;
			}

			std::cerr << Test::NAME << " test #" << (i + 1)
			          << " passed" << std::endl;
		}
	}
}

int main()
{
	test_suite(INSERT_TESTS, ARRAY_SIZE(INSERT_TESTS) );
	test_suite(SUBSTR_TESTS, ARRAY_SIZE(SUBSTR_TESTS) );
	test_suite(ERASE_TESTS, ARRAY_SIZE(ERASE_TESTS) );
	test_suite(APPEND_TESTS, ARRAY_SIZE(APPEND_TESTS) );
	test_suite(PREPEND_TESTS, ARRAY_SIZE(PREPEND_TESTS) );

	return EXIT_SUCCESS;
}
