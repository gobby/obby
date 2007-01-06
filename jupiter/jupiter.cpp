// jupiter.cpp
// 0x539 dev group, 2005
// based on papers of the ACE project, see http://ace.iserver.ch

#include <cerrno>
#include <string>
#include <iostream>
#include <fstream>
#include <stdexcept>
#include <list>
#include <vector>
#include <sstream>

typedef std::string::size_type position;
typedef std::string document;

class operation
{
public:
	operation(operation* original = NULL)
	 : m_original(original) { }

	virtual ~operation() { delete m_original; }

	virtual operation* clone() const = 0;

	virtual void apply(document& doc) const = 0;

	virtual operation* transform(const operation& base_op, bool client) const = 0;

	virtual operation* transform_insert(position pos, const std::string& text, bool client) const = 0;
	virtual operation* transform_delete(position pos, position len, bool client) const = 0;

protected:
	operation* clone_original() const
	{
		// TODO: refcount m_original
		//if(m_original == NULL) // TODO: Enable this if required
			return NULL;
		return m_original->clone();
	}

	operation* m_original;
};

class no_operation : public operation
{
public:
	no_operation(operation* original = NULL)
	 : operation(original) { }

	virtual operation* clone() const {
		return new no_operation(clone_original() );
	}

	virtual void apply(document& doc) const { /* does nothing */ }

	virtual operation* transform(const operation& base_op, bool client) const
	{
		// no-op
		return base_op.clone();
	}

	virtual operation* transform_insert(position pos, const std::string& text, bool client) const
	{
		// nothing happens to no-op
		return clone();
	}

	virtual operation* transform_delete(position pos, position len, bool client) const
	{
		// nothing happens to no-op
		return clone();
	}

protected:
};

class split_operation : public operation
{
public:
	split_operation(operation* first, operation* second,
	                operation* original)
	 : operation(original), m_first(first), m_second(second) { }

	virtual ~split_operation() { delete m_first; delete m_second; }

	const operation* get_first() const { return m_first; }
	const operation* get_second() const { return m_second; }

	virtual operation* clone() const {
		return new split_operation(
			m_first->clone(), m_second->clone(), clone_original()
		);
	}

	virtual void apply(document& doc) const
	{
		m_second->apply(doc);
		m_first->apply(doc);

		// Transform second operation with first because the first one
		// has already been applied to the document.
	//	operation* second = m_first->transform(*m_second, true);
	//	second->apply(doc);
	//	delete second;
	}

	virtual operation* transform(const operation& base_op, bool client) const
	{
		// Transform base_op against first operation
		operation* op1 = m_second->transform(base_op, client);
		// Transform result against second
		operation* op2 = m_first->transform(*op1, client);
		// Delete first, no longer needed
		delete op1;
		// Return result
		return op2;
	}

	virtual operation* transform_insert(position pos, const std::string& text, bool client) const
	{	return new split_operation(
			m_first->transform_insert(pos, text, client),
			m_second->transform_insert(pos, text, client),
			clone() // TODO: Does a split operation need an original op?
		);
	}

	virtual operation* transform_delete(position pos, position len, bool client) const
	{
		return new split_operation(
			m_first->transform_delete(pos, len, client),
			m_second->transform_delete(pos, len, client),
			clone() // TODO: Does a split operation need an original op?
		);
	}
protected:
	operation* m_first;
	operation* m_second;
};


class insert_operation : public operation
{
public:
	insert_operation(position pos, const std::string& text,
	                 operation* original)
	 : operation(original), m_pos(pos), m_text(text) { }

	virtual operation* clone() const {
		return new insert_operation(m_pos, m_text, clone_original() );
	}

	virtual void apply(document& doc) const
	{
		if(m_pos > doc.length() )
			throw std::logic_error("insert_operation::apply");

		doc.insert(m_pos, m_text);
	}

	virtual operation* transform(const operation& base_op, bool client) const
	{
		return base_op.transform_insert(m_pos, m_text, client);
	}

	// Insert/Insert
	virtual operation* transform_insert(position pos, const std::string& text, bool client) const
	{
		if(m_pos < pos)
		{
			// Case 1
			return new insert_operation(m_pos, m_text, clone() );
		}
		else if(m_pos == pos)
		{
			// Special case
			// TODO!
			if(client)
				return new insert_operation(m_pos, m_text, clone() );
			else
				return new insert_operation(m_pos + m_text.length(), m_text, clone() );
		}
		else
		{
			// Case 2
			return new insert_operation(
				m_pos + text.length(),
				m_text,
				clone()
			);
		}
	}

	virtual operation* transform_delete(position pos, position len, bool client) const
	{
		if(m_pos <= pos)
		{
			// Case 3
			return new insert_operation(
				m_pos,
				m_text,
				clone()
			);
		}
		else if(m_pos > pos + len)
		{
			// Case 4
			return new insert_operation(
				m_pos - len,
				m_text,
				clone()
			);
		}
		else
		{
			// Case 5
			return new insert_operation(
				pos,
				m_text,
				clone()
			);
		}
	}

protected:
	position m_pos;
	std::string m_text;
};

class delete_operation : public operation
{
public:
	delete_operation(position pos, const std::string& text,
	                 operation* original)
	 : operation(original), m_pos(pos), m_text(text) {} 

	virtual operation* clone() const {
		return new delete_operation(m_pos, m_text, clone_original() );
	}

	virtual void apply(document& doc) const
	{
		if(m_pos + m_text.length() > doc.length() )
			throw std::logic_error("delete_operation::apply");

		if(doc.substr(m_pos, m_text.length() ) != m_text)
			throw std::logic_error("delete_operation::apply");

		doc.erase(m_pos, m_text.length() );
	}

	virtual operation* transform(const operation& base_op, bool client) const
	{
		return base_op.transform_delete(m_pos, m_text.length(), client);
	}

	virtual operation* transform_insert(position pos, const std::string& text, bool client) const
	{
		if(m_pos + m_text.length() < pos)
		{
			// Case 6
			return new delete_operation(
				m_pos,
				m_text,
				clone()
			);
		}
		else if(pos <= m_pos)
		{
			// Case 7
			return new delete_operation(
				m_pos + text.length(),
				m_text,
				clone()
			);
		}
		else
		{
			// Case 8
			return new split_operation(
				new delete_operation(
					m_pos,
					m_text.substr(0, pos - m_pos),
					NULL
				),
				new delete_operation(
					pos + text.length(),
					m_text.substr(pos - m_pos),
					NULL
				),
				clone()
			);
		}
	}

	virtual operation* transform_delete(position pos, position len, bool client) const
	{
		if(m_pos + m_text.length() < pos)
		{
			// Case 9
			return new delete_operation(
				m_pos,
				m_text,
				clone()
			);
		}
		else if(m_pos >= pos + len)
		{
			// Case 10
			return new delete_operation(
				m_pos - len,
				m_text,
				clone()
			);
		}
		else if(pos <= m_pos && pos + len >= m_pos + m_text.length() )
		{
			// Case 11
			return new no_operation(clone() );
		}
		else if(pos <= m_pos && pos + len < m_pos + m_text.length() )
		{
			// Case 12
			return new delete_operation(
				pos,
				m_text.substr(pos + len - m_pos),
				clone()
			);
		}
		else if(pos > m_pos && pos + len >= m_pos + m_text.length() )
		{
			// Case 13
			return new delete_operation(
				m_pos,
				m_text.substr(0, pos - m_pos),
				clone()
			);
		}
		else
		{
			// Case 14
			return new delete_operation(
				m_pos,
				m_text.substr(0, pos - m_pos) +
				m_text.substr(pos + len - m_pos),
				clone()
			);
		}
	}
protected:
	position m_pos;
	std::string m_text;
};

template<typename time_type>
class basic_vector_time
{
public:
	basic_vector_time(time_type local, time_type remote)
	 : m_local_count(local), m_remote_count(remote) { }

	bool operator==(const basic_vector_time& other)
	{
		return other.m_local_count == m_local_count &&
		       other.m_remote_count == m_remote_count;
	}

	bool operator!=(const basic_vector_time& other)
	{
		return other.m_local_count != m_local_count ||
		       other.m_remote_count != m_remote_count;
	}

	/* The following have no practical use, just to use vector_time in
	 * a std::map. The local count is preferred to the remote count
	 */
	bool operator<(const basic_vector_time& other);
	bool operator<=(const basic_vector_time& other);
	bool operator>(const basic_vector_time& other);
	bool operator>=(const basic_vector_time& other);

	time_type get_local_count() const { return m_local_count; }
	time_type get_remote_count() const { return m_remote_count; }

	time_type inc_local_count() { return ++m_local_count; }
	time_type inc_remote_count() { return ++m_remote_count; }

	virtual std::string to_string() const
	{
		std::stringstream stream;
		stream << m_local_count << "/" << m_remote_count;
		return stream.str();
	}
private:
	time_type m_local_count;
	time_type m_remote_count;
};

/* vector_time type used within obby */
typedef basic_vector_time<unsigned int> vector_time;

class record
{
public:
	record(unsigned int from, vector_time timestamp, const operation& op)
	 : m_from(from), m_timestamp(timestamp), m_operation(op.clone()) { }
	~record() { delete m_operation; }

	operation* get_operation() const { return m_operation; }
	const vector_time& get_time() const { return m_timestamp; }
protected:
	unsigned int m_from;
	vector_time m_timestamp;
	operation* m_operation;
};

class inclusion_transformation
{
public:
	/** Includes the effect of trans_op into base_op.
	 */
	operation* transform(const operation& base_op,
	                     const operation& trans_op,
	                     bool client)
	{
		return trans_op.transform(base_op, client);
	}
protected:
};

class algorithm
{
public:
	class operation_wrapper
	{
	public:
		operation_wrapper(const operation& op, unsigned int count)
		 : m_operation(op.clone() ), m_count(count) { }
		~operation_wrapper() { delete m_operation; }

		operation* get_operation() const { return m_operation; }
		unsigned int get_count() const { return m_count; }

		void reset_operation(const operation& new_op)
			{ delete m_operation; m_operation = new_op.clone(); }

	protected:
		operation* m_operation;
		unsigned int m_count;
	};

	typedef std::list<operation_wrapper*> acklist;

	algorithm(std::string& document, unsigned int id, bool client)
	 : m_document(document), m_time(0, 0), m_id(id), m_client(client) { }

	/** Perform a local operation. The record to be sent to other users
	 * is returned.
	 */
	std::auto_ptr<record> local_op(const operation& op)
	{
		// Apply op locally
		op.apply(m_document);
		// Generate request
		std::auto_ptr<record> rec(new record(m_id, m_time, op) );
		// Add to outgoing queue
		add_ack_request(op);
		// Generated new message
		m_time.inc_local_count();
		return rec;
	}

	/** Receives a remote operation request.
	 */
	void remote_op(const record& rec)
	{
		// Check preconditions before transforming
		check_preconditions(rec);
		// Discard acknowledged operations
		discard_operations(rec);
		// Transform operation
		std::auto_ptr<const operation> op(transform(*rec.get_operation() ));
		// Apply new operation
		op->apply(m_document);
		// Got new message
		m_time.inc_remote_count();
	}

	/** Adds an operation to the ack request list.
	 */
	void add_ack_request(const operation& op)
	{
		const split_operation* split_op =
			dynamic_cast<const split_operation*>(&op);

		if(split_op == NULL)
		{
			m_ack_list.push_back(
				new operation_wrapper(
					op,
					m_time.get_local_count()
				)
			);
		}
		else
		{
			add_ack_request(*split_op->get_first() );
			add_ack_request(*split_op->get_second() );
		}
	}

	/** Discard from the other side acknowledged operations.
	 */
	void discard_operations(const record& rec)
	{
		for(acklist::iterator iter = m_ack_list.begin();
		    iter != m_ack_list.end();)
		{
			if( (*iter)->get_count() <
			    rec.get_time().get_remote_count() )
				iter = m_ack_list.erase(iter);
			else
				++ iter;
		}

		if(rec.get_time().get_local_count() !=
		   m_time.get_remote_count() )
			throw std::logic_error("algorithm::discard_operations");
	}

	operation* transform(const operation& op)
	{
		operation* new_op = op.clone();

		for(acklist::const_iterator iter = m_ack_list.begin();
		    iter != m_ack_list.end(); ++ iter)
		{
			// Get current existing operation
			operation* existing_op = (*iter)->get_operation();
			// Transform new operation against existing one
			operation* new_trans_op = m_it.transform(
				*new_op,
				*existing_op,
				m_client
			);
			// Trasnform existing operation against new one
			operation* existing_trans_op = m_it.transform(
				*existing_op,
				*new_op,
				!m_client
			);
			// Replace existing operation by transformed one
			(*iter)->reset_operation(*existing_trans_op);
			// Replace new operation by transformed one
			delete new_op; new_op = new_trans_op;
		}

		return new_op;
	}

	void check_preconditions(const record& rec) const
	{
		if(!m_ack_list.empty() && rec.get_time().get_remote_count() <
		   m_ack_list.front()->get_count() )
			throw std::logic_error("algorithm::check_preconditions (#1)");

		if(rec.get_time().get_remote_count() > m_time.get_local_count() )
			throw std::logic_error("algorithm::check_preconditions (#2)");

		if(rec.get_time().get_local_count() != m_time.get_remote_count() )
			throw std::logic_error("algorithm::check_preconditions (#3)");
	}
protected:
	document& m_document;
	inclusion_transformation m_it;
	vector_time m_time;
	unsigned int m_id;
	bool m_client;
	acklist m_ack_list;
};

// TEST
class test_scenario
{
public:
	static void test(const std::string& line)
	{
		// Split into init, operations, result
		std::vector<std::string> fs = split_line(line, "|");
		if(fs.size() != 3)
			throw std::runtime_error("Expected 3 sections");

		std::string init = fs[0];
		std::string result = fs[2];

		std::string doc1(init), doc2(init);
		algorithm site1(doc1, 1, false), site2(doc2, 2, true);

		std::list<record*> rec1list;
		std::list<record*> rec2list;

		std::vector<std::string> ops = split_line(fs[1], ",");
		for(std::vector<std::string>::size_type i = 0; i < ops.size(); ++ i)
		{
			if(ops[i].empty() ) continue;

			std::vector<std::string> vs = split_line(ops[i], "->");
			if(vs.size() != 2)
				throw std::runtime_error("Expected site->op");

			unsigned long site = strtoul(vs[0].c_str(), NULL, 0);
			if(site < 1 || site > 2)
				throw std::runtime_error("Site must be 1 or 2");

			std::vector<std::string> op = split_line(vs[1], "(");
			if(op.size() != 2)
				throw std::runtime_error("Expected op(desc)");

			if(op[0] != "ins" && op[0] != "del")
				throw std::runtime_error("Unsupported operation '" + op[0] + "'");

			operation* new_op = NULL;
			if(op[0] == "ins")
			{
				std::vector<std::string> desc = split_line(op[1], "@");
				if(desc.size() != 2)
					throw std::runtime_error("Expected ins(text@position)");
				
				errno = 0;
				std::string text = desc[0];
				position pos = strtoul(desc[1].c_str(), NULL, 0);

				if(errno != 0)
					throw std::runtime_error("Expected numerical position");

				new_op = new insert_operation(pos, text, NULL);
			}
			else
			{
				std::vector<std::string> desc = split_line(op[1], "-");
				if(desc.size() != 2)
					throw std::runtime_error("Expected del(from-to)");
				
				errno = 0;
				position from = strtoul(desc[0].c_str(), NULL, 0);
				if(errno != 0)
					throw std::runtime_error("Expected numerical position");

				errno = 0;
				position to = strtoul(desc[1].c_str(), NULL, 0);
				if(errno != 0)
					throw std::runtime_error("Expected numerical position");

				if(site == 1)
					new_op = new delete_operation(from, doc1.substr(from, to - from), NULL);
				else
					new_op = new delete_operation(from, doc2.substr(from, to - from), NULL);
			}

			// Apply OP
			if(site == 1)
				apply_local(site1, new_op, rec1list);
			else
				apply_local(site2, new_op, rec2list);

			delete new_op;
		}

		// Apply changes remotely
		apply_remote(site1, rec2list);
		apply_remote(site2, rec1list);

		// Compare results
		if(doc1 != doc2 || doc1 != result)
			throw std::runtime_error("failed: Docs were \"" + doc1 + "\" and \"" + doc2 + "\", but expected \"" + result + "\"");
	}

	static void apply_local(algorithm& algo, operation* op, std::list<record*>& reclist)
	{
		std::auto_ptr<record> rec = algo.local_op(*op);
		reclist.push_back(rec.get() );
		rec.release();
	}

	static void apply_remote(algorithm& algo, std::list<record*>& reclist)
	{
		for(std::list<record*>::iterator iter = reclist.begin(); iter != reclist.end(); ++ iter)
		{
			algo.remote_op(**iter);
		}
	}

	static std::vector<std::string> split_line(const std::string& line, const std::string& separator)
	{
		std::vector<std::string> result;
		std::string::size_type pos = 0, prev = 0;
		while( (pos = line.find(separator, pos)) != std::string::npos)
		{
			result.push_back(line.substr(prev, pos - prev) );

			pos += separator.length();
			prev = pos;
		}
		result.push_back(line.substr(prev) );
		return result;
	}

protected:
};

int main(int argc, char* argv[])
{
	std::cout << argv[0] << std::endl;
	std::cout << "Program to test the Jupiter implementation" << std::endl;
	std::cout << "See the file \"base_file\" for how to build tests" << std::endl;
	std::cout << std::endl;

	if(argc < 2)
	{
		std::cerr << "Usage: " << argv[0] << " test-file" << std::endl;
		return EXIT_FAILURE;
	}

	std::ifstream file(argv[1]);
	std::string line; unsigned int count = 0; unsigned int success = 0;
	unsigned int line_num = 0;
	while(std::getline(file, line) )
	{
		++ line_num;
		if(line.empty() )
			continue;
		if(line[0] == '#')
			continue;

		++ count;
		std::cout << "Test " << count << "(" << line_num << "): ";

		try
		{
			test_scenario::test(line);
		}
		catch(std::exception& e)
		{
			std::cout << e.what() << std::endl;
			continue;
		}

		std::cout << "passed!" << std::endl;
		++ success;
	}

	std::cout << success << " out of " << count << " tests passed!" << std::endl;
	return EXIT_SUCCESS;
}

