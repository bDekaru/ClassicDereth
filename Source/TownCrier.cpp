
#include "StdAfx.h"
#include "TownCrier.h"
#include "ChatMsgs.h"
#include "World.h"
#include "Player.h"
#include "SpellCastingManager.h"
#include "Config.h"

CTownCrier::CTownCrier()
{
	m_Qualities.SetInt(ITEM_USEABLE_INT, USEABLE_REMOTE);
	m_Qualities.SetFloat(USE_RADIUS_FLOAT, 3.0);
}

CTownCrier::~CTownCrier()
{
}

std::string CTownCrier::GetNewsText(bool paid)
{
	std::vector<std::string> phrases;
	phrases.push_back("What's the story behind these Banished creatures? They sure sound scary to me.");
	phrases.push_back("I was speaking with a Lugian friend of mine the other day when he mentioned something about a great disturbance. He refused to elaborate, though I plied him with enough ale to drop a horse.");
	phrases.push_back("The Queen is looking for help in finding out how Carlo di Cenza found his way to Dereth. Talk to Antius Blackmoor in Yaraq if you wish to help!");
	phrases.push_back("Last month I found what I thought was the most powerful sword I have yet come across. But this month, I found an even better one! This a good time to be a hunter!");
	phrases.push_back("The Queen is greatly disturbed by the recent attack on the Royal Vaults. She fears we may not have heard the last of these mysterious Lugians.");
	phrases.push_back("The wind from the north grows cold. I fear a storm may be on the horizon.");
	phrases.push_back("I can't wait to put this back together!");
	phrases.push_back("Has it really been a year since poor Samuel was discharged from the Royal Guard? I would like to say Eastham was better for it, but... well... Sam hasn't done much outside of drink.");
	phrases.push_back("Who is this Sezzherei I keep hearing about? And what does he want with the Caul?");
	phrases.push_back("Have you found them yet? Those Singularity Caul reliquaries I keep hearing about? I hear people have found some strange items in them!");
	phrases.push_back("If you stand on the northern shores and listen closely enough, you can hear the screams of war. What madness makes its home up there? And when will it find its way to our lands?");
	phrases.push_back("Lord Kresovus has put out a call for help. The Queen offered her aid, but he said he would rather seek assistance from independent adventurers. I wonder what that's all about?");
	phrases.push_back("My good friend Loh-Gann Huhjj found a page from Carlo di Cenza's journal on the beach near the northern Beach Fort. It appears to be part of a much larger work. I sure hope he finds all the pieces. I am very interested in this Viamontian's past.");
	phrases.push_back("A friend of mine was hunting the Singularity Caul when he came across a powerful new creature. It was a tough fight, he said, but he got a nifty spear out of the deal.");
	phrases.push_back("The vendors in Shoushi, Yaraq, and Holtburg must have benefited from those newly relocated casinos. They seem to have several more pyreals in their possession than usual.");
	phrases.push_back("So, Samuel in Eastham claims to have some sort of new wonder drink! He says it cured him of his nightmares. So, I went to the Singularity Caul, got some of those bits he makes the drink from, and gave them over... Can't say that it did much for me outside of clinging to my throat on the way down...");
	phrases.push_back("Has that 50,000 pyreal piece of armor been cluttering up your house chest for too long? Well, go to Shoushi, Yaraq, or Holtburg and sell it!");
	phrases.push_back("Wait, you can't have this handful of blackened mineral! It's mine! If you want some for yourself, you'll have to get it on your own!");

	return phrases[Random::GenInt(0, (DWORD)(phrases.size() - 1))];
}

int CTownCrier::DoUseResponse(CWeenieObject *player)
{
	if (!IsCompletelyIdle())
	{
		return WERROR_ACTIONS_LOCKED;
	}

	if ((m_LastUsed + 5.0) < Timer::cur_time)
	{
		m_LastUsedBy = player->GetID();
		m_LastUsed = Timer::cur_time;

		MovementParameters params;
		TurnToObject(player->GetID(), &params);
	}

	return WERROR_NONE;
}

int CTownCrier::Use(CPlayerWeenie *pOther)
{
	CGenericUseEvent *useEvent = new CGenericUseEvent();
	useEvent->_target_id = GetID();
	useEvent->_do_use_emote = false;
	useEvent->_do_use_message = false;
	pOther->ExecuteUseEvent(useEvent);

	return WERROR_NONE;
}

void CTownCrier::HandleMoveToDone(DWORD error)
{
	CWeenieObject::HandleMoveToDone(error);
	
	if (CWeenieObject *pOther = g_pWorld->FindObject(m_LastUsedBy))
	{
		pOther->SendNetMessage(DirectChat(GetNewsText(false).c_str(), GetName().c_str(), GetID(), pOther->GetID(), LTT_SPEECH_DIRECT), PRIVATE_MSG, TRUE);
		
		if (g_pConfig->TownCrierBuffs())
		{
			MakeSpellcastingManager();

			m_SpellcastingManager->CastSpellInstant(pOther->GetID(), ArmorOther7_SpellID);
			m_SpellcastingManager->CastSpellInstant(pOther->GetID(), BladeProtectionOther7_SpellID);
			m_SpellcastingManager->CastSpellInstant(pOther->GetID(), PiercingProtectionOther7_SpellID);
			m_SpellcastingManager->CastSpellInstant(pOther->GetID(), BludgeonProtectionOther7_SpellID);
			m_SpellcastingManager->CastSpellInstant(pOther->GetID(), FireProtectionOther7_SpellID);
			m_SpellcastingManager->CastSpellInstant(pOther->GetID(), ColdProtectionOther7_SpellID);
			m_SpellcastingManager->CastSpellInstant(pOther->GetID(), AcidProtectionOther7_SpellID);
			m_SpellcastingManager->CastSpellInstant(pOther->GetID(), LightningProtectionOther7_SpellID);

			m_SpellcastingManager->CastSpellInstant(pOther->GetID(), StrengthOther7_SpellID);
			m_SpellcastingManager->CastSpellInstant(pOther->GetID(), EnduranceOther7_SpellID);
			m_SpellcastingManager->CastSpellInstant(pOther->GetID(), CoordinationOther7_SpellID);
			m_SpellcastingManager->CastSpellInstant(pOther->GetID(), QuicknessOther7_SpellID);
			m_SpellcastingManager->CastSpellInstant(pOther->GetID(), FocusOther7_SpellID);
			m_SpellcastingManager->CastSpellInstant(pOther->GetID(), WillPowerOther7_SpellID);

			m_SpellcastingManager->CastSpellInstant(pOther->GetID(), CreatureEnchantmentMasteryOther7_SpellID);
			m_SpellcastingManager->CastSpellInstant(pOther->GetID(), LifeMagicMasteryOther7_SpellID);
			m_SpellcastingManager->CastSpellInstant(pOther->GetID(), ItemEnchantmentMasteryOther7_SpellID);
			m_SpellcastingManager->CastSpellInstant(pOther->GetID(), WarMagicMasteryOther7_SpellID);
			m_SpellcastingManager->CastSpellInstant(pOther->GetID(), ManaMasteryOther7_SpellID);

			m_SpellcastingManager->CastSpellInstant(pOther->GetID(), SprintOther7_SpellID);
			m_SpellcastingManager->CastSpellInstant(pOther->GetID(), JumpingMasteryOther7_SpellID);
			m_SpellcastingManager->CastSpellInstant(pOther->GetID(), ImpregnabilityOther7_SpellID);
			m_SpellcastingManager->CastSpellInstant(pOther->GetID(), InvulnerabilityOther7_SpellID);
			m_SpellcastingManager->CastSpellInstant(pOther->GetID(), MagicResistanceOther7_SpellID);
			m_SpellcastingManager->CastSpellInstant(pOther->GetID(), FealtyOther7_SpellID);
			m_SpellcastingManager->CastSpellInstant(pOther->GetID(), LeadershipMasteryOther7_SpellID);

			m_SpellcastingManager->CastSpellInstant(pOther->GetID(), SwordMasteryOther7_SpellID);
			m_SpellcastingManager->CastSpellInstant(pOther->GetID(), AxeMasteryOther7_SpellID);
			m_SpellcastingManager->CastSpellInstant(pOther->GetID(), MaceMasteryOther7_SpellID);
			m_SpellcastingManager->CastSpellInstant(pOther->GetID(), SpearMasteryOther7_SpellID);
			m_SpellcastingManager->CastSpellInstant(pOther->GetID(), StaffMasteryOther7_SpellID);
			m_SpellcastingManager->CastSpellInstant(pOther->GetID(), DaggerMasteryOther7_SpellID);
			m_SpellcastingManager->CastSpellInstant(pOther->GetID(), UnarmedCombatMasteryOther7_SpellID);

			m_SpellcastingManager->CastSpellInstant(pOther->GetID(), CrossbowMasteryOther7_SpellID);
			m_SpellcastingManager->CastSpellInstant(pOther->GetID(), BowMasteryOther7_SpellID);
			m_SpellcastingManager->CastSpellInstant(pOther->GetID(), ThrownWeaponMasteryOther7_SpellID);

			pOther->MakeSpellcastingManager();			
			pOther->m_SpellcastingManager->CastSpellInstant(pOther->GetID(), BloodDrinker7_SpellID);
			pOther->m_SpellcastingManager->CastSpellInstant(pOther->GetID(), Heartseeker7_SpellID);
			pOther->m_SpellcastingManager->CastSpellInstant(pOther->GetID(), Defender7_SpellID);
			pOther->m_SpellcastingManager->CastSpellInstant(pOther->GetID(), Swiftkiller7_SpellID);

			DoForcedMotion(Motion_YMCA);
		}
		else
		{
			DoForcedMotion(Motion_WaveHigh);
		}
	}
}

DWORD CTownCrier::OnReceiveInventoryItem(CWeenieObject *source, CWeenieObject *item, DWORD desired_slot)
{
	if (item->m_Qualities.id == W_COINSTACK_CLASS)
		DoUseResponse(source);
	g_pWorld->RemoveEntity(item);
	return 0;
}


