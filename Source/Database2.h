#pragma once

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
	virtual bool Query(const char *query);
	virtual unsigned int LastError();
	virtual CSQLResult *GetResult();
	virtual std::string EscapeString(std::string unescaped);
	virtual uint64_t GetInsertID() override;
	virtual void *GetInternalConnection();

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

protected:
	CSQLConnection *m_pConnection = NULL;

	// async thread resources
	static DWORD WINAPI InternalThreadProcStatic(LPVOID lpThis);
	virtual DWORD InternalThreadProc();
	virtual void ProcessAsyncQueries() { }

	bool m_bQuit = false;
	HANDLE m_hMakeTick = NULL;
	HANDLE m_hAsyncThread = NULL;
	CSQLConnection *m_pAsyncConnection = NULL;
};

class CMYSQLQuery
{
public:
	virtual ~CMYSQLQuery() { }

	virtual bool PerformQuery(MYSQL *c) = 0;
};

class CMYSQLSaveHouseQuery : public CMYSQLQuery
{
public:
	CMYSQLSaveHouseQuery(unsigned int house_id, void *data, unsigned int data_length);
	virtual ~CMYSQLSaveHouseQuery() override;

	virtual bool PerformQuery(MYSQL *c) override;

protected:
	unsigned int _house_id;
	void *_data;
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
	void *_data;
	unsigned int _data_length;
};

class CMYSQLDatabase : public CDatabase2
{
public:
	CMYSQLDatabase(const char *host, unsigned int port, const char *user, const char *password, const char *defaultdatabasename);
	virtual ~CMYSQLDatabase();

	void QueueAsyncQuery(CMYSQLQuery *query);
	virtual void Tick() override;

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
	double m_fLastAsyncRefresh;
	bool m_bDisabled;

	CLockable _asyncQueueLock;
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

	DWORD guid;
	ObjDesc objdesc;
	PhysicsDesc physics;
	PublicWeenieDesc weenie;
	std::map<DWORD, DWORD> dwordProperties;
	std::map<DWORD, UINT64> qwordProperties;
	std::map<DWORD, DWORD> boolProperties;
	std::map<DWORD, double> floatProperties;
	std::map<DWORD, std::string> stringProperties;
	std::map<DWORD, DWORD> dataIDProperties;
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

