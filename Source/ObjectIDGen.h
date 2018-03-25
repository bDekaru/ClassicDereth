
#pragma once

enum eGUIDClass {
	ePresetGUID = 0,
	ePlayerGUID = 1,
	// eStaticGUID = 2,
	eDynamicGUID = 3,
	// eItemGUID = 4
};

class CObjectIDGenerator
{
public:
	CObjectIDGenerator();
	~CObjectIDGenerator();
	
	void LoadState();

	DWORD GenerateGUID(eGUIDClass guidClass);

protected:
	DWORD m_dwHintDynamicGUID;
};

