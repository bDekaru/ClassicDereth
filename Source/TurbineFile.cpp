
#include <StdAfx.h>

#include "TurbineData.h"
#include "TurbineFile.h"

#include "Database.h"

TurbineFile::TurbineFile(uint32_t dwID)
{
	m_dwID = dwID;
	m_pbData = NULL;
	m_dwLength = 0;
}

TurbineFile::TurbineFile(uint32_t dwID, BYTE *data, uint32_t length)
{
	m_dwID = dwID;
	m_pbData = data;
	m_dwLength = length;
}

TurbineFile::~TurbineFile()
{
	SafeDeleteArray(m_pbData);
	m_dwLength = 0;
}

#if 0
void TurbineFile::Initialize(BYTE* pbDataFile, uint32_t dwSectorStart, uint32_t dwSectorSize, uint32_t dwFileLength)
{
	//One extra sector so it doesn't overflow.
	BYTE *pbData = new BYTE[dwFileLength + (sizeof(uint32_t) * dwSectorSize)];
	BYTE *pbDataPtr = pbData;

	uint32_t dwNextSector = dwSectorStart;
	uint32_t dwSectorData = sizeof(uint32_t) * (dwSectorSize - 1);

	while (dwNextSector)
	{
		BYTE* pbSector = pbDataFile + dwNextSector;

		dwNextSector = *((uint32_t *)pbSector);
		pbSector += sizeof(uint32_t);

		memcpy(pbDataPtr, pbSector, dwSectorData);
		pbDataPtr += dwSectorData;
	}

	m_pbData = pbData;
	m_dwLength = dwFileLength;

	if (m_dwID == 0xEDEAFFFF)
	{
		WORD *pwTerrain = (WORD *)(m_pbData + 8);

		WORD terrain[9][9] = //'P'
		{
			{  40,  24,  24,  24,  24,  40,  40,  40,  40 },
			{  40,  24,  24,  24,  24,  24,  40,  40,  40 },
			{  40,  24,  24,  40,  24,  24,  40,  40,  40 },
			{  40,  24,  24,  40,  24,  24,  40,  40,  40 },
			{  40,  24,  24,  24,  24,  24,  40,  40,  40 },
			{  40,  24,  24,  24,  24,  40,  40,  40,  40 },
			{  40,  24,  24,  40,  40,  40,  40,  40,  40 },
			{  40,  24,  24,  40,  40,  40,  40,  40,  40 },
			{  40,  24,  24,  40,  40,  40,  40,  40,  40 }
		};

		for (int x = 0; x < 9; x++) {
			for (int y = 0; y < 9; y++) {
				*pwTerrain = terrain[8 - y][x];
				pwTerrain++;
			}
		}
	}
	else if (m_dwID == 0xEEEAFFFF)
	{
		WORD *pwTerrain = (WORD *)(m_pbData + 8);

		WORD terrain[9][9] = //'H'
		{
			{  40, 24, 24,  40,  40,  24,  24,  40,  40 },
			{  40, 24, 24,  40,  40,  24,  24,  40,  40 },
			{  40, 24, 24,  40,  40,  24,  24,  40,  40 },
			{  40, 24, 24,  24,  24,  24,  24,  40,  40 },
			{  40, 24, 24,  24,  24,  24,  24,  40,  40 },
			{  40, 24, 24,  40,  40,  24,  24,  40,  40 },
			{  40, 24, 24,  40,  40,  24,  24,  40,  40 },
			{  40, 24, 24,  40,  40,  24,  24,  40,  40 },
			{  40, 24, 24,  40,  40,  24,  24,  40,  40 }
		};

		for (int x = 0; x < 9; x++) {
			for (int y = 0; y < 9; y++) {
				*pwTerrain = terrain[8 - y][x];
				pwTerrain++;
			}
		}
	}
	else if (m_dwID == 0xEFEAFFFF)
	{
		WORD *pwTerrain = (WORD *)(m_pbData + 8);

		WORD terrain[9][9] = //'A'
		{
			{  40,  40,  40,  24,  24,  40,  40,  40,  40 },
			{  40,  40,  24,  24,  24,  24,  40,  40,  40 },
			{  40,  24,  24,  40,  40,  24,  24,  40,  40 },
			{  40,  24,  24,  40,  40,  24,  24,  40,  40 },
			{  40,  24,  24,  24,  24,  24,  24,  40,  40 },
			{  40,  24,  24,  24,  24,  24,  24,  40,  40 },
			{  40,  24,  24,  40,  40,  24,  24,  40,  40 },
			{  40,  24,  24,  40,  40,  24,  24,  40,  40 },
			{  40,  24,  24,  40,  40,  24,  24,  40,  40 }
		};

		for (int x = 0; x < 9; x++) {
			for (int y = 0; y < 9; y++) {
				*pwTerrain = terrain[8 - y][x];
				pwTerrain++;
			}
		}
	}
	else if (m_dwID == 0xF0EAFFFF)
	{
		WORD *pwTerrain = (WORD *)(m_pbData + 8);

		WORD terrain[9][9] = //'T'
		{
			{  40,  24,  24,  24,  24,  24,  24,  40,  40 },
			{  40,  24,  24,  24,  24,  24,  24,  40,  40 },
			{  40,  40,  40,  24,  24,  40,  40,  40,  40 },
			{  40,  40,  40,  24,  24,  40,  40,  40,  40 },
			{  40,  40,  40,  24,  24,  40,  40,  40,  40 },
			{  40,  40,  40,  24,  24,  40,  40,  40,  40 },
			{  40,  40,  40,  24,  24,  40,  40,  40,  40 },
			{  40,  40,  40,  24,  24,  40,  40,  40,  40 },
			{  40,  40,  40,  24,  24,  40,  40,  40,  40 }
		};

		for (int x = 0; x < 9; x++) {
			for (int y = 0; y < 9; y++) {
				*pwTerrain = terrain[8 - y][x];
				pwTerrain++;
			}
		}
	}
	else if (m_dwID == 0xEEE9FFFF)
	{
		WORD *pwTerrain = (WORD *)(m_pbData + 8);

		WORD terrain[9][9] = //'A'
		{
			{  40,  40,  40,  40,  40,  40,  40,  40,  40 },
			{  40,  40,  40,  24,  24,  40,  40,  40,  40 },
			{  40,  40,  24,  24,  24,  24,  40,  40,  40 },
			{  40,  24,  24,  40,  40,  24,  24,  40,  40 },
			{  40,  24,  24,  40,  40,  24,  24,  40,  40 },
			{  40,  24,  24,  24,  24,  24,  24,  40,  40 },
			{  40,  24,  24,  24,  24,  24,  24,  40,  40 },
			{  40,  24,  24,  40,  40,  24,  24,  40,  40 },
			{  40,  24,  24,  40,  40,  24,  24,  40,  40 }
		};

		for (int x = 0; x < 9; x++) {
			for (int y = 0; y < 9; y++) {
				*pwTerrain = terrain[8 - y][x];
				pwTerrain++;
			}
		}
	}
	else if (m_dwID == 0xEFE9FFFF)
	{
		WORD *pwTerrain = (WORD *)(m_pbData + 8);

		WORD terrain[9][9] = //'C'
		{
			{  40,  40,  40,  40,   40,  40,  40,  40,  40 },
			{  40,  40,  40,  24,  24,  24,  40,  40,  40 },
			{  40,  40,  24,  24,  24,  24,  40,  40,  40 },
			{  40,  40,  24,  24,  40,  40,  40,  40,  40 },
			{  40,  40,  24,  24,  40,  40,  40,  40,  40 },
			{  40,  40,  24,  24,  40,  40,  40,  40,  40 },
			{  40,  40,  24,  24,  40,  40,  40,  40,  40 },
			{  40,  40,  24,  24,  24,  24,  40,  40,  40 },
			{  40,  40,  40,  24,  24,  24,  40,  40,  40 }
		};

		for (int x = 0; x < 9; x++) {
			for (int y = 0; y < 9; y++) {
				*pwTerrain = terrain[8 - y][x];
				pwTerrain++;
			}
		}
	}
	else
	{
		/*FILE* cellFile = g_pDB->DataFileOpen(csprintf("land\\cell-%08X", m_dwID));
		if ( cellFile ) {
			delete [] pbData;

			m_dwLength	= fsize( cellFile );
			m_pbData	= new BYTE[ m_dwLength ];
			fread( m_pbData, sizeof(BYTE), m_dwLength, cellFile );
			fclose( cellFile );
		} */
	}

	/*
		BYTE *pbHeight = m_pbData + 8 + (81 * 2);
		BYTE heights[9][9] =
		{
			{   0,   0,   0,   0,   0,   0,   0,   0,   0 },
			{   0,   0,   0,   0,   0,   0,   0,   0,   0 },
			{   0,   0,   0,   0,   0,   0,   0,   0,   0 },
			{   0,   0,   0,   0,   0,   0,   0,   0,   0 },
			{   0,   0,   0,   0,   0,   0,   0,   0,   0 },
			{   0,   0,   0,   0,   0,   0,   0,   0,   0 },
			{   0,   0,   0,   0,   0,   0,   0,   0,   0 },
			{   0,   0,   0,   0,   0,   0,   0,   0,   0 },
			{   0,   0,   0,   0,   0,   0,   0,   0,   0 }
		};

		for ( int x = 0; x < 9; x++ ) {
			for ( int y = 0; y < 9; y++ ) {
				*pbHeight = heights[y][x];
				pbHeight++;
			}
		}
	*/
}
#endif

BYTE *TurbineFile::GetData()
{
	return m_pbData + sizeof(uint32_t);
}

uint32_t TurbineFile::GetLength()
{
	return m_dwLength;
}