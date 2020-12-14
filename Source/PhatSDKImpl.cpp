
#include <StdAfx.h>
#include "PhatSDK.h"
#include "ServerCellManager.h"
#include "InferredPortalData.h"

class CPhatSDKServerImpl : public CPhatSDKImpl
{
public:
	static void Init()
	{
		if (!g_pPhatSDK)
		{
			g_pPhatSDK = new CPhatSDKServerImpl();
		}
	}

	static void Cleanup()
	{
		SafeDelete(g_pPhatSDK);
	}

	virtual ~CPhatSDKServerImpl() = default;

	double GetCurrTime() override
	{
		return g_pGlobals->Time();
	}

	double GetRandomFloat(double min, double max) override
	{
		return Random::GenFloat(min, max);
	}

	BYTE *GetPortalDataEntry(uint32_t id, uint32_t *length) override
	{
		DATEntry entry;
		if (DATDisk::pPortal && DATDisk::pPortal->GetData(id, &entry))
		{
			if (length)
				*length = entry.Length;

			return entry.Data;
		}

		return NULL;
	}

	BYTE *GetCellDataEntry(uint32_t id, uint32_t *length) override
	{
		DATEntry entry;
		if (DATDisk::pCell && DATDisk::pCell->GetData(id, &entry))
		{
			if (length)
				*length = entry.Length;

			return entry.Data;
		}

		return NULL;
	}

	CEnvCell *EnvCell_GetVisible(uint32_t cell_id, bool bDoPostLoad = true) override
	{
		return (CEnvCell *)g_pCellManager->GetObjCell(cell_id, bDoPostLoad);
	}

	CQuestDefDB *GetQuestDefDB() override
	{
		if (!g_pPortalDataEx)
		{
			return NULL;
		}

		return &g_pPortalDataEx->_questDefDB;
	}
};

void InitPhatSDK()
{
	CPhatSDKServerImpl::Init();
}

void CleanupPhatSDK()
{
	CPhatSDKServerImpl::Cleanup();
}


