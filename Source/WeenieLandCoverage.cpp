
#include <StdAfx.h>
#include "WeenieLandCoverage.h"


void SaveBitmapFile(const char *szFile, UINT Width, UINT Height, UINT Bpp, LPVOID lpData)
{
//	// Bpp = Bits per Pixel
//	UINT BytesPerPixel = Bpp >> 3;

//	// Create the output file
//	HANDLE hFile = CreateFile(szFile, GENERIC_WRITE,
//		0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

//	if (hFile == INVALID_HANDLE_VALUE)
//	{
//		// DEBUGOUT("Couldn't create output bitmap file: \"%s\"!\r\n", szFile);
//		return;
//	}

//	// Create the Bitmap Header
//	BITMAPFILEHEADER FileHeader;
//	BITMAPINFOHEADER InfoHeader;

//	// Scan lines must be uint32_t-aligned.
//	UINT LineSize = Width * BytesPerPixel;
//	UINT LinePadding = 0;

//	if (LineSize & 3)
//		LinePadding += 4 - (LineSize & 3);

//	FileHeader.bfType = 'MB';
//	FileHeader.bfSize = sizeof(FileHeader) + sizeof(InfoHeader) + (LineSize + LinePadding) * Height;
//	FileHeader.bfReserved1 = 0;
//	FileHeader.bfReserved2 = 0;
//	FileHeader.bfOffBits = sizeof(FileHeader) + sizeof(InfoHeader);

//	InfoHeader.biSize = sizeof(InfoHeader);
//	InfoHeader.biWidth = Width;
//	InfoHeader.biHeight = Height;
//	InfoHeader.biPlanes = 1;
//	InfoHeader.biBitCount = Bpp;

//	InfoHeader.biCompression = BI_RGB;
//	InfoHeader.biSizeImage = FileHeader.bfSize;
//	InfoHeader.biXPelsPerMeter = 0;
//	InfoHeader.biYPelsPerMeter = 0;
//	InfoHeader.biClrUsed = 0;
//	InfoHeader.biClrImportant = 0;

//	// Write the Bitmap File Header.
//	uint32_t Dummy;

//	if (!WriteFile(hFile, &FileHeader, sizeof(FileHeader), &Dummy, NULL))
//	{
//		//DEBUGOUT("Failed writing bitmap file header!\r\n");
//	}

//	if (!WriteFile(hFile, &InfoHeader, sizeof(InfoHeader), &Dummy, NULL))
//	{
//		//DEBUGOUT("Failed writing bitmap info header!\r\n");
//	}

//	// Bitmap's prefer bottom-up format, so we're doing it that way.
//	for (long y = (long)Height - 1; y >= 0; y--)
//	{
//		WriteFile(hFile, (BYTE *)lpData + y*LineSize, LineSize, &Dummy, NULL);

//		uint32_t Padding = 0;
//		WriteFile(hFile, &Padding, LinePadding, &Dummy, NULL);
//	}

//	CloseHandle(hFile);
}

void WeenieLandCoverage::SaveCoverageBitmap()
{
//	BYTE *imageData = new BYTE[256 * 8 * 256 * 8 * 3];
//	for (uint32_t y = 0; y < (256 * 8); y++)
//	{
//		for (uint32_t x = 0; x < (256 * 8); x++)
//		{
//			BYTE set = 0;

//			uint32_t landcell_id = 0;

//			landcell_id |= 0xFF000000 & ((x >> 3) << 24);
//			landcell_id |= 0x00FF0000 & ((y >> 3) << 16);
//			landcell_id |= ((x & 7) << 3);
//			landcell_id |= ((y & 7) << 0);

//			if (m_CreatureSpawns.find(landcell_id) != m_CreatureSpawns.end())
//				set = 0xFF;

//			BYTE *output = &imageData[((y * 256 * 8) + x) * 3];
//			output[0] = set;
//			output[1] = set;
//			output[2] = set;
//		}
//	}

//	SaveBitmapFile("c:\\coverage_map.bmp", 256 * 8, 256 * 8, 24, imageData);
//	delete [] imageData;
}

WeenieLandCoverage::WeenieLandCoverage()
{
}

WeenieLandCoverage::~WeenieLandCoverage()
{
//	Reset();
}

void WeenieLandCoverage::Reset()
{
//	m_CreatureSpawns.clear();
}

void WeenieLandCoverage::Initialize()
{
//#ifndef QUICKSTART
//	WINLOG(Data, Normal, "Loading weenie spawns...\n");
//	SERVER_INFO << "Loading weenie spawns...";
//	LoadLocalStorage();
//	WINLOG(Data, Normal, "Loaded %d cells with spawns...\n", m_CreatureSpawns.size());
//	SERVER_INFO << "Loaded" << m_CreatureSpawns.size() << "cells with spawns...";
//	uint32_t totalSpawns = 0;
//	for (auto spawn : m_CreatureSpawns)
//	{
//		totalSpawns += spawn.second.num_used;
//	}

//	SERVER_INFO << "Loaded" << totalSpawns << "spawns within them...";;
//#endif
}

void WeenieLandCoverage::LoadLocalStorage()
{
//	BYTE *data = NULL;
//	uint32_t length = 0;
//	if (LoadDataFromFile("data\\weenie\\landspawns\\generated.bin", &data, &length))
//	{
//		BinaryReader reader(data, length);

//		uint32_t num = reader.Read<uint32_t>();
//		for (uint32_t i = 0; i < num; i++)
//		{
//			uint32_t cell_id = reader.Read<uint32_t>();
//			m_CreatureSpawns[cell_id].UnPack(&reader);
//		}

//		delete [] data;
//	}

//	SaveCoverageBitmap();
}

//SmartArray<uint32_t> *WeenieLandCoverage::GetSpawnsForCell(uint32_t cell_id)
//{
//	return (SmartArray<uint32_t> *)m_CreatureSpawns.lookup(cell_id);
//}

