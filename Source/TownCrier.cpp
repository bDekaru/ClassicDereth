
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

			switch (g_pConfig->TownCrierBuffLevel())
			{
				case 1: 
					m_SpellcastingManager->CastSpellInstant(pOther->GetID(), ArmorOther1_SpellID);
					m_SpellcastingManager->CastSpellInstant(pOther->GetID(), BladeProtectionOther1_SpellID);
					m_SpellcastingManager->CastSpellInstant(pOther->GetID(), PiercingProtectionOther1_SpellID);
					m_SpellcastingManager->CastSpellInstant(pOther->GetID(), BludgeonProtectionOther1_SpellID);
					m_SpellcastingManager->CastSpellInstant(pOther->GetID(), FireProtectionOther1_SpellID);
					m_SpellcastingManager->CastSpellInstant(pOther->GetID(), ColdProtectionOther1_SpellID);
					m_SpellcastingManager->CastSpellInstant(pOther->GetID(), AcidProtectionOther1_SpellID);
					m_SpellcastingManager->CastSpellInstant(pOther->GetID(), LightningProtectionOther1_SpellID);

					m_SpellcastingManager->CastSpellInstant(pOther->GetID(), StrengthOther1_SpellID);
					m_SpellcastingManager->CastSpellInstant(pOther->GetID(), EnduranceOther1_SpellID);
					m_SpellcastingManager->CastSpellInstant(pOther->GetID(), CoordinationOther1_SpellID);
					m_SpellcastingManager->CastSpellInstant(pOther->GetID(), QuicknessOther1_SpellID);
					m_SpellcastingManager->CastSpellInstant(pOther->GetID(), FocusOther1_SpellID);
					m_SpellcastingManager->CastSpellInstant(pOther->GetID(), WillpowerOther1_SpellID);

					m_SpellcastingManager->CastSpellInstant(pOther->GetID(), CreatureEnchantmentMasteryOther1_SpellID);
					m_SpellcastingManager->CastSpellInstant(pOther->GetID(), LifeMagicMasteryOther1_SpellID);
					m_SpellcastingManager->CastSpellInstant(pOther->GetID(), ItemEnchantmentMasteryOther1_SpellID);
					m_SpellcastingManager->CastSpellInstant(pOther->GetID(), WarMagicMasteryOther1_SpellID);
					m_SpellcastingManager->CastSpellInstant(pOther->GetID(), ManaMasteryOther1_SpellID);

					m_SpellcastingManager->CastSpellInstant(pOther->GetID(), SprintOther1_SpellID);
					m_SpellcastingManager->CastSpellInstant(pOther->GetID(), JumpingMasteryOther1_SpellID);
					m_SpellcastingManager->CastSpellInstant(pOther->GetID(), ImpregnabilityOther1_SpellID);
					m_SpellcastingManager->CastSpellInstant(pOther->GetID(), InvulnerabilityOther1_SpellID);
					m_SpellcastingManager->CastSpellInstant(pOther->GetID(), MagicResistanceOther1_SpellID);
					m_SpellcastingManager->CastSpellInstant(pOther->GetID(), FealtyOther1_SpellID);
					m_SpellcastingManager->CastSpellInstant(pOther->GetID(), LeadershipMasteryOther1_SpellID);

					m_SpellcastingManager->CastSpellInstant(pOther->GetID(), SwordMasteryOther1_SpellID);
					m_SpellcastingManager->CastSpellInstant(pOther->GetID(), AxeMasteryOther1_SpellID);
					m_SpellcastingManager->CastSpellInstant(pOther->GetID(), MaceMasteryOther1_SpellID);
					m_SpellcastingManager->CastSpellInstant(pOther->GetID(), SpearMasteryOther1_SpellID);
					m_SpellcastingManager->CastSpellInstant(pOther->GetID(), StaffMasteryOther1_SpellID);
					m_SpellcastingManager->CastSpellInstant(pOther->GetID(), DaggerMasteryOther1_SpellID);
					m_SpellcastingManager->CastSpellInstant(pOther->GetID(), UnarmedCombatMasteryOther1_SpellID);

					m_SpellcastingManager->CastSpellInstant(pOther->GetID(), CrossBowMasteryOther1_SpellID);
					m_SpellcastingManager->CastSpellInstant(pOther->GetID(), BowMasteryOther1_SpellID);
					m_SpellcastingManager->CastSpellInstant(pOther->GetID(), ThrownWeaponMasteryOther1_SpellID);

					pOther->MakeSpellcastingManager();
					pOther->m_SpellcastingManager->CastSpellInstant(pOther->GetID(), BloodDrinker1_SpellID);
					pOther->m_SpellcastingManager->CastSpellInstant(pOther->GetID(), HeartSeeker1_SpellID);
					pOther->m_SpellcastingManager->CastSpellInstant(pOther->GetID(), Defender1_SpellID);
					pOther->m_SpellcastingManager->CastSpellInstant(pOther->GetID(), SwiftKiller1_SpellID);

					if (g_pConfig->TownCrierBuffBanes())
					{
						pOther->m_SpellcastingManager->CastSpellInstant(pOther->GetID(), Impenetrability1_SpellID);
						pOther->m_SpellcastingManager->CastSpellInstant(pOther->GetID(), Bladebane1_SpellID);
						pOther->m_SpellcastingManager->CastSpellInstant(pOther->GetID(), BludgeonBane1_SpellID);
						pOther->m_SpellcastingManager->CastSpellInstant(pOther->GetID(), PiercingBane1_SpellID);
						pOther->m_SpellcastingManager->CastSpellInstant(pOther->GetID(), FlameBane1_SpellID);
						pOther->m_SpellcastingManager->CastSpellInstant(pOther->GetID(), FrostBane1_SpellID);
						pOther->m_SpellcastingManager->CastSpellInstant(pOther->GetID(), LightningBane1_SpellID);
						pOther->m_SpellcastingManager->CastSpellInstant(pOther->GetID(), AcidBane1_SpellID);
					}
					break;       
				case 2: 
					m_SpellcastingManager->CastSpellInstant(pOther->GetID(), ArmorOther2_SpellID);
					m_SpellcastingManager->CastSpellInstant(pOther->GetID(), BladeProtectionOther2_SpellID);
					m_SpellcastingManager->CastSpellInstant(pOther->GetID(), PiercingProtectionOther2_SpellID);
					m_SpellcastingManager->CastSpellInstant(pOther->GetID(), BludgeonProtectionOther2_SpellID);
					m_SpellcastingManager->CastSpellInstant(pOther->GetID(), FireProtectionOther2_SpellID);
					m_SpellcastingManager->CastSpellInstant(pOther->GetID(), ColdProtectionOther2_SpellID);
					m_SpellcastingManager->CastSpellInstant(pOther->GetID(), AcidProtectionOther2_SpellID);
					m_SpellcastingManager->CastSpellInstant(pOther->GetID(), LightningProtectionOther2_SpellID);

					m_SpellcastingManager->CastSpellInstant(pOther->GetID(), StrengthOther2_SpellID);
					m_SpellcastingManager->CastSpellInstant(pOther->GetID(), EnduranceOther2_SpellID);
					m_SpellcastingManager->CastSpellInstant(pOther->GetID(), CoordinationOther2_SpellID);
					m_SpellcastingManager->CastSpellInstant(pOther->GetID(), QuicknessOther2_SpellID);
					m_SpellcastingManager->CastSpellInstant(pOther->GetID(), FocusOther2_SpellID);
					m_SpellcastingManager->CastSpellInstant(pOther->GetID(), WillpowerOther2_SpellID);

					m_SpellcastingManager->CastSpellInstant(pOther->GetID(), CreatureEnchantmentMasteryOther2_SpellID);
					m_SpellcastingManager->CastSpellInstant(pOther->GetID(), LifeMagicMasteryOther2_SpellID);
					m_SpellcastingManager->CastSpellInstant(pOther->GetID(), ItemEnchantmentMasteryOther2_SpellID);
					m_SpellcastingManager->CastSpellInstant(pOther->GetID(), WarMagicMasteryOther2_SpellID);
					m_SpellcastingManager->CastSpellInstant(pOther->GetID(), ManaMasteryOther2_SpellID);

					m_SpellcastingManager->CastSpellInstant(pOther->GetID(), SprintOther2_SpellID);
					m_SpellcastingManager->CastSpellInstant(pOther->GetID(), JumpingMasteryOther2_SpellID);
					m_SpellcastingManager->CastSpellInstant(pOther->GetID(), ImpregnabilityOther2_SpellID);
					m_SpellcastingManager->CastSpellInstant(pOther->GetID(), InvulnerabilityOther2_SpellID);
					m_SpellcastingManager->CastSpellInstant(pOther->GetID(), MagicResistanceOther2_SpellID);
					m_SpellcastingManager->CastSpellInstant(pOther->GetID(), FealtyOther2_SpellID);
					m_SpellcastingManager->CastSpellInstant(pOther->GetID(), LeadershipMasteryOther2_SpellID);

					m_SpellcastingManager->CastSpellInstant(pOther->GetID(), SwordMasteryOther2_SpellID);
					m_SpellcastingManager->CastSpellInstant(pOther->GetID(), AxeMasteryOther2_SpellID);
					m_SpellcastingManager->CastSpellInstant(pOther->GetID(), MaceMasteryOther2_SpellID);
					m_SpellcastingManager->CastSpellInstant(pOther->GetID(), SpearMasteryOther2_SpellID);
					m_SpellcastingManager->CastSpellInstant(pOther->GetID(), StaffMasteryOther2_SpellID);
					m_SpellcastingManager->CastSpellInstant(pOther->GetID(), DaggerMasteryOther2_SpellID);
					m_SpellcastingManager->CastSpellInstant(pOther->GetID(), UnarmedCombatMasteryOther2_SpellID);

					m_SpellcastingManager->CastSpellInstant(pOther->GetID(), CrossBowMasteryOther2_SpellID);
					m_SpellcastingManager->CastSpellInstant(pOther->GetID(), BowMasteryOther2_SpellID);
					m_SpellcastingManager->CastSpellInstant(pOther->GetID(), ThrownWeaponMasteryOther2_SpellID);

					pOther->MakeSpellcastingManager();
					pOther->m_SpellcastingManager->CastSpellInstant(pOther->GetID(), BloodDrinker2_SpellID);
					pOther->m_SpellcastingManager->CastSpellInstant(pOther->GetID(), HeartSeeker2_SpellID);
					pOther->m_SpellcastingManager->CastSpellInstant(pOther->GetID(), Defender2_SpellID);
					pOther->m_SpellcastingManager->CastSpellInstant(pOther->GetID(), SwiftKiller2_SpellID);

					if (g_pConfig->TownCrierBuffBanes())
					{
						pOther->m_SpellcastingManager->CastSpellInstant(pOther->GetID(), Impenetrability2_SpellID);
						pOther->m_SpellcastingManager->CastSpellInstant(pOther->GetID(), BladeBane2_SpellID);
						pOther->m_SpellcastingManager->CastSpellInstant(pOther->GetID(), BludgeonBane2_SpellID);
						pOther->m_SpellcastingManager->CastSpellInstant(pOther->GetID(), PiercingBane2_SpellID);
						pOther->m_SpellcastingManager->CastSpellInstant(pOther->GetID(), FlameBane2_SpellID);
						pOther->m_SpellcastingManager->CastSpellInstant(pOther->GetID(), FrostBane2_SpellID);
						pOther->m_SpellcastingManager->CastSpellInstant(pOther->GetID(), LightningBane2_SpellID);
						pOther->m_SpellcastingManager->CastSpellInstant(pOther->GetID(), AcidBane2_SpellID);
					}
					break;
				case 3:
					m_SpellcastingManager->CastSpellInstant(pOther->GetID(), ArmorOther3_SpellID);
					m_SpellcastingManager->CastSpellInstant(pOther->GetID(), BladeProtectionOther3_SpellID);
					m_SpellcastingManager->CastSpellInstant(pOther->GetID(), PiercingProtectionOther3_SpellID);
					m_SpellcastingManager->CastSpellInstant(pOther->GetID(), BludgeonProtectionOther3_SpellID);
					m_SpellcastingManager->CastSpellInstant(pOther->GetID(), FireProtectionOther3_SpellID);
					m_SpellcastingManager->CastSpellInstant(pOther->GetID(), ColdProtectionOther3_SpellID);
					m_SpellcastingManager->CastSpellInstant(pOther->GetID(), AcidProtectionOther3_SpellID);
					m_SpellcastingManager->CastSpellInstant(pOther->GetID(), LightningProtectionOther3_SpellID);

					m_SpellcastingManager->CastSpellInstant(pOther->GetID(), StrengthOther3_SpellID);
					m_SpellcastingManager->CastSpellInstant(pOther->GetID(), EnduranceOther3_SpellID);
					m_SpellcastingManager->CastSpellInstant(pOther->GetID(), CoordinationOther3_SpellID);
					m_SpellcastingManager->CastSpellInstant(pOther->GetID(), QuicknessOther3_SpellID);
					m_SpellcastingManager->CastSpellInstant(pOther->GetID(), FocusOther3_SpellID);
					m_SpellcastingManager->CastSpellInstant(pOther->GetID(), WillpowerOther3_SpellID);

					m_SpellcastingManager->CastSpellInstant(pOther->GetID(), CreatureEnchantmentMasteryOther3_SpellID);
					m_SpellcastingManager->CastSpellInstant(pOther->GetID(), LifeMagicMasteryOther3_SpellID);
					m_SpellcastingManager->CastSpellInstant(pOther->GetID(), ItemEnchantmentMasteryOther3_SpellID);
					m_SpellcastingManager->CastSpellInstant(pOther->GetID(), WarMagicMasteryOther3_SpellID);
					m_SpellcastingManager->CastSpellInstant(pOther->GetID(), ManaMasteryOther3_SpellID);

					m_SpellcastingManager->CastSpellInstant(pOther->GetID(), SprintOther3_SpellID);
					m_SpellcastingManager->CastSpellInstant(pOther->GetID(), JumpingMasteryOther3_SpellID);
					m_SpellcastingManager->CastSpellInstant(pOther->GetID(), ImpregnabilityOther3_SpellID);
					m_SpellcastingManager->CastSpellInstant(pOther->GetID(), InvulnerabilityOther3_SpellID);
					m_SpellcastingManager->CastSpellInstant(pOther->GetID(), MagicResistanceOther3_SpellID);
					m_SpellcastingManager->CastSpellInstant(pOther->GetID(), FealtyOther3_SpellID);
					m_SpellcastingManager->CastSpellInstant(pOther->GetID(), LeadershipMasteryOther3_SpellID);

					m_SpellcastingManager->CastSpellInstant(pOther->GetID(), SwordMasteryOther3_SpellID);
					m_SpellcastingManager->CastSpellInstant(pOther->GetID(), AxeMasteryOther3_SpellID);
					m_SpellcastingManager->CastSpellInstant(pOther->GetID(), MaceMasteryOther3_SpellID);
					m_SpellcastingManager->CastSpellInstant(pOther->GetID(), SpearMasteryOther3_SpellID);
					m_SpellcastingManager->CastSpellInstant(pOther->GetID(), StaffMasteryOther3_SpellID);
					m_SpellcastingManager->CastSpellInstant(pOther->GetID(), DaggerMasteryOther3_SpellID);
					m_SpellcastingManager->CastSpellInstant(pOther->GetID(), UnarmedCombatMasteryOther3_SpellID);

					m_SpellcastingManager->CastSpellInstant(pOther->GetID(), CrossBowMasteryOther3_SpellID);
					m_SpellcastingManager->CastSpellInstant(pOther->GetID(), BowMasteryOther3_SpellID);
					m_SpellcastingManager->CastSpellInstant(pOther->GetID(), ThrownWeaponMasteryOther3_SpellID);

					pOther->MakeSpellcastingManager();
					pOther->m_SpellcastingManager->CastSpellInstant(pOther->GetID(), BloodDrinker3_SpellID);
					pOther->m_SpellcastingManager->CastSpellInstant(pOther->GetID(), HeartSeeker3_SpellID);
					pOther->m_SpellcastingManager->CastSpellInstant(pOther->GetID(), Defender3_SpellID);
					pOther->m_SpellcastingManager->CastSpellInstant(pOther->GetID(), SwiftKiller3_SpellID);

					if (g_pConfig->TownCrierBuffBanes())
					{
						pOther->m_SpellcastingManager->CastSpellInstant(pOther->GetID(), Impenetrability3_SpellID);
						pOther->m_SpellcastingManager->CastSpellInstant(pOther->GetID(), BladeBane3_SpellID);
						pOther->m_SpellcastingManager->CastSpellInstant(pOther->GetID(), BludgeonBane3_SpellID);
						pOther->m_SpellcastingManager->CastSpellInstant(pOther->GetID(), PiercingBane3_SpellID);
						pOther->m_SpellcastingManager->CastSpellInstant(pOther->GetID(), FlameBane3_SpellID);
						pOther->m_SpellcastingManager->CastSpellInstant(pOther->GetID(), FrostBane3_SpellID);
						pOther->m_SpellcastingManager->CastSpellInstant(pOther->GetID(), LightningBane3_SpellID);
						pOther->m_SpellcastingManager->CastSpellInstant(pOther->GetID(), AcidBane3_SpellID);
					}
					break;
				case 4:
					m_SpellcastingManager->CastSpellInstant(pOther->GetID(), ArmorOther4_SpellID);
					m_SpellcastingManager->CastSpellInstant(pOther->GetID(), BladeProtectionOther4_SpellID);
					m_SpellcastingManager->CastSpellInstant(pOther->GetID(), PiercingProtectionOther4_SpellID);
					m_SpellcastingManager->CastSpellInstant(pOther->GetID(), BludgeonProtectionOther4_SpellID);
					m_SpellcastingManager->CastSpellInstant(pOther->GetID(), FireProtectionOther4_SpellID);
					m_SpellcastingManager->CastSpellInstant(pOther->GetID(), ColdProtectionOther4_SpellID);
					m_SpellcastingManager->CastSpellInstant(pOther->GetID(), AcidProtectionOther4_SpellID);
					m_SpellcastingManager->CastSpellInstant(pOther->GetID(), LightningProtectionOther4_SpellID);

					m_SpellcastingManager->CastSpellInstant(pOther->GetID(), StrengthOther4_SpellID);
					m_SpellcastingManager->CastSpellInstant(pOther->GetID(), EnduranceOther4_SpellID);
					m_SpellcastingManager->CastSpellInstant(pOther->GetID(), CoordinationOther4_SpellID);
					m_SpellcastingManager->CastSpellInstant(pOther->GetID(), QuicknessOther4_SpellID);
					m_SpellcastingManager->CastSpellInstant(pOther->GetID(), FocusOther4_SpellID);
					m_SpellcastingManager->CastSpellInstant(pOther->GetID(), WillpowerOther4_SpellID);

					m_SpellcastingManager->CastSpellInstant(pOther->GetID(), CreatureEnchantmentMasteryOther4_SpellID);
					m_SpellcastingManager->CastSpellInstant(pOther->GetID(), LifeMagicMasteryOther4_SpellID);
					m_SpellcastingManager->CastSpellInstant(pOther->GetID(), ItemEnchantmentMasteryOther4_SpellID);
					m_SpellcastingManager->CastSpellInstant(pOther->GetID(), WarMagicMasteryOther4_SpellID);
					m_SpellcastingManager->CastSpellInstant(pOther->GetID(), ManaMasteryOther4_SpellID);

					m_SpellcastingManager->CastSpellInstant(pOther->GetID(), SprintOther4_SpellID);
					m_SpellcastingManager->CastSpellInstant(pOther->GetID(), JumpingMasteryOther4_SpellID);
					m_SpellcastingManager->CastSpellInstant(pOther->GetID(), ImpregnabilityOther4_SpellID);
					m_SpellcastingManager->CastSpellInstant(pOther->GetID(), InvulnerabilityOther4_SpellID);
					m_SpellcastingManager->CastSpellInstant(pOther->GetID(), MagicResistanceOther4_SpellID);
					m_SpellcastingManager->CastSpellInstant(pOther->GetID(), FealtyOther4_SpellID);
					m_SpellcastingManager->CastSpellInstant(pOther->GetID(), LeadershipMasteryOther4_SpellID);

					m_SpellcastingManager->CastSpellInstant(pOther->GetID(), SwordMasteryOther4_SpellID);
					m_SpellcastingManager->CastSpellInstant(pOther->GetID(), AxeMasteryOther4_SpellID);
					m_SpellcastingManager->CastSpellInstant(pOther->GetID(), MaceMasteryOther4_SpellID);
					m_SpellcastingManager->CastSpellInstant(pOther->GetID(), SpearMasteryOther4_SpellID);
					m_SpellcastingManager->CastSpellInstant(pOther->GetID(), StaffMasteryOther4_SpellID);
					m_SpellcastingManager->CastSpellInstant(pOther->GetID(), DaggerMasteryOther4_SpellID);
					m_SpellcastingManager->CastSpellInstant(pOther->GetID(), UnarmedCombatMasteryOther4_SpellID);

					m_SpellcastingManager->CastSpellInstant(pOther->GetID(), CrossBowMasteryOther4_SpellID);
					m_SpellcastingManager->CastSpellInstant(pOther->GetID(), BowMasteryOther4_SpellID);
					m_SpellcastingManager->CastSpellInstant(pOther->GetID(), ThrownWeaponMasteryOther4_SpellID);

					pOther->MakeSpellcastingManager();
					pOther->m_SpellcastingManager->CastSpellInstant(pOther->GetID(), BloodDrinker4_SpellID);
					pOther->m_SpellcastingManager->CastSpellInstant(pOther->GetID(), HeartSeeker4_SpellID);
					pOther->m_SpellcastingManager->CastSpellInstant(pOther->GetID(), Defender4_SpellID);
					pOther->m_SpellcastingManager->CastSpellInstant(pOther->GetID(), SwiftKiller4_SpellID);

					if (g_pConfig->TownCrierBuffBanes())
					{
						pOther->m_SpellcastingManager->CastSpellInstant(pOther->GetID(), Impenetrability4_SpellID);
						pOther->m_SpellcastingManager->CastSpellInstant(pOther->GetID(), BladeBane4_SpellID);
						pOther->m_SpellcastingManager->CastSpellInstant(pOther->GetID(), BludgeonBane4_SpellID);
						pOther->m_SpellcastingManager->CastSpellInstant(pOther->GetID(), PiercingBane4_SpellID);
						pOther->m_SpellcastingManager->CastSpellInstant(pOther->GetID(), FlameBane4_SpellID);
						pOther->m_SpellcastingManager->CastSpellInstant(pOther->GetID(), FrostBane4_SpellID);
						pOther->m_SpellcastingManager->CastSpellInstant(pOther->GetID(), LightningBane4_SpellID);
						pOther->m_SpellcastingManager->CastSpellInstant(pOther->GetID(), AcidBane4_SpellID);
					}
					break;
				case 5:
					m_SpellcastingManager->CastSpellInstant(pOther->GetID(), ArmorOther5_SpellID);
					m_SpellcastingManager->CastSpellInstant(pOther->GetID(), BladeProtectionOther5_SpellID);
					m_SpellcastingManager->CastSpellInstant(pOther->GetID(), PiercingProtectionOther5_SpellID);
					m_SpellcastingManager->CastSpellInstant(pOther->GetID(), BludgeonProtectionOther5_SpellID);
					m_SpellcastingManager->CastSpellInstant(pOther->GetID(), FireProtectionOther5_SpellID);
					m_SpellcastingManager->CastSpellInstant(pOther->GetID(), ColdProtectionOther5_SpellID);
					m_SpellcastingManager->CastSpellInstant(pOther->GetID(), AcidProtectionOther5_SpellID);
					m_SpellcastingManager->CastSpellInstant(pOther->GetID(), LightningProtectionOther5_SpellID);

					m_SpellcastingManager->CastSpellInstant(pOther->GetID(), StrengthOther5_SpellID);
					m_SpellcastingManager->CastSpellInstant(pOther->GetID(), EnduranceOther5_SpellID);
					m_SpellcastingManager->CastSpellInstant(pOther->GetID(), CoordinationOther5_SpellID);
					m_SpellcastingManager->CastSpellInstant(pOther->GetID(), QuicknessOther5_SpellID);
					m_SpellcastingManager->CastSpellInstant(pOther->GetID(), FocusOther5_SpellID);
					m_SpellcastingManager->CastSpellInstant(pOther->GetID(), WillpowerOther5_SpellID);

					m_SpellcastingManager->CastSpellInstant(pOther->GetID(), CreatureEnchantmentMasteryOther5_SpellID);
					m_SpellcastingManager->CastSpellInstant(pOther->GetID(), LifeMagicMasteryOther5_SpellID);
					m_SpellcastingManager->CastSpellInstant(pOther->GetID(), ItemEnchantmentMasteryOther5_SpellID);
					m_SpellcastingManager->CastSpellInstant(pOther->GetID(), WarMagicMasteryOther5_SpellID);
					m_SpellcastingManager->CastSpellInstant(pOther->GetID(), ManaMasteryOther5_SpellID);

					m_SpellcastingManager->CastSpellInstant(pOther->GetID(), SprintOther5_SpellID);
					m_SpellcastingManager->CastSpellInstant(pOther->GetID(), JumpingMasteryOther5_SpellID);
					m_SpellcastingManager->CastSpellInstant(pOther->GetID(), ImpregnabilityOther5_SpellID);
					m_SpellcastingManager->CastSpellInstant(pOther->GetID(), InvulnerabilityOther5_SpellID);
					m_SpellcastingManager->CastSpellInstant(pOther->GetID(), MagicResistanceOther5_SpellID);
					m_SpellcastingManager->CastSpellInstant(pOther->GetID(), FealtyOther5_SpellID);
					m_SpellcastingManager->CastSpellInstant(pOther->GetID(), LeadershipMasteryOther5_SpellID);

					m_SpellcastingManager->CastSpellInstant(pOther->GetID(), SwordMasteryOther5_SpellID);
					m_SpellcastingManager->CastSpellInstant(pOther->GetID(), AxeMasteryOther5_SpellID);
					m_SpellcastingManager->CastSpellInstant(pOther->GetID(), MaceMasteryOther5_SpellID);
					m_SpellcastingManager->CastSpellInstant(pOther->GetID(), SpearMasteryOther5_SpellID);
					m_SpellcastingManager->CastSpellInstant(pOther->GetID(), StaffMasteryOther5_SpellID);
					m_SpellcastingManager->CastSpellInstant(pOther->GetID(), DaggerMasteryOther5_SpellID);
					m_SpellcastingManager->CastSpellInstant(pOther->GetID(), UnarmedCombatMasteryOther5_SpellID);

					m_SpellcastingManager->CastSpellInstant(pOther->GetID(), CrossBowMasteryOther5_SpellID);
					m_SpellcastingManager->CastSpellInstant(pOther->GetID(), BowMasteryOther5_SpellID);
					m_SpellcastingManager->CastSpellInstant(pOther->GetID(), ThrownWeaponMasteryOther5_SpellID);

					pOther->MakeSpellcastingManager();
					pOther->m_SpellcastingManager->CastSpellInstant(pOther->GetID(), BloodDrinker5_SpellID);
					pOther->m_SpellcastingManager->CastSpellInstant(pOther->GetID(), HeartSeeker5_SpellID);
					pOther->m_SpellcastingManager->CastSpellInstant(pOther->GetID(), Defender5_SpellID);
					pOther->m_SpellcastingManager->CastSpellInstant(pOther->GetID(), SwiftKiller5_SpellID);

					if (g_pConfig->TownCrierBuffBanes())
					{
						pOther->m_SpellcastingManager->CastSpellInstant(pOther->GetID(), Impenetrability5_SpellID);
						pOther->m_SpellcastingManager->CastSpellInstant(pOther->GetID(), BladeBane5_SpellID);
						pOther->m_SpellcastingManager->CastSpellInstant(pOther->GetID(), BludgeonBane5_SpellID);
						pOther->m_SpellcastingManager->CastSpellInstant(pOther->GetID(), PiercingBane5_SpellID);
						pOther->m_SpellcastingManager->CastSpellInstant(pOther->GetID(), FlameBane5_SpellID);
						pOther->m_SpellcastingManager->CastSpellInstant(pOther->GetID(), FrostBane5_SpellID);
						pOther->m_SpellcastingManager->CastSpellInstant(pOther->GetID(), LightningBane5_SpellID);
						pOther->m_SpellcastingManager->CastSpellInstant(pOther->GetID(), AcidBane5_SpellID);
					}
					break;
				case 6:
					m_SpellcastingManager->CastSpellInstant(pOther->GetID(), ArmorOther6_SpellID);
					m_SpellcastingManager->CastSpellInstant(pOther->GetID(), BladeProtectionOther6_SpellID);
					m_SpellcastingManager->CastSpellInstant(pOther->GetID(), PiercingProtectionOther6_SpellID);
					m_SpellcastingManager->CastSpellInstant(pOther->GetID(), BludgeonProtectionOther6_SpellID);
					m_SpellcastingManager->CastSpellInstant(pOther->GetID(), FireProtectionOther6_SpellID);
					m_SpellcastingManager->CastSpellInstant(pOther->GetID(), ColdProtectionOther6_SpellID);
					m_SpellcastingManager->CastSpellInstant(pOther->GetID(), AcidProtectionOther6_SpellID);
					m_SpellcastingManager->CastSpellInstant(pOther->GetID(), LightningProtectionOther6_SpellID);

					m_SpellcastingManager->CastSpellInstant(pOther->GetID(), StrengthOther6_SpellID);
					m_SpellcastingManager->CastSpellInstant(pOther->GetID(), EnduranceOther6_SpellID);
					m_SpellcastingManager->CastSpellInstant(pOther->GetID(), CoordinationOther6_SpellID);
					m_SpellcastingManager->CastSpellInstant(pOther->GetID(), QuicknessOther6_SpellID);
					m_SpellcastingManager->CastSpellInstant(pOther->GetID(), FocusOther6_SpellID);
					m_SpellcastingManager->CastSpellInstant(pOther->GetID(), WillpowerOther6_SpellID);

					m_SpellcastingManager->CastSpellInstant(pOther->GetID(), CreatureEnchantmentMasteryOther6_SpellID);
					m_SpellcastingManager->CastSpellInstant(pOther->GetID(), LifeMagicMasteryOther6_SpellID);
					m_SpellcastingManager->CastSpellInstant(pOther->GetID(), ItemEnchantmentMasteryOther6_SpellID);
					m_SpellcastingManager->CastSpellInstant(pOther->GetID(), WarMagicMasteryOther6_SpellID);
					m_SpellcastingManager->CastSpellInstant(pOther->GetID(), ManaMasteryOther6_SpellID);

					m_SpellcastingManager->CastSpellInstant(pOther->GetID(), SprintOther6_SpellID);
					m_SpellcastingManager->CastSpellInstant(pOther->GetID(), JumpingMasteryOther6_SpellID);
					m_SpellcastingManager->CastSpellInstant(pOther->GetID(), ImpregnabilityOther6_SpellID);
					m_SpellcastingManager->CastSpellInstant(pOther->GetID(), InvulnerabilityOther6_SpellID);
					m_SpellcastingManager->CastSpellInstant(pOther->GetID(), MagicResistanceOther6_SpellID);
					m_SpellcastingManager->CastSpellInstant(pOther->GetID(), FealtyOther6_SpellID);
					m_SpellcastingManager->CastSpellInstant(pOther->GetID(), LeadershipMasteryOther6_SpellID);

					m_SpellcastingManager->CastSpellInstant(pOther->GetID(), SwordMasteryOther6_SpellID);
					m_SpellcastingManager->CastSpellInstant(pOther->GetID(), AxeMasteryOther6_SpellID);
					m_SpellcastingManager->CastSpellInstant(pOther->GetID(), MaceMasteryOther6_SpellID);
					m_SpellcastingManager->CastSpellInstant(pOther->GetID(), SpearMasteryOther6_SpellID);
					m_SpellcastingManager->CastSpellInstant(pOther->GetID(), StaffMasteryOther6_SpellID);
					m_SpellcastingManager->CastSpellInstant(pOther->GetID(), DaggerMasteryOther6_SpellID);
					m_SpellcastingManager->CastSpellInstant(pOther->GetID(), UnarmedCombatMasteryOther6_SpellID);

					m_SpellcastingManager->CastSpellInstant(pOther->GetID(), CrossBowMasteryOther6_SpellID);
					m_SpellcastingManager->CastSpellInstant(pOther->GetID(), BowMasteryOther6_SpellID);
					m_SpellcastingManager->CastSpellInstant(pOther->GetID(), ThrownWeaponMasteryOther6_SpellID);

					pOther->MakeSpellcastingManager();
					pOther->m_SpellcastingManager->CastSpellInstant(pOther->GetID(), BloodDrinker6_SpellID);
					pOther->m_SpellcastingManager->CastSpellInstant(pOther->GetID(), HeartSeeker6_SpellID);
					pOther->m_SpellcastingManager->CastSpellInstant(pOther->GetID(), Defender6_SpellID);
					pOther->m_SpellcastingManager->CastSpellInstant(pOther->GetID(), SwiftKiller6_SpellID);

					if (g_pConfig->TownCrierBuffBanes())
					{
						pOther->m_SpellcastingManager->CastSpellInstant(pOther->GetID(), Impenetrability6_SpellID);
						pOther->m_SpellcastingManager->CastSpellInstant(pOther->GetID(), BladeBane6_SpellID);
						pOther->m_SpellcastingManager->CastSpellInstant(pOther->GetID(), BludgeonBane6_SpellID);
						pOther->m_SpellcastingManager->CastSpellInstant(pOther->GetID(), PiercingBane6_SpellID);
						pOther->m_SpellcastingManager->CastSpellInstant(pOther->GetID(), FlameBane6_SpellID);
						pOther->m_SpellcastingManager->CastSpellInstant(pOther->GetID(), FrostBane6_SpellID);
						pOther->m_SpellcastingManager->CastSpellInstant(pOther->GetID(), LightningBane6_SpellID);
						pOther->m_SpellcastingManager->CastSpellInstant(pOther->GetID(), AcidBane6_SpellID);
					}
					break;
				case 7:
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

					if (g_pConfig->TownCrierBuffBanes())
					{
						pOther->m_SpellcastingManager->CastSpellInstant(pOther->GetID(), Impenetrability7_SpellID);
						pOther->m_SpellcastingManager->CastSpellInstant(pOther->GetID(), BladeBane7_SpellID);
						pOther->m_SpellcastingManager->CastSpellInstant(pOther->GetID(), BludgeonBane7_SpellID);
						pOther->m_SpellcastingManager->CastSpellInstant(pOther->GetID(), PiercingBane7_SpellID);
						pOther->m_SpellcastingManager->CastSpellInstant(pOther->GetID(), FlameBane7_SpellID);
						pOther->m_SpellcastingManager->CastSpellInstant(pOther->GetID(), FrostBane7_SpellID);
						pOther->m_SpellcastingManager->CastSpellInstant(pOther->GetID(), LightningBane7_SpellID);
						pOther->m_SpellcastingManager->CastSpellInstant(pOther->GetID(), AcidBane7_SpellID);
					}
					break;
			}

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


