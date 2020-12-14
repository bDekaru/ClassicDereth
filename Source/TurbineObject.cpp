

#include <StdAfx.h>
#include "TurbineObject.h"

TurbineObject::TurbineObject(uint32_t dwID)
{
	m_dwID = dwID;
}

TurbineObject::~TurbineObject()
{
}

void TurbineObject::Initialize(BYTE *pbData, uint32_t dwLength)
{
}

uint32_t TurbineObject::GetID()
{
	return m_dwID;
}