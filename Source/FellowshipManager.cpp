
#include "StdAfx.h"
#include "FellowshipManager.h"
#include "World.h"
#include "WeenieObject.h"
#include "Player.h"
#include "ChatMsgs.h"

#define MAX_FELLOWSHIP_MEMBERS 9
#define FELLOWSHIP_OPTIONAL_UPDATING 1

DEFINE_PACK(Fellow)
{
	pWriter->Write<unsigned int>(_cp_cache);
	pWriter->Write<unsigned int>(_lum_cache);
	pWriter->Write<unsigned int>(_level);
	pWriter->Write<unsigned int>(_max_health);
	pWriter->Write<unsigned int>(_max_stamina);
	pWriter->Write<unsigned int>(_max_mana);
	pWriter->Write<unsigned int>(_current_health);
	pWriter->Write<unsigned int>(_current_stamina);
	pWriter->Write<unsigned int>(_current_mana);
	pWriter->Write<unsigned int>(_share_loot);
	pWriter->WriteString(_name);
}

DEFINE_UNPACK(Fellow)
{
	return true;
}

DEFINE_PACK(Fellowship)
{
	_fellowship_table.Pack(pWriter);

	pWriter->WriteString(_name);
	pWriter->Write<DWORD>(_leader);
	pWriter->Write<int>(_share_xp);
	pWriter->Write<int>(_even_xp_split);
	pWriter->Write<int>(_open_fellow);
	pWriter->Write<int>(_locked);

	_fellows_departed.Pack(pWriter);

}

DEFINE_UNPACK(Fellowship)
{
	return true;
}

void Fellowship::UpdateData()
{
	bool bAllOver50 = false;
	int minLevel = -1;
	int maxLevel = -1;
	int leaderLevel = -1;

	for (auto &entry : _fellowship_table)
	{
		CPlayerWeenie *player = g_pWorld->FindPlayer(entry.first);
		if (player)
		{
			entry.second._name = player->GetName();
			unsigned int level = entry.second._level = player->InqIntQuality(LEVEL_INT, 0);

			entry.second._current_health = player->GetHealth();
			entry.second._current_stamina = player->GetStamina();
			entry.second._current_mana = player->GetMana();

			entry.second._max_health = player->GetMaxHealth();
			entry.second._max_stamina = player->GetMaxStamina();
			entry.second._max_mana = player->GetMaxMana();

			entry.second._cp_cache = 0; // TODO
			entry.second._lum_cache = 0; // TODO
			entry.second._share_loot = _share_loot ? 0x0010 : 0;

			if (minLevel <= 0 || level < minLevel)
				minLevel = level;
			if (maxLevel <= 0 || level > maxLevel)
				maxLevel = level;

			entry.second._cachedWeenie = player;

			if (entry.first == _leader)
				leaderLevel = level;
		}
	}

	if (minLevel >= 50)
		bAllOver50 = true;

	/*
	int levelSpread = maxLevel - minLevel;
	_even_xp_split = (bAllOver50 || levelSpread <= 5) && _desiredShareXP;
	_share_xp = (bAllOver50 || levelSpread <= 10) && _desiredShareXP;
	*/

	bool bWithin5Levels = ((abs(leaderLevel - maxLevel) <= 5) && (abs(leaderLevel - minLevel) <= 5)) ? true : false;
	bool bWithin10Levels = ((abs(leaderLevel - maxLevel) <= 10) && (abs(leaderLevel - minLevel) <= 10)) ? true : false;

	_even_xp_split = (bAllOver50 || bWithin5Levels) && _desiredShareXP;
	_share_xp = (bAllOver50 || bWithin10Levels) && _desiredShareXP;

	unsigned int xpPortionSum = CalculateExperienceProportionSum();

	for (auto &entry : _fellowship_table)
	{
		if (_even_xp_split)
			entry.second.splitPercent = FellowshipManager::GetEvenSplitXPPctg((DWORD) _fellowship_table.size());
		else
			entry.second.splitPercent = FellowshipManager::GetExperienceProportion(entry.second._level) / (double)xpPortionSum;
	}
}

double Fellowship::CalculateDegradeMod(CWeenieObject *source, CWeenieObject *target)
{
	WORD source_block_id = source->m_Position.objcell_id >> 16;
	WORD source_cell_id = source->m_Position.objcell_id & 0xFFFF;
	WORD target_block_id = target->m_Position.objcell_id >> 16;
	WORD target_cell_id = target->m_Position.objcell_id & 0xFFFF;

	if (source_cell_id >= 0x100 || target_cell_id >= 0x100)
	{
		if (source_block_id != target_block_id)
			return 0.0;
	}

	// both are outside, or in a building/dungeon in the same land block
	double dist = source->DistanceTo(target);
	double numClicks = (dist / 24.0);

	if (numClicks <= 25)
		return 1.0; // full XP within 2.5 map units
	if (numClicks >= 50)
		return 0.0; // no XP beyond 5.0 map units

	return max(0.0, min(1.0, 1.0 - ((numClicks - 25.0) / 25.0)));
}

void Fellowship::GiveXP(CWeenieObject *source, long long amount, bool bShowText)
{
	UpdateData();

	if (!_share_xp)
	{
		source->GiveXP(amount, bShowText);
	}
	else
	{
		for (auto &entry : _fellowship_table)
		{
			if (entry.first == source->GetID())
				entry.second._cp_cache += amount;

			CWeenieObject *other = entry.second._cachedWeenie;
			if (other)
			{
				double degradeMod = CalculateDegradeMod(source, other);

				long long xpGained = (long long) (amount * entry.second.splitPercent * degradeMod);
				if (xpGained > 0)
					other->GiveXP(xpGained, bShowText);
			}
		}
	}
}

unsigned int Fellowship::CalculateExperienceProportionSum()
{
	// make sure the levels are correct in the data before calling this
	unsigned int sum = 0;

	for (auto &entry : _fellowship_table)
		sum += FellowshipManager::GetExperienceProportion(entry.second._level);

	return sum;
}

void Fellowship::TickUpdate()
{

	// check if any need updating

#if FELLOWSHIP_OPTIONAL_UPDATING 
	bool bNeedUpdate = false;
	for (auto &entry : _fellowship_table)
	{
		if (entry.second._updates)
		{
			bNeedUpdate = true;
			break;
		}
	}

	if (!bNeedUpdate)
		return;
#endif
	UpdateData();

	BinaryWriter updateMessage;
	updateMessage.Write<DWORD>(0x2BE);
	Pack(&updateMessage);

	for (auto &entry : _fellowship_table)
	{
#if FELLOWSHIP_OPTIONAL_UPDATING 
		if (entry.second._updates)
#endif
		{
			CPlayerWeenie *player = g_pWorld->FindPlayer(entry.first);
			if (player)
				player->SendNetMessage(&updateMessage, PRIVATE_MSG, TRUE, FALSE);
		}
	}
}

void Fellowship::SendUpdate(int updateType)
{
	UpdateData();

	switch (updateType)
	{
	case Fellow_UpdateFull:
		{
			BinaryWriter updateMessage;
			updateMessage.Write<DWORD>(0x2BE);
			Pack(&updateMessage);

			for (auto &entry : _fellowship_table)
			{
				CPlayerWeenie *player = g_pWorld->FindPlayer(entry.first);
				if (player)
					player->SendNetMessage(&updateMessage, PRIVATE_MSG, TRUE, FALSE);
			}
		}
	}
}

bool Fellowship::IsEmpty()
{
	return _fellowship_table.empty();
}

bool Fellowship::IsLocked()
{
	return _locked ? true : false;
}

bool Fellowship::IsOpen()
{
	return _open_fellow ? true : false;
}

bool Fellowship::IsFull()
{
	if (_fellowship_table.size() >= MAX_FELLOWSHIP_MEMBERS)
		return true;

	return false;
}

void Fellowship::Disband(DWORD disbander_id)
{
	BinaryWriter disbandMessage;
	disbandMessage.Write<DWORD>(0x2BF);
	for (auto &entry : _fellowship_table)
	{
		CPlayerWeenie *player = g_pWorld->FindPlayer(entry.first);
		if (player)
		{
			player->SendNetMessage(&disbandMessage, PRIVATE_MSG, TRUE, FALSE);
			player->m_Qualities.RemoveString(FELLOWSHIP_STRING);
		}
	}
	_fellowship_table.clear();
}

void Fellowship::Dismiss(DWORD dismissee_id)
{
	BinaryWriter dismissMessage;
	dismissMessage.Write<DWORD>(0xA4);
	dismissMessage.Write<DWORD>(dismissee_id);

	for (auto &entry : _fellowship_table)
	{
		if (CPlayerWeenie *player = g_pWorld->FindPlayer(entry.first))
		{
			player->SendNetMessage(&dismissMessage, PRIVATE_MSG, TRUE, FALSE);
		}
	}

	_fellowship_table.remove(dismissee_id);
	if (CPlayerWeenie *player = g_pWorld->FindPlayer(dismissee_id))
	{
		player->m_Qualities.RemoveString(FELLOWSHIP_STRING);
	}

	SendUpdate(Fellow_UpdateFull);
}

void Fellowship::Quit(DWORD quitter_id)
{
	BinaryWriter quitMessage;
	quitMessage.Write<DWORD>(0xA3);
	quitMessage.Write<DWORD>(quitter_id);

	std::string quitter_name;
	if (CPlayerWeenie *player = g_pWorld->FindPlayer(quitter_id))
	{
		player->m_Qualities.RemoveString(FELLOWSHIP_STRING);
		quitter_name = player->GetName();
	}
	// BinaryWriter *quitText = ServerText(csprintf("%s has left the Fellowship.", quitter_name.c_str()), LTT_DEFAULT);

	for (auto &entry : _fellowship_table)
	{
		if (CPlayerWeenie *player = g_pWorld->FindPlayer(entry.first))
		{
			player->SendNetMessage(&quitMessage, PRIVATE_MSG, TRUE, FALSE);
			// player->SendNetMessage(quitText, PRIVATE_MSG, FALSE, FALSE);
		}
	}

	// delete quitText;

	_fellowship_table.remove(quitter_id);
	if (quitter_id == _leader && _fellowship_table.size() >= 1)
	{
		AssignNewLeader(_fellowship_table.begin()->first);
	}

	SendUpdate(Fellow_UpdateFull);
}

void Fellowship::Recruit(DWORD recruitee_id)
{
	Fellow newFellow;
	_fellowship_table.add(recruitee_id, &newFellow);

	UpdateData();

	Fellow *f = _fellowship_table.lookup(recruitee_id);

	BinaryWriter recruitMessage;
	recruitMessage.Write<DWORD>(0x2C0);
	recruitMessage.Write<DWORD>(recruitee_id);
	f->Pack(&recruitMessage);
	recruitMessage.Write<DWORD>(Fellow_UpdateFull);
	
	for (auto &entry : _fellowship_table)
	{
		if (CPlayerWeenie *player = g_pWorld->FindPlayer(entry.first))
		{
			if (entry.first == recruitee_id)
			{
				// full update to the new recruit
				BinaryWriter updateMessage;
				updateMessage.Write<DWORD>(0x2BE);
				Pack(&updateMessage);

				player->SendNetMessage(&updateMessage, PRIVATE_MSG, TRUE, FALSE);
				player->m_Qualities.SetString(FELLOWSHIP_STRING, _name);
			}
			else
			{
				// partial update to everyone else
				player->SendNetMessage(&recruitMessage, PRIVATE_MSG, TRUE, FALSE);
			}
		}
	}
}

void Fellowship::ChangeOpen(BOOL open)
{
	_open_fellow = open;
	
	std::string leader_name;
	CWeenieObject *leader_weenie = g_pWorld->FindPlayer(_leader);
	if (leader_weenie)
		leader_name = leader_weenie->GetName();

	for (auto &entry : _fellowship_table)
	{
		CPlayerWeenie *player = g_pWorld->FindPlayer(entry.first);
		if (player)
		{
			if (entry.first == _leader)
				player->SendNetMessage(ServerText(csprintf("You have %s your Fellowship.", open ? "opened" : "closed"), LTT_DEFAULT), PRIVATE_MSG, FALSE, TRUE);
			else
				player->SendNetMessage(ServerText(csprintf("%s has %s your Fellowship.", leader_name.c_str(), open ? "opened" : "closed"), LTT_DEFAULT), PRIVATE_MSG, FALSE, TRUE);
		}
	}
}

void Fellowship::AssignNewLeader(DWORD new_leader_id)
{
	_leader = new_leader_id;

	CWeenieObject *leader_weenie = g_pWorld->FindPlayer(new_leader_id);
	if (leader_weenie)
	{
		std::string new_leader_name = leader_weenie->GetName();

		for (auto &entry : _fellowship_table)
		{
			CPlayerWeenie *player = g_pWorld->FindPlayer(entry.first);
			if (player)
			{
				if (entry.first == new_leader_id)
					player->SendNetMessage(ServerText("You are now the leader of your Fellowship.", LTT_DEFAULT), PRIVATE_MSG, FALSE, TRUE);
				else
					player->SendNetMessage(ServerText(csprintf("%s is now the leader of your Fellowship.", new_leader_name.c_str()), LTT_DEFAULT), PRIVATE_MSG, FALSE, TRUE);
			}
		}
	}

	SendUpdate(Fellow_UpdateFull);
}

void Fellowship::Chat(DWORD sender_id, const char *text)
{
	CWeenieObject *sender_weenie = g_pWorld->FindPlayer(sender_id);
	if (sender_weenie)
	{
		sender_weenie->SendNetMessage(ServerText(csprintf("You say to your fellowship, \"%s\"", text), LTT_FELLOWSHIP_CHANNEL), PRIVATE_MSG, FALSE, TRUE);

		std::string sender_name = sender_weenie->GetName();
		for (auto &entry : _fellowship_table)
		{
			if (entry.first != sender_id)
			{
				CPlayerWeenie *player = g_pWorld->FindPlayer(entry.first);
				if (player)
					player->SendNetMessage(ServerText(csprintf("%s says to your fellowship, \"%s\"", sender_name.c_str(), text), LTT_FELLOWSHIP_CHANNEL), PRIVATE_MSG, FALSE, TRUE);
			}
		}
	}
}

void Fellowship::SetUpdates(DWORD fellow_id, BOOL on)
{
	Fellow *f = _fellowship_table.lookup(fellow_id);
	if (!f)
		return;

	f->_updates = on;
}

FellowshipManager::FellowshipManager()
{
}

FellowshipManager::~FellowshipManager()
{
	for (auto &entry : m_Fellowships)
		delete entry.second;

	m_Fellowships.clear();
}

void FellowshipManager::Tick()
{
	if (m_fNextUpdates <= Timer::cur_time)
	{
		for (auto &fs : m_Fellowships)
		{
			fs.second->TickUpdate();
		}

		m_fNextUpdates = Timer::cur_time + 1.0;
	}
}

Fellowship *FellowshipManager::GetFellowship(const std::string &name)
{
	std::unordered_map<std::string, Fellowship *>::iterator entry = m_Fellowships.find(name);

	if (entry != m_Fellowships.end())
		return entry->second;

	return NULL;
}

int FellowshipManager::Create(const std::string &name, DWORD creator_id, BOOL shareXP)
{
	if (GetFellowship(name))
		return WERROR_FELLOWSHIP_UNCLEAN_NAME; // in use

	Fellowship *fs = new Fellowship();

	fs->_name = name;
	fs->_leader = creator_id;
	fs->_desiredShareXP = shareXP;
	fs->_locked = false;
	fs->_open_fellow = false;
	fs->_even_xp_split = true;
	fs->_share_loot = false;

	Fellow f;
	fs->_fellowship_table.add(creator_id, &f);

	CWeenieObject *playerWeenie = g_pWorld->FindPlayer(creator_id);
	if (CPlayerWeenie *player = playerWeenie->AsPlayer())
	{
		player->m_Qualities.SetString(FELLOWSHIP_STRING, name);
		fs->_share_loot = player->ShareFellowshipLoot();
	}

	m_Fellowships[name] = fs;

	fs->SendUpdate(Fellow_UpdateFull);
	return WERROR_NONE;
}

int FellowshipManager::Disband(const std::string &name, DWORD disbander_id)
{	
	std::unordered_map<std::string, Fellowship *>::iterator i = m_Fellowships.find(name);
	if (i == m_Fellowships.end())
		return WERROR_FELLOWSHIP_NO_FELLOWSHIP;

	Fellowship *fs = i->second;
	if (fs->_leader != disbander_id)
		return WERROR_FELLOWSHIP_NOT_LEADER;

	m_Fellowships.erase(i);
	fs->Disband(disbander_id);
	delete fs;

	return WERROR_NONE;
}

int FellowshipManager::Quit(const std::string &name, DWORD quitter_id)
{
	std::unordered_map<std::string, Fellowship *>::iterator i = m_Fellowships.find(name);
	if (i == m_Fellowships.end())
		return WERROR_FELLOWSHIP_NO_FELLOWSHIP;

	Fellowship *fs = i->second;
	fs->Quit(quitter_id);

	if (fs->IsEmpty())
	{
		m_Fellowships.erase(i);
		delete fs;
	}

	return WERROR_NONE;
}

int FellowshipManager::Dismiss(const std::string &name, DWORD dismisser_id, DWORD dismissee_id)
{
	std::unordered_map<std::string, Fellowship *>::iterator i = m_Fellowships.find(name);
	if (i == m_Fellowships.end())
		return WERROR_FELLOWSHIP_NO_FELLOWSHIP;

	Fellowship *fs = i->second;
	if (fs->_leader != dismisser_id)
		return WERROR_FELLOWSHIP_NOT_LEADER;

	CWeenieObject *dismissee_weenie = g_pWorld->FindPlayer(dismissee_id);
	if (!dismissee_weenie)
		return WERROR_NO_OBJECT;
	if (!dismissee_weenie->HasFellowship())
		return WERROR_NO_OBJECT;
	if (strcmp(dismissee_weenie->InqStringQuality(FELLOWSHIP_STRING, "").c_str(), name.c_str()))
		return WERROR_NO_OBJECT;

	fs->Dismiss(dismissee_id);

	if (fs->IsEmpty())
	{
		m_Fellowships.erase(i);
		delete fs;
	}

	return WERROR_NONE;
}

int FellowshipManager::Recruit(const std::string &name, DWORD recruiter_id, DWORD recruitee_id)
{
	std::unordered_map<std::string, Fellowship *>::iterator i = m_Fellowships.find(name);
	if (i == m_Fellowships.end())
		return WERROR_FELLOWSHIP_NO_FELLOWSHIP;

	Fellowship *fs = i->second;
	if (fs->IsFull())
		return WERROR_FELLOWSHIP_FULL;
	if (!fs->IsOpen() && fs->_leader != recruiter_id)
		return WERROR_FELLOWSHIP_NOT_LEADER;
	if (fs->IsLocked())
		return WERROR_FELLOWSHIP_LOCKED_RECRUITER;

	CPlayerWeenie *recruiter_weenie = g_pWorld->FindPlayer(recruiter_id);
	if (!recruiter_weenie)
		return WERROR_NO_OBJECT;
	if (recruiter_weenie->IsBusyOrInAction())
		return WERROR_ACTIONS_LOCKED;

	CPlayerWeenie *recruited_weenie = g_pWorld->FindPlayer(recruitee_id);
	if (!recruited_weenie)
		return WERROR_NO_OBJECT;

	if (recruiter_weenie->DistanceTo(recruited_weenie, true) >= 3.0)
		return WERROR_TOO_FAR;

	if (recruited_weenie->IsBusyOrInAction())
		return WERROR_ACTIONS_LOCKED;
	if (recruited_weenie->HasFellowship())
		return WERROR_FELLOWSHIP_MEMBER;
	if (recruited_weenie->GetCharacterOptions() & IgnoreFellowshipRequests_CharacterOption)
		return WERROR_FELLOWSHIP_IGNORING_REQUESTS;

	fs->Recruit(recruitee_id);

	if (fs->IsEmpty())
	{
		m_Fellowships.erase(i);
		delete fs;
	}

	recruiter_weenie->DoForcedMotion(Motion_BowDeep);

	return WERROR_NONE;
}

int FellowshipManager::ChangeOpen(const std::string &name, DWORD changer_id, BOOL open)
{
	std::unordered_map<std::string, Fellowship *>::iterator i = m_Fellowships.find(name);
	if (i == m_Fellowships.end())
		return WERROR_FELLOWSHIP_NO_FELLOWSHIP;

	Fellowship *fs = i->second;
	if (fs->_leader != changer_id)
		return WERROR_FELLOWSHIP_NOT_LEADER;

	fs->ChangeOpen(open);
	return WERROR_NONE;
}

int FellowshipManager::AssignNewLeader(const std::string &name, DWORD changer_id, DWORD new_leader_id)
{
	std::unordered_map<std::string, Fellowship *>::iterator i = m_Fellowships.find(name);
	if (i == m_Fellowships.end())
		return WERROR_FELLOWSHIP_NO_FELLOWSHIP;

	if (changer_id == new_leader_id)
		return WERROR_FELLOWSHIP_MEMBER;

	Fellowship *fs = i->second;
	if (fs->_leader != changer_id)
		return WERROR_FELLOWSHIP_NOT_LEADER;
	if (!fs->_fellowship_table.lookup(new_leader_id))
		return WERROR_NO_OBJECT;

	fs->AssignNewLeader(new_leader_id);
	return WERROR_NONE;
}

int FellowshipManager::RequestUpdates(const std::string &name, DWORD requester_id, BOOL on)
{
	std::unordered_map<std::string, Fellowship *>::iterator i = m_Fellowships.find(name);
	if (i == m_Fellowships.end())
		return WERROR_FELLOWSHIP_NO_FELLOWSHIP;

	Fellowship *fs = i->second;
	fs->SetUpdates(requester_id, on);
	return WERROR_NONE;
}

double FellowshipManager::GetEvenSplitXPPctg(unsigned int uiNumFellows)
{
	switch (uiNumFellows)
	{
	case 1:
		return 1.0;
	case 2:
		return 0.75;
	case 3:
		return 0.6;
	case 4:
		return 0.55;
	case 5:
		return 0.5;
	case 6:
		return 0.45;
	case 7:
		return 0.4;
	case 8:
		return 0.35;
	case 9:
		return 0.31111111;
	case 0xA:
		return 0.28;
	default:
		return 0.0;
	}
}

unsigned __int64 FellowshipManager::GetExperienceProportion(unsigned int level)
{
	return ExperienceSystem::ExperienceToRaiseLevel(level, level + 1);
}

void FellowshipManager::Chat(const std::string &name, DWORD sender_id, const char *text)
{
	std::unordered_map<std::string, Fellowship *>::iterator i = m_Fellowships.find(name);
	if (i == m_Fellowships.end())
		return;

	Fellowship *fs = i->second;
	fs->Chat(sender_id, text);
}
