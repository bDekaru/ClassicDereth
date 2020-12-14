#include <StdAfx.h>
#include "easylogging++.h"
#include "Client.h"
#include "WeenieObject.h"
#include "Monster.h"
#include "Player.h"
#include "World.h"
#include "WeenieFactory.h"

#include "ClientCommands.h"

//CLIENT_COMMAND(setq, "type id value", "Set a quality of last assessed item", ADMIN_ACCESS)
//{
//	if (argc != 3)
//		return true;
//
//	STypeInt stat = (STypeInt)atoi(argv[0]);
//	int32_t value = atoi(argv[1]);
//
//	CWeenieObject* item = g_pWorld->FindObject(pPlayer->m_LastAssessed);
//
//	if (item) {
//		item->m_Qualities.SetInt(stat, value);
//		item->NotifyIntStatUpdated(stat);
//	}
//
//	return false;
//}

CLIENT_COMMAND(setint, "statid value", "Set Int Stat of last assessed item", ADMIN_ACCESS, SERVER_CATEGORY)
{
	if (argc < 2)
		return true;

	STypeInt stat = (STypeInt)atoi(argv[0]);
	int32_t value = atoi(argv[1]);

	CWeenieObject* item = g_pWorld->FindObject(pPlayer->m_LastAssessed);

	if (item) {
		item->m_Qualities.SetInt(stat, value);
		item->NotifyIntStatUpdated(stat);
	}

	return false;
}

CLIENT_COMMAND(removeint, "statid", "Remove Int Stat of last assessed item", ADMIN_ACCESS, SERVER_CATEGORY)
{
	if (argc < 1)
		return true;

	STypeInt stat = (STypeInt)atoi(argv[0]);

	CWeenieObject* item = g_pWorld->FindObject(pPlayer->m_LastAssessed);

	if (item)
	{
		int statValue;
		if (item->m_Qualities.InqInt(stat, statValue))
		{
			item->m_Qualities.RemoveInt(stat);
			item->NotifyIntStatUpdated(stat);
		}
	}

	return false;
}

CLIENT_COMMAND(wdsetint, "wcid statid value", "Set Int Stat for wcid", ADMIN_ACCESS, SERVER_CATEGORY)
{
	if (argc < 3)
		return true;

	uint32_t wcid = (uint32_t)atoi(argv[0]);
	STypeInt stat = (STypeInt)atoi(argv[1]);
	int32_t value = atoi(argv[2]);

	CWeenieDefaults *weenie = g_pWeenieFactory->GetWeenieDefaults(wcid);

	if (weenie)
	{
		weenie->m_Qualities.SetInt(stat, value);
	}
	else
	{
		pPlayer->SendText("WCID not found!", LTT_DEFAULT);
	}

	return false;
}

CLIENT_COMMAND(setint64, "statid value", "Set Int64 Stat of last assessed item", ADMIN_ACCESS, SERVER_CATEGORY)
{
	if (argc < 2)
		return true;

	STypeInt64 stat = (STypeInt64)atoi(argv[0]);
	int64_t value = atol(argv[1]);

	CWeenieObject* item = g_pWorld->FindObject(pPlayer->m_LastAssessed);

	if (item) {
		item->m_Qualities.SetInt64(stat, value);
		item->NotifyInt64StatUpdated(stat);
	}

	return false;
}

CLIENT_COMMAND(removeint64, "statid", "Remove Int64 Stat of last assessed item", ADMIN_ACCESS, SERVER_CATEGORY)
{
	if (argc < 1)
		return true;

	STypeInt64 stat = (STypeInt64)atoi(argv[0]);

	CWeenieObject* item = g_pWorld->FindObject(pPlayer->m_LastAssessed);

	if (item)
	{
		int64_t statValue;
		if (item->m_Qualities.InqInt64(stat, statValue))
		{
			item->m_Qualities.RemoveInt64(stat);
			item->NotifyInt64StatUpdated(stat);
		}
	}

	return false;
}

CLIENT_COMMAND(wdsetint64, "wcid statid value", "Set Int64 Stat for wcid", ADMIN_ACCESS, SERVER_CATEGORY)
{
	if (argc < 3)
		return true;

	uint32_t wcid = (uint32_t)atoi(argv[0]);
	STypeInt64 stat = (STypeInt64)atoi(argv[1]);
	int64_t value = atol(argv[2]);

	CWeenieDefaults *weenie = g_pWeenieFactory->GetWeenieDefaults(wcid);

	if (weenie)
	{
		weenie->m_Qualities.SetInt64(stat, value);
	}
	else
	{
		pPlayer->SendText("WCID not found!", LTT_DEFAULT);
	}

	return false;
}

CLIENT_COMMAND(setfloat, "statid value", "Set Float Stat of last assessed item", ADMIN_ACCESS, SERVER_CATEGORY)
{
	if (argc < 2)
		return true;

	STypeFloat stat = (STypeFloat)atoi(argv[0]);
	float value = atof(argv[1]);

	CWeenieObject* item = g_pWorld->FindObject(pPlayer->m_LastAssessed);

	if (item) {
		item->m_Qualities.SetFloat(stat, value);
		item->NotifyFloatStatUpdated(stat);
	}

	return false;
}

CLIENT_COMMAND(removefloat, "statid", "Remove Float Stat of last assessed item", ADMIN_ACCESS, SERVER_CATEGORY)
{
	if (argc < 1)
		return true;

	STypeFloat stat = (STypeFloat)atoi(argv[0]);

	CWeenieObject* item = g_pWorld->FindObject(pPlayer->m_LastAssessed);

	if (item)
	{
		double statValue;
		if (item->m_Qualities.InqFloat(stat, statValue))
		{
			item->m_Qualities.RemoveFloat(stat);
			item->NotifyFloatStatUpdated(stat);
		}
	}

	return false;
}

CLIENT_COMMAND(wdsetfloat, "wcid statid value", "Set Float Stat for wcid", ADMIN_ACCESS, SERVER_CATEGORY)
{
	if (argc < 3)
		return true;

	uint32_t wcid = (uint32_t)atoi(argv[0]);
	STypeFloat stat = (STypeFloat)atoi(argv[1]);
	float value = atof(argv[2]);

	CWeenieDefaults *weenie = g_pWeenieFactory->GetWeenieDefaults(wcid);

	if (weenie)
	{
		weenie->m_Qualities.SetFloat(stat, value);
	}
	else
	{
		pPlayer->SendText("WCID not found!", LTT_DEFAULT);
	}

	return false;
}

CLIENT_COMMAND(setdid, "statid value", "Set DID Stat of last assessed item", ADMIN_ACCESS, SERVER_CATEGORY)
{
	if (argc < 2)
		return true;

	STypeDID stat = (STypeDID)atoi(argv[0]);
	uint32_t value = (uint32_t)atoi(argv[1]);

	CWeenieObject* item = g_pWorld->FindObject(pPlayer->m_LastAssessed);

	if (item) {
		item->m_Qualities.SetDataID(stat, value);
		item->NotifyDIDStatUpdated(stat);
	}

	return false;
}

CLIENT_COMMAND(removedid, "statid", "Remove DID Stat of last assessed item", ADMIN_ACCESS, SERVER_CATEGORY)
{
	if (argc < 1)
		return true;

	STypeDID stat = (STypeDID)atoi(argv[0]);

	CWeenieObject* item = g_pWorld->FindObject(pPlayer->m_LastAssessed);

	if (item)
	{
		uint32_t statValue;
		if (item->m_Qualities.InqDataID(stat, statValue))
		{
			item->m_Qualities.RemoveDataID(stat);
			item->NotifyDIDStatUpdated(stat);
		}
	}

	return false;
}

CLIENT_COMMAND(wdsetdid, "wcid statid value", "Set SID Stat for wcid", ADMIN_ACCESS, SERVER_CATEGORY)
{
	if (argc < 3)
		return true;

	uint32_t wcid = (uint32_t)atoi(argv[0]);
	STypeDID stat = (STypeDID)atoi(argv[1]);
	uint32_t value = (uint32_t)atoi(argv[2]);

	CWeenieDefaults *weenie = g_pWeenieFactory->GetWeenieDefaults(wcid);

	if (weenie)
	{
		weenie->m_Qualities.SetDataID(stat, value);
	}
	else
	{
		pPlayer->SendText("WCID not found!", LTT_DEFAULT);
	}

	return false;
}

CLIENT_COMMAND(setiid, "statid value", "Set IID Stat of last assessed item", ADMIN_ACCESS, SERVER_CATEGORY)
{
	if (argc < 2)
		return true;

	STypeIID stat = (STypeIID)atoi(argv[0]);
	uint32_t value = (uint32_t)atoi(argv[1]);

	CWeenieObject* item = g_pWorld->FindObject(pPlayer->m_LastAssessed);

	if (item) {
		item->m_Qualities.SetInstanceID(stat, value);
		item->NotifyIIDStatUpdated(stat);
	}

	return false;
}

CLIENT_COMMAND(removeiid, "statid", "Remove IID Stat of last assessed item", ADMIN_ACCESS, SERVER_CATEGORY)
{
	if (argc < 1)
		return true;

	STypeIID stat = (STypeIID)atoi(argv[0]);

	CWeenieObject* item = g_pWorld->FindObject(pPlayer->m_LastAssessed);

	if (item)
	{
		uint32_t statValue;
		if (item->m_Qualities.InqInstanceID(stat, statValue))
		{
			item->m_Qualities.RemoveInstanceID(stat);
			item->NotifyIIDStatUpdated(stat);
		}
	}

	return false;
}

CLIENT_COMMAND(wdsetiid, "wcid statid value", "Set IID Stat for wcid", ADMIN_ACCESS, SERVER_CATEGORY)
{
	if (argc < 3)
		return true;

	uint32_t wcid = (uint32_t)atoi(argv[0]);
	STypeIID stat = (STypeIID)atoi(argv[1]);
	uint32_t value = (uint32_t)atoi(argv[2]);

	CWeenieDefaults *weenie = g_pWeenieFactory->GetWeenieDefaults(wcid);

	if (weenie)
	{
		weenie->m_Qualities.SetInstanceID(stat, value);
	}
	else
	{
		pPlayer->SendText("WCID not found!", LTT_DEFAULT);
	}

	return false;
}

CLIENT_COMMAND(setbool, "statid value (0, 1)", "Set Bool Stat of last assessed item", ADMIN_ACCESS, SERVER_CATEGORY)
{
	if (argc < 2)
		return true;

	STypeBool stat = (STypeBool)atoi(argv[0]);
	int32_t value = atoi(argv[1]);

	CWeenieObject* item = g_pWorld->FindObject(pPlayer->m_LastAssessed);

	if (item) {
		item->m_Qualities.SetBool(stat, value);
		item->NotifyBoolStatUpdated(stat);
	}

	return false;
}

CLIENT_COMMAND(wdsetbool, "wcid statid value (0, 1)", "Set Bool Stat for wcid", ADMIN_ACCESS, SERVER_CATEGORY)
{
	if (argc < 3)
		return true;

	uint32_t wcid = (uint32_t)atoi(argv[0]);
	STypeBool stat = (STypeBool)atoi(argv[1]);
	int32_t value = atoi(argv[2]);

	CWeenieDefaults *weenie = g_pWeenieFactory->GetWeenieDefaults(wcid);

	if (weenie)
	{
		weenie->m_Qualities.SetBool(stat, value);
	}
	else
	{
		pPlayer->SendText("WCID not found!", LTT_DEFAULT);
	}

	return false;
}

CLIENT_COMMAND(setstring, "statid value", "Set String Stat of last assessed item", ADMIN_ACCESS, SERVER_CATEGORY)
{
	if (argc < 2)
		return true;

	STypeString stat = (STypeString)atoi(argv[0]);

	CWeenieObject* item = g_pWorld->FindObject(pPlayer->m_LastAssessed);

	if (item) {
		item->m_Qualities.SetString(stat, argv[1]);
		item->NotifyStringStatUpdated(stat);
	}

	return false;
}

CLIENT_COMMAND(wdsetstring, "wcid statid value", "Set String Stat for wcid", ADMIN_ACCESS, SERVER_CATEGORY)
{
	if (argc < 3)
		return true;

	uint32_t wcid = (uint32_t)atoi(argv[0]);
	STypeString stat = (STypeString)atoi(argv[1]);

	CWeenieDefaults *weenie = g_pWeenieFactory->GetWeenieDefaults(wcid);

	if (weenie)
	{
		weenie->m_Qualities.SetString(stat, argv[2]);
	}
	else
	{
		pPlayer->SendText("WCID not found!", LTT_DEFAULT);
	}

	return false;
}

CLIENT_COMMAND(setattr, "attrid value", "Set Attribute base value of last assessed item", ADMIN_ACCESS, SERVER_CATEGORY)
{
	if (argc < 2)
		return true;

	STypeAttribute attr = (STypeAttribute)atoi(argv[0]);
	uint32_t value = (uint32_t)atoi(argv[1]);

	CWeenieObject* item = g_pWorld->FindObject(pPlayer->m_LastAssessed);

	if (item) {
		item->m_Qualities.SetAttribute(attr, value);
		item->NotifyAttributeStatUpdated(attr);
	}

	return false;
}

CLIENT_COMMAND(wdsetattr, "wcid attrid value", "Set Attribute base value for wcid", ADMIN_ACCESS, SERVER_CATEGORY)
{
	if (argc < 3)
		return true;

	uint32_t wcid = (uint32_t)atoi(argv[0]);
	STypeAttribute attr = (STypeAttribute)atoi(argv[1]);
	uint32_t value = (uint32_t)atoi(argv[2]);

	CWeenieDefaults *weenie = g_pWeenieFactory->GetWeenieDefaults(wcid);

	if (weenie)
	{
		weenie->m_Qualities.SetAttribute(attr, value);
	}
	else
	{
		pPlayer->SendText("WCID not found!", LTT_DEFAULT);
	}

	return false;
}

CLIENT_COMMAND(setvital, "vitalid value", "Set Vital base value of last assessed item", ADMIN_ACCESS, SERVER_CATEGORY)
{
	if (argc < 2)
		return true;

	STypeAttribute2nd attr = (STypeAttribute2nd)atoi(argv[0]);
	uint32_t value = (uint32_t)atoi(argv[1]);

	CWeenieObject* item = g_pWorld->FindObject(pPlayer->m_LastAssessed);

	if (item) {
		item->m_Qualities.SetAttribute2nd(attr, value);
		item->NotifyAttribute2ndStatUpdated(attr);
	}

	return false;
}

CLIENT_COMMAND(wdsetvital, "wcid vitalid value", "Set Vital base value for wcid", ADMIN_ACCESS, SERVER_CATEGORY)
{
	if (argc < 3)
		return true;

	uint32_t wcid = (uint32_t)atoi(argv[0]);
	STypeAttribute2nd attr = (STypeAttribute2nd)atoi(argv[1]);
	uint32_t value = (uint32_t)atoi(argv[2]);

	CWeenieDefaults *weenie = g_pWeenieFactory->GetWeenieDefaults(wcid);

	if (weenie)
	{
		weenie->m_Qualities.SetAttribute2nd(attr, value);
	}
	else
	{
		pPlayer->SendText("WCID not found!", LTT_DEFAULT);
	}

	return false;
}

CLIENT_COMMAND(setskill, "skillid value", "Set Skill base value of last assessed item", ADMIN_ACCESS, SERVER_CATEGORY)
{
	if (argc < 2)
		return true;

	STypeSkill attr = (STypeSkill)atoi(argv[0]);
	uint32_t value = (uint32_t)atoi(argv[1]);

	CWeenieObject* item = g_pWorld->FindObject(pPlayer->m_LastAssessed);

	if (item) {
		Skill skill;
		item->m_Qualities.InqSkill(attr, skill);
		if (skill._sac == SKILL_ADVANCEMENT_CLASS::UNTRAINED_SKILL_ADVANCEMENT_CLASS)
			skill._sac = SKILL_ADVANCEMENT_CLASS::TRAINED_SKILL_ADVANCEMENT_CLASS;

		skill._init_level = value;

		item->m_Qualities.SetSkill(attr, skill);
		item->NotifySkillStatUpdated(attr);
	}

	return false;
}

CLIENT_COMMAND(wdsetskill, "wcid skillid value", "Set Skill base value for wcid", ADMIN_ACCESS, SERVER_CATEGORY)
{
	if (argc < 3)
		return true;

	uint32_t wcid = (uint32_t)atoi(argv[0]);
	STypeSkill attr = (STypeSkill)atoi(argv[1]);
	uint32_t value = (uint32_t)atoi(argv[2]);

	CWeenieDefaults *weenie = g_pWeenieFactory->GetWeenieDefaults(wcid);

	if (weenie)
	{
		Skill skill;
		weenie->m_Qualities.InqSkill(attr, skill);
		if (skill._sac == SKILL_ADVANCEMENT_CLASS::UNTRAINED_SKILL_ADVANCEMENT_CLASS)
			skill._sac = SKILL_ADVANCEMENT_CLASS::TRAINED_SKILL_ADVANCEMENT_CLASS;

		skill._init_level = value;

		weenie->m_Qualities.SetSkill(attr, skill);
	}
	else
	{
		pPlayer->SendText("WCID not found!", LTT_DEFAULT);
	}

	return false;
}

CLIENT_COMMAND(addmodelswap, "index swap", "Add an ObjDesc model swap", ADMIN_ACCESS, SERVER_CATEGORY)
{
	if (argc < 2)
		return true;

	uint32_t idx = (uint32_t)atoi(argv[0]);
	uint32_t swap = (uint32_t)atoi(argv[1]);

	CWeenieObject* item = g_pWorld->FindObject(pPlayer->m_LastAssessed);

	if (item) {
		AnimPartChange *change = new AnimPartChange(idx, swap);
		item->m_ObjDescOverride.AddAnimPartChange(change);
		item->m_bObjDescOverride = true;

		item->NotifyObjectUpdated(false);
	}

	return false;
}

CLIENT_COMMAND(wdaddmodelswap, "wcid index swap", "Add an ObjDesc model swap", ADMIN_ACCESS, SERVER_CATEGORY)
{
	if (argc < 3)
		return true;

	uint32_t wcid = (uint32_t)atoi(argv[0]);
	uint32_t idx = (uint32_t)atoi(argv[1]);
	uint32_t swap = (uint32_t)atoi(argv[2]);

	CWeenieDefaults *weenie = g_pWeenieFactory->GetWeenieDefaults(wcid);

	if (weenie)
	{
		AnimPartChange *change = new AnimPartChange(idx, swap);
		weenie->m_WorldObjDesc.AddAnimPartChange(change);
	}
	else
	{
		pPlayer->SendText("WCID not found!", LTT_DEFAULT);
	}

	return false;
}

CLIENT_COMMAND(addspelltoitem, "spell id", "Add Spell to last assessed", ADMIN_ACCESS, SERVER_CATEGORY)
{
	if (argc < 1)
		return true;

	uint32_t spellid = (uint32_t)atoi(argv[0]);

	CSpellTable *pSpellTable = MagicSystem::GetSpellTable();
	if (!spellid || !pSpellTable || !pSpellTable->GetSpellBase(spellid))
		return true; 

	CWeenieObject* item = g_pWorld->FindObject(pPlayer->m_LastAssessed);

	if (item) {
		item->m_Qualities.AddSpell(spellid);
		item->NotifyObjectUpdated(false);
	}

	return false;
}
