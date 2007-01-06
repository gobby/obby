#include <iostream>

#define protected public
#include "text.hpp"
#undef protected

#define ARRAY_SIZE(array) (sizeof(array) / sizeof(array[0]))

using namespace obby;

namespace
{
	const user* USERS[] = {
		new user(1, "pi", obby::colour(255, 255, 0) ),
		new user(2, "pa", obby::colour(255, 255, 0) ),
		new user(3, "po", obby::colour(255, 255, 0) )
	};

	struct insert_test {
		const char* first;
		text::size_type pos;
		const char* second;
		const char* expected;
	};

	struct substr_test {
		const char* str;
		text::size_type pos;
		text::size_type len;
		const char* expected;
	};

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
		{ "[1]b[2]a", 1, "[2]f[1]o[2]o", "[1]b[2]f[1]o[2]oa" }
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

	class desc_error: public std::logic_error
	{
	public:
		desc_error(const std::string& error_message):
			std::logic_error(error_message) {}
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

		return iter1 == txt1.chunk_end() && iter2 == txt2.chunk_end();
	}

	void test_insert(const insert_test& test)
	{
		try
		{
			text base(make_text_from_desc(test.first) );
			text ins(make_text_from_desc(test.second) );

			base.insert(test.pos, ins);

			if(test.expected == NULL)
			{
				throw std::logic_error(
					"Insert should fail, but it has not"
				);
			}

			text exp(make_text_from_desc(test.expected) );
			if(compare_text(base, exp) == false)
			{
				throw std::logic_error(
					"Result does not match expectation:\n"
					"Expected " + make_desc_from_text(exp) +
					", got " + make_desc_from_text(base)
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

	void test_substr(const substr_test& test)
	{
		try
		{
			text base(make_text_from_desc(test.str) );
			text substrd = base.substr(test.pos, test.len);

			if(test.expected == NULL)
			{
				throw std::logic_error(
					"Substr should fail, but it has not"
				);
			}

			text exp(make_text_from_desc(test.expected) );
			if(compare_text(substrd, exp) == false)
			{
				throw std::logic_error(
					"Result does not match expectation:\n"
					"Expected " + make_desc_from_text(exp) +
					", got " + make_desc_from_text(base)
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

	template<typename Test, void(*TestFunc)(const Test&)>
	void test_suite(const Test* tests, std::size_t num, const char* desc)
	{
		for(std::size_t i = 0; i < num; ++ i)
		{
			try
			{
				TestFunc(tests[i]);
			}
			catch(std::exception& e)
			{
				std::cerr << desc << " test #" << (i + 1)
				          << " failed:\n" << e.what()
				          << "\n" << std::endl;

				continue;
			}

			std::cerr << desc << " test #" << (i + 1)
			          << " passed" << std::endl;
		}
	}
}

int main()
{
	test_suite<insert_test, test_insert>(
		INSERT_TESTS,
		ARRAY_SIZE(INSERT_TESTS),
		"insert"
	);

	test_suite<substr_test, test_substr>(
		SUBSTR_TESTS,
		ARRAY_SIZE(SUBSTR_TESTS),
		"substr"
	);

	return EXIT_SUCCESS;
}
