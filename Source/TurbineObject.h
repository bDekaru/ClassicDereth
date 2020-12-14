
#pragma once

#define DF_CELL		1
#define DF_PORTAL	2

class TurbineObject
{
public:
	TurbineObject(uint32_t dwID);
	virtual ~TurbineObject();

	uint32_t GetID();

	virtual void Initialize(BYTE *pbData, uint32_t dwLength);

protected:
	uint32_t m_dwID;
};