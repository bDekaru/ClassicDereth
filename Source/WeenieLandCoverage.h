
#pragma once

class WeenieLandCoverage
{
public:
	WeenieLandCoverage();
	~WeenieLandCoverage();

	void Reset();
	void Initialize();

	//SmartArray<uint32_t> *GetSpawnsForCell(uint32_t cell_id);

protected:
	void LoadLocalStorage();
	void SaveCoverageBitmap();

	PackableHashTable<uint32_t, SmartArray<uint32_t>> m_CreatureSpawns;
};

