
#include <StdAfx.h>
#include "FellowshipManager.h"
#include "World.h"
#include "WeenieObject.h"
#include "Player.h"
#include "ChatMsgs.h"
#include "Config.h"

#define MAX_FELLOWSHIP_MEMBERS 9
#define FELLOWSHIP_OPTIONAL_UPDATING 1

Fellow::Fellow(uint32_t player_id) : Fellow::Fellow(g_pWorld->FindPlayer(player_id))
{
}

Fellow::Fellow(CWeenieObject* weenie)
{
	if (weenie)
	{
		_cachedWeenie = weenie;

		_name = weenie->GetName();
		_level = weenie->InqIntQuality(LEVEL_INT, 0);

		// share loot from char options?
		CPlayerWeenie *player = weenie->AsPlayer();
		_share_loot = (player->GetCharacterOptions() & FellowshipShareLoot_CharacterOption) != 0;
		if (_share_loot) _share_loot = 0x0010;

		_current_health = weenie->GetHealth();
		_current_stamina = weenie->GetStamina();
		_current_mana = weenie->GetMana();

		_max_health = weenie->GetMaxHealth();
		_max_stamina = weenie->GetMaxStamina();
		_max_mana = weenie->GetMaxMana();
	}
}

Fellow& Fellow::operator=(const Fellow &other)
{
	_cachedWeenie = other._cachedWeenie;

	_name = other._name;
	_level = other._level;

	_share_loot = other._share_loot;

	_current_health = other._current_health;
	_current_stamina = other._current_stamina;
	_current_mana = other._current_mana;

	_max_health = other._max_health;
	_max_stamina = other._max_stamina;
	_max_mana = other._max_mana;

	return *this;
}

bool Fellow::updated(const Fellow &other) const
{
	if (!_cachedWeenie || !other._cachedWeenie)
		return false;

	if (_cachedWeenie->GetID() != other._cachedWeenie->GetID())
		return false;

	if (_level != other._level)
		return true;

	if (_max_health != other._max_health || _max_stamina != other._max_stamina || _max_mana != other._max_mana)
		return true;

	// this is a noisy packet, so we'll apply a threshold to updates
	if (other._max_health > 0 && (float)abs((int)_current_health - (int)other._current_health) / (float)other._max_health >= g_pConfig->GetFellowHealthThreshold())
		return true;

	if (other._max_stamina > 0 && (float)abs((int)_current_stamina - (int)other._current_stamina) / (float)other._max_stamina >= g_pConfig->GetFellowStamThreshold())
		return true;

	if (other._max_mana > 0 && (float)abs((int)_current_mana - (int)other._current_mana) / (float)other._max_mana >= g_pConfig->GetFellowManaThreshold())
		return true;

	return false;
}

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
	pWriter->Write<uint32_t>(_leader);
	pWriter->Write<int>(_share_xp);
	pWriter->Write<int>(_even_xp_split);
	pWriter->Write<int>(_open_fellow);
	pWriter->Write<int>(_locked);

	_fellows_departed.Pack(pWriter);
	if (_fellows_departed.empty())
	{
		// Bad Decal, it needs another int
		pWriter->Write<int>(0);
	}
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
			//entry.second._name = player->GetName();
			unsigned int level = entry.second._level;

			//entry.second._current_health = player->GetHealth();
			//entry.second._current_stamina = player->GetStamina();
			//entry.second._current_mana = player->GetMana();

			//entry.second._max_health = player->GetMaxHealth();
			//entry.second._max_stamina = player->GetMaxStamina();
			//entry.second._max_mana = player->GetMaxMana();

			//entry.second._cp_cache = 0; // TODO
			//entry.second._lum_cache = 0; // TODO
			//entry.second._share_loot = _share_loot ? 0x0010 : 0;

			if (minLevel <= 0 || level < minLevel)
				minLevel = level;
			if (maxLevel <= 0 || level > maxLevel)
				maxLevel = level;

			//entry.second._cachedWeenie = player;

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
	double xpPortionPctg = 1.0 / (double)xpPortionSum;

	for (auto &entry : _fellowship_table)
	{
		if (_even_xp_split)
			entry.second.splitPercent = FellowshipManager::GetEvenSplitXPPctg((uint32_t) _fellowship_table.size());
		else
			entry.second.splitPercent = xpPortionPctg * (double)entry.second._level;
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

void Fellowship::GiveXP(CWeenieObject *source, int64_t amount, ExperienceHandlingType flags, bool bShowText)
{
	UpdateData();

	flags_clear(flags, ExperienceHandlingType::ShareWithFellows);

	// TODO: Apply fellowship experience handling

	if (!_share_xp)
	{
		source->GiveXP(amount, flags, bShowText);
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

				int64_t xpGained = (int64_t) (amount * entry.second.splitPercent * degradeMod);
				if (xpGained > 0)
					other->GiveXP(xpGained, flags, bShowText);
			}
		}
	}
}

void Fellowship::GiveLum(CWeenieObject *source, int64_t amount, bool bShowText)
{
	

	if (!_share_xp)
	{
		source->GiveLum(amount, bShowText);
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

				int64_t xpGained = (int64_t)(amount * entry.second.splitPercent * degradeMod);
				if (xpGained > 0)
					other->GiveLum(xpGained, bShowText);
			}
		}
	}
}

unsigned int Fellowship::CalculateExperienceProportionSum()
{
	// make sure the levels are correct in the data before calling this
	unsigned int sum = 0;

	for (auto &entry : _fellowship_table)
		sum += entry.second._level;

	return sum;
}

bool Fellowship::ShouldUpdate(uint32_t fellow_id)
{
	CPlayerWeenie *player = g_pWorld->FindPlayer(fellow_id);
	if (!player)
		return false;

	Fellow updated(player);
	Fellow *current = _fellowship_table.lookup(fellow_id);
	if (current && current->updated(updated))
	{
		*current = updated;
		return true;
	}

	return false;
}

void Fellowship::FullUpdate()
{
	UpdateData();

	BinaryWriter updateMessage;
	updateMessage.Write<uint32_t>(0x2BE);
	Pack(&updateMessage);

	for (auto &entry : _fellowship_table)
	{
		CPlayerWeenie *player = g_pWorld->FindPlayer(entry.first);
		if (player)
			player->SendNetMessage(&updateMessage, PRIVATE_MSG, TRUE, FALSE);
	}
}

void Fellowship::VitalsUpdate(uint32_t player_id)
{
	if (ShouldUpdate(player_id))
	{
		UpdateData();
		Fellow *f = _fellowship_table.lookup(player_id);
		if (!f)
			return;

		BinaryWriter updateMessage;
		updateMessage.Write<uint32_t>(0x2C0);
		updateMessage.Write<uint32_t>(player_id);
		f->Pack(&updateMessage);
		updateMessage.Write<uint32_t>(Fellow_UpdateVitals);

		for (auto &entry : _fellowship_table)
		{
			if (CPlayerWeenie *player = g_pWorld->FindPlayer(entry.first))
			{
				player->SendNetMessage(&updateMessage, PRIVATE_MSG, TRUE, FALSE);
			}
		}
	}
}

void Fellowship::StatsUpdate(uint32_t player_id)
{
	if (ShouldUpdate(player_id))
	{
		UpdateData();
		Fellow *f = _fellowship_table.lookup(player_id);
		if (!f)
			return;

		BinaryWriter updateMessage;
		updateMessage.Write<uint32_t>(0x2C0);
		updateMessage.Write<uint32_t>(player_id);
		f->Pack(&updateMessage);
		updateMessage.Write<uint32_t>(Fellow_UpdateStats);

		for (auto &entry : _fellowship_table)
		{
			if (CPlayerWeenie *player = g_pWorld->FindPlayer(entry.first))
			{
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

void Fellowship::Disband(uint32_t disbander_id)
{
	BinaryWriter disbandMessage;
	disbandMessage.Write<uint32_t>(0x2BF);
	for (auto &entry : _fellowship_table)
	{
		CPlayerWeenie *player = g_pWorld->FindPlayer(entry.first);
		if (player)
		{
			player->SendNetMessage(&disbandMessage, PRIVATE_MSG, TRUE, FALSE);
			player->LeaveFellowship();
		}
	}
	_fellowship_table.clear();
}

void Fellowship::RemoveFellow(uint32_t dismissee_id)
{
	BinaryWriter dismissMessage;
	dismissMessage.Write<uint32_t>(0xA4);
	dismissMessage.Write<uint32_t>(dismissee_id);

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
		player->LeaveFellowship();
	}

	FullUpdate();
}

void Fellowship::Quit(uint32_t quitter_id)
{
	BinaryWriter quitMessage;
	quitMessage.Write<uint32_t>(0xA3);
	quitMessage.Write<uint32_t>(quitter_id);

	std::string quitter_name;
	if (CPlayerWeenie *player = g_pWorld->FindPlayer(quitter_id))
	{
		player->LeaveFellowship();
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

	if (IsLocked())
	{
		uint32_t what = 0;
		_fellows_departed.add(quitter_id, &what);
	}

	FullUpdate();
}

bool Fellowship::CanJoin(uint32_t recruitee_id)
{
	if (IsLocked())
	{
		fellows_departed_table::iterator itr = _fellows_departed.find(recruitee_id);
		if (itr != _fellows_departed.end())
		{
			return true;
		}
		return false;
	}
	return true;
}

void Fellowship::AddFellow(uint32_t recruitee_id)
{
	Fellow newFellow(recruitee_id);
	_fellowship_table.add(recruitee_id, &newFellow);

	UpdateData();

	Fellow *f = _fellowship_table.lookup(recruitee_id);

	BinaryWriter recruitMessage;
	recruitMessage.Write<uint32_t>(0x2C0);
	recruitMessage.Write<uint32_t>(recruitee_id);
	f->Pack(&recruitMessage);
	recruitMessage.Write<uint32_t>(Fellow_UpdateFull);
	
	for (auto &entry : _fellowship_table)
	{
		if (CPlayerWeenie *player = g_pWorld->FindPlayer(entry.first))
		{
			if (entry.first == recruitee_id)
			{
				// full update to the new recruit
				BinaryWriter updateMessage;
				updateMessage.Write<uint32_t>(0x2BE);
				Pack(&updateMessage);

				player->SendNetMessage(&updateMessage, PRIVATE_MSG, TRUE, FALSE);
			}
			else
			{
				// partial update to everyone else
				player->SendNetMessage(&recruitMessage, PRIVATE_MSG, TRUE, FALSE);
			}
		}
	}

	fellows_departed_table::iterator itr = _fellows_departed.find(recruitee_id);
	if (itr != _fellows_departed.end())
	{
		_fellows_departed.erase(itr);
	}
}

void Fellowship::ChangeOpen(BOOL open)
{
	if (_open_fellow == open)
		return;

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

	FullUpdate();
}

void Fellowship::AssignNewLeader(uint32_t new_leader_id)
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

	FullUpdate();
}

void Fellowship::Chat(uint32_t sender_id, const char *text)
{
	CWeenieObject *sender_weenie = g_pWorld->FindPlayer(sender_id);
	if (sender_weenie)
	{
		std::string sender_name = sender_weenie->GetName();

		sender_weenie->SendNetMessage(ChannelChat(Fellow_ChannelID, NULL, text), PRIVATE_MSG, FALSE, TRUE);

		for (auto &entry : _fellowship_table)
		{
			if (entry.first != sender_id)
			{
				CPlayerWeenie *player = g_pWorld->FindPlayer(entry.first);
				if (player)
					player->SendNetMessage(ChannelChat(Fellow_ChannelID, sender_name.c_str(), text), PRIVATE_MSG, FALSE, TRUE);
			}
		}
	}
}

void Fellowship::SetUpdates(uint32_t fellow_id, BOOL on)
{
	Fellow *f = _fellowship_table.lookup(fellow_id);
	if (!f)
		return;

	f->_updates = on;
}

void FellowshipManager::Tick()
{

}

int FellowshipManager::Create(const std::string &name, uint32_t creator_id, BOOL shareXP)
{
	CWeenieObject *playerWeenie = g_pWorld->FindPlayer(creator_id);
	if (CPlayerWeenie *player = playerWeenie->AsPlayer())
	{
		fellowship_ptr_t fs = std::make_shared<Fellowship>();

		fs->_name = name;
		fs->_leader = creator_id;
		fs->_desiredShareXP = shareXP;
		fs->_locked = false;
		fs->_open_fellow = false;
		fs->_even_xp_split = true;
		fs->_share_loot = false;

		Fellow f(player);
		fs->_fellowship_table.add(creator_id, &f);
		fs->_share_loot = player->ShareFellowshipLoot();

		player->JoinFellowship(fs);

		fs->FullUpdate();
	}

	return WERROR_NONE;
}

int FellowshipManager::Disband(const fellowship_ptr_t &fellowship, uint32_t disbander_id)
{
	if (!fellowship)
		return WERROR_FELLOWSHIP_NO_FELLOWSHIP;

	if (fellowship->_leader != disbander_id)
		return WERROR_FELLOWSHIP_NOT_LEADER;

	fellowship->Disband(disbander_id);

	return WERROR_NONE;
}

int FellowshipManager::Quit(const fellowship_ptr_t &fellowship, uint32_t quitter_id)
{
	if (!fellowship)
		return WERROR_FELLOWSHIP_NO_FELLOWSHIP;

	fellowship->Quit(quitter_id);

	return WERROR_NONE;
}

int FellowshipManager::Dismiss(const fellowship_ptr_t &fellowship, uint32_t dismisser_id, uint32_t dismissee_id)
{
	if (!fellowship)
		return WERROR_FELLOWSHIP_NO_FELLOWSHIP;

	if (fellowship->_leader != dismisser_id)
		return WERROR_FELLOWSHIP_NOT_LEADER;

	CWeenieObject *dismissee_weenie = g_pWorld->FindPlayer(dismissee_id);
	if (!dismissee_weenie)
		return WERROR_NO_OBJECT;
	if (!dismissee_weenie->HasFellowship())
		return WERROR_NO_OBJECT;

	fellowship_ptr_t other = dismissee_weenie->GetFellowship();
	if (fellowship != other)
		return WERROR_NO_OBJECT;

	fellowship->RemoveFellow(dismissee_id);

	return WERROR_NONE;
}

int FellowshipManager::Recruit(const fellowship_ptr_t &fellowship, uint32_t recruiter_id, uint32_t recruitee_id)
{
	if (!fellowship)
		return WERROR_FELLOWSHIP_NO_FELLOWSHIP;

	if (fellowship->IsFull())
		return WERROR_FELLOWSHIP_FULL;
	if (!fellowship->IsOpen() && fellowship->_leader != recruiter_id)
		return WERROR_FELLOWSHIP_NOT_LEADER;

	CPlayerWeenie *recruiter_weenie = g_pWorld->FindPlayer(recruiter_id);
	if (!recruiter_weenie)
		return WERROR_NO_OBJECT;
	if (recruiter_weenie->IsBusyOrInAction())
		return WERROR_ACTIONS_LOCKED;

	CPlayerWeenie *recruited_weenie = g_pWorld->FindPlayer(recruitee_id);
	if (!recruited_weenie)
		return WERROR_NO_OBJECT;

	if (!fellowship->CanJoin(recruitee_id))
	{
		recruiter_weenie->NotifyWeenieErrorWithString(WERROR_FELLOWSHIP_LOCKED_RECRUITER, recruited_weenie->GetName().c_str());
		recruited_weenie->NotifyWeenieError(WERROR_FELLOWSHIP_LOCKED_RECRUITEE);
		return WERROR_NONE;
	}

	if (recruiter_weenie->DistanceTo(recruited_weenie, true) >= 16.0)
		return WERROR_TOO_FAR;

	if (recruited_weenie->IsBusyOrInAction())
		return WERROR_ACTIONS_LOCKED;
	if (recruited_weenie->HasFellowship())
		return WERROR_FELLOWSHIP_MEMBER;
	if (recruited_weenie->GetCharacterOptions() & IgnoreFellowshipRequests_CharacterOption)
		return WERROR_FELLOWSHIP_IGNORING_REQUESTS;

	fellowship->AddFellow(recruitee_id);
	recruited_weenie->JoinFellowship(fellowship);

	recruiter_weenie->DoForcedMotion(Motion_BowDeep);

	return WERROR_NONE;
}

int FellowshipManager::ChangeOpen(const fellowship_ptr_t &fellowship, uint32_t changer_id, BOOL open)
{
	if (!fellowship)
		return WERROR_FELLOWSHIP_NO_FELLOWSHIP;

	if (fellowship->_leader != changer_id)
		return WERROR_FELLOWSHIP_NOT_LEADER;

	if (fellowship->IsLocked())
		return WERROR_FELLOWSHIP_FELLOW_LOCKED_CAN_NOT_OPEN;

	fellowship->ChangeOpen(open);
	return WERROR_NONE;
}

int FellowshipManager::AssignNewLeader(const fellowship_ptr_t &fellowship, uint32_t changer_id, uint32_t new_leader_id)
{
	if (!fellowship)
		return WERROR_FELLOWSHIP_NO_FELLOWSHIP;

	if (changer_id == new_leader_id)
		return WERROR_FELLOWSHIP_MEMBER;

	if (fellowship->_leader != changer_id)
		return WERROR_FELLOWSHIP_NOT_LEADER;
	if (!fellowship->_fellowship_table.lookup(new_leader_id))
		return WERROR_NO_OBJECT;

	fellowship->AssignNewLeader(new_leader_id);
	return WERROR_NONE;
}

int FellowshipManager::RequestUpdates(const fellowship_ptr_t &fellowship, uint32_t requester_id, BOOL on)
{
	if (!fellowship)
		return WERROR_FELLOWSHIP_NO_FELLOWSHIP;

	fellowship->SetUpdates(requester_id, on);
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

uint64_t FellowshipManager::GetExperienceProportion(unsigned int level)
{
	return ExperienceSystem::ExperienceToRaiseLevel(level, level + 1);
}

void FellowshipManager::Chat(const fellowship_ptr_t &fellowship, uint32_t sender_id, const char *text)
{
	if (!fellowship)
		return;

	fellowship->Chat(sender_id, text);
}
