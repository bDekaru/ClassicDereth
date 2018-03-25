
#pragma once

class CInferredCellData
{
public:
	void Init();
	CLandBlockExtendedData *GetLandBlockData(DWORD landblock);

	CLandBlockExtendedDataTable _data;
	CLandBlockExtendedDataTable _jsonData;
};

