#pragma once

#include <memory>
#include "../Lib/mysql/include/mysql.h"

///////////////////////////////////////////////////////////////////////////////
// clean up functors

struct mysql_close_func_t
{
	void operator()(MYSQL* s) { if (s) mysql_close(s); }
};

struct mysql_stmt_delete_func_t
{
	void operator()(MYSQL_STMT* s) { if (s) mysql_stmt_close(s); }
};

///////////////////////////////////////////////////////////////////////////////
// smart pointer aliases

using mysql_ptr_t = std::unique_ptr< MYSQL_STMT, mysql_close_func_t >;
using mysql_stmt_ptr_t = std::unique_ptr< MYSQL_STMT, mysql_stmt_delete_func_t >;

///////////////////////////////////////////////////////////////////////////////
// statement / parameter helpers

class mysql_param_list_base;
class mysql_statement_base;

///////////////////////////////////////////////////////////////////////////////
// parameter manipulation

class mysql_param_list_base
{
private:
	MYSQL_BIND *m_params_ptr;
	int m_count;

protected:
	mysql_param_list_base(MYSQL_BIND *params, int count)
		: m_params_ptr(params), m_count(count)
	{
	}

public:
	inline virtual void bind(int index, bool &val)
	{
		bind(index, &val);
	}
	inline virtual void bind(int index, bool *val)
	{
		_ASSERT(index < m_count);
		m_params_ptr[index].buffer = val;
		m_params_ptr[index].buffer_type = MYSQL_TYPE_BIT;
	}

	// short
	inline virtual void bind(int index, int16_t &val)
	{
		bind(index, &val);
	}
	inline virtual void bind(int index, int16_t *val)
	{
		_ASSERT(index < m_count);
		m_params_ptr[index].buffer = val;
		m_params_ptr[index].buffer_type = MYSQL_TYPE_SHORT;
	}

	inline virtual void bind(int index, uint16_t &val)
	{
		bind(index, &val);
	}
	inline virtual void bind(int index, uint16_t *val)
	{
		_ASSERT(index < m_count);
		m_params_ptr[index].buffer = val;
		m_params_ptr[index].buffer_type = MYSQL_TYPE_SHORT;
		m_params_ptr[index].is_unsigned = true;
	}

	// int
	inline virtual void bind(int index, int32_t &val)
	{
		bind(index, &val);
	}
	inline virtual void bind(int index, int32_t *val)
	{
		_ASSERT(index < m_count);
		m_params_ptr[index].buffer = val;
		m_params_ptr[index].buffer_type = MYSQL_TYPE_LONG;
	}
	inline virtual void bind(int index, double &val)
	{
		bind(index, &val);
	}

	inline virtual void bind(int index, double *val)
	{
		_ASSERT(index < m_count);
		m_params_ptr[index].buffer = val;
		m_params_ptr[index].buffer_type = MYSQL_TYPE_DOUBLE;
	}

	inline virtual void bind(int index, uint32_t &val)
	{
		bind(index, &val);
	}
	inline virtual void bind(int index, uint32_t *val)
	{
		_ASSERT(index < m_count);
		m_params_ptr[index].buffer = val;
		m_params_ptr[index].buffer_type = MYSQL_TYPE_LONG;
		m_params_ptr[index].is_unsigned = true;
	}

	inline virtual void bind(int index, long &val)
	{
		bind(index, &val);
	}
	inline virtual void bind(int index, long *val)
	{
		_ASSERT(index < m_count);
		m_params_ptr[index].buffer = val;
		m_params_ptr[index].buffer_type = MYSQL_TYPE_LONG;
	}

	inline virtual void bind(int index, unsigned long &val)
	{
		bind(index, &val);
	}
	inline virtual void bind(int index, unsigned long *val)
	{
		_ASSERT(index < m_count);
		m_params_ptr[index].buffer = val;
		m_params_ptr[index].buffer_type = MYSQL_TYPE_LONG;
		m_params_ptr[index].is_unsigned = true;
	}

	inline virtual void bind(int index, void *val, size_t length, unsigned long *result_length = nullptr)
	{
		_ASSERT(index < m_count);
		m_params_ptr[index].buffer = val;
		m_params_ptr[index].buffer_length = (unsigned long)length;
		m_params_ptr[index].buffer_type = MYSQL_TYPE_BLOB;
		if (result_length)
			m_params_ptr[index].length = result_length;
	}

	inline virtual void bind(int index, std::string &val)
	{
		bind(index, val, val.length());
	}
	inline virtual void bind(int index, std::string *val)
	{
		bind(index, val, val->length());
	}
	inline virtual void bind(int index, std::string &val, size_t length, unsigned long *result_length = nullptr)
	{
		_ASSERT(index < m_count);
		if (val.capacity() < length)
			val.reserve(length);

		m_params_ptr[index].buffer = val.data();
		m_params_ptr[index].buffer_length = (unsigned long)length;
		m_params_ptr[index].buffer_type = MYSQL_TYPE_VAR_STRING;
		if (result_length)
			m_params_ptr[index].length = result_length;
	}

	inline virtual void bind(int index, char *val)
	{
		bind(index, val, strlen(val));
	}
	inline virtual void bind(int index, const char *val)
	{
		bind(index, const_cast<char*>(val), strlen(val));
	}
	inline virtual void bind(int index, char *val, size_t length, unsigned long *result_length = nullptr)
	{
		_ASSERT(index < m_count);
		m_params_ptr[index].buffer = val;
		m_params_ptr[index].buffer_length = (unsigned long)length;
		m_params_ptr[index].buffer_type = MYSQL_TYPE_VAR_STRING;
		if (result_length)
			m_params_ptr[index].length = result_length;
	}

	template<typename T>
	inline void bindarg(int index, T& arg) { bind(index, arg); }

	template<typename T>
	inline void bindargs(T& arg) { bindarg(m_count - 1, arg); }

	template<typename T, typename ...Args>
	inline void bindargs(T arg, Args&... args)
	{
		bindarg(m_count - (sizeof...(args) + 1), arg);
		bindargs(args...);
	}
};

///////////////////////////////////////////////////////////////////////////////
// statement parameters

template<int _Count>
class mysql_statement_params :
	public mysql_param_list_base
{
public:
	mysql_statement_params() :
		mysql_param_list_base(m_params, _Count),
		m_params{ { 0 } }
	{
	}

	operator MYSQL_BIND*() { return m_params; }

private:
	MYSQL_BIND m_params[_Count];
};

///////////////////////////////////////////////////////////////////////////////
// statement results

template<int _Count>
class mysql_statement_results :
	public mysql_param_list_base
{
	friend class mysql_statement_base;

public:
	mysql_statement_results() :
		mysql_param_list_base(m_cols, _Count),
		m_cols{ { 0 } }
	{
		for (int i = 0; i < _Count; i++)
		{
			m_cols[i].is_null = &m_cols[i].is_null_value;
			m_cols[i].length = &m_cols[i].length_value;
		}
	}

	bool isNull(int index)
	{
		_ASSERT(index < _Count);
		return m_cols[index].is_null_value == 0;
	}

	size_t length(int index)
	{
		_ASSERT(index < _Count);
		return m_cols[index].length_value;
	}

	bool next()
	{
		int res = mysql_stmt_fetch(m_statement);
		return res == 0;
	}

	operator MYSQL_BIND*() { return m_cols; }

private:
	MYSQL_BIND m_cols[_Count];
	MYSQL_STMT *m_statement;
};

///////////////////////////////////////////////////////////////////////////////
// statement base for friending results

class mysql_statement_base
{
protected:
	mysql_statement_base(MYSQL *dbc) : m_dbc(dbc), m_statement(dbc ? mysql_stmt_init(dbc) : nullptr), m_buffered(false)
	{  }

public:
	mysql_statement_base(mysql_statement_base&&) = default;
	mysql_statement_base& operator=(mysql_statement_base&&) = default;

	virtual ~mysql_statement_base() noexcept
	{
		if (m_statement && m_buffered)
			mysql_stmt_free_result(m_statement.get());
	}

	inline operator bool() { return (bool)m_statement; }
	inline bool valid() { return (bool)m_statement; }

	template<int _ColCount>
	bool bindResults(mysql_statement_results<_ColCount> &results)
	{
		if (mysql_stmt_bind_result(m_statement.get(), results))
			return false;

		results.m_statement = m_statement.get();

		return true;
	}

	inline std::string error() const { return mysql_error(m_dbc); }

	virtual inline bool execute() { return execute(true); }

	virtual bool execute(bool buffer)
	{
		if (mysql_stmt_execute(m_statement.get()))
			return false;

		if (buffer)
		{
			if (mysql_stmt_store_result(m_statement.get()))
				return false;

			m_buffered = true;
		}

		return true;
	}

	virtual uint64_t lastInsertId()
	{
		return mysql_insert_id(m_dbc);
	}

protected:
	MYSQL *m_dbc;
	mysql_stmt_ptr_t m_statement;
	bool m_buffered;

};

///////////////////////////////////////////////////////////////////////////////
// statement

template<int _Count>
class mysql_statement :
	public mysql_statement_base,
	public mysql_param_list_base
{
public:
	mysql_statement(MYSQL *dbc, std::string query) :
		mysql_statement_base(dbc),
		mysql_param_list_base(m_params, _Count),
		m_query(query), m_params()
	{
		if (valid())
			if (mysql_stmt_prepare(m_statement.get(), m_query.c_str(), m_query.length()))
				m_statement.reset(nullptr);
	}

	mysql_statement(mysql_statement&&) = default;
	mysql_statement& operator=(mysql_statement&&) = default;

	virtual ~mysql_statement() noexcept override = default;

	virtual bool execute() override { return mysql_statement_base::execute(); }

	virtual bool execute(bool buffer) override
	{
		if (mysql_stmt_bind_param(m_statement.get(), m_params))
			return false;

		return mysql_statement_base::execute(buffer);
	}

protected:
	std::string m_query;
	mysql_statement_params<_Count> m_params;

};

///////////////////////////////////////////////////////////////////////////////
// statement specialization for zero parameters

template<>
class mysql_statement<0> :
	public mysql_statement_base,
	public mysql_param_list_base
{
public:
	mysql_statement(MYSQL *dbc, std::string query) :
		mysql_statement_base(dbc),
		mysql_param_list_base(nullptr, 0),
		m_query(query)
	{
		if (valid())
			if (mysql_stmt_prepare(m_statement.get(), m_query.c_str(), (unsigned long)m_query.length()))
				m_statement.reset(nullptr);
	}

	virtual bool execute() override { return mysql_statement_base::execute(); }

	virtual bool execute(bool buffer) override
	{
		return mysql_statement_base::execute(buffer);
	}

protected:
	std::string m_query;
};
