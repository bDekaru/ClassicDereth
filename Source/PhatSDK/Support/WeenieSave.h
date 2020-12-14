
#pragma once

#include "Packable.h"

class CWeenieSave : public PackObj
{
public:
	CWeenieSave();
	virtual ~CWeenieSave();

	DECLARE_PACKABLE()

	void Destroy();

	uint32_t m_SaveVersion = 0;
	uint32_t m_SaveTimestamp = 0;
	uint32_t m_SaveInstanceTS = 0;
	CACQualities m_Qualities;
	ObjDesc m_ObjDesc;
	ObjDesc m_WornObjDesc; // not ideal, but we'll keep it around for now
	PlayerModule *_playerModule = NULL;
	QuestTable *_questTable = NULL;

	PackableList<uint32_t> _equipment;
	PackableList<uint32_t> _inventory;
	PackableList<uint32_t> _packs;
};



