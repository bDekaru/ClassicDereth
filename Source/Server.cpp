
#include <StdAfx.h>
#include "Server.h"
#include "Database.h"
#include "World.h"
#include "Network.h"
#include "BinaryWriter.h"
#include "ChatMsgs.h"
#include "Database2.h"
#include "DatabaseIO.h"
#include "ObjCache.h"
#include "ObjectIDGen.h"
#include "RegionDesc.h"
#include "ClothingTable.h"
#include "ClothingCache.h"
#include "ServerCellManager.h"
#include "WeenieFactory.h"
#include "TreasureFactory.h"
#include "WeenieLandCoverage.h"
#include "InferredPortalData.h"
#include "InferredCellData.h"
#include "AllegianceManager.h"
#include "Player.h"
#include "GameEventManager.h"
#include "House.h"
#include "HouseManager.h"
#include "easylogging++.h"
#include "ChessManager.h"
#include "RecipeFactory.h"

// should all be encapsulated realistically, but we aren't going to multi-instance the server...
CDatabase *g_pDB = NULL;
CMYSQLDatabase *g_pDB2 = NULL;
CMYSQLDatabase *g_pDB2Async = NULL;
CMYSQLDatabase *g_pDBDynamicIDs = NULL;
CDatabaseIO *g_pDBIO = NULL;
CGameDatabase *g_pGameDatabase = NULL;
ServerCellManager *g_pCellManager = NULL;
CWorld *g_pWorld = NULL;
AllegianceManager *g_pAllegianceManager = NULL;
FellowshipManager *g_pFellowshipManager = NULL;
GameEventManager *g_pGameEventManager = NULL;
CObjectIDGenerator *g_pObjectIDGen = NULL;
CNetwork *g_pNetwork = NULL;
CWeenieFactory *g_pWeenieFactory = NULL;
CTreasureFactory *g_pTreasureFactory = NULL;
WeenieLandCoverage *g_pWeenieLandCoverage = NULL;
CInferredPortalData *g_pPortalDataEx = NULL;
CInferredCellData *g_pCellDataEx = NULL;
TURBINEPORTAL *g_pPortal = NULL;
TURBINECELL *g_pCell = NULL;
CHouseManager *g_pHouseManager = NULL;
RecipeFactory *g_pRecipeFactory = NULL;

CPhatServer::CPhatServer(const char *configFilePath)
{
	m_running = false;

	g_pConfig = &m_Config;
	m_Config.Load(configFilePath);

	m_fStartupTime = g_pGlobals->UpdateTime();

	m_serverThread = std::thread([&]() { InternalThreadProc(); });
}

CPhatServer::~CPhatServer()
{
	m_running = false;

	if (m_serverThread.joinable())
		m_serverThread.join();
}

uint32_t WINAPI CPhatServer::InternalThreadProcStatic(LPVOID lpThis)
{
	return ((CPhatServer *)lpThis)->InternalThreadProc();
}

uint32_t CPhatServer::InternalThreadProc()
{
	srand((unsigned int)time(NULL));
	Random::Init();

	time_point server_start = steady_clock::now();

	if (Init())
	{
		m_running = true;

		constexpr clock_duration target_rate(steady_clock::period::den / 30);
		constexpr clock_duration min_tick(1);

		while (m_running)
		{
			time_point tick_start = steady_clock::now();

			// consider reseeding the PRNG on some interval

			Tick();

			clock_duration tick_elapsed = steady_clock::now() - tick_start;
			clock_duration tick_rest = std::max(min_tick, target_rate - tick_elapsed);

			if (g_pConfig->FastTick())
				std::this_thread::yield();
			else
				std::this_thread::sleep_for(tick_rest);
		}
	}

	Shutdown();

	return 0;
}

bool CPhatServer::Init()
{
	unsigned long bind_ip = m_Config.BindIP();
	//assert(sizeof(bind_ip) == sizeof(in_addr));
	memset(&m_hostaddr, 0, sizeof(in_addr));
	m_hostaddr.s_addr = bind_ip;
	m_hostport = m_Config.BindPort();

	g_pGlobals->ResetPackets();

	g_pPortal = new TURBINEPORTAL();
	g_pCell = new TURBINECELL();

	ObjCaches::InitCaches(true, g_pGlobals->GetGameData("Data", "json").c_str());

	InitPhatDataBin(g_pGlobals->GetGameData("Data", "cache.bin").c_str());

	g_pPortalDataEx = new CInferredPortalData();
	g_pPortalDataEx->Init();

	g_pCellDataEx = new CInferredCellData();
	g_pCellDataEx->Init();

	g_pWeenieLandCoverage = new WeenieLandCoverage();

	// g_pWeenieLandCoverage->Initialize(); don't use this anymore

	g_pWeenieFactory = new CWeenieFactory();
	g_pWeenieFactory->Initialize();

	g_pTreasureFactory = new CTreasureFactory();
	g_pTreasureFactory->Initialize();

	g_pRecipeFactory = new RecipeFactory();
	g_pRecipeFactory->Initialize();

	g_pDB = new CDatabase(); // Old, dumb, bad

	g_pDBIO = new CDatabaseIO();
	g_pDB2 = new CMYSQLDatabase(
		m_Config.DatabaseIP(),
		m_Config.DatabasePort(),
		m_Config.DatabaseUsername(),
		m_Config.DatabasePassword(),
		m_Config.DatabaseName()); // Newer, shiny, makes pancakes in the morning

	g_pDBDynamicIDs = new CMYSQLDatabase(
		m_Config.DatabaseIP(),
		m_Config.DatabasePort(),
		m_Config.DatabaseUsername(),
		m_Config.DatabasePassword(),
		m_Config.DatabaseName());

	g_pHouseManager = new CHouseManager();

	g_pObjectIDGen = new CObjectIDGenerator();

	if (!g_pObjectIDGen->IsIdRangeValid())
	{
		WINLOG(Temp, Normal, "ID system failed to start.\n");
		SERVER_INFO << "ID system failed to start.";
		return false;
	}

	g_pGameDatabase = new CGameDatabase();
	g_pCellManager = new ServerCellManager();
	g_pAllegianceManager = new AllegianceManager();
	g_pFellowshipManager = new FellowshipManager();

	g_pGameEventManager = new GameEventManager();
	g_pGameEventManager->Initialize();

	g_pWorld = new CWorld();
	g_pWorld->Init();

	g_pGameDatabase->Init();

	//#ifndef QUICKSTART
	// LoadClothingTable();
	g_ClothingCache.LoadAll();
	//#endif

	// network should always be last
	g_pNetwork = new CNetwork(this, m_hostaddr, m_hostport);


	WINLOG(Temp, Normal, "The server is now online.\n");
	SERVER_INFO << "The server is now online.";

#if 0
	FILE *fp = fopen("d:\\temp\\houses.bin", "wb");

	// key dwelling ID, location of a dwelling portal
	PackableHashTable<uint32_t, PackableList<Position>> _dwellingDests;

	for (uint32_t i = 0; i <= 0xFFFF; i++)
	{
		uint32_t house_portal_iid = 0;
		Position house_portal_pos = 0;

		CLandBlockExtendedData *data = g_pCellDataEx->GetLandBlockData((uint32_t)i << 16);
		if (data)
		{
			for (uint32_t i = 0; i < data->weenies.num_used; i++)
			{
				uint32_t wcid = data->weenies.array_data[i].wcid;

				CWeenieDefaults *defaults = g_pWeenieFactory->GetWeenieDefaults(wcid);

				if (defaults->m_Qualities.m_WeenieType == HousePortal_WeenieType)
				{
					house_portal_iid = data->weenies.array_data[i].iid;
					house_portal_pos = data->weenies.array_data[i].pos;
					break;
				}
			}

			if (house_portal_iid)
			{
				for (uint32_t i = 0; i < data->weenie_links.num_used; i++)
				{
					uint32_t source_id = data->weenie_links.array_data[i].source;
					uint32_t target_id = data->weenie_links.array_data[i].target;

					if (source_id == house_portal_iid)
					{
						// get the target house
						for (uint32_t i = 0; i < data->weenies.num_used; i++)
						{
							uint32_t wcid = data->weenies.array_data[i].wcid;
							uint32_t iid = data->weenies.array_data[i].iid;

							if (iid == target_id)
							{
								CWeenieDefaults *defaults = g_pWeenieFactory->GetWeenieDefaults(wcid);

								if (defaults->m_Qualities.m_WeenieType == House_WeenieType)
								{
									_dwellingDests[defaults->m_Qualities.GetDID(HOUSEID_DID, 0)].push_back(house_portal_pos);
									break;
								}
							}
						}
					}
				}
			}
		}

		/*
		CLandBlockExtendedData *data = g_pCellDataEx->GetLandBlockData((uint32_t)i << 16);
		if (data)
		{
		for (uint32_t i = 0; i < data->weenies.num_used; i++)
		{
		uint32_t wcid = data->weenies.array_data[i].wcid;
		Position pos = data->weenies.array_data[i].pos;
		uint32_t iid = data->weenies.array_data[i].iid;

		if ((pos.objcell_id & 0xFFFF) >= 0x100)
		{
		}
		else
		{
		pos.objcell_id &= 0xFFFF0000;
		pos.objcell_id |= 1;
		pos.adjust_to_outside();
		}

		CWeenieDefaults *defaults = g_pWeenieFactory->GetWeenieDefaults(wcid);

		if (defaults->m_Qualities.m_WeenieType == House_WeenieType)
		{
		uint32_t houseIID = 0;
		uint32_t houseDID = 0;
		defaults->m_Qualities.InqInstanceID(HOUSE_IID, houseIID);
		defaults->m_Qualities.InqDataID(HOUSEID_DID, houseDID);

		if (houseDID || houseIID)
		{
		fprintf(fp, "%04X %08X %08X %X\n", i, pos.objcell_id, houseIID, houseDID);
		}
		}
		}
		}
		*/
	}

	BinaryWriter outData;
	_dwellingDests.Pack(&outData);
	fwrite(outData.GetData(), outData.GetSize(), 1, fp);
	fclose(fp);
#endif

	return true;
}

void CPhatServer::Shutdown()
{
	if (g_pNetwork)
	{
		SystemBroadcast("ATTENTION - This Asheron's Call Server is shutting down NOW!!!!");

		g_pNetwork->LogoutAll();
		g_pNetwork->Think();
#ifndef _DEBUG
		Sleep(5000); // Ugly but we're shutting down anyway, this allows the logout animation to complete before we kick everyone out of the server.
#endif
		g_pNetwork->CompleteLogoutAll();
		g_pNetwork->Think();

		SafeDelete(g_pNetwork);
	}
	g_pAllegianceManager->Save();
	SafeDelete(g_pWorld);
	SafeDelete(g_pGameEventManager);
	SafeDelete(g_pFellowshipManager);
	SafeDelete(g_pAllegianceManager);
	SafeDelete(g_pCellManager);
	SafeDelete(g_pGameDatabase);
	SafeDelete(g_pHouseManager);
	SafeDelete(g_pDB2);
	SafeDelete(g_pDBDynamicIDs);
	SafeDelete(g_pDBIO);
	SafeDelete(g_pDB);

	SafeDelete(g_pCellDataEx);
	SafeDelete(g_pPortalDataEx);

	SafeDelete(g_pTreasureFactory);
	SafeDelete(g_pRecipeFactory);
	SafeDelete(g_pWeenieFactory);
	SafeDelete(g_pWeenieLandCoverage);
	CleanupPhatDataBin();

	ObjCaches::DestroyCaches();
	SafeDelete(g_pCell);
	SafeDelete(g_pPortal);
	SafeDelete(g_pObjectIDGen);

	// for (int i = 0; i < m_socketCount; i++)
	// {
	//	if (m_sockets[i] != INVALID_SOCKET)
	//	{
	//		closesocket(m_sockets[i]);
	//		m_sockets[i] = INVALID_SOCKET;
	//	}
	// }
}

CStatTracker &CPhatServer::Stats()
{
	return m_Stats;
}

CPhatACServerConfig &CPhatServer::Config()
{
	return m_Config;
}

void CPhatServer::InitializeSocket(unsigned short port, in_addr address)
{
	//SOCKADDR_IN localhost;
	//localhost.sin_family = AF_INET;
	//localhost.sin_addr = address;

	//for (int i = 0; i < m_socketCount; i++)
	//	m_sockets[i] = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

	//u_short startport = port;
	//u_short failport = startport + 30;
	//while (port < failport)
	//{
	//	localhost.sin_port = htons(port);
	//	if (!bind(m_sockets[0], (struct sockaddr *)&localhost, sizeof(SOCKADDR_IN)))
	//	{
	//		WINLOG(Temp, Normal, "Bound to port %u!\n", port);
	//		SERVER_INFO << "Bound to port" << port << "!";
	//		break;
	//	}
	//	WINLOG(Temp, Normal, "Failed bind on port %u!\n", port);
	//	SERVER_ERROR << "Failed bind on port:" << port;
	//	port++;
	//}

	//if (port == failport)
	//{
	//	WINLOG(Temp, Normal, "Failure to bind socket!\n");
	//	SERVER_ERROR << "Failed bind on socket";
	//}
	//else
	//{
	//	// SendMessage(GetDlgItem(g_pGlobals->GetWindowHandle(), IDC_SERVERPORT), WM_SETTEXT, 0, (LPARAM)csprintf("%u", port));

	//	int basePort = port;

	//	// Try to bind the other ports
	//	for (int i = 1; i < m_socketCount; i++)
	//	{
	//		localhost.sin_port = htons(basePort + i);
	//		if (bind(m_sockets[i], (struct sockaddr *)&localhost, sizeof(SOCKADDR_IN)))
	//		{
	//			WINLOG(Temp, Normal, "Failure to bind socket port %d!\n", basePort + i);
	//			SERVER_ERROR << "Failed bind on socket port:" << (basePort + i);
	//		}
	//	}
	//}

	////Non-blocking sockets are more fun
	//for (int i = 0; i < m_socketCount; i++)
	//{
	//	unsigned long arg = 1;
	//	ioctlsocket(m_sockets[i], FIONBIO, &arg);

	//	int sndbuf = 131072;
	//	int rcvbuf = 131072;
	//	setsockopt(m_sockets[i], SOL_SOCKET, SO_SNDBUF, (char *)&sndbuf, sizeof(sndbuf));
	//	setsockopt(m_sockets[i], SOL_SOCKET, SO_RCVBUF, (char *)&rcvbuf, sizeof(rcvbuf));
	//}
}

void CPhatServer::SystemBroadcast(char *text)
{
	if (!g_pNetwork || !g_pWorld)
		return;

	WINLOG(Temp, Normal, "Broadcast, \"%s\"\n", text);
	SERVER_INFO << "Broadcast:" << text;
	g_pWorld->BroadcastGlobal(ServerBroadcast("System", text, LTT_DEFAULT), PRIVATE_MSG);
}

void CPhatServer::Tick(void)
{
	g_pGlobals->UpdateTime();
	g_pPhatSDK->UpdateInternalTime();
	m_Stats.StartServerFrame();

	ObjCaches::UseTime();

	g_pObjectIDGen->Think();

	if (GameTime::current_game_time)
		GameTime::current_game_time->UseTime();

	g_pNetwork->Think();
	g_pWorld->Think();

	g_pFellowshipManager->Tick();
	g_pAllegianceManager->Tick();
	g_pDB2->Tick();

	sChessManager->Update();

	m_Stats.EndServerFrame();
}

u_short CPhatServer::GetPort()
{
	return m_hostport;
}

void CPhatServer::KickClient(WORD index)
{
	g_pNetwork->KickClient(index);
}

void CPhatServer::BanClient(WORD index)
{
	KickClient(index); //for now
}
