

#include <StdAfx.h>
#include "TurbinePortal.h"

TurbinePortal::TurbinePortal()
{
#if PRE_TOD_DATA_FILES
	LoadFile("portal.dat");
#else
	LoadFile("client_portal.dat");
#endif

	DATDisk::pPortal = m_pDATDisk;
}

TurbinePortal::~TurbinePortal()
{
	DATDisk::pPortal = NULL;
}
