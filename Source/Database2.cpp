
#include <StdAfx.h>
#include "Database2.h"
#include "World.h"
#include "Portal.h"
#include "PhysicsDesc.h"
#include "PublicWeenieDesc.h"
#include "Door.h"
#include "Lifestone.h"
#include "SHA512.h"
#include "Monster.h"
#include "ClothingCache.h"
#include "ClothingTable.h"
#include "ObjDesc.h"
#include "MonsterAI.h"
#include "TownCrier.h"
#include "Vendor.h"
#include "WeenieFactory.h"
#include "DatabaseIO.h"

CMYSQLResult::CMYSQLResult(MYSQL_RES *Result)
{
	m_Result = Result;
}

CMYSQLResult::~CMYSQLResult()
{
	Free();
}

void CMYSQLResult::Free()
{
	if (m_Result)
	{
		mysql_free_result(m_Result);
		m_Result = NULL;
	}
}

SQLResultRow_t CMYSQLResult::FetchRow()
{
	if (!m_Result)
	{
		return NULL;
	}

	return (SQLResultRow_t)mysql_fetch_row(m_Result);
}

uint64_t CMYSQLResult::ResultRows()
{
	if (!m_Result)
	{
		return 0;
	}

	uint64_t numRows = (uint64_t)mysql_num_rows(m_Result);

	return numRows;
}

int CSQLConnection::s_NumConnectAttempts = 0;

CMYSQLConnection::CMYSQLConnection(MYSQL *connection)
{
	m_InternalConnection = connection;
}

CMYSQLConnection::~CMYSQLConnection()
{
	Close();
}

CMYSQLConnection *CMYSQLConnection::Create(const char *host, unsigned int port, const char *user, const char *password, const char *defaultdatabase)
{
	MYSQL *sqlobject = mysql_init(NULL);

	if (!sqlobject)
	{
		return NULL;
	}

	int connect_timeout = MYSQL_CONNECT_TIMEOUT;
	mysql_options(sqlobject, MYSQL_OPT_CONNECT_TIMEOUT, (char *)&connect_timeout);

	MYSQL *sqlconnection;

	{
		s_NumConnectAttempts++;

		CStopWatch connectTiming;

		sqlconnection = mysql_real_connect(sqlobject, host, user, password, defaultdatabase, port, NULL, 0);

		if (sqlconnection && connectTiming.GetElapsed() >= 1.0)
		{
			SERVER_WARN << "mysql_real_connect() took" << connectTiming.GetElapsed();
		}
	}

	if (sqlconnection == NULL)
	{
		if (mysql_errno(sqlobject) == EINTR)
		{
			mysql_close(sqlobject);

			sqlobject = mysql_init(NULL);

			if (!sqlobject)
			{
				return NULL;
			}

			int connect_timeout = MYSQL_CONNECT_TIMEOUT;
			mysql_options(sqlobject, MYSQL_OPT_CONNECT_TIMEOUT, (char *)&connect_timeout);

			s_NumConnectAttempts++;

			CStopWatch connectTiming;

			sqlconnection = mysql_real_connect(sqlobject, host, user, password, defaultdatabase, port, NULL, 0);

			if (sqlconnection && connectTiming.GetElapsed() >= 1.0)
			{
				SERVER_WARN << "mysql_real_connect() re-attempt took"<< connectTiming.GetElapsed();
			}

			if (sqlconnection == NULL)
			{
				SERVER_WARN << "Failed to create mysql connection after two tries:"<< mysql_error(sqlobject);

				mysql_close(sqlobject);
				return NULL;
			}
			else
			{
				SERVER_WARN << "Received EINTR while attempting to connect to mysql, but re-attempt succeeded.";
			}
		}
		else
		{
			if (CSQLConnection::s_NumConnectAttempts > 1)
			{
				// Only show warning if not the first connection attempt
				SERVER_WARN << "mysql_real_connect() failed:" << mysql_error(sqlobject);
				WINLOG(Data, Warning, "Failed to establish connection to database.\n");
			}

			mysql_close(sqlobject);
			return NULL;
		}
	}

	if (sqlconnection)
	{
		// Disable auto-reconnect (probably already disabled)
		// sqlconnection->reconnect = false;

		return new CMYSQLConnection(sqlconnection);
	}

	return NULL;
}

void CMYSQLConnection::Close()
{
	if (m_InternalConnection)
	{
		mysql_close(m_InternalConnection);
		m_InternalConnection = NULL;
	}
}

bool CMYSQLConnection::Query(const char *query)
{
	if (!m_InternalConnection)
	{
		return false;
	}

	int errorCode;

	{
		CStopWatch queryTiming;

		errorCode = mysql_query(m_InternalConnection, query);

		if (queryTiming.GetElapsed() >= 1.0)
		{
			SERVER_WARN << "MYSQL query" << query << "took" << queryTiming.GetElapsed() << "seconds.";
		}
	}

	if (errorCode != 0)
	{
		SERVER_ERROR << "MYSQL query" << query << "errored" << errorCode;
		return false;
	}

	return true;
}

unsigned int CMYSQLConnection::LastError()
{
	return m_InternalConnection ? (unsigned int)mysql_errno(m_InternalConnection) : 0;
}

CSQLResult *CMYSQLConnection::GetResult()
{
	if (!m_InternalConnection)
	{
		return NULL;
	}

	MYSQL_RES *Result = mysql_store_result(m_InternalConnection);

	if (!Result)
	{
		return NULL;
	}

	return new CMYSQLResult(Result);
}

std::string CMYSQLConnection::EscapeString(std::string unescaped)
{
	if (!m_InternalConnection)
	{
		return "";
	}
	
	char *escaped = new char[(unescaped.length() * 2) + 1];
	escaped[0] = '\0';
	mysql_real_escape_string(m_InternalConnection, escaped, unescaped.c_str(), (uint32_t) unescaped.length());

	std::string result = escaped;
	delete[] escaped;

	return result;
}

uint64_t CMYSQLConnection::GetInsertID()
{
	if (!m_InternalConnection)
	{
		return 0;
	}

	return mysql_insert_id(m_InternalConnection);
}

void *CMYSQLConnection::GetInternalConnection()
{
	return (void *)m_InternalConnection;
}

CDatabase2::CDatabase2()
	: m_queryThread([&]() { InternalThreadProc(); })
{
	m_pConnection = NULL;
}

CDatabase2::~CDatabase2()
{
	m_bQuit = true;
	m_signal.notify_all();

	if (m_pConnection != nullptr)
	{
		m_pConnection->Close();
		delete m_pConnection;
		m_pConnection = nullptr;
	}

	if (m_queryThread.joinable())
		m_queryThread.join();

	if (m_pAsyncConnection != nullptr)
	{
		m_pAsyncConnection->Close();
		delete m_pAsyncConnection;
		m_pAsyncConnection = nullptr;
	}
}

void CDatabase2::Tick()
{
}

void CDatabase2::AsyncTick()
{
}

bool CDatabase2::Query(const char *format, ...)
{
	if (!m_pConnection)
	{
		return false;
	}

	bool success = false;

	va_list args;
	va_start(args, format);

	int charCount = _vscprintf(format, args) + 1;

	char *charBuffer = new char[charCount];
	charBuffer[0] = '\0';
	vsnprintf(charBuffer, charCount, format, args);
	charBuffer[charCount - 1] = '\0';

	if (m_pConnection)
	{
		success = m_pConnection->Query(charBuffer);
	}

	delete[] charBuffer;

	va_end(args);

	return success;
}

CSQLResult *CDatabase2::GetResult()
{
	if (!m_pConnection)
	{
		return NULL;
	}

	return m_pConnection->GetResult();
}

std::string CDatabase2::EscapeString(std::string unescaped)
{
	if (!m_pConnection)
	{
		return "";
	}

	return m_pConnection->EscapeString(unescaped);
}

uint64_t CDatabase2::GetInsertID()
{
	if (!m_pConnection)
	{
		return 0;
	}

	return m_pConnection->GetInsertID();
}

void *CDatabase2::GetInternalConnection()
{
	if (!m_pConnection)
	{
		return NULL;
	}

	return m_pConnection->GetInternalConnection();
}

void *CDatabase2::GetInternalAsyncConnection()
{
	if (!m_pAsyncConnection)
	{
		return NULL;
	}

	return m_pAsyncConnection->GetInternalConnection();
}

uint32_t WINAPI CDatabase2::InternalThreadProcStatic(LPVOID lpThis)
{
	return ((CDatabase2 *)lpThis)->InternalThreadProc();
}

uint32_t CDatabase2::InternalThreadProc()
{
	do
	{
		std::unique_lock lock(m_signalLock);
		m_signal.wait_for(lock, std::chrono::seconds(1));

		ProcessAsyncQueries();
		AsyncTick();

	} while (!m_bQuit);

	return 0;
}

CMYSQLSaveWeenieQuery::CMYSQLSaveWeenieQuery(unsigned int weenie_id, unsigned int top_level_object_id, unsigned int block_id, void *data, unsigned int data_length)
{
	_weenie_id = weenie_id;
	_top_level_object_id = top_level_object_id;
	_block_id = block_id;

	if (data_length > 0)
	{
		_data = new BYTE[data_length];
		memcpy(_data, data, data_length);
	}
	else
	{
		_data = NULL;
	}

	_data_length = data_length;
}

CMYSQLSaveWeenieQuery::~CMYSQLSaveWeenieQuery()
{
	SafeDeleteArray(_data);
}

bool CMYSQLSaveWeenieQuery::PerformQuery(MYSQL *c)
{
	if (!c)
	{
		SERVER_ERROR << "Cannot perform save query; no connection.";
		return false;
	}

	mysql_statement<4> statement(c, "CALL blob_update_weenie(?, ?, ?, ?)");
	if (statement)
	{
		statement.bind(0, _weenie_id);
		statement.bind(1, _top_level_object_id);
		statement.bind(2, _block_id);
		statement.bind(3, _data, _data_length);

		if (statement.execute())
		{
			g_pDBIO->DecrementPendingSave(_weenie_id);
			return true;
		}
	}

	SERVER_ERROR << "mysql_statement error on CreateOrUpdateWeenie for" << _weenie_id << "(" << _data_length << "):" << mysql_error(c);
	return false;
}

CMYSQLSaveHouseQuery::CMYSQLSaveHouseQuery(unsigned int house_id, void *data, unsigned int data_length)
{
	_house_id = house_id;

	if (data_length > 0)
	{
		_data = new BYTE[data_length];
		memcpy(_data, data, data_length);
	}
	else
	{
		_data = NULL;
	}

	_data_length = data_length;
}

CMYSQLSaveHouseQuery::~CMYSQLSaveHouseQuery()
{
	SafeDeleteArray(_data);
}

bool CMYSQLSaveHouseQuery::PerformQuery(MYSQL *c)
{
	if (!c)
	{
		SERVER_ERROR << "Cannot perform save query; no connection.";
		return false;
	}

	mysql_statement<2> statement(c, "CALL blob_update_house(?, ?)");
	if (statement)
	{
		statement.bind(0, _house_id);
		statement.bind(1, _data, _data_length);

		if (statement.execute())
		{
			g_pDBIO->DecrementPendingSave(_house_id);
			return true;
		}
	}

	SERVER_ERROR << "mysql_statement error on CreateOrUpdateHouseData for" << _house_id << "(" << _data_length << "):" << mysql_error(c);
	return false;

}

CMYSQLDatabase::CMYSQLDatabase(const char *host, unsigned int port, const char *user, const char *password, const char *defaultdatabasename)
{
	m_DatabaseHost = host;
	m_DatabasePort = port;
	m_DatabaseUser = user;
	m_DatabasePassword = password;
	m_DatabaseName = defaultdatabasename;
	m_bDisabled = false;

	RefreshConnection();
	RefreshAsyncConnection();

	if (!m_pConnection || !m_pAsyncConnection)
	{
		// If we can't connect the first time, just disable this feature.
		SERVER_WARN << "MySQL database functionality disabled.";
		m_bDisabled = true;
	}
}

CMYSQLDatabase::~CMYSQLDatabase()
{
}

void CMYSQLDatabase::RefreshConnection()
{
	// If we have an old connection, close that one.
	if (m_pConnection)
	{
		delete m_pConnection;
		m_pConnection = NULL;
	}

	m_pConnection = CMYSQLConnection::Create(m_DatabaseHost.c_str(), m_DatabasePort, m_DatabaseUser.c_str(), m_DatabasePassword.c_str(), m_DatabaseName.c_str());
	m_fLastRefresh = g_pGlobals->Time();
}

void CMYSQLDatabase::RefreshAsyncConnection()
{
	// If we have an old connection, close that one.
	if (m_pAsyncConnection)
	{
		delete m_pAsyncConnection;
		m_pAsyncConnection = NULL;
	}

	m_pAsyncConnection = CMYSQLConnection::Create(m_DatabaseHost.c_str(), m_DatabasePort, m_DatabaseUser.c_str(), m_DatabasePassword.c_str(), m_DatabaseName.c_str());
	m_fLastAsyncRefresh = g_pGlobals->Time();
}

void CMYSQLDatabase::QueueAsyncQuery(CMYSQLQuery *query)
{
	{
		std::scoped_lock lock(m_lock);
		_asyncQueries.push_back(query);
	}
	Signal();
}

void CMYSQLDatabase::ProcessAsyncQueries()
{
	while (true)
	{
		CMYSQLQuery *queuedQuery = NULL;

		{
			std::scoped_lock lock(m_lock);
			if (!_asyncQueries.empty())
			{
				queuedQuery = _asyncQueries.front();
				_asyncQueries.pop_front();
			}
		}

		if (!queuedQuery)
		{
			break;
    	}

		try
		{
			if (queuedQuery->PerformQuery((MYSQL *)GetInternalAsyncConnection()))
			{
				delete queuedQuery;
			}
			else
			{
				std::scoped_lock lock(m_lock);
				_asyncQueries.push_front(queuedQuery);

				SERVER_ERROR << "Error while performing query:" << queuedQuery;

				// there was an error, stop for now
				break;
			}
		}
		catch (...)
		{
			SERVER_ERROR << "Unable to delete query";
		}
	}
}

void CMYSQLDatabase::Tick()
{
	/*
	if ((m_fLastRefresh + 60.0) < g_pGlobals->Time())
	{
		// Refresh connection every 60 seconds to avoid stale connections.
		RefreshConnection();
	}
	*/
}

void CMYSQLDatabase::AsyncTick()
{
	if ((m_fLastAsyncRefresh + 60.0) < g_pGlobals->Time())
	{
		// Refresh connection every 60 seconds to avoid stale connections.
		RefreshAsyncConnection();
	}
}

CGameDatabase::CGameDatabase()
{
	m_bLoadedPortals = false;
}

CGameDatabase::~CGameDatabase()
{
	for (auto &data : m_CapturedMonsterData)
	{
		delete data.second;
	}

	for (auto &data : m_CapturedItemData)
	{
		delete data.second;
	}

	for (auto &data : m_CapturedStaticsData)
	{
		delete data;
	}

	for (auto &data : m_CapturedArmorData)
	{
		delete data.second;
	}
}

void CGameDatabase::Init()
{
#ifndef QUICKSTART
	// Loads from MYSQL database
	LoadTeleTownList();

#if 0
	// Load data files for now
	LoadCapturedMonsterData();
	LoadCapturedItemData();
	LoadCapturedArmorData();
	// LoadStaticsData();
	// LoadBSD();
#endif

#else
	LoadTeleTownList();

#if 0
	LoadCapturedMonsterData();
#endif

#endif
}

void CGameDatabase::LoadBSD()
{
	BYTE *data = NULL;
	uint32_t length = 0;

	if (LoadDataFromFile("data/misc/bsd.dat", &data, &length))
	{
		BinaryReader reader(data, length);
		
		do
		{
			Position pos;
			pos.UnPack(&reader);
			if (reader.GetLastError())
				break;

			m_BSDSpawns.push_back(pos);
		} while (1);

		delete [] data;
	}
}

void CGameDatabase::SpawnBSD()
{
	for (auto &spawnPos : m_BSDSpawns)
	{
		CCapturedWorldObjectInfo *pMonsterInfo = g_pGameDatabase->GetCapturedMonsterData("Tusker Guard");

		if (!pMonsterInfo)
			return;

		std::list<CWeenieObject *> results;
		g_pWorld->EnumNearby(spawnPos, 10.0f, &results);
		
		bool bHasTusker = false;
		for (auto &check : results)
		{
			if (!strcmp(check->GetName().c_str(), "Tusker Guard"))
			{
				bHasTusker = true;
				break;
			}
		}

		if (bHasTusker)
			continue;

		CMonsterWeenie *pMonster = (CMonsterWeenie *)g_pGameDatabase->CreateFromCapturedData(pMonsterInfo);

		// Modify these
		pMonster->SetScale(pMonsterInfo->physics.object_scale);
		pMonster->SetInitialPosition(spawnPos);

		// Add and spawn it
		g_pWorld->CreateEntity(pMonster);
	}
}

void CGameDatabase::LoadAerfalle()
{
	// TEMPORARY FOR TESTING DUNGEONS
	// Consolidate or remove this method of loading (this was copied and pasted)

	BYTE *data;
	uint32_t length;

	if (LoadDataFromFile("data/misc/aerfalle.dat", &data, &length))
	{
		BinaryReader reader(data, length);
		unsigned int count = reader.ReadUInt32();

		for (unsigned int i = 0; i < count; i++)
		{
			reader.ReadString();

			CCapturedWorldObjectInfo *pObjectInfo = new CCapturedWorldObjectInfo;

			uint32_t a = reader.ReadUInt32(); // size of this
			uint32_t b = reader.ReadUInt32(); // 0xF745
			pObjectInfo->guid = reader.ReadUInt32(); // GUID

			pObjectInfo->objdesc.UnPack(&reader);
			pObjectInfo->physics.Unpack(reader);
			pObjectInfo->weenie.UnPack(&reader);

			if (!reader.GetLastError())
			{
				m_CapturedAerfalleData.push_back(pObjectInfo);
			}
			else
			{
				DEBUG_BREAK();
				delete pObjectInfo;

				break;
			}
		}

		delete [] data;
	}
}

std::string CGameDatabase::ConvertNameForLookup(std::string name)
{
	std::transform(name.begin(), name.end(), name.begin(), ::tolower);
	name.erase(remove_if(name.begin(), name.end(), isspace), name.end());
	return name;
}

void CGameDatabase::LoadStaticsData()
{
	// THIS IS ALL TEMPORARY FOR TESTING
	unsigned int spawned = 0;

	BYTE *data;
	uint32_t length;

	if (LoadDataFromFile("data/misc/statics.dat", &data, &length))
	{
		BinaryReader reader(data, length);
		unsigned int count = reader.ReadUInt32();

		for (unsigned int i = 0; i < count; i++)
		{
			CCapturedWorldObjectInfo *pObjectInfo = new CCapturedWorldObjectInfo;
			
			reader.ReadString();
			uint32_t dwObjDataLen = reader.ReadUInt32(); // size of this

			reader.ReadUInt32(); // 0xf745
			pObjectInfo->guid = reader.ReadUInt32(); // GUID
			
			pObjectInfo->objdesc.UnPack(&reader);
			pObjectInfo->physics.Unpack(reader);
			pObjectInfo->weenie.UnPack(&reader);

			if (reader.GetLastError())
			{
				DEBUG_BREAK();
				delete pObjectInfo;

				break;
			}

			uint32_t dwIDDataLen = reader.ReadUInt32();
			uint32_t offsetStart = reader.GetOffset();

			if (dwIDDataLen > 0)
			{
				BinaryReader idReader(reader.GetDataPtr(), dwIDDataLen);

				idReader.ReadUInt32(); // 0xf7b0
				idReader.ReadUInt32(); // character
				idReader.ReadUInt32(); // sequence
				idReader.ReadUInt32(); // game event (0xc9)

				idReader.ReadUInt32(); // object
				uint32_t flags = idReader.ReadUInt32(); // flags
				idReader.ReadUInt32(); // success

				enum AppraisalProfilePackHeader {
					Packed_None = 0,
					Packed_IntStats = (1 << 0),
					Packed_BoolStats = (1 << 1),
					Packed_FloatStats = (1 << 2),
					Packed_StringStats = (1 << 3),
					Packed_SpellList = (1 << 4),
					Packed_WeaponProfile = (1 << 5),
					Packed_HookProfile = (1 << 6),
					Packed_ArmorProfile = (1 << 7),
					Packed_CreatureProfile = (1 << 8),
					Packed_ArmorEnchant = (1 << 9),
					Packed_ResistEnchant = (1 << 10),
					Packed_WeaponEnchant = (1 << 11),
					Packed_DataIDStats = (1 << 12),
					Packed_Int64Stats = (1 << 13),
					Packed_ArmorLevels = (1 << 14)
				};

				if (flags & Packed_IntStats)
				{
					pObjectInfo->uint32_tProperties = idReader.ReadMap<uint32_t, uint32_t>();
				}

				if (flags & Packed_Int64Stats)
					pObjectInfo->qwordProperties = idReader.ReadMap<uint32_t, UINT64>();

				if (flags & Packed_BoolStats)
					pObjectInfo->boolProperties = idReader.ReadMap<uint32_t, uint32_t>();

				if (flags & Packed_FloatStats)
					pObjectInfo->floatProperties = idReader.ReadMap<uint32_t, double>();

				if (idReader.GetLastError())
				{
					DEBUG_BREAK();
				}
			}			

			reader.SetOffset(offsetStart + dwIDDataLen);

			m_CapturedStaticsData.push_back(pObjectInfo);
		}

		delete [] data;
	}

#if 1 // deprecated reimplement
	for (auto pSpawnInfo : m_CapturedStaticsData)
	{
		uint32_t cell_id = pSpawnInfo->physics.pos.objcell_id;

		if (!cell_id)
			continue;

		m_CapturedStaticsDataByLandBlock[cell_id >> 16].push_back(pSpawnInfo);
	}

#if 0
	for (auto pSpawnInfo : m_CapturedStaticsData)
	{
		CWeenieObject *pSpawn;

		if (pSpawnInfo->guid >= 0x80000000)
			continue; // Dynamic perhaps

		if (pSpawnInfo->weenie._bitfield & BitfieldIndex::BF_CORPSE)
			continue;
		if (pSpawnInfo->weenie._bitfield & BitfieldIndex::BF_PLAYER)
			continue;
		if (pSpawnInfo->physics.state & PhysicsState::MISSILE_PS)
			continue;

		if (pSpawnInfo->weenie._bitfield & BitfieldIndex::BF_DOOR)
			pSpawn = new CBaseDoor();
		else if (pSpawnInfo->weenie._type == ITEM_TYPE::TYPE_CREATURE)
			pSpawn = new CMonsterWeenie();
		else if (pSpawnInfo->weenie._type == ITEM_TYPE::TYPE_PORTAL)
			pSpawn = new CPortal();
		else if (pSpawnInfo->weenie._type == ITEM_TYPE::TYPE_LIFESTONE)
			pSpawn = new CBaseLifestone();
		else
			pSpawn = new CWeenieObject();

		pSpawn->id = pSpawnInfo->guid; // g_pWorld->GenerateGUID(eDynamicGUID);
		pSpawn->m_miBaseModel = pSpawnInfo->appearance;
		pSpawn->SetSetupID(pSpawnInfo->physics.setup_id);
		pSpawn->SetMotionTableID(pSpawnInfo->physics.mtable_id);
		pSpawn->SetSoundTableID(pSpawnInfo->physics.stable_id);
		pSpawn->SetPETableID(pSpawnInfo->physics.phstable_id);
		pSpawn->SetScale(pSpawnInfo->physics.object_scale);
		pSpawn->SetItemType(pSpawnInfo->weenie._type);
		pSpawn->SetName(pSpawnInfo->weenie._name.c_str());
		pSpawn->SetInitialPosition(pSpawnInfo->physics.pos);

		/*
		if (pSpawnInfo->physics.movement_buffer)
		{
			pSpawn->m_AnimOverrideData = pSpawnInfo->physics.movement_buffer;
			pSpawn->m_AnimOverrideDataLen = pSpawnInfo->physics.movement_buffer_length;
			pSpawn->m_AutonomousMovement = pSpawnInfo->physics.autonomous_movement;
		}
		*/

		pSpawn->m_bDontClear = true;
		pSpawn->m_RadarVis = pSpawnInfo->weenie._radar_enum;
		pSpawn->SetRadarBlipColor(pSpawnInfo->weenie._blipColor);
		pSpawn->m_TargetType = pSpawnInfo->weenie._targetType;
		pSpawn->m_WCID = pSpawnInfo->weenie._wcid;
		pSpawn->m_Qualities.SetInt(ENCUMB_VAL_INT, pSpawnInfo->weenie._burden);
		pSpawn->m_Qualities.SetInt(VALUE_INT, pSpawnInfo->weenie._value);
		pSpawn->SetInitialPhysicsState(pSpawnInfo->physics.state);
		pSpawn->m_Qualities.SetFloat(TRANSLUCENCY_FLOAT, pSpawnInfo->physics.translucency);

		g_pWorld->CreateEntity(pSpawn);

		spawned++;
	}
#endif
#endif

	if (!spawned)
	{
		SERVER_WARN << "Spawn data not included. Spawning functionality may be limited.";
	}
	else
	{
		SERVER_INFO << "Spawned" << spawned << "persistent dynamic objects!";
	}
}

void CGameDatabase::SpawnStaticsForLandBlock(WORD lb_gid)
{
	std::list<CCapturedWorldObjectInfo *> *pStatics = GetStaticsForLandBlock(lb_gid);
	if (!pStatics)
		return;

	for (auto &pSpawnInfo : *pStatics)
	{
		// CWeenieObject *pSpawn;

		if (pSpawnInfo->guid >= 0x80000000)
		{
			if (!strcmp(pSpawnInfo->weenie._name.c_str(), "Town Crier"))
			{				
			}
			else
				continue; // Dynamic perhaps

			// make sure there isn't another one nearby
			std::list<CWeenieObject *> results;
			g_pWorld->EnumNearby(pSpawnInfo->physics.pos, 4.0f, &results);

			bool bFound = false;
			for (auto weenie : results)
			{
				if (weenie->m_Qualities.id == pSpawnInfo->weenie._wcid)
				{
					bFound = true;
					break;
				}
			}

			if (bFound)
				continue;
		}

		if (pSpawnInfo->weenie._bitfield & BitfieldIndex::BF_CORPSE)
			continue;
		if (pSpawnInfo->weenie._bitfield & BitfieldIndex::BF_PLAYER)
			continue;
		if (pSpawnInfo->physics.state & PhysicsState::MISSILE_PS)
			continue;

		CWeenieObject *weenie = g_pWeenieFactory->CreateWeenieByClassID(pSpawnInfo->weenie._wcid, &pSpawnInfo->physics.pos, false);
		if (weenie)
		{
			// weenie->m_ObjDesc = pSpawnInfo->objdesc;
			weenie->SetName(pSpawnInfo->weenie._name.c_str());
			weenie->m_bDontClear = true;

			g_pWorld->CreateEntity(weenie);
		}

#if 0
		if (pSpawnInfo->weenie._bitfield & BitfieldIndex::BF_DOOR)
			pSpawn = new CBaseDoor();
		else if (pSpawnInfo->weenie._type == ITEM_TYPE::TYPE_LIFESTONE)
			pSpawn = new CBaseLifestone();
		else if (pSpawnInfo->weenie._type == ITEM_TYPE::TYPE_CREATURE)
		{
			if (!strcmp(pSpawnInfo->weenie._name.c_str(), "Town Crier"))
				pSpawn = new CTownCrier();
			else if (pSpawnInfo->weenie._bitfield & BF_VENDOR)
				pSpawn = new CVendor();
			else
				pSpawn = new CMonsterWeenie();
		}
		else if (pSpawnInfo->weenie._type == ITEM_TYPE::TYPE_PORTAL)
			pSpawn = new CPortal();
		else
			pSpawn = new CWeenieObject();

		pSpawn->id = pSpawnInfo->guid; // g_pWorld->GenerateGUID(eDynamicGUID);
		pSpawn->m_miBaseModel = pSpawnInfo->appearance;

		if (pSpawnInfo->physics.bitfield & PhysicsDescInfo::CSETUP)
			pSpawn->SetSetupID(pSpawnInfo->physics.setup_id);

		if (pSpawnInfo->physics.bitfield & PhysicsDescInfo::MTABLE)
			pSpawn->SetMotionTableID(pSpawnInfo->physics.mtable_id);

		if (pSpawnInfo->physics.bitfield & PhysicsDescInfo::STABLE)
			pSpawn->SetSoundTableID(pSpawnInfo->physics.stable_id);

		if (pSpawnInfo->physics.bitfield & PhysicsDescInfo::PETABLE)
			pSpawn->SetPETableID(pSpawnInfo->physics.phstable_id);

		if (pSpawnInfo->physics.bitfield & PhysicsDescInfo::OBJSCALE)
			pSpawn->SetScale(pSpawnInfo->physics.object_scale);

		pSpawn->SetItemType(pSpawnInfo->weenie._type);
		pSpawn->SetName(pSpawnInfo->weenie._name.c_str());
		pSpawn->SetInitialPosition(pSpawnInfo->physics.pos);

		/*
		if (pSpawnInfo->physics.movement_buffer)
		{
		pSpawn->m_AnimOverrideData = pSpawnInfo->physics.movement_buffer;
		pSpawn->m_AnimOverrideDataLen = pSpawnInfo->physics.movement_buffer_length;
		pSpawn->m_AutonomousMovement = pSpawnInfo->physics.autonomous_movement;
		}
		*/

		pSpawn->m_bDontClear = true;
		
		uint32_t weenieHeader = 0;
		pSpawnInfo->weenie.set_pack_header(&weenieHeader);

		if (weenieHeader & PWD_Packed_Useability)
			pSpawn->m_Qualities.SetInt(ITEM_USEABLE_INT, pSpawnInfo->weenie._useability);

		if (weenieHeader & PWD_Packed_UseRadius)
			pSpawn->m_Qualities.SetFloat(USE_RADIUS_FLOAT, pSpawnInfo->weenie._useRadius);

		if (weenieHeader & PWD_Packed_RadarEnum)
			pSpawn->m_Qualities.SetInt(SHOWABLE_ON_RADAR_INT, pSpawnInfo->weenie._radar_enum);

		if (weenieHeader & PWD_Packed_BlipColor)
			pSpawn->SetRadarBlipColor(pSpawnInfo->weenie._blipColor);

		pSpawn->m_Qualities.SetInt(TARGET_TYPE_INT, pSpawnInfo->weenie._targetType);
		pSpawn->m_Qualities.id = pSpawnInfo->weenie._wcid;

		if (weenieHeader & PWD_Packed_Burden)
			pSpawn->m_Qualities.SetInt(ENCUMB_VAL_INT, pSpawnInfo->weenie._burden);

		if (weenieHeader & PWD_Packed_Value)
			pSpawn->m_Qualities.SetInt(VALUE_INT, pSpawnInfo->weenie._value);

		pSpawn->SetInitialPhysicsState(pSpawnInfo->physics.state);

		if (pSpawnInfo->physics.bitfield & PhysicsDescInfo::TRANSLUCENCY)
			pSpawn->m_fTranslucency = pSpawnInfo->physics.translucency;
				
		// Make attackable
		/*
		if (pSpawn->IsMonsterWeenie())
		{
			pSpawn->SetInitialPhysicsState(pSpawnInfo->physics.state & ~(REPORT_COLLISIONS_AS_ENVIRONMENT_PS));
		}
		*/

		g_pWorld->CreateEntity(pSpawn);

		/*
		if (pSpawn->IsMonsterWeenie())
		{
			CMonsterWeenie *pMonster = (CMonsterWeenie *)pSpawn;
			pMonster->m_MonsterAI = new MonsterAIManager(pMonster, pMonster->m_Position);
		}
		*/
#endif
	}
}

CCapturedWorldObjectInfo *CGameDatabase::GetRandomCapturedMonsterData()
{
	uint32_t numMonsters = (uint32_t) m_CapturedMonsterDataList.size();
	if (numMonsters < 1)
		return NULL;

	unsigned int monster = Random::GenUInt(0, numMonsters - 1);

	return m_CapturedMonsterDataList[monster];
}


std::list<CCapturedWorldObjectInfo *> *CGameDatabase::GetStaticsForLandBlock(WORD lb_gid)
{
	std::map<WORD, std::list<CCapturedWorldObjectInfo *>>::iterator i = m_CapturedStaticsDataByLandBlock.find(lb_gid);

	if (i == m_CapturedStaticsDataByLandBlock.end())
		return NULL;

	return &i->second;
}

CCapturedWorldObjectInfo *CGameDatabase::GetCapturedMonsterData(const char *name)
{
	std::map<std::string, CCapturedWorldObjectInfo *>::iterator i = m_CapturedMonsterData.find(ConvertNameForLookup(name));

	if (i == m_CapturedMonsterData.end())
		return NULL;

	return i->second;
}

void CGameDatabase::LoadCapturedMonsterData()
{
	// THIS IS ALL TEMPORARY FOR TESTING
	// Consolidate or remove this method of loading (this was copied and pasted)

	BYTE *data;
	uint32_t length;

	if (LoadDataFromFile("data/misc/monsters.dat", &data, &length))
	{
		BinaryReader reader(data, length);
		unsigned int count = reader.ReadUInt32();

		for (unsigned int i = 0; i < count; i++)
		{
			CCapturedWorldObjectInfo *pMonsterInfo = new CCapturedWorldObjectInfo;
			pMonsterInfo->m_ObjName = reader.ReadString();

			uint32_t dwObjDataLen = reader.ReadUInt32(); // size of this

			reader.ReadUInt32(); // 0xf745
			uint32_t dwGUID = reader.ReadUInt32(); // GUID

			pMonsterInfo->objdesc.UnPack(&reader);
			pMonsterInfo->physics.Unpack(reader);
			pMonsterInfo->weenie.UnPack(&reader);

			if (reader.GetLastError())
			{
				DEBUG_BREAK();
				delete pMonsterInfo;
				break;
			}

			uint32_t dwIDDataLen = reader.ReadUInt32();
			uint32_t offsetStart = reader.GetOffset();

			if (dwIDDataLen > 0)
			{
				BinaryReader idReader(reader.GetDataPtr(), dwIDDataLen);

				idReader.ReadUInt32(); // 0xf7b0
				idReader.ReadUInt32(); // character
				idReader.ReadUInt32(); // sequence
				idReader.ReadUInt32(); // game event (0xc9)

				idReader.ReadUInt32(); // object
				uint32_t flags = idReader.ReadUInt32(); // flags
				idReader.ReadUInt32(); // success

				enum AppraisalProfilePackHeader {
					Packed_None = 0,
					Packed_IntStats = (1 << 0),
					Packed_BoolStats = (1 << 1),
					Packed_FloatStats = (1 << 2),
					Packed_StringStats = (1 << 3),
					Packed_SpellList = (1 << 4),
					Packed_WeaponProfile = (1 << 5),
					Packed_HookProfile = (1 << 6),
					Packed_ArmorProfile = (1 << 7),
					Packed_CreatureProfile = (1 << 8),
					Packed_ArmorEnchant = (1 << 9),
					Packed_ResistEnchant = (1 << 10),
					Packed_WeaponEnchant = (1 << 11),
					Packed_DataIDStats = (1 << 12),
					Packed_Int64Stats = (1 << 13),
					Packed_ArmorLevels = (1 << 14)
				};

				if (idReader.GetLastError())
				{
					DEBUG_BREAK();
				}

				if (flags & Packed_IntStats)
				{
					pMonsterInfo->uint32_tProperties = idReader.ReadMap<uint32_t, uint32_t>();
				}

				if (idReader.GetLastError())
				{
					DEBUG_BREAK();
				}

				if (flags & Packed_Int64Stats)
				{
					pMonsterInfo->qwordProperties = idReader.ReadMap<uint32_t, UINT64>();
				}

				if (idReader.GetLastError())
				{
					DEBUG_BREAK();
				}

				if (flags & Packed_BoolStats)
				{
					pMonsterInfo->boolProperties = idReader.ReadMap<uint32_t, uint32_t>();
				}

				if (idReader.GetLastError())
				{
					DEBUG_BREAK();
				}

				if (flags & Packed_FloatStats)
				{
					pMonsterInfo->floatProperties = idReader.ReadMap<uint32_t, double>();
				}

				if (idReader.GetLastError())
				{
					DEBUG_BREAK();
				}

				if (flags & Packed_StringStats)
				{
					pMonsterInfo->stringProperties = idReader.ReadMap<uint32_t>();
				}

				if (idReader.GetLastError())
				{
					DEBUG_BREAK();
				}

				if (flags & Packed_DataIDStats)
				{
					pMonsterInfo->dataIDProperties = idReader.ReadMap<uint32_t, uint32_t>();
				}

				if (idReader.GetLastError())
				{
					DEBUG_BREAK();
				}

				if (flags & Packed_SpellList)
				{
					idReader.ReadMap<WORD, WORD>();
				}

				if (idReader.GetLastError())
				{
					DEBUG_BREAK();
				}

				if (flags & Packed_ArmorProfile)
				{
					// Protections
					idReader.ReadFloat();
					idReader.ReadFloat();
					idReader.ReadFloat();
					idReader.ReadFloat();
					idReader.ReadFloat();
					idReader.ReadFloat();
					idReader.ReadFloat();
					idReader.ReadFloat();
				}

				if (idReader.GetLastError())
				{
					DEBUG_BREAK();
				}

				if (flags & Packed_CreatureProfile)
				{
					uint32_t flags = idReader.ReadUInt32();					
				}
			}

			reader.SetOffset(offsetStart + dwIDDataLen);

			m_CapturedMonsterData.insert(std::pair<std::string, CCapturedWorldObjectInfo*>(ConvertNameForLookup(pMonsterInfo->m_ObjName), pMonsterInfo));

			if (!(pMonsterInfo->weenie._bitfield & BitfieldIndex::BF_PLAYER))
			{
				m_CapturedMonsterDataList.push_back(pMonsterInfo);
			}
		}

		delete [] data;
	}
}

CCapturedWorldObjectInfo *CGameDatabase::GetRandomCapturedItemData()
{
	uint32_t numItems = (uint32_t) m_CapturedItemDataList.size();

	if (numItems < 1)
	{
		return NULL;
	}

	unsigned int Item = Random::GenUInt(0, numItems - 1);

	return m_CapturedItemDataList[Item];
}

CCapturedWorldObjectInfo *CGameDatabase::GetCapturedItemData(const char *name)
{
	std::map<std::string, CCapturedWorldObjectInfo *>::iterator i = m_CapturedItemData.find(ConvertNameForLookup(name));

	if (i == m_CapturedItemData.end())
		return NULL;

	return i->second;
}

void CGameDatabase::LoadCapturedItemData()
{
	// THIS IS ALL TEMPORARY FOR TESTING
	// Consolidate or remove this method of loading (this was copied and pasted)

	BYTE *data;
	uint32_t length;

	if (LoadDataFromFile("data/misc/items.dat", &data, &length))
	{
		BinaryReader reader(data, length);
		unsigned int count = reader.ReadUInt32();

		for (unsigned int i = 0; i < count; i++)
		{
			CCapturedWorldObjectInfo *pItemInfo = new CCapturedWorldObjectInfo;
			pItemInfo->m_ObjName = reader.ReadString();

			std::transform(pItemInfo->m_ObjName.begin(), pItemInfo->m_ObjName.end(), pItemInfo->m_ObjName.begin(), ::tolower);
			pItemInfo->m_ObjName.erase(remove_if(pItemInfo->m_ObjName.begin(), pItemInfo->m_ObjName.end(), isspace), pItemInfo->m_ObjName.end());

			// make all lowercase and ignore spaces for searching purposes

			uint32_t dwObjDataLen = reader.ReadUInt32(); // size of this
			uint32_t offset_start = reader.GetOffset();

			reader.ReadUInt32(); // 0xf745
			uint32_t dwGUID = reader.ReadUInt32(); // GUID

			pItemInfo->objdesc.UnPack(&reader);
			pItemInfo->physics.Unpack(reader);
			pItemInfo->weenie.UnPack(&reader);

			if (reader.GetLastError())
			{
				delete pItemInfo;
				break;
			}

			if (reader.GetOffset() != (offset_start + dwObjDataLen))
			{
				DEBUG_BREAK();
			}

			uint32_t dwIDDataLen = reader.ReadUInt32();
			uint32_t offsetStart = reader.GetOffset();

			if (dwIDDataLen > 0)
			{
				BinaryReader idReader(reader.GetDataPtr(), dwIDDataLen);

				idReader.ReadUInt32(); // 0xf7b0
				idReader.ReadUInt32(); // character
				idReader.ReadUInt32(); // sequence
				idReader.ReadUInt32(); // game event (0xc9)

				idReader.ReadUInt32(); // object
				uint32_t flags = idReader.ReadUInt32(); // flags
				idReader.ReadUInt32(); // success

				enum AppraisalProfilePackHeader {
					Packed_None = 0,
					Packed_IntStats = (1 << 0),
					Packed_BoolStats = (1 << 1),
					Packed_FloatStats = (1 << 2),
					Packed_StringStats = (1 << 3),
					Packed_SpellList = (1 << 4),
					Packed_WeaponProfile = (1 << 5),
					Packed_HookProfile = (1 << 6),
					Packed_ArmorProfile = (1 << 7),
					Packed_CreatureProfile = (1 << 8),
					Packed_ArmorEnchant = (1 << 9),
					Packed_ResistEnchant = (1 << 10),
					Packed_WeaponEnchant = (1 << 11),
					Packed_DataIDStats = (1 << 12),
					Packed_Int64Stats = (1 << 13),
					Packed_ArmorLevels = (1 << 14)
				};

				if (idReader.GetLastError())
				{
					DEBUG_BREAK();
				}

				if (flags & Packed_IntStats)
				{
					pItemInfo->uint32_tProperties = idReader.ReadMap<uint32_t, uint32_t>();
				}

				if (idReader.GetLastError())
				{
					DEBUG_BREAK();
				}

				if (flags & Packed_Int64Stats)
				{
					pItemInfo->qwordProperties = idReader.ReadMap<uint32_t, UINT64>();
				}

				if (idReader.GetLastError())
				{
					DEBUG_BREAK();
				}

				if (flags & Packed_BoolStats)
				{
					pItemInfo->boolProperties = idReader.ReadMap<uint32_t, uint32_t>();
				}

				if (idReader.GetLastError())
				{
					DEBUG_BREAK();
				}

				if (flags & Packed_FloatStats)
				{
					pItemInfo->floatProperties = idReader.ReadMap<uint32_t, double>();
				}

				if (idReader.GetLastError())
				{
					DEBUG_BREAK();
				}

				/*
				if (flags & Packed_StringStats)
				pItemInfo->stringProperties = idReader.ReadMap<uint32_t>();

				if (flags & Packed_DataIDStats)
				pItemInfo->dataIDProperties = idReader.ReadMap<uint32_t, uint32_t>();
				*/
			}

			reader.SetOffset(offsetStart + dwIDDataLen);

			m_CapturedItemData.insert(std::pair<std::string, CCapturedWorldObjectInfo*>(pItemInfo->m_ObjName, pItemInfo));

			if (!(pItemInfo->weenie._bitfield & BitfieldIndex::BF_PLAYER))
			{
				m_CapturedItemDataList.push_back(pItemInfo);
			}
		}

		delete [] data;
	}
}


CCapturedWorldObjectInfo *CGameDatabase::GetRandomCapturedArmorData()
{
	uint32_t numItems = (uint32_t) m_CapturedArmorDataList.size();
	if (numItems < 1)
		return NULL;

	unsigned int Item = Random::GenUInt(0, numItems - 1);
	return m_CapturedArmorDataList[Item];
}

CCapturedWorldObjectInfo *CGameDatabase::GetCapturedArmorData(const char *name)
{
	std::string searchName = name;
	std::transform(searchName.begin(), searchName.end(), searchName.begin(), ::tolower);
	searchName.erase(remove_if(searchName.begin(), searchName.end(), isspace), searchName.end());

	std::map<std::string, CCapturedWorldObjectInfo *>::iterator i = m_CapturedArmorData.find(searchName);

	if (i == m_CapturedArmorData.end())
		return NULL;

	return i->second;
}

void CGameDatabase::LoadCapturedArmorData()
{
	// THIS IS ALL TEMPORARY FOR TESTING
	// Consolidate or remove this method of loading (this was copied and pasted)

	BYTE *data;
	uint32_t length;

	if (LoadDataFromFile("data/misc/armor.dat", &data, &length))
	{
		BinaryReader reader(data, length);
		unsigned int count = reader.ReadUInt32();

		for (unsigned int i = 0; i < count; i++)
		{
			CCapturedWorldObjectInfo *pItemInfo = new CCapturedWorldObjectInfo;
			pItemInfo->m_ObjName = reader.ReadString();

			std::transform(pItemInfo->m_ObjName.begin(), pItemInfo->m_ObjName.end(), pItemInfo->m_ObjName.begin(), ::tolower);
			pItemInfo->m_ObjName.erase(remove_if(pItemInfo->m_ObjName.begin(), pItemInfo->m_ObjName.end(), isspace), pItemInfo->m_ObjName.end());

			// make all lowercase and ignore spaces for searching purposes

			uint32_t dwObjDataLen = reader.ReadUInt32(); // size of this
			uint32_t offset_start = reader.GetOffset();

			reader.ReadUInt32(); // 0xf745
			uint32_t dwGUID = reader.ReadUInt32(); // GUID

			pItemInfo->objdesc.UnPack(&reader);
			pItemInfo->physics.Unpack(reader);
			pItemInfo->weenie.UnPack(&reader);

			if (reader.GetLastError())
			{
				delete pItemInfo;
				break;
			}

			if (reader.GetOffset() != (offset_start + dwObjDataLen))
			{
				DEBUG_BREAK();
			}

			uint32_t dwIDDataLen = reader.ReadUInt32();
			uint32_t offsetStart = reader.GetOffset();

			if (dwIDDataLen > 0)
			{
				BinaryReader idReader(reader.GetDataPtr(), dwIDDataLen);

				idReader.ReadUInt32(); // 0xf7b0
				idReader.ReadUInt32(); // character
				idReader.ReadUInt32(); // sequence
				idReader.ReadUInt32(); // game event (0xc9)

				idReader.ReadUInt32(); // object
				uint32_t flags = idReader.ReadUInt32(); // flags
				idReader.ReadUInt32(); // success

				enum AppraisalProfilePackHeader {
					Packed_None = 0,
					Packed_IntStats = (1 << 0),
					Packed_BoolStats = (1 << 1),
					Packed_FloatStats = (1 << 2),
					Packed_StringStats = (1 << 3),
					Packed_SpellList = (1 << 4),
					Packed_WeaponProfile = (1 << 5),
					Packed_HookProfile = (1 << 6),
					Packed_ArmorProfile = (1 << 7),
					Packed_CreatureProfile = (1 << 8),
					Packed_ArmorEnchant = (1 << 9),
					Packed_ResistEnchant = (1 << 10),
					Packed_WeaponEnchant = (1 << 11),
					Packed_DataIDStats = (1 << 12),
					Packed_Int64Stats = (1 << 13),
					Packed_ArmorLevels = (1 << 14)
				};

				if (idReader.GetLastError())
				{
					DEBUG_BREAK();
				}

				if (flags & Packed_IntStats)
					pItemInfo->uint32_tProperties = idReader.ReadMap<uint32_t, uint32_t>();

				if (idReader.GetLastError())
				{
					DEBUG_BREAK();
				}

				if (flags & Packed_Int64Stats)
				{
					pItemInfo->qwordProperties = idReader.ReadMap<uint32_t, UINT64>();
				}

				if (idReader.GetLastError())
				{
					DEBUG_BREAK();
				}

				if (flags & Packed_BoolStats)
				{
					pItemInfo->boolProperties = idReader.ReadMap<uint32_t, uint32_t>();
				}

				if (idReader.GetLastError())
				{
					DEBUG_BREAK();
				}

				if (flags & Packed_FloatStats)
				{
					pItemInfo->floatProperties = idReader.ReadMap<uint32_t, double>();
				}

				if (idReader.GetLastError())
				{
					DEBUG_BREAK();
				}

				/* .... etc...
				if (flags & Packed_StringStats)
				pItemInfo->stringProperties = idReader.ReadMap<uint32_t>();

				if (flags & Packed_DataIDStats)
				pItemInfo->dataIDProperties = idReader.ReadMap<uint32_t, uint32_t>();
				*/
			}

			reader.SetOffset(offsetStart + dwIDDataLen);

			uint32_t dwWornModelDataLen = reader.ReadUInt32();
			offsetStart = reader.GetOffset();

			/*
			{
				reader.ReadBYTE();
				numPalette = reader.ReadBYTE();
				numTex = reader.ReadBYTE();
				numModel = reader.ReadBYTE();

				if (numPalette)
				{
					pItemInfo->wornAppearance.dwBasePalette = reader.ReadPackeduint32_t(); // actually packed, fix this
					for (int j = 0; j < numPalette; j++)
					{
						uint32_t replacement = reader.ReadPackeduint32_t(); // actually packed, fix this
						BYTE offset = reader.ReadBYTE();
						BYTE range = reader.ReadBYTE();
						pItemInfo->wornAppearance.lPalettes.push_back(PaletteRpl(replacement, offset, range));
					}
				}

				for (int j = 0; j < numTex; j++)
				{
					BYTE index = reader.ReadBYTE();
					uint32_t oldT = reader.ReadPackeduint32_t();
					uint32_t newT = reader.ReadPackeduint32_t();
					pItemInfo->wornAppearance.lTextures.push_back(TextureRpl(index, oldT, newT));
				}

				for (int j = 0; j < numModel; j++)
				{
					BYTE index = reader.ReadBYTE();
					uint32_t newM = reader.ReadPackeduint32_t();
					pItemInfo->wornAppearance.lModels.push_back(ModelRpl(index, newM));
				}
			}
			*/

			reader.SetOffset(offsetStart + dwWornModelDataLen);

			m_CapturedArmorData.insert(std::pair<std::string, CCapturedWorldObjectInfo*>(pItemInfo->m_ObjName, pItemInfo));
			m_CapturedArmorDataList.push_back(pItemInfo);
		}

		delete [] data;
	}
}

uint32_t Parseuint32_tFromStringHex(const char *hex)
{
	return strtoul(hex, NULL, 16);
}

float ParseFloatFromStringHex(const char *hex)
{
	uint32_t hexVal = Parseuint32_tFromStringHex(hex);
	return *((float *)&hexVal);
}

void CGameDatabase::LoadTeleTownList()
{
	bool bQuerySuccess = g_pDB2->Query("SELECT `ID`, `Description`, `Command`, `Landblock`, `Position_X`, `Position_Y`, `Position_Z`, `Orientation_W`, `Orientation_X`, `Orientation_Y`, `Orientation_Z` from teletowns");

	if (bQuerySuccess)
	{
		CSQLResult *Result = g_pDB2->GetResult();

		if (Result)
		{
			SQLResultRow_t ResultRow;
			while (ResultRow = Result->FetchRow())
			{
				TeleTownList_t t;
				t.m_teleString = ResultRow[2];
				t.position.objcell_id = Parseuint32_tFromStringHex(ResultRow[3]);
				t.position.frame.m_origin.x = ParseFloatFromStringHex(ResultRow[4]);
				t.position.frame.m_origin.y = ParseFloatFromStringHex(ResultRow[5]);
				t.position.frame.m_origin.z = ParseFloatFromStringHex(ResultRow[6]);
				t.position.frame.m_angles.w = ParseFloatFromStringHex(ResultRow[7]);
				t.position.frame.m_angles.x = ParseFloatFromStringHex(ResultRow[8]);
				t.position.frame.m_angles.y = ParseFloatFromStringHex(ResultRow[9]);
				t.position.frame.m_angles.z = ParseFloatFromStringHex(ResultRow[10]);
				t.position.frame.cache();

				g_pWorld->InsertTeleportLocation(t);
			}

#ifndef PUBLIC_BUILD
			SERVER_INFO << "Added" << Result->ResultRows() << "teleport locations.";
#endif
			delete Result;
		}
	}
}

CWeenieObject *CGameDatabase::CreateFromCapturedData(CCapturedWorldObjectInfo *pObjectInfo)
{
	if (!pObjectInfo)
	{
		return NULL;
	}

	CWeenieObject *pObject;

	if (pObjectInfo->weenie._type & ITEM_TYPE::TYPE_CREATURE)
	{
		pObject = new CMonsterWeenie();
	}
	else if (pObjectInfo->weenie._location ||
		(pObjectInfo->weenie._type &
		(ITEM_TYPE::TYPE_ARMOR | ITEM_TYPE::TYPE_CLOTHING | ITEM_TYPE::TYPE_FOOD | ITEM_TYPE::TYPE_JEWELRY |
			ITEM_TYPE::TYPE_KEY | ITEM_TYPE::TYPE_MAGIC_WIELDABLE | ITEM_TYPE::TYPE_MANASTONE | ITEM_TYPE::TYPE_CLOTHING |
			ITEM_TYPE::TYPE_FOOD | ITEM_TYPE::TYPE_JEWELRY | ITEM_TYPE::TYPE_KEY | ITEM_TYPE::TYPE_MAGIC_WIELDABLE |
			ITEM_TYPE::TYPE_MANASTONE | ITEM_TYPE::TYPE_MELEE_WEAPON | ITEM_TYPE::TYPE_MISSILE_WEAPON | ITEM_TYPE::TYPE_MONEY |
			ITEM_TYPE::TYPE_PROMISSORY_NOTE | ITEM_TYPE::TYPE_SPELL_COMPONENTS | ITEM_TYPE::TYPE_CASTER)))
	{
		pObject = new CWeenieObject();
	}
	else
	{
		pObject = new CWeenieObject();
	}

	// pObject->SetID(pObjectInfo->guid);
	pObject->m_ObjDescOverride = pObjectInfo->objdesc;
	pObject->SetSetupID(pObjectInfo->physics.setup_id);
	pObject->SetMotionTableID(pObjectInfo->physics.mtable_id);
	pObject->SetSoundTableID(pObjectInfo->physics.stable_id);
	pObject->SetPETableID(pObjectInfo->physics.phstable_id);
	pObject->SetScale(pObjectInfo->physics.object_scale);
	pObject->SetItemType(pObjectInfo->weenie._type);
	pObject->SetName(pObjectInfo->weenie._name.c_str());
	pObject->SetInitialPosition(pObjectInfo->physics.pos);

	pObject->SetIcon(pObjectInfo->weenie._iconID);
	pObject->m_Qualities.SetInt(UI_EFFECTS_INT, pObjectInfo->weenie._effects);
	pObject->m_Qualities.SetInt(COMBAT_USE_INT, pObjectInfo->weenie._combatUse);
	pObject->m_Qualities.SetInt(LOCATIONS_INT, pObjectInfo->weenie._valid_locations);
	pObject->m_Qualities.SetInt(CLOTHING_PRIORITY_INT, pObjectInfo->weenie._priority);

	pObject->m_Qualities.SetInt(SHOWABLE_ON_RADAR_INT, pObjectInfo->weenie._radar_enum);
	pObject->SetRadarBlipColor((RadarBlipEnum) pObjectInfo->weenie._blipColor);
	pObject->m_Qualities.SetInt(TARGET_TYPE_INT, pObjectInfo->weenie._targetType);
	pObject->m_Qualities.id = pObjectInfo->weenie._wcid;
	// pObject->m_PhysicsState = pObjectInfo->physics.state;
	pObject->m_Qualities.SetFloat(TRANSLUCENCY_FLOAT, pObjectInfo->physics.translucency);

	/* TODO
	pObject->m_Qualities.SetInt(pObjectInfo->uint32_tProperties);
	pObject->m_Qualities.SetInt64(pObjectInfo->qwordProperties);
	pObject->m_Qualities.SetBool(pObjectInfo->boolProperties);
	pObject->m_Qualities.m_FloatStats.Set(pObjectInfo->floatProperties);
	pObject->m_Qualities.SetString(pObjectInfo->stringProperties);
	pObject->m_Qualities.SetDataID(pObjectInfo->dataIDProperties);
	*/

	// pObject->m_miWornModel = pObjectInfo->wornAppearance;

#if 0
	if (pObjectInfo->weenie._type & (ITEM_TYPE::TYPE_ARMOR | ITEM_TYPE::TYPE_CLOTHING))
	{
		uint32_t iconID = pObjectInfo->weenie._iconID;

		if (iconID)
		{
			uint32_t paletteKey;
			ClothingTable *pCT = g_ClothingCache.GetTableByIndexOfIconID(iconID, 0, &paletteKey);

			if (pCT)
			{
				CWeenieObject *pItem = pObject;
				pItem->m_WornObjDesc.Clear();
				pItem->m_WornObjDesc.paletteID = 0x0400007E; // pObject->m_miBaseModel.dwBasePalette;

				double shade = 1.0;

				pCT->BuildObjDesc(0x02000001, paletteKey, &ShadePackage(Random::GenFloat(0.0, 1.0)), &pItem->m_WornObjDesc);

				// use the original palettes specified?		
				pItem->m_WornObjDesc.ClearSubpalettes();
				Subpalette *pSPC = pObject->m_ObjDescOverride.firstSubpal;
				while (pSPC)
				{
					pItem->m_WornObjDesc.AddSubpalette(new Subpalette(*pSPC));
					pSPC = pSPC->next;
				}
#if 0
				/*
				Subpalette *pSPC = od.firstSubpal;
				while (pSPC)
				{
					pItem->m_miWornModel.lPalettes.push_back(PaletteRpl((WORD)pSPC->subID, pSPC->offset >> 3, pSPC->numcolors >> 3));
					pSPC = pSPC->next;
				}
				*/

				Subpalette *pSPC = pObject->m_ObjDesc.firstSubpal;
				while (pSPC)
				{
					pItem->m_miWornModel.lPalettes.push_back(PaletteRpl((WORD)pSPC->subID, pSPC->offset >> 3, pSPC->numcolors >> 3));
					pSPC = pSPC->next;
				}
				// pItem->m_miWornModel.lPalettes = pItem->m_miBaseModel.lPalettes;

				TextureMapChange *pTMC = od.firstTMChange;
				while (pTMC)
				{
					pItem->m_miWornModel.lTextures.push_back(TextureRpl(pTMC->part_index, (WORD)pTMC->old_tex_id, (WORD)pTMC->new_tex_id));
					pTMC = pTMC->next;
				}

				AnimPartChange *pAPC = od.firstAPChange;
				while (pAPC)
				{
					pItem->m_miWornModel.lModels.push_back(ModelRpl(pAPC->part_index, (WORD)pAPC->part_id));
					pAPC = pAPC->next;
				}
#endif

				ClothingTable::Release(pCT);
			}
		}
	}
#endif

	return pObject;
}


