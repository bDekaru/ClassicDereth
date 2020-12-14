
#include <StdAfx.h>
#include "ObjectIDGen.h"
#include "DatabaseIO.h"
#include "Server.h"

const uint32_t IDQUEUEMAX = 250000;
const uint32_t IDQUEUEMIN = 75000;
const uint32_t IDRANGESTART = 0x90000000;
const uint32_t IDRANGEEND = 0xf0000000;

const uint32_t IDEPHEMERALSTART = 0x60000000;
const uint32_t IDEPHEMERALMASK = 0x6fffffff;


CObjectIDGenerator::CObjectIDGenerator()
{
	m_ephemeral = IDEPHEMERALSTART;
	if (g_pConfig->UseIncrementalID())
	{
		WINLOG(Data, Normal, "Using Incremental ID system.....\n");
	}
	else if (g_pConfig->IDScanType() == 0)
	{
		// Verify IDRanges tables exists and has values
		if (g_pDBIO->IDRangeTableExistsAndValid())
		{
			WINLOG(Data, Normal, "Using ID Mined system.....\n");
		}
		else
		{
			isIdRangeValid = false;
		}
	}

	LoadRangeStart();
	if (!g_pConfig->UseIncrementalID())
	{
		while (listOfIdsForWeenies.size() < IDQUEUEMAX)
		{
			LoadState();
		}
	}

	isServerStarting = false;
}

CObjectIDGenerator::~CObjectIDGenerator()
{
}

CMYSQLQuery* CObjectIDGenerator::GetQuery()
{
	CMYSQLQuery *query = nullptr;

	switch (g_pConfig->IDScanType())
	{
	case 0:
		query = new IdRangeTableQuery(
			m_dwHintDynamicGUID, IDQUEUEMAX, listOfIdsForWeenies, m_lock, m_bLoadingState);
		break;

	case 1:
		query = new ScanWeenieTableQuery(
			m_dwHintDynamicGUID, IDQUEUEMAX, listOfIdsForWeenies, m_lock, m_bLoadingState);
		break;
	}


	return query;
}

void CObjectIDGenerator::LoadState()
{

	if (!g_pConfig->UseIncrementalID())
	{
		//DEBUG_DATA << "Queue ID Scan Type: " << g_pConfig->IDScanType();
		if (currentGUID != m_dwHintDynamicGUID)
		{
			if (listOfIdsForWeenies.size() < IDQUEUEMAX)
			{
				g_pDBDynamicIDs->QueueAsyncQuery(GetQuery());
				SaveRangeStart();
				currentGUID = m_dwHintDynamicGUID;
				if (m_dwHintDynamicGUID > IDRANGEEND)
					m_dwHintDynamicGUID = IDRANGESTART;
			}
		}
	}
	else
		m_dwHintDynamicGUID = g_pDBIO->GetHighestWeenieID(IDRANGESTART, IDRANGEEND);
}

uint32_t CObjectIDGenerator::GenerateGUID(eGUIDClass type)
{
	switch (type)
	{
	case eDynamicGUID:
	{

		uint32_t result = 0;

		if (!g_pConfig->UseIncrementalID())
		{
			if (outOfIds || listOfIdsForWeenies.empty())
			{
				WINLOG(Data, Normal, "Dynamic GUID overflow!\n");
				SERVER_ERROR << "Dynamic GUID overflow!";
				return 0;
			}

			{
				std::scoped_lock lock(m_lock);
				result = listOfIdsForWeenies.front();
				listOfIdsForWeenies.pop();
				//DEBUG_DATA << "Id Count:" << listOfIdsForWeenies.size();
			}
		}
		else
		{
			if (m_dwHintDynamicGUID >= IDRANGEEND)
			{
				SERVER_ERROR << "Dynamic GUID overflow!";
				return 0;
			}
			result = ++m_dwHintDynamicGUID;
		}

		return result;
	}
	case eEphemeral:
	{
		m_ephemeral &= IDEPHEMERALMASK;
		return m_ephemeral++;
	}
	}

	return 0;
}

void CObjectIDGenerator::SaveRangeStart()
{
	BinaryWriter banData;
	banData.Write<uint32_t>(m_dwHintDynamicGUID);
	if (!g_pConfig->UseIncrementalID())
	{
		g_pDBIO->CreateOrUpdateGlobalData(DBIO_GLOBAL_ID_RANGE_START, banData.GetData(), banData.GetSize());
	}
	m_bLoadingState = false;
}

void CObjectIDGenerator::LoadRangeStart()
{
	void *data = NULL;
	uint32_t length = 0;
	if (!g_pConfig->UseIncrementalID())
	{
		m_dwHintDynamicGUID = IDRANGESTART;
		m_bLoadingState = true;
		std::unique_ptr<CMYSQLQuery> query(GetQuery());
		query->PerformQuery((MYSQL*)g_pDBDynamicIDs->GetInternalConnection());
	}
	else
		m_dwHintDynamicGUID = g_pDBIO->GetHighestWeenieID(IDRANGESTART, IDRANGEEND);
}

void CObjectIDGenerator::Think()
{
	if (!isServerStarting && !m_bLoadingState)
	{
		if (!m_bLoadingState && listOfIdsForWeenies.size() < IDQUEUEMIN)
		{
			m_bLoadingState = true;
			LoadState();
		}
	}
}

bool IdRangeTableQuery::PerformQuery(MYSQL *c)
{
	if (!c)
		return false;

	MYSQL_STMT *statement = mysql_stmt_init(c);
	bool failed = false;

	if (statement)
	{
		std::string query = "SELECT unused FROM idranges WHERE unused > ? limit 250000";
		mysql_stmt_prepare(statement, query.c_str(), query.length());

		MYSQL_BIND params = { 0 };
		params.buffer = &m_start;
		params.buffer_length = sizeof(uint32_t);
		params.buffer_type = MYSQL_TYPE_LONG;
		params.is_unsigned = true;

		failed = mysql_stmt_bind_param(statement, &params);
		if (!failed)
		{
			failed = mysql_stmt_execute(statement);
			if (!failed)
			{
				uint32_t value = 0;
				MYSQL_BIND result = { 0 };
				result.buffer = &value;
				result.buffer_length = sizeof(uint32_t);
				result.buffer_type = MYSQL_TYPE_LONG;
				result.is_unsigned = true;

				failed = mysql_stmt_bind_result(statement, &result);
				bool done = failed;

				while (!done)
				{
					std::scoped_lock lock(m_lock);

					for (int i = 0; i < 2500 && !failed; i++)
					{
						if (!(done = mysql_stmt_fetch(statement)))
						{
							m_queue.push(value);
							m_start = value;
						}
					}
					std::this_thread::yield();
				}
			}
		}

		mysql_stmt_close(statement);
	}

	m_busy = false;

	if (failed)
		SERVER_ERROR << "Error in IdRangeTableQuery::PerformQuery: " << mysql_error(c);

	return !failed;

}

bool ScanWeenieTableQuery::PerformQuery(MYSQL *c)
{
	if (!c)
		return false;

	MYSQL_STMT *statement = mysql_stmt_init(c);
	bool failed = false;

	if (statement)
	{
		std::string query =
			"SELECT IFNULL(w.id, ?) + 1 AS l, "
			"(SELECT IFNULL(id, 4278190080) FROM weenies WHERE id > ? AND id > w.id ORDER BY id LIMIT 1) AS u "
			"FROM weenies w "
			"LEFT JOIN weenies l ON l.id = w.id + 1 and l.id is null "
			"WHERE w.id > ? "
			"ORDER BY w.id "
			"LIMIT 1000;";

		mysql_stmt_prepare(statement, query.c_str(), query.length());

		MYSQL_BIND params[3] = { 0 };
		params[0].buffer = &m_start;
		params[0].buffer_length = sizeof(uint32_t);
		params[0].buffer_type = MYSQL_TYPE_LONG;
		params[0].is_unsigned = true;
		params[1].buffer = &m_start;
		params[1].buffer_length = sizeof(uint32_t);
		params[1].buffer_type = MYSQL_TYPE_LONG;
		params[1].is_unsigned = true;
		params[2].buffer = &m_start;
		params[2].buffer_length = sizeof(uint32_t);
		params[2].buffer_type = MYSQL_TYPE_LONG;
		params[2].is_unsigned = true;

		failed = mysql_stmt_bind_param(statement, params);
		if (!failed)
		{
			//mysql_stmt_store_result(statement);

			if (mysql_stmt_execute(statement))
			{
				SERVER_ERROR << "mysql statement failed to scan for IDs";
			}
			else
			{
				uint32_t l = 0;
				uint32_t u = -1;
				MYSQL_BIND result[2] = { 0 };
				result[0].buffer = &l;
				result[0].buffer_length = sizeof(uint32_t);
				result[0].buffer_type = MYSQL_TYPE_LONG;
				result[0].is_unsigned = true;

				result[1].buffer = &u;
				result[1].buffer_length = sizeof(uint32_t);
				result[1].buffer_type = MYSQL_TYPE_LONG;
				result[1].is_unsigned = true;

				failed = mysql_stmt_bind_result(statement, result);
				bool done = failed;

				mysql_stmt_store_result(statement);

				while (!done)
				{
					if (!(done = mysql_stmt_fetch(statement)))
					{
						//DEBUG_DATA << "Lower:" << l << " Upper:" << u;
						//DEBUG_DATA << "ScanWeenieTableQuery, result: " << l << " - " << u;

						// we need to save the lower bound
						// that way if this range is ever revisited by the query,
						// it won't be skipped entirely
						//m_start = l;

						// we need to keep reading results even it we met our target
						//   to ensure the buffers are all flushed
						while (l < u)
						{
							{
								std::scoped_lock lock(m_lock);
								for (int i = 0; i < 5000 && l < u; i++, l++)
								{
									m_queue.push(l);
									m_start = l;
								}
							}
						}
					}
				}
			}
		}
		else
			SERVER_ERROR << "mysql bind failed in scan for IDs";

		mysql_stmt_close(statement);
	}

	DEBUG_DATA << "m_queue size:" << m_queue.size();

	m_busy = false;

	if (failed)
		SERVER_ERROR << "Error in ScanWeenieTableQuery::PerformQuery: " << mysql_error(c);

	return !failed;

}
