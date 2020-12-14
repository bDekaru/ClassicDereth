#pragma once

#include "Database2.h"

using idqueue_t = std::queue<unsigned int, std::deque<unsigned int>>;

enum eGUIDClass {
	ePresetGUID = 0,
	ePlayerGUID = 1,
	// eStaticGUID = 2,
	eDynamicGUID = 3,
	eEphemeral = 4
};

class IdRangeTableQuery : public CMYSQLQuery
{
public:
	IdRangeTableQuery(uint32_t &start, uint32_t end, idqueue_t &queue, std::mutex &lock, bool &busy)
		: m_start(start), m_end(end), m_queue(queue), m_lock(lock), m_busy(busy)
	{
	}

	virtual ~IdRangeTableQuery() override { };

	virtual bool PerformQuery(MYSQL *c) override;

protected:
	uint32_t &m_start;
	uint32_t m_end;
	idqueue_t &m_queue;
	std::mutex &m_lock;
	bool &m_busy;
};

class ScanWeenieTableQuery : public CMYSQLQuery
{
public:
	ScanWeenieTableQuery(uint32_t &start, uint32_t end, idqueue_t &queue, std::mutex &lock, bool &busy)
		: m_start(start), m_end(end), m_queue(queue), m_lock(lock), m_busy(busy)
	{
	}

	virtual ~ScanWeenieTableQuery() override { };

	virtual bool PerformQuery(MYSQL *c) override;

protected:
	uint32_t & m_start;
	uint32_t m_end;
	idqueue_t &m_queue;
	std::mutex &m_lock;
	bool &m_busy;
};

class CObjectIDGenerator
{
public:
	CObjectIDGenerator();
	virtual ~CObjectIDGenerator();

	void LoadState();
	void SaveRangeStart();
	void LoadRangeStart();
	bool IsIdRangeValid(){ return isIdRangeValid;}

	uint32_t GenerateGUID(eGUIDClass guidClass);

	void Think();

protected:
	virtual CMYSQLQuery* GetQuery();

protected:
	std::mutex m_lock;

	uint32_t m_dwHintDynamicGUID = 0x80000000;
	uint32_t currentGUID = 0;
	bool outOfIds = false;
	bool m_bLoadingState = false;
	bool queryInProgress = false;
	std::queue<unsigned int, std::deque<unsigned int>> listOfIdsForWeenies;
	bool isIdRangeValid = true;
	uint32_t m_ephemeral;
	bool isServerStarting = true;
};
