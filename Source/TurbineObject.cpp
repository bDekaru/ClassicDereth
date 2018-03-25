

#include "StdAfx.h"
#include "TurbineObject.h"

TurbineObject::TurbineObject(DWORD dwID)
{
	m_dwID = dwID;
}

TurbineObject::~TurbineObject()
{
}

void TurbineObject::Initialize(BYTE *pbData, DWORD dwLength)
{
}

DWORD TurbineObject::GetID()
{
	return m_dwID;
}