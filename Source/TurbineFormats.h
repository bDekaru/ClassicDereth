
#pragma once

//Shamelessly referenced from Todd.
struct BlockData
{
	uint32_t dwID;
	BOOL bObject;
	WORD wSurface[9][9];
	BYTE bHeight[9][9];
};