
#pragma once

class WeenieLandCoverage
{
public:
	WeenieLandCoverage();
	~WeenieLandCoverage();

	void Reset();
	void Initialize();

	SmartArray<DWORD> *GetSpawnsForCell(DWORD cell_id);

protected:
	void LoadLocalStorage();
	void SaveCoverageBitmap();

	PackableHashTable<DWORD, SmartArray<DWORD>> m_CreatureSpawns;
};
