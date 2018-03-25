
#pragma once

#define DF_CELL		1
#define DF_PORTAL	2

class TurbineObject
{
public:
	TurbineObject(DWORD dwID);
	virtual ~TurbineObject();

	DWORD GetID();

	virtual void Initialize(BYTE *pbData, DWORD dwLength);

protected:
	DWORD m_dwID;
};