

#include "StdAfx.h"
#include "TurbineCell.h"

TurbineCell::TurbineCell()
{
#if PRE_TOD_DATA_FILES
	LoadFile("cell.dat");
#else
	LoadFile("client_cell_1.dat");
#endif

	DATDisk::pCell = m_pDATDisk;
}

TurbineCell::~TurbineCell()
{
	DATDisk::pCell = NULL;
}

