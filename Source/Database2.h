#pragma once

#include <condition_variable>
#include <mutex>
#include <thread>

#include "DatabaseUtil.h"

#define MYSQL_CONNECT_TIMEOUT 3

typedef char** SQLResultRow_t;

class CSQLResult
{
public:
	CSQLResult() { }
	virtual ~CSQLResult() { }

	virtual SQLResultRow_t FetchRow() = 0;
	virtual uint64_t ResultRows() = 0;

	static unsigned int SafeUInt(const char *value)
	{
		return value ? strtoul(value, NULL, 10) : 0;
	}
	static int SafeInt(const char *value)
	{
		return value ? atoi(value) : 0;
	}

	static const char *SafeString(const char *value)
	{
		return value ? value : "";
	}
};

class CMYSQLResult : public CSQLResult
{
public:
	CMYSQLResult(MYSQL_RES *);
	virtual ~CMYSQLResult();
	virtual void Free();

	virtual SQLResultRow_t FetchRow() override;
	virtual uint64_t ResultRows() override;

private:
	MYSQL_RES *m_Result;
};

class CSQLConnection
{
public:
	virtual ~CSQLConnection() { }
	virtual void Close() = 0;
	virtual bool Query(const char *query) = 0;
	virtual unsigned int LastError() = 0;
	virtual CSQLResult *GetResult() = 0;
	virtual std::string EscapeString(std::string unescaped) = 0;
	virtual uint64_t GetInsertID() = 0;
	virtual void *GetInternalConnection() = 0;

	static int s_NumConnectAttempts;
};

class CMYSQLConnection : public CSQLConnection
{
public:
	virtual ~CMYSQLConnection();
	static CMYSQLConnection *Create(const char *host, unsigned int port, const char *user, const char *password, const char *defaultdatabase);
	virtual void Close() override;
	virtual bool Query(const char *query) override;
	virtual unsigned int LastError() override;
	virtual CSQLResult *GetResult() override;
	virtual std::string EscapeString(std::string unescaped) override;
	virtual uint64_t GetInsertID() override;
	virtual void *GetInternalConnection() override;

private:
	CMYSQLConnection(MYSQL *connection);

	MYSQL *m_InternalConnection;
};

class CDatabase2
{
public:
	CDatabase2();
	virtual ~CDatabase2();

	virtual uint64_t GetInsertID();
	virtual std::string EscapeString(std::string unescaped);
	virtual bool Query(const char *format, ...);
	virtual void Tick();
	virtual void AsyncTick();
	virtual CSQLResult *GetResult();
	virtual void *GetInternalConnection();
	virtual void *GetInternalAsyncConnection();

	template<typename ...Args>
	mysql_statement<sizeof...(Args)> QueryEx(const char * query, Args&... args)
	{
		MYSQL *sql = (MYSQL *)GetInternalConnection();

		mysql_statement<sizeof...(args)> statement(sql, query);
		if (statement)
		{
			if constexpr (sizeof...(args) > 0)
				statement.bindargs(args...);

			if (statement.execute())
			{
				return statement;
			}
		}

		return mysql_statement<sizeof...(args)>(nullptr, "");
	}

	mysql_statement<0> QueryEx(const char * query)
	{
		MYSQL *sql = (MYSQL *)GetInternalConnection();

		mysql_statement<0> statement(sql, query);
		if (statement)
		{
			if (statement.execute())
			{
				return statement;
			}
		}

		return mysql_statement<0>(nullptr, "");
	}

	template<int _Count>
	mysql_statement<_Count> CreateQuery(const char * query)
	{
		MYSQL *sql = (MYSQL *)GetInternalConnection();

		//mysql_statement<_Count> statement(sql, query);
		//return std::move(statement);
		return mysql_statement<_Count>(sql, query);
	}

protected:
	CSQLConnection *m_pConnection = NULL;

	// async thread resources
	static uint32_t WINAPI InternalThreadProcStatic(LPVOID lpThis);
	virtual uint32_t InternalThreadProc();
	virtual void ProcessAsyncQueries() { }

	inline void Signal()
	{
		//std::unique_lock lock(m_signalLock);
		m_signal.notify_all();
	}

	bool m_bQuit = false;
	CSQLConnection *m_pAsyncConnection = NULL;

	std::mutex m_signalLock;
	std::condition_variable m_signal;
	std::thread m_queryThread;
};

class CMYSQLQuery
{
public:
	virtual ~CMYSQLQuery() { }

	virtual bool PerformQuery(MYSQL *c) = 0;
};

// the following can only use 'basic' arguments, no byte arrays!
template<typename ...Args>
class generic_query : public CMYSQLQuery
{
public:
	generic_query(std::string query, Args... args)
		: m_query(query), m_args{ args... }
	{
	}

	bool PerformQuery(MYSQL *c) override
	{
		MYSQL *sql = (MYSQL *)c;
		if (!c) return false;

		mysql_statement<sizeof...(Args)> statement(sql, m_query);
		if (statement)
		{
			if constexpr (sizeof...(Args) > 0)
			{
				int index = 0;
				std::apply(
					[&](auto &... val)
					{
						((statement.bind(index++, val)), ...);
					},
					m_args);
			}

			if (statement.execute())
			{
				return true;
			}
		}

		return false;
	}

private:

protected:
	std::string m_query;
	std::tuple<Args...> m_args;
};

class CMYSQLSaveHouseQuery : public CMYSQLQuery
{
public:
	CMYSQLSaveHouseQuery(unsigned int house_id, void *data, unsigned int data_length);
	virtual ~CMYSQLSaveHouseQuery() override;

	virtual bool PerformQuery(MYSQL *c) override;

protected:
	unsigned int _house_id;
	uint8_t *_data;
	unsigned int _data_length;
};

class CMYSQLSaveWeenieQuery : public CMYSQLQuery
{
public:
	CMYSQLSaveWeenieQuery(unsigned int weenie_id, unsigned int top_level_object_id, unsigned int block_id, void *data, unsigned int data_length);
	virtual ~CMYSQLSaveWeenieQuery() override;

	virtual bool PerformQuery(MYSQL *c) override;

protected:
	unsigned int _weenie_id;
	unsigned int _top_level_object_id;
	unsigned int _block_id;
	uint8_t *_data;
	unsigned int _data_length;
};

class CMYSQLDatabase : public CDatabase2
{
public:
	CMYSQLDatabase(const char *host, unsigned int port, const char *user, const char *password, const char *defaultdatabasename);
	virtual ~CMYSQLDatabase();

	void QueueAsyncQuery(CMYSQLQuery *query);
	virtual void Tick() override;

	template<typename ...Args>
	void QueueAsyncQuery(std::string query, Args&... args)
	{
		generic_query<Args...> *q = new generic_query<Args...>(query, args...);
		QueueAsyncQuery((CMYSQLQuery*)q);
	}

protected:
	virtual void AsyncTick() override;

private:
	void RefreshConnection();

	void RefreshAsyncConnection();
	virtual void ProcessAsyncQueries() override;

	std::string m_DatabaseHost;
	unsigned int m_DatabasePort;
	std::string m_DatabaseUser;
	std::string m_DatabasePassword;
	std::string m_DatabaseName;

	double m_fLastRefresh;
	double m_fLastAsyncRefresh = 0.0;
	bool m_bDisabled;

	//CLockable _asyncQueueLock;
	std::mutex m_lock;
	std::list<CMYSQLQuery *> _asyncQueries;
};

#include "PhysicsDesc.h"
#include "PublicWeenieDesc.h"

class CCapturedWorldObjectInfo
{
public:
	CCapturedWorldObjectInfo() {
		m_ObjData = NULL;
		m_ObjDataLen = 0;
	}
	~CCapturedWorldObjectInfo() {
		Free();
	}
	void Free() {
		if (m_ObjData)
		{
			delete[] m_ObjData;
			m_ObjData = NULL;
		}
	}

	std::string m_ObjName;
	BYTE *m_ObjData;
	unsigned m_ObjDataLen;

	uint32_t guid;
	ObjDesc objdesc;
	PhysicsDesc physics;
	PublicWeenieDesc weenie;
	std::map<uint32_t, uint32_t> uint32_tProperties;
	std::map<uint32_t, UINT64> qwordProperties;
	std::map<uint32_t, uint32_t> boolProperties;
	std::map<uint32_t, double> floatProperties;
	std::map<uint32_t, std::string> stringProperties;
	std::map<uint32_t, uint32_t> dataIDProperties;
	ObjDesc wornobjdesc;
};

class CGameDatabase
{
public:
	CGameDatabase();
	virtual ~CGameDatabase();

	void Init();

	bool LoadedPortals() { return m_bLoadedPortals; }

	std::list<CCapturedWorldObjectInfo *> *GetStaticsForLandBlock(WORD lb_gid);
	void SpawnStaticsForLandBlock(WORD lb_gid);

	CCapturedWorldObjectInfo *GetCapturedMonsterData(const char *name);
	CCapturedWorldObjectInfo *GetCapturedItemData(const char *name);
	CCapturedWorldObjectInfo *GetCapturedArmorData(const char *name);
	CCapturedWorldObjectInfo *GetRandomCapturedMonsterData();
	CCapturedWorldObjectInfo *GetRandomCapturedItemData();
	CCapturedWorldObjectInfo *GetRandomCapturedArmorData();

	void SpawnBSD();

	class CWeenieObject *CreateFromCapturedData(CCapturedWorldObjectInfo *pObjectInfo);

protected:

	std::string ConvertNameForLookup(std::string name);

	void LoadAerfalle();	
	void LoadCapturedMonsterData();	
	void LoadCapturedItemData();
	void LoadCapturedArmorData();
	void LoadTeleTownList();
	void LoadStaticsData();
	void LoadBSD();

	bool m_bLoadedPortals;

	std::list<Position> m_BSDSpawns;

	std::list<CCapturedWorldObjectInfo *> m_CapturedStaticsData;
	std::map<WORD, std::list<CCapturedWorldObjectInfo *>> m_CapturedStaticsDataByLandBlock;
	std::list<CCapturedWorldObjectInfo *> m_CapturedAerfalleData;
	std::map<std::string, CCapturedWorldObjectInfo *> m_CapturedMonsterData;
	std::vector<CCapturedWorldObjectInfo *> m_CapturedMonsterDataList;
	std::map<std::string, CCapturedWorldObjectInfo *> m_CapturedItemData;
	std::vector<CCapturedWorldObjectInfo *> m_CapturedItemDataList;
	std::map<std::string, CCapturedWorldObjectInfo *> m_CapturedArmorData;
	std::vector<CCapturedWorldObjectInfo *> m_CapturedArmorDataList;
};

extern CGameDatabase *g_pGameDatabase;

