
#include <StdAfx.h>
#include "PhatSDK.h"
#include "QuestDefDB.h"

DEFINE_PACK(QuestDef)
{
	UNFINISHED();
}

DEFINE_UNPACK(QuestDef)
{
	_mindelta = pReader->Read<int>();
	_maxsolves = pReader->Read<int>();
	_fullname = pReader->ReadString();
	try
	{
		// these are obfuscated, swap low/high nibbles
		for (int i = 0; i < _fullname.size(); i++)
			_fullname[i] = (char)(BYTE)((BYTE)((BYTE)_fullname[i] << 4) | (BYTE)((BYTE)_fullname[i] >> 4));
	}
	catch(...)
	{
		SERVER_ERROR << "Error reading quest from DAT";
	}

	return true;
}

DEFINE_PACK_JSON(QuestDef)
{
	writer["mindelta"] = _mindelta;
	writer["maxsolves"] = _maxsolves;
	writer["fullname"] = _fullname;
}

DEFINE_UNPACK_JSON(QuestDef)
{
	_mindelta = reader["mindelta"];
	_maxsolves = reader["maxsolves"];
	_fullname = reader["fullname"].get<std::string>();
	return true;
}

DEFINE_PACK(QuestDefDB)
{
	UNFINISHED();
}

DEFINE_UNPACK(QuestDefDB)
{
	// pReader->Read<uint32_t>(); // id

	_defs.UnPack(pReader);

#ifdef PHATSDK_IS_SERVER
	// in order to fix quest casing...
	PackableHashTableWithJson<std::string, QuestDef, std::string> defNoCase;
	for (auto &entry : _defs)
	{
		std::string lowerCaseDef = entry.first;
		std::transform(lowerCaseDef.begin(), lowerCaseDef.end(), lowerCaseDef.begin(), ::tolower);
		defNoCase[lowerCaseDef] = entry.second;
	}
	_defs = defNoCase;
#endif

	return true;
}

DEFINE_PACK_JSON(QuestDefDB)
{
	_defs.PackJson(writer);
}

DEFINE_UNPACK_JSON(QuestDefDB)
{
	_defs.UnPackJson(reader);

#ifdef PHATSDK_IS_SERVER
	// in order to fix quest casing...
	PackableHashTableWithJson<std::string, QuestDef, std::string> defNoCase;
	for (auto &entry : _defs)
	{
		std::string lowerCaseDef = entry.first;
		std::transform(lowerCaseDef.begin(), lowerCaseDef.end(), lowerCaseDef.begin(), ::tolower);
		defNoCase[lowerCaseDef] = entry.second;
	}
	_defs = defNoCase;
#endif
	return true;
}

QuestDef *CQuestDefDB::GetQuestDef(const char *questName)
{
	CQuestDefDB *questDB = g_pPhatSDK->GetQuestDefDB();
	if (!questDB)
	{
		return NULL;
	}
	
#ifdef PHATSDK_IS_SERVER
	// in order to fix quest casing...
	std::string lowerCaseDef = questName;
	std::transform(lowerCaseDef.begin(), lowerCaseDef.end(), lowerCaseDef.begin(), ::tolower);
	return questDB->_defs.lookup(lowerCaseDef.c_str());
#else
	return questDB->_defs.lookup(questName);
#endif
}
