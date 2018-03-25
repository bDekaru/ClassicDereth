
#pragma once

//Shamelessly referenced from Todd.
struct BlockData
{
	DWORD dwID;
	BOOL bObject;
	WORD wSurface[9][9];
	BYTE bHeight[9][9];
};