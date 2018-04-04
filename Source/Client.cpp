
#include "StdAfx.h"

#include "Client.h"
#include "ClientEvents.h"
#include "Config.h"

// Network access
#include "Network.h"
#include "PacketController.h"
#include "BinaryReader.h"
#include "BinaryWriter.h"

// Database access
#include "Database.h"
#include "Database2.h"
#include "DatabaseIO.h"

#include "WeenieFactory.h"

// World access
#include "World.h"

// Player access
#include "WeenieObject.h"
#include "Monster.h"
#include "Player.h"

// Command access
#include "ClientCommands.h"
#include "ChatMsgs.h"
#include "AllegianceManager.h"

// CClient - for client/server interaction
CClient::CClient(SOCKADDR_IN *peer, WORD slot, AccountInformation_t &accountInfo)
{
	memcpy(&m_vars.addr, peer, sizeof(SOCKADDR_IN));

	m_AccountInfo = accountInfo;

	m_vars.slot = slot;
	m_vars.account = accountInfo.username;
	
	m_pPC = new CPacketController(this);
	m_pEvents = new CClientEvents(this);
}

CClient::~CClient()
{
	SafeDelete(m_pEvents);
	SafeDelete(m_pPC);
}

void CClient::IncomingBlob(BlobPacket_s *blob, double recvTime)
{
	if (!IsAlive())
		return;

	m_pPC->IncomingBlob(blob, recvTime);
}

void CClient::Think()
{
	if (!IsAlive())
		return;

	WorldThink();

	m_pPC->Think();

	// packet control can signal to kill
	if (!m_pPC->IsAlive())
		Kill(__FILE__, __LINE__);
}

void CClient::ThinkOutbound()
{
	if (!IsAlive())
		return;

	if (m_pPC && m_pPC - IsAlive())
	{
		m_pPC->ThinkOutbound();
	}
}

void CClient::WorldThink()
{
	if (!m_vars.bInWorld)
	{
		if (m_vars.bNeedChars)
		{
			UpdateLoginScreen();
			m_vars.bNeedChars = false;
		}
	}

	m_pEvents->Think();
}

const AccountInformation_t &CClient::GetAccountInfo()
{
	return m_AccountInfo;
}

const std::list<CharacterDesc_t> &CClient::GetCharacters()
{
	return m_Characters;
}

bool CClient::HasCharacter(DWORD character_weenie_id)
{
	for (auto &character : m_Characters)
	{
		if (character.weenie_id == character_weenie_id)
			return true;
	}

	return false;
}

DWORD CClient::IncCharacterInstanceTS(DWORD character_weenie_id)
{
	WORD newInstanceTS = 0;

	// just make this work
	m_Characters = g_pDBIO->GetCharacterList(m_AccountInfo.id);

	for (auto &character : m_Characters)
	{
		if (character.weenie_id == character_weenie_id)
		{
			newInstanceTS = character.instance_ts + 1;
			g_pDBIO->SetCharacterInstanceTS(character.weenie_id, newInstanceTS);
		}
	}

	// just make this work
	m_Characters = g_pDBIO->GetCharacterList(m_AccountInfo.id);
	return newInstanceTS;
}

void CClient::UpdateLoginScreen()
{
	m_CharactersSent = m_Characters = g_pDBIO->GetCharacterList(m_AccountInfo.id);

	BinaryWriter CharacterList;
	CharacterList.Write<DWORD>(0xF658);
	CharacterList.Write<DWORD>(0); // check what this is
	CharacterList.Write<DWORD>(m_Characters.size());

	for (auto &character : m_Characters)
	{
		CharacterList.Write<DWORD>(character.weenie_id);
		CharacterList.WriteString(character.name);
		CharacterList.Write<DWORD>(0); // delete period, TODO
	}

	CharacterList.Write<DWORD>(0);
	CharacterList.Write<DWORD>(11); // max characters I'm assuming
	CharacterList.WriteString(m_AccountInfo.username);
	CharacterList.Write<DWORD>(1); // what are these
	CharacterList.Write<DWORD>(1); // what are these
	SendNetMessage(CharacterList.GetData(), CharacterList.GetSize(), PRIVATE_MSG);

	BinaryWriter ServerName;
	ServerName.Write<DWORD>(0xF7E1);
	ServerName.Write<DWORD>(g_pWorld->GetNumPlayers()); // num connections
	ServerName.Write<DWORD>(-1); // Max connections
	ServerName.WriteString(g_pConfig->WorldName());
	SendNetMessage(ServerName.GetData(), ServerName.GetSize(), PRIVATE_MSG);

	BinaryWriter DDD_InterrogationMessage;
	DDD_InterrogationMessage.Write<DWORD>(0xF7E5);
	DDD_InterrogationMessage.Write<DWORD>(1); // servers region
	DDD_InterrogationMessage.Write<DWORD>(1); // name rule language
	DDD_InterrogationMessage.Write<DWORD>(1); // product id
	DDD_InterrogationMessage.Write<DWORD>(2); // supports languages (2)
	DDD_InterrogationMessage.Write<DWORD>(0); // language #1
	DDD_InterrogationMessage.Write<DWORD>(1); // language #2
	SendNetMessage(DDD_InterrogationMessage.GetData(), DDD_InterrogationMessage.GetSize(), EVENT_MSG);
}

void CClient::EnterWorld()
{
	DWORD EnterWorld = 0xF7DF; // 0xF7C7;

	SendNetMessage(&EnterWorld, sizeof(DWORD), 9);
	LOG(Client, Normal, "Client #%u is entering the world.\n", m_vars.slot);

	m_vars.bInWorld = TRUE;

	BinaryWriter setChatChannels;
	setChatChannels.Write<DWORD>(0x295);
	setChatChannels.Write<DWORD>(Allegiance_ChatChannel);
	setChatChannels.Write<DWORD>(General_ChatChannel);
	setChatChannels.Write<DWORD>(Trade_ChatChannel);
	setChatChannels.Write<DWORD>(LFG_ChatChannel);
	setChatChannels.Write<DWORD>(Roleplay_ChatChannel);
	setChatChannels.Write<DWORD>(Olthoi_ChatChannel);
	setChatChannels.Write<DWORD>(Society_ChatChannel);
	setChatChannels.Write<DWORD>(SocietyCelHan_ChatChannel);
	setChatChannels.Write<DWORD>(SocietyEldWeb_ChatChannel);
	setChatChannels.Write<DWORD>(SocietyRadBlo_ChatChannel);
	SendNetMessage(&setChatChannels, PRIVATE_MSG, FALSE, FALSE);
}

void CClient::ExitWorld()
{
	DWORD ExitWorld = 0xF653;
	SendNetMessage(&ExitWorld, sizeof(DWORD), PRIVATE_MSG);
	LOG(Client, Normal, "Client #%u is exiting the world.\n", m_vars.slot);

	m_pPC->ResetEvent();

	UpdateLoginScreen();

	m_vars.bInWorld = FALSE;
}

void CClient::SendNetMessage(BinaryWriter* pMessage, WORD group, BOOL event, BOOL del)
{
	if (!pMessage)
		return;

	SendNetMessage(pMessage->GetData(), pMessage->GetSize(), group, event);

	if (del)
		delete pMessage;
}
void CClient::SendNetMessage(void *data, DWORD length, WORD group, BOOL game_event)
{
	if (!IsAlive())
		return;

	if (!data || !length)
		return;

	if (g_bDebugToggle)
	{
		LOG(Network, Normal, "%.03f Sending response (group %u) to %s:\n", g_pGlobals->Time(), group, (GetEvents() && GetEvents()->GetPlayer()) ? GetEvents()->GetPlayer()->GetName().c_str() : "[unknown]");
		LOG_BYTES(Network, Normal, data, length);
	}

	m_pPC->QueueNetMessage(data, length, group, game_event ? GetEvents()->GetPlayerID() : 0);
}

BOOL CClient::CheckNameValidity(const char *name, int access, std::string &resultName)
{
	resultName = "";

	if (!strncmp(name, "Contributor ", strlen("Contributor ")))
	{
		if (access >= ADVOCATE_ACCESS)
		{
			resultName = "+Contributor ";
			name += strlen("Contributor ");
		}
		else
			return FALSE;
	}

	int len = (int)strlen(name);

	if ((len < 3) || (len > 32))
		return FALSE;

	int i = 0;
	while (i < len)
	{
		char letter = name[i];
		if (!(letter >= 'A' && letter <= 'Z') && !(letter >= 'a' || letter <= 'z') && !(letter == '\'') && !(letter == ' ') && !(letter == '-'))
			break;
		i++;
	}
	if (i == len)
	{
		resultName += name;
		return TRUE;
	}

	return FALSE;
}

void CClient::DeleteCharacter(BinaryReader *pReader)
{
	std::string account_name = pReader->ReadString(); // account
	if (strcmp(account_name.c_str(), m_vars.account.c_str()))
		return;

	int slot = pReader->Read<int>();
	if (slot < 0 || slot >= m_Characters.size() || slot >= m_CharactersSent.size())
		return; // out of range

	auto entry = m_CharactersSent.begin();
	std::advance(entry, slot);

	g_pAllegianceManager->BreakAllAllegiance(entry->weenie_id);
	g_pDBIO->DeleteCharacter(entry->weenie_id);
	g_pDBIO->DeleteWeenie(entry->weenie_id);
	m_CharactersSent.erase(entry);

	BinaryWriter response;
	response.Write<DWORD>(0xF655);
	SendNetMessage(response.GetData(), response.GetSize(), PRIVATE_MSG);
}

void CClient::CreateCharacter(BinaryReader *pReader)
{
	DWORD errorCode = CG_VERIFICATION_RESPONSE_CORRUPT;

	std::string account_name = pReader->ReadString();
	if (strcmp(account_name.c_str(), m_vars.account.c_str()))
		return;

	ACCharGenResult cg;
	cg.UnPack(pReader);

	std::string resultName;

	if (pReader->GetLastError())
		goto BadData;
	
	cg.strength = max(min(cg.strength, 100), 10);
	cg.endurance = max(min(cg.endurance, 100), 10);
	cg.coordination = max(min(cg.coordination, 100), 10);
	cg.quickness = max(min(cg.quickness, 100), 10);
	cg.focus = max(min(cg.focus, 100), 10);
	cg.self = max(min(cg.self, 100), 10);
	
	int totalAttribs = cg.strength + cg.endurance + cg.coordination + cg.quickness + cg.focus + cg.self;
	if (totalAttribs > 330)
	{
		goto BadData;
	}

	//if (cg.heritageGroup <= 0 || cg.heritageGroup >= 14)
	if (cg.heritageGroup <= 0 || cg.heritageGroup >= 4)
	{
		BinaryWriter response;
		response.Write<DWORD>(0xF643);
		response.Write<DWORD>(errorCode);
		SendNetMessage(response.GetData(), response.GetSize(), PRIVATE_MSG);

		BinaryWriter popupString;
		popupString.Write<DWORD>(4);
		popupString.WriteString("Only Aluvian, Gharu'ndim and Sho heritages are supported.");
		SendNetMessage(&popupString, PRIVATE_MSG, FALSE, FALSE);

		return;
		//goto BadData;
	}

	if (cg.gender < 0 || cg.gender >= 3) // allow invalid gender? not sure
	{
		goto BadData;
	}

	SkillTable *pSkillTable = SkillSystem::GetSkillTable();
	int numCreditsUsed = 0;
	cg.numSkills = max(0, min(100, cg.numSkills));

	ACCharGenData *cgd = CachedCharGenData;
	assert(cgd != NULL);

	HeritageGroup_CG *heritageGroup = cgd->mHeritageGroupList.lookup(cg.heritageGroup);
	Sex_CG *scg = NULL;

	for (DWORD i = 0; i < cg.numSkills; i++)
	{
		switch (cg.skillAdvancementClasses[i])
		{
		case SKILL_ADVANCEMENT_CLASS::UNDEF_SKILL_ADVANCEMENT_CLASS:
		case SKILL_ADVANCEMENT_CLASS::UNTRAINED_SKILL_ADVANCEMENT_CLASS:
		case SKILL_ADVANCEMENT_CLASS::TRAINED_SKILL_ADVANCEMENT_CLASS:
		case SKILL_ADVANCEMENT_CLASS::SPECIALIZED_SKILL_ADVANCEMENT_CLASS:
			break;
			
		default:
			goto BadData;
		}

		const SkillBase *pSkillBase = pSkillTable->GetSkillBaseRaw((STypeSkill) i);
		if (pSkillBase != NULL)
		{
			//first we check our heritage specific skill costs.
			bool found = false;
			for (DWORD j = 0; j < heritageGroup->mSkillList.num_used; j++)
			{
				if (heritageGroup->mSkillList.array_data[j].skillNum == i)
				{
					if (cg.skillAdvancementClasses[i] == SKILL_ADVANCEMENT_CLASS::TRAINED_SKILL_ADVANCEMENT_CLASS)
						numCreditsUsed += heritageGroup->mSkillList.array_data[j].normalCost;
					else if (cg.skillAdvancementClasses[i] == SKILL_ADVANCEMENT_CLASS::SPECIALIZED_SKILL_ADVANCEMENT_CLASS)
						numCreditsUsed += heritageGroup->mSkillList.array_data[j].primaryCost;
					found = true;
					break;
				}
			}

			//then if not found check regular skill costs.
			if (!found)
			{
				if (cg.skillAdvancementClasses[i] == SKILL_ADVANCEMENT_CLASS::TRAINED_SKILL_ADVANCEMENT_CLASS)
					numCreditsUsed += pSkillBase->_trained_cost;
				else if (cg.skillAdvancementClasses[i] == SKILL_ADVANCEMENT_CLASS::SPECIALIZED_SKILL_ADVANCEMENT_CLASS)
					numCreditsUsed += pSkillBase->_specialized_cost;
			}
		}
		else
		{
			if (cg.skillAdvancementClasses[i] == SKILL_ADVANCEMENT_CLASS::TRAINED_SKILL_ADVANCEMENT_CLASS ||
				cg.skillAdvancementClasses[i] == SKILL_ADVANCEMENT_CLASS::SPECIALIZED_SKILL_ADVANCEMENT_CLASS)
			{
				goto BadData;
			}
		}
	}

	if (numCreditsUsed > 50)
	{
		goto BadData;
	}

	// need to reformat name here...
	if (!CheckNameValidity(cg.name.c_str(), GetAccessLevel(), resultName))
	{
		errorCode = CG_VERIFICATION_RESPONSE_NAME_BANNED;
		goto BadData;
	}

	// should check variables to make sure everythings within restriction

	// wHairTextures[m_wGender][m_wHairStyle];
	// WORD wHairTextures[2][4] = { { 0x10B8, 0x10B8, 0x10B8, 0x10B7 }, { 0x11FD, 0x11FD, 0x11FD, 0x10B7 } };

	// group 4, 0x0000F643, 0x00000001, <guid>, <char name[str]>, 0x00000000
	{
		// check if name exists
		if (g_pDBIO->IsCharacterNameOpen(resultName.c_str()))
		{
			const int MIN_PLAYER_GUID = 0x50000000;
			const int MAX_PLAYER_GUID = 0x6FFFFFFF;

			unsigned int newCharacterGUID = g_pDBIO->GetHighestWeenieID(MIN_PLAYER_GUID, MAX_PLAYER_GUID) + 1;

			if (g_pDBIO->CreateCharacter(m_AccountInfo.id, newCharacterGUID, resultName.c_str()))
			{
				m_Characters = g_pDBIO->GetCharacterList(m_AccountInfo.id);
				
				// start pReader Zaikhil
				Position startPos;
				// startPos = Position(0x7D64001D, Vector(74.748657f, 97.934601f, 12.004999f), Quaternion(0.926898f, 0.000000f, 0.000000f, -0.375314f)); // zaikhil
				// startPos = Position(0x9722003A, Vector(168.354004f, 24.618000f, 102.005005f), Quaternion(-0.922790f, 0.000000f, 0.000000f, -0.385302f));
				startPos = Position(0xD3380005, Vector(5.500000f, 109.800003f, 168.463333f), Quaternion(1.000000f, 0.000000f, 0.000000f, 0.000000f));

				// WCID "1" is always a player weenie
				CWeenieObject *weenie = g_pWeenieFactory->CreateWeenieByClassID(1, &startPos, false);

				if(!weenie)
					goto BadData;

				weenie->SetID(newCharacterGUID);

				// alter parameters for character creation here
				weenie->SetName(resultName.c_str());

				weenie->m_Qualities.SetInt(HERITAGE_GROUP_INT, cg.heritageGroup);
				weenie->m_Qualities.SetInt(GENDER_INT, cg.gender);
				weenie->m_Qualities.SetAttribute(STRENGTH_ATTRIBUTE, cg.strength);
				weenie->m_Qualities.SetAttribute(ENDURANCE_ATTRIBUTE, cg.endurance);
				weenie->m_Qualities.SetAttribute(COORDINATION_ATTRIBUTE, cg.coordination);
				weenie->m_Qualities.SetAttribute(QUICKNESS_ATTRIBUTE, cg.quickness);
				weenie->m_Qualities.SetAttribute(FOCUS_ATTRIBUTE, cg.focus);
				weenie->m_Qualities.SetAttribute(SELF_ATTRIBUTE, cg.self);

				if (heritageGroup)
				{
					weenie->m_Qualities.SetString(HERITAGE_GROUP_STRING, heritageGroup->name.c_str());

					if (cg.templateNum >= 0 && cg.templateNum < heritageGroup->mTemplateList.num_used)
						weenie->m_Qualities.SetString(TEMPLATE_STRING, heritageGroup->mTemplateList.array_data[cg.templateNum].name.c_str());

					scg = heritageGroup->mGenderList.lookup(cg.gender);
					if (scg)
					{
						weenie->m_Qualities.SetDataID(PALETTE_BASE_DID, scg->basePalette);
						weenie->m_Qualities.SetDataID(ICON_DID, scg->iconImage);
						weenie->m_Qualities.SetDataID(SOUND_TABLE_DID, scg->soundTable);
						weenie->m_Qualities.SetDataID(MOTION_TABLE_DID, scg->motionTable);
						weenie->m_Qualities.SetDataID(COMBAT_TABLE_DID, scg->combatTable);
						weenie->m_Qualities.SetDataID(PHYSICS_EFFECT_TABLE_DID, scg->physicsTable);
						weenie->m_Qualities.SetDataID(SETUP_DID, scg->setup);
						//weenie->m_ObjDesc = scg->objDesc;
						//weenie->m_ObjDesc.paletteID = scg->basePalette;

						cg.hairStyle = max(0, min(scg->mHairStyleList.num_used - 1, cg.hairStyle));
						HairStyle_CG *hairStyle = &scg->mHairStyleList.array_data[cg.hairStyle];
						//weenie->m_ObjDesc += hairStyle->objDesc;

						if (hairStyle->objDesc.firstAPChange)
							weenie->m_Qualities.SetDataID(HEAD_OBJECT_DID, hairStyle->objDesc.firstAPChange->part_id);

						if (cg.eyesStrip != -1)
						{
							cg.eyesStrip = max(0, min(scg->mEyeStripList.num_used - 1, cg.eyesStrip));
							EyesStrip_CG *eyesStrip = &scg->mEyeStripList.array_data[cg.eyesStrip];
							if (eyesStrip)
							{
								if (hairStyle->bald)
								{
									//weenie->m_ObjDesc += eyesStrip->objDesc_Bald;
									weenie->m_Qualities.SetDataID(DEFAULT_EYES_TEXTURE_DID, eyesStrip->objDesc_Bald.firstTMChange->old_tex_id);
									weenie->m_Qualities.SetDataID(EYES_TEXTURE_DID, eyesStrip->objDesc_Bald.firstTMChange->new_tex_id);
								}
								else
								{
									//weenie->m_ObjDesc += eyesStrip->objDesc;
									weenie->m_Qualities.SetDataID(DEFAULT_EYES_TEXTURE_DID, eyesStrip->objDesc.firstTMChange->old_tex_id);
									weenie->m_Qualities.SetDataID(EYES_TEXTURE_DID, eyesStrip->objDesc.firstTMChange->new_tex_id);
								}
							}
						}
						if (cg.noseStrip != -1)
						{
							cg.noseStrip = max(0, min(scg->mNoseStripList.num_used - 1, cg.noseStrip));
							FaceStrip_CG *faceStrip = &scg->mNoseStripList.array_data[cg.noseStrip];
							if (faceStrip)
							{
								//weenie->m_ObjDesc += faceStrip->objDesc;
								weenie->m_Qualities.SetDataID(DEFAULT_NOSE_TEXTURE_DID, faceStrip->objDesc.firstTMChange->old_tex_id);
								weenie->m_Qualities.SetDataID(NOSE_TEXTURE_DID, faceStrip->objDesc.firstTMChange->new_tex_id);
							}
						}
						if (cg.mouthStrip != -1)
						{
							cg.mouthStrip = max(0, min(scg->mMouthStripList.num_used - 1, cg.mouthStrip));
							FaceStrip_CG *faceStrip = &scg->mMouthStripList.array_data[cg.mouthStrip];
							if (faceStrip)
							{
								// weenie->m_ObjDesc += faceStrip->objDesc;
								weenie->m_Qualities.SetDataID(DEFAULT_MOUTH_TEXTURE_DID, faceStrip->objDesc.firstTMChange->old_tex_id);
								weenie->m_Qualities.SetDataID(MOUTH_TEXTURE_DID, faceStrip->objDesc.firstTMChange->new_tex_id);
							}
						}
						
						PalSet *ps;
						if (ps = PalSet::Get(scg->skinPalSet))
						{
							DWORD skinPalette = ps->GetPaletteID(cg.skinShade);
							weenie->m_Qualities.SetDataID(SKIN_PALETTE_DID, skinPalette);
							//weenie->m_ObjDesc.AddSubpalette(new Subpalette(skinPalette, 0 << 3, 0x18 << 3));
							PalSet::Release(ps);
						}

						cg.hairColor = max(0, min(scg->mHairColorList.num_used - 1, cg.hairColor));
						if (ps = PalSet::Get(scg->mHairColorList.array_data[cg.hairColor]))
						{
							DWORD hairPalette = ps->GetPaletteID(cg.hairShade);
							weenie->m_Qualities.SetDataID(HAIR_PALETTE_DID, hairPalette);
							//weenie->m_ObjDesc.AddSubpalette(new Subpalette(hairPalette, 0x18 << 3, 0x8 << 3));
							PalSet::Release(ps);
						}					
												
						if (cg.eyeColor != -1)
						{
							cg.eyeColor = max(0, min(scg->mEyeColorList.num_used - 1, cg.eyeColor));
							DWORD eyesPalette = scg->mEyeColorList.array_data[cg.eyeColor];
							weenie->m_Qualities.SetDataID(EYES_PALETTE_DID, eyesPalette);
							//weenie->m_ObjDesc.AddSubpalette(new Subpalette(eyesPalette, 0x20 << 3, 0x8 << 3));
						}

						/*
						EYES_TEXTURE_DID,
						NOSE_TEXTURE_DID,
						MOUTH_TEXTURE_DID,
						DEFAULT_EYES_TEXTURE_DID,
						DEFAULT_NOSE_TEXTURE_DID,
						DEFAULT_MOUTH_TEXTURE_DID,
						HAIR_PALETTE_DID,
						EYES_PALETTE_DID,
						SKIN_PALETTE_DID,
						HEAD_OBJECT_DID,
						*/
					}
				}

				for (DWORD i = 0; i < cg.numSkills; i++)
				{
					weenie->m_Qualities.SetSkillAdvancementClass((STypeSkill)i, cg.skillAdvancementClasses[i]);
					if (cg.skillAdvancementClasses[i] == SKILL_ADVANCEMENT_CLASS::SPECIALIZED_SKILL_ADVANCEMENT_CLASS)
						weenie->m_Qualities.SetSkillLevel((STypeSkill)i, 10);
					else if (cg.skillAdvancementClasses[i] == SKILL_ADVANCEMENT_CLASS::TRAINED_SKILL_ADVANCEMENT_CLASS)
						weenie->m_Qualities.SetSkillLevel((STypeSkill)i, 5);
				}
				
				weenie->m_Qualities.SetInt(AGE_INT, 0);

				/*
				cg.startArea = max(0, min(cgd->mStartAreaList.num_used - 1, cg.startArea));

				assert(cgd->mStartAreaList.array_data[cg.startArea].mPositionList.num_used >= 1);

				startPos = cgd->mStartAreaList.array_data[cg.startArea].mPositionList.array_data[0];
				weenie->SetInitialPosition(startPos);
				*/
				
				startPos = Position(0xA9B00006, Vector(24.258204f, 123.777000f, 63.060749f), Quaternion(1, 0, 0, 0)); // holtburg

				switch (cg.startArea)
				{
				default:
				case 0:
					// Holtburg
					startPos = Position(g_pConfig->HoltburgStartPosition());

					if (!startPos.objcell_id)
					{
						startPos = Position(0xA9B00015, Vector(53.780079f, 105.814995f, 64.005005f), Quaternion(-0.933116f, 0, 0, -0.359575f));
						//startPos = Position(0xA9B00006, Vector(24.258204f, 123.777000f, 63.060749f), Quaternion(1, 0, 0, 0));
					}

					break;

				case 1:
					// Shoushi
					startPos = Position(g_pConfig->ShoushiStartPosition());

					if (!startPos.objcell_id)
					{
						startPos = Position(0xDE51001D, Vector(87.350517f, 114.857246f, 16.004999f), Quaternion(0.843159f, 0.0, 0.0, 0.537665f));
						//startPos = Position(0xDE51000C, Vector(26.712753f, 89.279999f, 17.778936f), Quaternion(0.931082f, 0.0, 0.0, -0.364811f));
					}

					break;

				case 2:
					// Yaraq
					startPos = Position(g_pConfig->YaraqStartPosition());

					if (!startPos.objcell_id)
					{
						startPos = Position(0x7D680012, Vector(56.949219f, 29.860384f, 14.981730f), Quaternion(0.475755f, 0.0, 0.0, -0.879578f));
						//startPos = Position(0x7D680019, Vector(79.102280f, 19.573767f, 12.821287f), Quaternion(-0.656578f, 0.0, 0.0, 0.754258f));
					}

					break;

				case 3:
					// Sanamar
					startPos = Position(g_pConfig->SanamarStartPosition());

					if (!startPos.objcell_id)
					{
						//redirect to Holtburg as Sanamar is post-ToD and so incomplete.
						startPos = Position(0xA9B00015, Vector(53.780079f, 105.814995f, 64.005005f), Quaternion(-0.933116f, 0, 0, -0.359575f));
						//startPos = Position(0xA9B00006, Vector(24.258204f, 123.777000f, 63.060749f), Quaternion(1, 0, 0, 0));
					}

					break;
				}

				weenie->SetInitialPosition(startPos);
				weenie->m_Qualities.SetPosition(SANCTUARY_POSITION, startPos);
				weenie->m_Qualities.SetInt(AVAILABLE_SKILL_CREDITS_INT, 50 - max(0, min(50, numCreditsUsed)));
				weenie->m_Qualities.SetInt(CONTAINERS_CAPACITY_INT, 7);

				weenie->SetMaxVitals(false);

				weenie->m_Qualities._create_list->clear(); //Clear the create list as we don't want what's in it.
				g_pWorld->CreateEntity(weenie); //Briefly add the weenie to the world so we don't get errors when adding the starting gear.

				weenie->GiveXP(g_pConfig->OverrideStartingXP()); //Add additional XP Override at character creation 

				//add starter gear
				GenerateStarterGear(weenie, cg, scg);

				weenie->Save();

				g_pWorld->RemoveEntity(weenie);
				//delete weenie; //RemoveEntity already performs the deletion.

				BinaryWriter response;
				response.Write<DWORD>(0xF643);
				response.Write<DWORD>(1);
				response.Write<DWORD>(newCharacterGUID);
				response.WriteString(cg.name.c_str());
				response.Write<DWORD>(0);
				SendNetMessage(response.GetData(), response.GetSize(), PRIVATE_MSG);
			}
			else
			{
				LOG(Client, Error, "Failed to create character.\n");
				BinaryWriter response;
				response.Write<DWORD>(0xF643);
				response.Write<DWORD>(CG_VERIFICATION_RESPONSE_DATABASE_DOWN); // update this error number
				SendNetMessage(response.GetData(), response.GetSize(), PRIVATE_MSG);
			}
		}
		else
		{		
			LOG(Client, Normal, "Character name already exists.\n");
			BinaryWriter response;
			response.Write<DWORD>(0xF643);
			response.Write<DWORD>(CG_VERIFICATION_RESPONSE_NAME_IN_USE); // name already exists
			SendNetMessage(response.GetData(), response.GetSize(), PRIVATE_MSG);
		}

	}
	return;

BadData:
	{
		BinaryWriter response;
		response.Write<DWORD>(0xF643);
		response.Write<DWORD>(errorCode);
		SendNetMessage(response.GetData(), response.GetSize(), PRIVATE_MSG);
	}
}

void CClient::GenerateStarterGear(CWeenieObject *weenieObject, ACCharGenResult cg, Sex_CG *scg)
{
	CMonsterWeenie *weenie = weenieObject->AsMonster();
	if (weenie == NULL)
		return;

	if (cg.headgearStyle != UINT_MAX)
		weenie->SpawnWielded(cg.headgearStyle, scg->mHeadgearList, cg.headgearColor, scg->mClothingColorsList, cg.headgearShade);	
	weenie->SpawnWielded(cg.shirtStyle, scg->mShirtList, cg.shirtColor, scg->mClothingColorsList, cg.shirtShade);
	weenie->SpawnWielded(cg.trousersStyle, scg->mPantsList, cg.trousersColor, scg->mClothingColorsList, cg.trousersShade);
	weenie->SpawnWielded(cg.footwearStyle, scg->mFootwearList, cg.footwearColor, scg->mClothingColorsList, cg.footwearShade);

	//weenie->m_Qualities.SetInt(COIN_VALUE_INT, GetAccessLevel() >= ADMIN_ACCESS ? 500000000 : 10000);
	weenie->SpawnInContainer(W_COINSTACK_CLASS, 500);
	weenie->SpawnInContainer(W_TUTORIALBOOK_CLASS, 1);

	if (cg.skillAdvancementClasses[ALCHEMY_SKILL] >= SKILL_ADVANCEMENT_CLASS::TRAINED_SKILL_ADVANCEMENT_CLASS)
		weenie->SpawnInContainer(W_MORTARANDPESTLE_CLASS, 1);
	if (cg.skillAdvancementClasses[ALCHEMY_SKILL] >= SKILL_ADVANCEMENT_CLASS::SPECIALIZED_SKILL_ADVANCEMENT_CLASS)
		weenie->SpawnInContainer(W_GEMAZURITE_CLASS, 3);

	if (cg.skillAdvancementClasses[COOKING_SKILL] >= SKILL_ADVANCEMENT_CLASS::TRAINED_SKILL_ADVANCEMENT_CLASS)
	{
		weenie->SpawnInContainer(W_FLOUR_CLASS, 6);
		weenie->SpawnInContainer(W_WATER_CLASS, 6);
	}

	if (cg.skillAdvancementClasses[FLETCHING_SKILL] >= SKILL_ADVANCEMENT_CLASS::TRAINED_SKILL_ADVANCEMENT_CLASS)
	{
		weenie->SpawnInContainer(W_ARROWHEAD_CLASS, 30);
		weenie->SpawnInContainer(W_ARROWSHAFT_CLASS, 30);
	}

	if (cg.skillAdvancementClasses[HEALING_SKILL] >= SKILL_ADVANCEMENT_CLASS::TRAINED_SKILL_ADVANCEMENT_CLASS)
		weenie->SpawnInContainer(W_HEALINGKITCRUDE_CLASS, 1);
	if (cg.skillAdvancementClasses[HEALING_SKILL] >= SKILL_ADVANCEMENT_CLASS::SPECIALIZED_SKILL_ADVANCEMENT_CLASS)
		weenie->SpawnInContainer(W_HEALINGKITCRUDE_CLASS, 1);

	if (cg.skillAdvancementClasses[LOCKPICK_SKILL] >= SKILL_ADVANCEMENT_CLASS::TRAINED_SKILL_ADVANCEMENT_CLASS)
		weenie->SpawnInContainer(W_LOCKPICKCRUDE_CLASS, 1);

	if (cg.skillAdvancementClasses[ITEM_APPRAISAL_SKILL] >= SKILL_ADVANCEMENT_CLASS::TRAINED_SKILL_ADVANCEMENT_CLASS ||
		cg.skillAdvancementClasses[ARMOR_APPRAISAL_SKILL] >= SKILL_ADVANCEMENT_CLASS::TRAINED_SKILL_ADVANCEMENT_CLASS ||
		cg.skillAdvancementClasses[WEAPON_APPRAISAL_SKILL] >= SKILL_ADVANCEMENT_CLASS::TRAINED_SKILL_ADVANCEMENT_CLASS ||
		cg.skillAdvancementClasses[MAGIC_ITEM_APPRAISAL_SKILL] >= SKILL_ADVANCEMENT_CLASS::TRAINED_SKILL_ADVANCEMENT_CLASS)
	{
		weenie->SpawnInContainer(W_TINKERINGTOOL_CLASS, 1);
	}

	switch (cg.heritageGroup)
	{
	case Default:
	case Aluvian_HeritageGroup:
		weenie->SpawnInContainer(W_BREAD_CLASS, 1);
		if (cg.skillAdvancementClasses[AXE_SKILL] >= SKILL_ADVANCEMENT_CLASS::TRAINED_SKILL_ADVANCEMENT_CLASS)
			weenie->SpawnInContainer(W_NEWBIEAXEHAND_CLASS, 1);
		if (cg.skillAdvancementClasses[BOW_SKILL] >= SKILL_ADVANCEMENT_CLASS::TRAINED_SKILL_ADVANCEMENT_CLASS)
		{
			weenie->SpawnInContainer(W_NEWBIEBOWSHORT_CLASS, 1);
			weenie->SpawnInContainer(W_ARROW_CLASS, 30);
		}
		if (cg.skillAdvancementClasses[CROSSBOW_SKILL] >= SKILL_ADVANCEMENT_CLASS::TRAINED_SKILL_ADVANCEMENT_CLASS)
		{
			weenie->SpawnInContainer(W_NEWBIECROSSBOW_CLASS, 1);
			weenie->SpawnInContainer(W_BOLT_CLASS, 30);
		}
		if (cg.skillAdvancementClasses[DAGGER_SKILL] >= SKILL_ADVANCEMENT_CLASS::TRAINED_SKILL_ADVANCEMENT_CLASS)
			weenie->SpawnInContainer(W_NEWBIEKNIFE_CLASS, 1);
		if (cg.skillAdvancementClasses[MACE_SKILL] >= SKILL_ADVANCEMENT_CLASS::TRAINED_SKILL_ADVANCEMENT_CLASS)
			weenie->SpawnInContainer(W_NEWBIECLUB_CLASS, 1);
		if (cg.skillAdvancementClasses[SPEAR_SKILL] >= SKILL_ADVANCEMENT_CLASS::TRAINED_SKILL_ADVANCEMENT_CLASS)
			weenie->SpawnInContainer(W_NEWBIESPEAR_CLASS, 1);
		if (cg.skillAdvancementClasses[STAFF_SKILL] >= SKILL_ADVANCEMENT_CLASS::TRAINED_SKILL_ADVANCEMENT_CLASS)
			weenie->SpawnInContainer(W_NEWBIEQUARTERSTAFF_CLASS, 1);
		if (cg.skillAdvancementClasses[SWORD_SKILL] >= SKILL_ADVANCEMENT_CLASS::TRAINED_SKILL_ADVANCEMENT_CLASS)
			weenie->SpawnInContainer(W_NEWBIESWORDSHORT_CLASS, 1);
		if (cg.skillAdvancementClasses[THROWN_WEAPON_SKILL] >= SKILL_ADVANCEMENT_CLASS::TRAINED_SKILL_ADVANCEMENT_CLASS)
			weenie->SpawnInContainer(W_NEWBIEDART_CLASS, 15);

		if (cg.skillAdvancementClasses[CREATURE_ENCHANTMENT_SKILL] >= SKILL_ADVANCEMENT_CLASS::TRAINED_SKILL_ADVANCEMENT_CLASS ||
			cg.skillAdvancementClasses[ITEM_ENCHANTMENT_SKILL] >= SKILL_ADVANCEMENT_CLASS::TRAINED_SKILL_ADVANCEMENT_CLASS ||
			cg.skillAdvancementClasses[LIFE_MAGIC_SKILL] >= SKILL_ADVANCEMENT_CLASS::TRAINED_SKILL_ADVANCEMENT_CLASS ||
			cg.skillAdvancementClasses[WAR_MAGIC_SKILL] >= SKILL_ADVANCEMENT_CLASS::TRAINED_SKILL_ADVANCEMENT_CLASS)
			weenie->SpawnInContainer(W_NEWBIEWANDALUVIAN_CLASS, 1);

		break;
	case Gharundim_HeritageGroup:
		weenie->SpawnInContainer(W_CHEESE_CLASS, 1);
		if (cg.skillAdvancementClasses[AXE_SKILL] >= SKILL_ADVANCEMENT_CLASS::TRAINED_SKILL_ADVANCEMENT_CLASS)
			weenie->SpawnInContainer(W_NEWBIETUNGI_CLASS, 1);
		if (cg.skillAdvancementClasses[BOW_SKILL] >= SKILL_ADVANCEMENT_CLASS::TRAINED_SKILL_ADVANCEMENT_CLASS)
		{
			weenie->SpawnInContainer(W_NEWBIEYAG_CLASS, 1);
			weenie->SpawnInContainer(W_BOLT_CLASS, 30);
		}
		if (cg.skillAdvancementClasses[CROSSBOW_SKILL] >= SKILL_ADVANCEMENT_CLASS::TRAINED_SKILL_ADVANCEMENT_CLASS)
		{
			weenie->SpawnInContainer(W_NEWBIECROSSBOW_CLASS, 1);
			weenie->SpawnInContainer(W_ARROW_CLASS, 30);
		}
		if (cg.skillAdvancementClasses[DAGGER_SKILL] >= SKILL_ADVANCEMENT_CLASS::TRAINED_SKILL_ADVANCEMENT_CLASS)
			weenie->SpawnInContainer(W_JAMBIYA_CLASS, 1);
		if (cg.skillAdvancementClasses[MACE_SKILL] >= SKILL_ADVANCEMENT_CLASS::TRAINED_SKILL_ADVANCEMENT_CLASS)
			weenie->SpawnInContainer(W_NEWBIEKASRULLAH_CLASS, 1);
		if (cg.skillAdvancementClasses[SPEAR_SKILL] >= SKILL_ADVANCEMENT_CLASS::TRAINED_SKILL_ADVANCEMENT_CLASS)
			weenie->SpawnInContainer(W_NEWBIEBUDIAQ_CLASS, 1);
		if (cg.skillAdvancementClasses[STAFF_SKILL] >= SKILL_ADVANCEMENT_CLASS::TRAINED_SKILL_ADVANCEMENT_CLASS)
			weenie->SpawnInContainer(W_NEWBIENABUT_CLASS, 1);
		if (cg.skillAdvancementClasses[SWORD_SKILL] >= SKILL_ADVANCEMENT_CLASS::TRAINED_SKILL_ADVANCEMENT_CLASS)
			weenie->SpawnInContainer(W_NEWBIESIMI_CLASS, 1);
		if (cg.skillAdvancementClasses[THROWN_WEAPON_SKILL] >= SKILL_ADVANCEMENT_CLASS::TRAINED_SKILL_ADVANCEMENT_CLASS)
			weenie->SpawnInContainer(W_NEWBIEDART_CLASS, 15);

		if (cg.skillAdvancementClasses[CREATURE_ENCHANTMENT_SKILL] >= SKILL_ADVANCEMENT_CLASS::TRAINED_SKILL_ADVANCEMENT_CLASS ||
			cg.skillAdvancementClasses[ITEM_ENCHANTMENT_SKILL] >= SKILL_ADVANCEMENT_CLASS::TRAINED_SKILL_ADVANCEMENT_CLASS ||
			cg.skillAdvancementClasses[LIFE_MAGIC_SKILL] >= SKILL_ADVANCEMENT_CLASS::TRAINED_SKILL_ADVANCEMENT_CLASS ||
			cg.skillAdvancementClasses[WAR_MAGIC_SKILL] >= SKILL_ADVANCEMENT_CLASS::TRAINED_SKILL_ADVANCEMENT_CLASS)
			weenie->SpawnInContainer(W_NEWBIEWANDGHARUNDIM_CLASS, 1);

		break;
	case Sho_HeritageGroup:
		weenie->SpawnInContainer(W_APPLE_CLASS, 1);
		if (cg.skillAdvancementClasses[AXE_SKILL] >= SKILL_ADVANCEMENT_CLASS::TRAINED_SKILL_ADVANCEMENT_CLASS)
			weenie->SpawnInContainer(W_NEWBIESHOUONO_CLASS, 1);
		if (cg.skillAdvancementClasses[BOW_SKILL] >= SKILL_ADVANCEMENT_CLASS::TRAINED_SKILL_ADVANCEMENT_CLASS)
		{
			weenie->SpawnInContainer(W_NEWBIESHOUYUMI_CLASS, 1);
			weenie->SpawnInContainer(W_ARROW_CLASS, 30);
		}
		if (cg.skillAdvancementClasses[CROSSBOW_SKILL] >= SKILL_ADVANCEMENT_CLASS::TRAINED_SKILL_ADVANCEMENT_CLASS)
		{
			weenie->SpawnInContainer(W_NEWBIECROSSBOW_CLASS, 1);
			weenie->SpawnInContainer(W_BOLT_CLASS, 30);
		}
		if (cg.skillAdvancementClasses[DAGGER_SKILL] >= SKILL_ADVANCEMENT_CLASS::TRAINED_SKILL_ADVANCEMENT_CLASS)
			weenie->SpawnInContainer(W_NEWBIEKNIFE_CLASS, 1);
		if (cg.skillAdvancementClasses[MACE_SKILL] >= SKILL_ADVANCEMENT_CLASS::TRAINED_SKILL_ADVANCEMENT_CLASS)
			weenie->SpawnInContainer(W_NEWBIEJITTE_CLASS, 1);
		if (cg.skillAdvancementClasses[SPEAR_SKILL] >= SKILL_ADVANCEMENT_CLASS::TRAINED_SKILL_ADVANCEMENT_CLASS)
			weenie->SpawnInContainer(W_NEWBIEYARI_CLASS, 1);
		if (cg.skillAdvancementClasses[STAFF_SKILL] >= SKILL_ADVANCEMENT_CLASS::TRAINED_SKILL_ADVANCEMENT_CLASS)
			weenie->SpawnInContainer(W_NEWBIEJO_CLASS, 1);
		if (cg.skillAdvancementClasses[SWORD_SKILL] >= SKILL_ADVANCEMENT_CLASS::TRAINED_SKILL_ADVANCEMENT_CLASS)
			weenie->SpawnInContainer(W_NEWBIEYAOJI_CLASS, 1);
		if (cg.skillAdvancementClasses[THROWN_WEAPON_SKILL] >= SKILL_ADVANCEMENT_CLASS::TRAINED_SKILL_ADVANCEMENT_CLASS)
			weenie->SpawnInContainer(W_NEWBIESHURIKEN_CLASS, 15);

		if (cg.skillAdvancementClasses[CREATURE_ENCHANTMENT_SKILL] >= SKILL_ADVANCEMENT_CLASS::TRAINED_SKILL_ADVANCEMENT_CLASS ||
			cg.skillAdvancementClasses[ITEM_ENCHANTMENT_SKILL] >= SKILL_ADVANCEMENT_CLASS::TRAINED_SKILL_ADVANCEMENT_CLASS ||
			cg.skillAdvancementClasses[LIFE_MAGIC_SKILL] >= SKILL_ADVANCEMENT_CLASS::TRAINED_SKILL_ADVANCEMENT_CLASS ||
			cg.skillAdvancementClasses[WAR_MAGIC_SKILL] >= SKILL_ADVANCEMENT_CLASS::TRAINED_SKILL_ADVANCEMENT_CLASS)
			weenie->SpawnInContainer(W_NEWBIEWANDSHO_CLASS, 1);

		break;
	}

	weenie->SpawnInContainer(W_SACK_CLASS, 1);
	weenie->SpawnInContainer(W_CALLINGSTONE_CLASS, 1);

	CContainerWeenie *sack = NULL;
	for (auto pack : weenie->m_Packs)
	{
		if (pack->AsContainer())
		{
			sack = pack->AsContainer();
			break;
		}
	}

	if (sack == NULL)
		sack = weenie;

	if (cg.skillAdvancementClasses[CREATURE_ENCHANTMENT_SKILL] >= SKILL_ADVANCEMENT_CLASS::TRAINED_SKILL_ADVANCEMENT_CLASS)
	{
		sack->SpawnInContainer(W_SCARABLEAD_CLASS, 3);
		sack->SpawnInContainer(W_HYSSOP_CLASS, 10);
		sack->SpawnInContainer(W_ALCHEMREALGAR_CLASS, 10);

		weenie->m_Qualities.AddSpell(StrengthOther1_SpellID);
		sack->SpawnInContainer(W_MOONSTONE_CLASS, 10);
		sack->SpawnInContainer(W_POPLARTALISMAN_CLASS, 3);

		weenie->m_Qualities.AddSpell(InvulnerabilitySelf1_SpellID);
		sack->SpawnInContainer(W_HEMATITE_CLASS, 10);
		sack->SpawnInContainer(W_ROWANTALISMAN_CLASS, 3);
	}
	if (cg.skillAdvancementClasses[CREATURE_ENCHANTMENT_SKILL] >= SKILL_ADVANCEMENT_CLASS::SPECIALIZED_SKILL_ADVANCEMENT_CLASS)
	{
		weenie->m_Qualities.AddSpell(ArcaneEnlightenmentSelf1_SpellID);
		sack->SpawnInContainer(W_ALCHEMCADMIA_CLASS, 10);
		sack->SpawnInContainer(W_AGATE_CLASS, 10);
	}

	if (cg.skillAdvancementClasses[ITEM_ENCHANTMENT_SKILL] >= SKILL_ADVANCEMENT_CLASS::TRAINED_SKILL_ADVANCEMENT_CLASS)
	{
		sack->SpawnInContainer(W_SCARABLEAD_CLASS, 3);
		sack->SpawnInContainer(W_HYSSOP_CLASS, 10);
		sack->SpawnInContainer(W_ASHWOODTALISMAN_CLASS, 3);

		weenie->m_Qualities.AddSpell(BloodDrinker1_SpellID);
		sack->SpawnInContainer(W_ALCHEMQUICKSILVER_CLASS, 10);
		sack->SpawnInContainer(W_TURQUOISE_CLASS, 10);

		weenie->m_Qualities.AddSpell(Impenetrability1_SpellID);
		sack->SpawnInContainer(W_ALCHEMCOBALT_CLASS, 10);
		sack->SpawnInContainer(W_ONYX_CLASS, 10);
	}
	if (cg.skillAdvancementClasses[ITEM_ENCHANTMENT_SKILL] >= SKILL_ADVANCEMENT_CLASS::SPECIALIZED_SKILL_ADVANCEMENT_CLASS)
	{
		weenie->m_Qualities.AddSpell(SwiftKiller1_SpellID);
		sack->SpawnInContainer(W_ALCHEMSTIBNITE_CLASS, 10);
	}

	if (cg.skillAdvancementClasses[LIFE_MAGIC_SKILL] >= SKILL_ADVANCEMENT_CLASS::TRAINED_SKILL_ADVANCEMENT_CLASS)
	{
		sack->SpawnInContainer(W_SCARABLEAD_CLASS, 3);

		weenie->m_Qualities.AddSpell(ArmorOther1_SpellID);
		sack->SpawnInContainer(W_WORMWOOD_CLASS, 10);
		sack->SpawnInContainer(W_ONYX_CLASS, 10);
		sack->SpawnInContainer(W_ALCHEMCOBALT_CLASS, 10);
		sack->SpawnInContainer(W_EBONYTALISMAN_CLASS, 3);

		weenie->m_Qualities.AddSpell(HealSelf1_SpellID);
		sack->SpawnInContainer(W_HYSSOP_CLASS, 10);
		sack->SpawnInContainer(W_AMBER_CLASS, 10);
		sack->SpawnInContainer(W_ALCHEMCOLCOTHAR_CLASS, 10);
		sack->SpawnInContainer(W_WILLOWTALISMAN_CLASS, 3);
	}
	if (cg.skillAdvancementClasses[LIFE_MAGIC_SKILL] >= SKILL_ADVANCEMENT_CLASS::SPECIALIZED_SKILL_ADVANCEMENT_CLASS)
	{
		weenie->m_Qualities.AddSpell(RejuvenationOther1_SpellID);
		sack->SpawnInContainer(W_MUGWORT_CLASS, 10);
		sack->SpawnInContainer(W_ALCHEMCINNABAR_CLASS, 10);
		sack->SpawnInContainer(W_YEWTALISMAN_CLASS, 3);
	}

	if (cg.skillAdvancementClasses[WAR_MAGIC_SKILL] >= SKILL_ADVANCEMENT_CLASS::TRAINED_SKILL_ADVANCEMENT_CLASS)
	{
		sack->SpawnInContainer(W_SCARABLEAD_CLASS, 3);
		sack->SpawnInContainer(W_HAWTHORN_CLASS, 10);
		sack->SpawnInContainer(W_ONYX_CLASS, 10);
		sack->SpawnInContainer(W_BIRCHTALISMAN_CLASS, 3);

		weenie->m_Qualities.AddSpell(FlameBolt1_SpellID);
		sack->SpawnInContainer(W_ALCHEMTURPETH_CLASS, 10);

		weenie->m_Qualities.AddSpell(ForceBolt1_SpellID);
		sack->SpawnInContainer(W_ALCHEMREALGAR_CLASS, 10);
	}
	if (cg.skillAdvancementClasses[WAR_MAGIC_SKILL] >= SKILL_ADVANCEMENT_CLASS::SPECIALIZED_SKILL_ADVANCEMENT_CLASS)
	{
		weenie->m_Qualities.AddSpell(LightningBolt1_SpellID);
		sack->SpawnInContainer(W_ALCHEMCOBALT_CLASS, 10);
	}

	if (g_pConfig->EnableSpellFociStarterPack())
	{
		weenie->SpawnInContainer(W_PACKCREATUREESSENCE_CLASS, 1);
		weenie->SpawnInContainer(W_PACKITEMESSENCE_CLASS, 1);
		weenie->SpawnInContainer(W_PACKLIFEESSENCE_CLASS, 1);
		weenie->SpawnInContainer(W_PACKWARESSENCE_CLASS, 1);

		weenie->SpawnInContainer(W_TAPERPRISMATIC_CLASS, 25);
	}

	if(g_pConfig->PrimeNewCharacterSpellbook() > 0)
	{
		weenie->SpawnInContainer(W_SCARABLEAD_CLASS, 5);

		weenie->m_Qualities.AddSpell(ArmorSelf1_SpellID);
		weenie->m_Qualities.AddSpell(BladeProtectionSelf1_SpellID);
		weenie->m_Qualities.AddSpell(PiercingProtectionSelf1_SpellID);
		weenie->m_Qualities.AddSpell(BludgeonProtectionSelf1_SpellID);
		weenie->m_Qualities.AddSpell(FireProtectionSelf1_SpellID);
		weenie->m_Qualities.AddSpell(ColdProtectionSelf1_SpellID);
		weenie->m_Qualities.AddSpell(AcidProtectionSelf1_SpellID);
		weenie->m_Qualities.AddSpell(LightningProtectionSelf1_SpellID);

		weenie->m_Qualities.AddSpell(ImperilOther1_SpellID);
		weenie->m_Qualities.AddSpell(BladeVulnerabilityOther1_SpellID);
		weenie->m_Qualities.AddSpell(PiercingVulnerabilityOther1_SpellID);
		weenie->m_Qualities.AddSpell(BludgeonVulnerabilityOther1_SpellID);
		weenie->m_Qualities.AddSpell(FireVulnerabilityOther1_SpellID);
		weenie->m_Qualities.AddSpell(ColdVulnerabilityOther1_SpellID);
		weenie->m_Qualities.AddSpell(AcidVulnerabilityOther1_SpellID);
		weenie->m_Qualities.AddSpell(LightningVulnerabilityOther1_SpellID);

		weenie->m_Qualities.AddSpell(HealSelf1_SpellID);
		weenie->m_Qualities.AddSpell(HealOther1_SpellID);
		weenie->m_Qualities.AddSpell(RevitalizeSelf1_SpellID);
		weenie->m_Qualities.AddSpell(RevitalizeOther1_SpellID);
		weenie->m_Qualities.AddSpell(ManaBoostSelf1_SpellID);
		weenie->m_Qualities.AddSpell(ManaBoostOther1_SpellID);

		weenie->m_Qualities.AddSpell(StaminatoManaSelf1_SpellID);
		weenie->m_Qualities.AddSpell(HealthtoManaSelf1_SpellID);
		weenie->m_Qualities.AddSpell(StaminatoHealthSelf1_SpellID);

		weenie->m_Qualities.AddSpell(DrainHealth1_SpellID);
		weenie->m_Qualities.AddSpell(DrainStamina1_SpellID);
		weenie->m_Qualities.AddSpell(DrainMana1_SpellID);

		weenie->m_Qualities.AddSpell(HarmOther1_SpellID);
		weenie->m_Qualities.AddSpell(EnfeebleOther1_SpellID);
		weenie->m_Qualities.AddSpell(ManaDrainOther1_SpellID);

		weenie->m_Qualities.AddSpell(RegenerationSelf1_SpellID);
		weenie->m_Qualities.AddSpell(RejuvenationSelf1_SpellID);
		weenie->m_Qualities.AddSpell(ManaRenewalSelf1_SpellID);

		weenie->m_Qualities.AddSpell(MagicYieldOther1_SpellID);
		weenie->m_Qualities.AddSpell(VulnerabilityOther1_SpellID);
		weenie->m_Qualities.AddSpell(DefenselessnessOther1_SpellID);

		weenie->m_Qualities.AddSpell(StrengthSelf1_SpellID);
		weenie->m_Qualities.AddSpell(EnduranceSelf1_SpellID);
		weenie->m_Qualities.AddSpell(CoordinationSelf1_SpellID);
		weenie->m_Qualities.AddSpell(QuicknessSelf1_SpellID);
		weenie->m_Qualities.AddSpell(FocusSelf1_SpellID);
		weenie->m_Qualities.AddSpell(WillpowerSelf1_SpellID);

		weenie->m_Qualities.AddSpell(CreatureEnchantmentMasterySelf1_SpellID);
		weenie->m_Qualities.AddSpell(LifeMagicMasterySelf1_SpellID);
		weenie->m_Qualities.AddSpell(ItemEnchantmentMasterySelf1_SpellID);
		weenie->m_Qualities.AddSpell(WarMagicMasterySelf1_SpellID);
		weenie->m_Qualities.AddSpell(ManaMasterySelf1_SpellID);

		weenie->m_Qualities.AddSpell(ArcaneEnlightenmentSelf1_SpellID);
		weenie->m_Qualities.AddSpell(HealingMasterySelf1_SpellID);
		weenie->m_Qualities.AddSpell(SprintSelf1_SpellID);
		weenie->m_Qualities.AddSpell(JumpingMasterySelf1_SpellID);

		weenie->m_Qualities.AddSpell(ImpregnabilitySelf1_SpellID);
		weenie->m_Qualities.AddSpell(InvulnerabilitySelf1_SpellID);
		weenie->m_Qualities.AddSpell(MagicResistanceSelf1_SpellID);
		
		weenie->m_Qualities.AddSpell(FealtySelf1_SpellID);
		weenie->m_Qualities.AddSpell(LeadershipMasterySelf1_SpellID);

		weenie->m_Qualities.AddSpell(SwordMasterySelf1_SpellID);
		weenie->m_Qualities.AddSpell(AxeMasterySelf1_SpellID);
		weenie->m_Qualities.AddSpell(MaceMasterySelf1_SpellID);
		weenie->m_Qualities.AddSpell(SpearMasterySelf1_SpellID);
		weenie->m_Qualities.AddSpell(StaffMasterySelf1_SpellID);
		weenie->m_Qualities.AddSpell(DaggerMasterySelf1_SpellID);
		weenie->m_Qualities.AddSpell(UnarmedCombatMasterySelf1_SpellID);

		weenie->m_Qualities.AddSpell(CrossBowMasterySelf1_SpellID);
		weenie->m_Qualities.AddSpell(BowMasterySelf1_SpellID);
		weenie->m_Qualities.AddSpell(ThrownWeaponMasterySelf1_SpellID);

		weenie->m_Qualities.AddSpell(BloodDrinker1_SpellID);
		weenie->m_Qualities.AddSpell(HeartSeeker1_SpellID);
		weenie->m_Qualities.AddSpell(Defender1_SpellID);
		weenie->m_Qualities.AddSpell(SwiftKiller1_SpellID);

		weenie->m_Qualities.AddSpell(Impenetrability1_SpellID);
		weenie->m_Qualities.AddSpell(Bladebane1_SpellID);
		weenie->m_Qualities.AddSpell(BludgeonBane1_SpellID);
		weenie->m_Qualities.AddSpell(PiercingBane1_SpellID);
		weenie->m_Qualities.AddSpell(FlameBane1_SpellID);
		weenie->m_Qualities.AddSpell(FrostBane1_SpellID);
		weenie->m_Qualities.AddSpell(LightningBane1_SpellID);
		weenie->m_Qualities.AddSpell(AcidBane1_SpellID);

		weenie->m_Qualities.AddSpell(ForceBolt1_SpellID);
		weenie->m_Qualities.AddSpell(ForceArc1_SpellID);
		weenie->m_Qualities.AddSpell(ForceStreak1_SpellID);
		weenie->m_Qualities.AddSpell(WhirlingBlade1_SpellID);
		weenie->m_Qualities.AddSpell(BladeArc1_SpellID);
		weenie->m_Qualities.AddSpell(WhirlingBladeStreak1_SpellID);
		weenie->m_Qualities.AddSpell(ShockWave1_SpellID);
		weenie->m_Qualities.AddSpell(ShockArc1_SpellID);
		weenie->m_Qualities.AddSpell(ShockwaveStreak1_SpellID);
		weenie->m_Qualities.AddSpell(FlameBolt1_SpellID);
		weenie->m_Qualities.AddSpell(FlameArc1_SpellID);
		weenie->m_Qualities.AddSpell(FlameStreak1_SpellID);
		weenie->m_Qualities.AddSpell(FrostBolt1_SpellID);
		weenie->m_Qualities.AddSpell(FrostArc1_SpellID);
		weenie->m_Qualities.AddSpell(FrostStreak1_SpellID);
		weenie->m_Qualities.AddSpell(AcidStream1_SpellID);
		weenie->m_Qualities.AddSpell(AcidArc1_SpellID);
		weenie->m_Qualities.AddSpell(AcidStreak1_SpellID);
		weenie->m_Qualities.AddSpell(LightningBolt1_SpellID);
		weenie->m_Qualities.AddSpell(LightningArc1_SpellID);
		weenie->m_Qualities.AddSpell(LightningStreak1_SpellID);
	}
	if (g_pConfig->PrimeNewCharacterSpellbook() > 1)
	{
		weenie->SpawnInContainer(W_SCARABIRON_CLASS, 5);

		weenie->m_Qualities.AddSpell(ArmorSelf2_SpellID);
		weenie->m_Qualities.AddSpell(BladeProtectionSelf2_SpellID);
		weenie->m_Qualities.AddSpell(PiercingProtectionSelf2_SpellID);
		weenie->m_Qualities.AddSpell(BludgeonProtectionSelf2_SpellID);
		weenie->m_Qualities.AddSpell(FireProtectionSelf2_SpellID);
		weenie->m_Qualities.AddSpell(ColdProtectionSelf2_SpellID);
		weenie->m_Qualities.AddSpell(AcidProtectionSelf2_SpellID);
		weenie->m_Qualities.AddSpell(LightningProtectionSelf2_SpellID);

		weenie->m_Qualities.AddSpell(ImperilOther2_SpellID);
		weenie->m_Qualities.AddSpell(BladeVulnerabilityOther2_SpellID);
		weenie->m_Qualities.AddSpell(PiercingVulnerabilityOther2_SpellID);
		weenie->m_Qualities.AddSpell(BludgeonVulnerabilityOther2_SpellID);
		weenie->m_Qualities.AddSpell(FireVulnerabilityOther2_SpellID);
		weenie->m_Qualities.AddSpell(ColdVulnerabilityOther2_SpellID);
		weenie->m_Qualities.AddSpell(AcidVulnerabilityOther2_SpellID);
		weenie->m_Qualities.AddSpell(LightningVulnerabilityOther2_SpellID);

		weenie->m_Qualities.AddSpell(HealSelf2_SpellID);
		weenie->m_Qualities.AddSpell(HealOther2_SpellID);
		weenie->m_Qualities.AddSpell(RevitalizeSelf2_SpellID);
		weenie->m_Qualities.AddSpell(RevitalizeOther2_SpellID);
		weenie->m_Qualities.AddSpell(ManaBoostSelf2_SpellID);
		weenie->m_Qualities.AddSpell(ManaBoostOther2_SpellID);

		weenie->m_Qualities.AddSpell(StaminatoManaSelf2_SpellID);
		weenie->m_Qualities.AddSpell(HealthtoManaSelf2_SpellID);
		weenie->m_Qualities.AddSpell(StaminatoHealthSelf2_SpellID);

		weenie->m_Qualities.AddSpell(DrainHealth2_SpellID);
		weenie->m_Qualities.AddSpell(DrainStamina2_SpellID);
		weenie->m_Qualities.AddSpell(DrainMana2_SpellID);

		weenie->m_Qualities.AddSpell(HarmOther2_SpellID);
		weenie->m_Qualities.AddSpell(EnfeebleOther2_SpellID);
		weenie->m_Qualities.AddSpell(ManaDrainOther2_SpellID);

		weenie->m_Qualities.AddSpell(RegenerationSelf2_SpellID);
		weenie->m_Qualities.AddSpell(RejuvenationSelf2_SpellID);
		weenie->m_Qualities.AddSpell(ManaRenewalSelf2_SpellID);

		weenie->m_Qualities.AddSpell(MagicYieldOther2_SpellID);
		weenie->m_Qualities.AddSpell(VulnerabilityOther2_SpellID);
		weenie->m_Qualities.AddSpell(DefenselessnessOther2_SpellID);

		weenie->m_Qualities.AddSpell(StrengthSelf2_SpellID);
		weenie->m_Qualities.AddSpell(EnduranceSelf2_SpellID);
		weenie->m_Qualities.AddSpell(CoordinationSelf2_SpellID);
		weenie->m_Qualities.AddSpell(QuicknessSelf2_SpellID);
		weenie->m_Qualities.AddSpell(FocusSelf2_SpellID);
		weenie->m_Qualities.AddSpell(WillpowerSelf2_SpellID);

		weenie->m_Qualities.AddSpell(CreatureEnchantmentMasterySelf2_SpellID);
		weenie->m_Qualities.AddSpell(LifeMagicMasterySelf2_SpellID);
		weenie->m_Qualities.AddSpell(ItemEnchantmentMasterySelf2_SpellID);
		weenie->m_Qualities.AddSpell(WarMagicMasterySelf2_SpellID);
		weenie->m_Qualities.AddSpell(ManaMasterySelf2_SpellID);

		weenie->m_Qualities.AddSpell(ArcaneEnlightenmentSelf2_SpellID);
		weenie->m_Qualities.AddSpell(HealingMasterySelf2_SpellID);
		weenie->m_Qualities.AddSpell(SprintSelf2_SpellID);
		weenie->m_Qualities.AddSpell(JumpingMasterySelf2_SpellID);

		weenie->m_Qualities.AddSpell(ImpregnabilitySelf2_SpellID);
		weenie->m_Qualities.AddSpell(InvulnerabilitySelf2_SpellID);
		weenie->m_Qualities.AddSpell(MagicResistanceSelf2_SpellID);

		weenie->m_Qualities.AddSpell(FealtySelf2_SpellID);
		weenie->m_Qualities.AddSpell(LeadershipMasterySelf2_SpellID);

		weenie->m_Qualities.AddSpell(SwordMasterySelf2_SpellID);
		weenie->m_Qualities.AddSpell(AxeMasterySelf2_SpellID);
		weenie->m_Qualities.AddSpell(MaceMasterySelf2_SpellID);
		weenie->m_Qualities.AddSpell(SpearMasterySelf2_SpellID);
		weenie->m_Qualities.AddSpell(StaffMasterySelf2_SpellID);
		weenie->m_Qualities.AddSpell(DaggerMasterySelf2_SpellID);
		weenie->m_Qualities.AddSpell(UnarmedCombatMasterySelf2_SpellID);

		weenie->m_Qualities.AddSpell(CrossBowMasterySelf2_SpellID);
		weenie->m_Qualities.AddSpell(BowMasterySelf2_SpellID);
		weenie->m_Qualities.AddSpell(ThrownWeaponMasterySelf2_SpellID);

		weenie->m_Qualities.AddSpell(BloodDrinker2_SpellID);
		weenie->m_Qualities.AddSpell(HeartSeeker2_SpellID);
		weenie->m_Qualities.AddSpell(Defender2_SpellID);
		weenie->m_Qualities.AddSpell(SwiftKiller2_SpellID);

		weenie->m_Qualities.AddSpell(Impenetrability2_SpellID);
		weenie->m_Qualities.AddSpell(BladeBane2_SpellID);
		weenie->m_Qualities.AddSpell(BludgeonBane2_SpellID);
		weenie->m_Qualities.AddSpell(PiercingBane2_SpellID);
		weenie->m_Qualities.AddSpell(FlameBane2_SpellID);
		weenie->m_Qualities.AddSpell(FrostBane2_SpellID);
		weenie->m_Qualities.AddSpell(LightningBane2_SpellID);
		weenie->m_Qualities.AddSpell(AcidBane2_SpellID);

		weenie->m_Qualities.AddSpell(ForceBolt2_SpellID);
		weenie->m_Qualities.AddSpell(ForceArc2_SpellID);
		weenie->m_Qualities.AddSpell(ForceStreak2_SpellID);
		weenie->m_Qualities.AddSpell(WhirlingBlade2_SpellID);
		weenie->m_Qualities.AddSpell(BladeArc2_SpellID);
		weenie->m_Qualities.AddSpell(WhirlingBladeStreak2_SpellID);
		weenie->m_Qualities.AddSpell(ShockWave2_SpellID);
		weenie->m_Qualities.AddSpell(ShockArc2_SpellID);
		weenie->m_Qualities.AddSpell(ShockwaveStreak2_SpellID);
		weenie->m_Qualities.AddSpell(FlameBolt2_SpellID);
		weenie->m_Qualities.AddSpell(FlameArc2_SpellID);
		weenie->m_Qualities.AddSpell(FlameStreak2_SpellID);
		weenie->m_Qualities.AddSpell(FrostBolt2_SpellID);
		weenie->m_Qualities.AddSpell(FrostArc2_SpellID);
		weenie->m_Qualities.AddSpell(FrostStreak2_SpellID);
		weenie->m_Qualities.AddSpell(AcidStream2_SpellID);
		weenie->m_Qualities.AddSpell(AcidArc2_SpellID);
		weenie->m_Qualities.AddSpell(AcidStreak2_SpellID);
		weenie->m_Qualities.AddSpell(LightningBolt2_SpellID);
		weenie->m_Qualities.AddSpell(LightningArc2_SpellID);
		weenie->m_Qualities.AddSpell(LightningStreak2_SpellID);
	}
	if (g_pConfig->PrimeNewCharacterSpellbook() > 2)
	{
		weenie->SpawnInContainer(W_SCARABCOPPER_CLASS, 5);

		weenie->m_Qualities.AddSpell(ArmorSelf3_SpellID);
		weenie->m_Qualities.AddSpell(BladeProtectionSelf3_SpellID);
		weenie->m_Qualities.AddSpell(PiercingProtectionSelf3_SpellID);
		weenie->m_Qualities.AddSpell(BludgeonProtectionSelf3_SpellID);
		weenie->m_Qualities.AddSpell(FireProtectionSelf3_SpellID);
		weenie->m_Qualities.AddSpell(ColdProtectionSelf3_SpellID);
		weenie->m_Qualities.AddSpell(AcidProtectionSelf3_SpellID);
		weenie->m_Qualities.AddSpell(LightningProtectionSelf3_SpellID);

		weenie->m_Qualities.AddSpell(ImperilOther3_SpellID);
		weenie->m_Qualities.AddSpell(BladeVulnerabilityOther3_SpellID);
		weenie->m_Qualities.AddSpell(PiercingVulnerabilityOther3_SpellID);
		weenie->m_Qualities.AddSpell(BludgeonVulnerabilityOther3_SpellID);
		weenie->m_Qualities.AddSpell(FireVulnerabilityOther3_SpellID);
		weenie->m_Qualities.AddSpell(ColdVulnerabilityOther3_SpellID);
		weenie->m_Qualities.AddSpell(AcidVulnerabilityOther3_SpellID);
		weenie->m_Qualities.AddSpell(LightningVulnerabilityOther3_SpellID);

		weenie->m_Qualities.AddSpell(HealSelf3_SpellID);
		weenie->m_Qualities.AddSpell(HealOther3_SpellID);
		weenie->m_Qualities.AddSpell(RevitalizeSelf3_SpellID);
		weenie->m_Qualities.AddSpell(RevitalizeOther3_SpellID);
		weenie->m_Qualities.AddSpell(ManaBoostSelf3_SpellID);
		weenie->m_Qualities.AddSpell(ManaBoostOther3_SpellID);

		weenie->m_Qualities.AddSpell(StaminatoManaSelf3_SpellID);
		weenie->m_Qualities.AddSpell(HealthtoManaSelf3_SpellID);
		weenie->m_Qualities.AddSpell(StaminatoHealthSelf3_SpellID);

		weenie->m_Qualities.AddSpell(DrainHealth3_SpellID);
		weenie->m_Qualities.AddSpell(DrainStamina3_SpellID);
		weenie->m_Qualities.AddSpell(DrainMana3_SpellID);

		weenie->m_Qualities.AddSpell(HarmOther3_SpellID);
		weenie->m_Qualities.AddSpell(EnfeebleOther3_SpellID);
		weenie->m_Qualities.AddSpell(ManaDrainOther3_SpellID);

		weenie->m_Qualities.AddSpell(RegenerationSelf3_SpellID);
		weenie->m_Qualities.AddSpell(RejuvenationSelf3_SpellID);
		weenie->m_Qualities.AddSpell(ManaRenewalSelf3_SpellID);

		weenie->m_Qualities.AddSpell(MagicYieldOther3_SpellID);
		weenie->m_Qualities.AddSpell(VulnerabilityOther3_SpellID);
		weenie->m_Qualities.AddSpell(DefenselessnessOther3_SpellID);

		weenie->m_Qualities.AddSpell(StrengthSelf3_SpellID);
		weenie->m_Qualities.AddSpell(EnduranceSelf3_SpellID);
		weenie->m_Qualities.AddSpell(CoordinationSelf3_SpellID);
		weenie->m_Qualities.AddSpell(QuicknessSelf3_SpellID);
		weenie->m_Qualities.AddSpell(FocusSelf3_SpellID);
		weenie->m_Qualities.AddSpell(WillpowerSelf3_SpellID);

		weenie->m_Qualities.AddSpell(CreatureEnchantmentMasterySelf3_SpellID);
		weenie->m_Qualities.AddSpell(LifeMagicMasterySelf3_SpellID);
		weenie->m_Qualities.AddSpell(ItemEnchantmentMasterySelf3_SpellID);
		weenie->m_Qualities.AddSpell(WarMagicMasterySelf3_SpellID);
		weenie->m_Qualities.AddSpell(ManaMasterySelf3_SpellID);

		weenie->m_Qualities.AddSpell(ArcaneEnlightenmentSelf3_SpellID);
		weenie->m_Qualities.AddSpell(HealingMasterySelf3_SpellID);
		weenie->m_Qualities.AddSpell(SprintSelf3_SpellID);
		weenie->m_Qualities.AddSpell(JumpingMasterySelf3_SpellID);

		weenie->m_Qualities.AddSpell(ImpregnabilitySelf3_SpellID);
		weenie->m_Qualities.AddSpell(InvulnerabilitySelf3_SpellID);
		weenie->m_Qualities.AddSpell(MagicResistanceSelf3_SpellID);

		weenie->m_Qualities.AddSpell(FealtySelf3_SpellID);
		weenie->m_Qualities.AddSpell(LeadershipMasterySelf3_SpellID);

		weenie->m_Qualities.AddSpell(SwordMasterySelf3_SpellID);
		weenie->m_Qualities.AddSpell(AxeMasterySelf3_SpellID);
		weenie->m_Qualities.AddSpell(MaceMasterySelf3_SpellID);
		weenie->m_Qualities.AddSpell(SpearMasterySelf3_SpellID);
		weenie->m_Qualities.AddSpell(StaffMasterySelf3_SpellID);
		weenie->m_Qualities.AddSpell(DaggerMasterySelf3_SpellID);
		weenie->m_Qualities.AddSpell(UnarmedCombatMasterySelf3_SpellID);

		weenie->m_Qualities.AddSpell(CrossBowMasterySelf3_SpellID);
		weenie->m_Qualities.AddSpell(BowMasterySelf3_SpellID);
		weenie->m_Qualities.AddSpell(ThrownWeaponMasterySelf3_SpellID);

		weenie->m_Qualities.AddSpell(BloodDrinker3_SpellID);
		weenie->m_Qualities.AddSpell(HeartSeeker3_SpellID);
		weenie->m_Qualities.AddSpell(Defender3_SpellID);
		weenie->m_Qualities.AddSpell(SwiftKiller3_SpellID);

		weenie->m_Qualities.AddSpell(Impenetrability3_SpellID);
		weenie->m_Qualities.AddSpell(BladeBane3_SpellID);
		weenie->m_Qualities.AddSpell(BludgeonBane3_SpellID);
		weenie->m_Qualities.AddSpell(PiercingBane3_SpellID);
		weenie->m_Qualities.AddSpell(FlameBane3_SpellID);
		weenie->m_Qualities.AddSpell(FrostBane3_SpellID);
		weenie->m_Qualities.AddSpell(LightningBane3_SpellID);
		weenie->m_Qualities.AddSpell(AcidBane3_SpellID);

		weenie->m_Qualities.AddSpell(ForceBolt3_SpellID);
		weenie->m_Qualities.AddSpell(ForceArc3_SpellID);
		weenie->m_Qualities.AddSpell(ForceStreak3_SpellID);
		weenie->m_Qualities.AddSpell(WhirlingBlade3_SpellID);
		weenie->m_Qualities.AddSpell(BladeArc3_SpellID);
		weenie->m_Qualities.AddSpell(WhirlingBladeStreak3_SpellID);
		weenie->m_Qualities.AddSpell(ShockWave3_SpellID);
		weenie->m_Qualities.AddSpell(ShockArc3_SpellID);
		weenie->m_Qualities.AddSpell(ShockwaveStreak3_SpellID);
		weenie->m_Qualities.AddSpell(FlameBolt3_SpellID);
		weenie->m_Qualities.AddSpell(FlameArc3_SpellID);
		weenie->m_Qualities.AddSpell(FlameStreak3_SpellID);
		weenie->m_Qualities.AddSpell(FrostBolt3_SpellID);
		weenie->m_Qualities.AddSpell(FrostArc3_SpellID);
		weenie->m_Qualities.AddSpell(FrostStreak3_SpellID);
		weenie->m_Qualities.AddSpell(AcidStream3_SpellID);
		weenie->m_Qualities.AddSpell(AcidArc3_SpellID);
		weenie->m_Qualities.AddSpell(AcidStreak3_SpellID);
		weenie->m_Qualities.AddSpell(LightningBolt3_SpellID);
		weenie->m_Qualities.AddSpell(LightningArc3_SpellID);
		weenie->m_Qualities.AddSpell(LightningStreak3_SpellID);

		weenie->m_Qualities.AddSpell(PortalRecall_SpellID);
		weenie->m_Qualities.AddSpell(PortalTieRecall1_SpellID);
		weenie->m_Qualities.AddSpell(PortalTieRecall2_SpellID);
		weenie->m_Qualities.AddSpell(LifestoneRecall1_SpellID);

		weenie->m_Qualities.AddSpell(PortalTie1_SpellID);
		weenie->m_Qualities.AddSpell(PortalTie2_SpellID);
		weenie->m_Qualities.AddSpell(LifestoneTie1_SpellID);
	}
	if (g_pConfig->PrimeNewCharacterSpellbook() > 3)
	{
		weenie->SpawnInContainer(W_SCARABSILVER_CLASS, 5);

		weenie->m_Qualities.AddSpell(ArmorSelf4_SpellID);
		weenie->m_Qualities.AddSpell(BladeProtectionSelf4_SpellID);
		weenie->m_Qualities.AddSpell(PiercingProtectionSelf4_SpellID);
		weenie->m_Qualities.AddSpell(BludgeonProtectionSelf4_SpellID);
		weenie->m_Qualities.AddSpell(FireProtectionSelf4_SpellID);
		weenie->m_Qualities.AddSpell(ColdProtectionSelf4_SpellID);
		weenie->m_Qualities.AddSpell(AcidProtectionSelf4_SpellID);
		weenie->m_Qualities.AddSpell(LightningProtectionSelf4_SpellID);

		weenie->m_Qualities.AddSpell(ImperilOther4_SpellID);
		weenie->m_Qualities.AddSpell(BladeVulnerabilityOther4_SpellID);
		weenie->m_Qualities.AddSpell(PiercingVulnerabilityOther4_SpellID);
		weenie->m_Qualities.AddSpell(BludgeonVulnerabilityOther4_SpellID);
		weenie->m_Qualities.AddSpell(FireVulnerabilityOther4_SpellID);
		weenie->m_Qualities.AddSpell(ColdVulnerabilityOther4_SpellID);
		weenie->m_Qualities.AddSpell(AcidVulnerabilityOther4_SpellID);
		weenie->m_Qualities.AddSpell(LightningVulnerabilityOther4_SpellID);

		weenie->m_Qualities.AddSpell(HealSelf4_SpellID);
		weenie->m_Qualities.AddSpell(HealOther4_SpellID);
		weenie->m_Qualities.AddSpell(RevitalizeSelf4_SpellID);
		weenie->m_Qualities.AddSpell(RevitalizeOther4_SpellID);
		weenie->m_Qualities.AddSpell(ManaBoostSelf4_SpellID);
		weenie->m_Qualities.AddSpell(ManaBoostOther4_SpellID);

		weenie->m_Qualities.AddSpell(StaminatoManaSelf4_SpellID);
		weenie->m_Qualities.AddSpell(HealthtoManaSelf4_SpellID);
		weenie->m_Qualities.AddSpell(StaminatoHealthSelf4_SpellID);

		weenie->m_Qualities.AddSpell(DrainHealth4_SpellID);
		weenie->m_Qualities.AddSpell(DrainStamina4_SpellID);
		weenie->m_Qualities.AddSpell(DrainMana4_SpellID);

		weenie->m_Qualities.AddSpell(HarmOther4_SpellID);
		weenie->m_Qualities.AddSpell(EnfeebleOther4_SpellID);
		weenie->m_Qualities.AddSpell(ManaDrainOther4_SpellID);

		weenie->m_Qualities.AddSpell(RegenerationSelf4_SpellID);
		weenie->m_Qualities.AddSpell(RejuvenationSelf4_SpellID);
		weenie->m_Qualities.AddSpell(ManaRenewalSelf4_SpellID);

		weenie->m_Qualities.AddSpell(MagicYieldOther4_SpellID);
		weenie->m_Qualities.AddSpell(VulnerabilityOther4_SpellID);
		weenie->m_Qualities.AddSpell(DefenselessnessOther4_SpellID);

		weenie->m_Qualities.AddSpell(StrengthSelf4_SpellID);
		weenie->m_Qualities.AddSpell(EnduranceSelf4_SpellID);
		weenie->m_Qualities.AddSpell(CoordinationSelf4_SpellID);
		weenie->m_Qualities.AddSpell(QuicknessSelf4_SpellID);
		weenie->m_Qualities.AddSpell(FocusSelf4_SpellID);
		weenie->m_Qualities.AddSpell(WillpowerSelf4_SpellID);

		weenie->m_Qualities.AddSpell(CreatureEnchantmentMasterySelf4_SpellID);
		weenie->m_Qualities.AddSpell(LifeMagicMasterySelf4_SpellID);
		weenie->m_Qualities.AddSpell(ItemEnchantmentMasterySelf4_SpellID);
		weenie->m_Qualities.AddSpell(WarMagicMasterySelf4_SpellID);
		weenie->m_Qualities.AddSpell(ManaMasterySelf4_SpellID);

		weenie->m_Qualities.AddSpell(ArcaneEnlightenmentSelf4_SpellID);
		weenie->m_Qualities.AddSpell(HealingMasterySelf4_SpellID);
		weenie->m_Qualities.AddSpell(SprintSelf4_SpellID);
		weenie->m_Qualities.AddSpell(JumpingMasterySelf4_SpellID);

		weenie->m_Qualities.AddSpell(ImpregnabilitySelf4_SpellID);
		weenie->m_Qualities.AddSpell(InvulnerabilitySelf4_SpellID);
		weenie->m_Qualities.AddSpell(MagicResistanceSelf4_SpellID);

		weenie->m_Qualities.AddSpell(FealtySelf4_SpellID);
		weenie->m_Qualities.AddSpell(LeadershipMasterySelf4_SpellID);

		weenie->m_Qualities.AddSpell(SwordMasterySelf4_SpellID);
		weenie->m_Qualities.AddSpell(AxeMasterySelf4_SpellID);
		weenie->m_Qualities.AddSpell(MaceMasterySelf4_SpellID);
		weenie->m_Qualities.AddSpell(SpearMasterySelf4_SpellID);
		weenie->m_Qualities.AddSpell(StaffMasterySelf4_SpellID);
		weenie->m_Qualities.AddSpell(DaggerMasterySelf4_SpellID);
		weenie->m_Qualities.AddSpell(UnarmedCombatMasterySelf4_SpellID);

		weenie->m_Qualities.AddSpell(CrossBowMasterySelf4_SpellID);
		weenie->m_Qualities.AddSpell(BowMasterySelf4_SpellID);
		weenie->m_Qualities.AddSpell(ThrownWeaponMasterySelf4_SpellID);

		weenie->m_Qualities.AddSpell(BloodDrinker4_SpellID);
		weenie->m_Qualities.AddSpell(HeartSeeker4_SpellID);
		weenie->m_Qualities.AddSpell(Defender4_SpellID);
		weenie->m_Qualities.AddSpell(SwiftKiller4_SpellID);

		weenie->m_Qualities.AddSpell(Impenetrability4_SpellID);
		weenie->m_Qualities.AddSpell(BladeBane4_SpellID);
		weenie->m_Qualities.AddSpell(BludgeonBane4_SpellID);
		weenie->m_Qualities.AddSpell(PiercingBane4_SpellID);
		weenie->m_Qualities.AddSpell(FlameBane4_SpellID);
		weenie->m_Qualities.AddSpell(FrostBane4_SpellID);
		weenie->m_Qualities.AddSpell(LightningBane4_SpellID);
		weenie->m_Qualities.AddSpell(AcidBane4_SpellID);

		weenie->m_Qualities.AddSpell(ForceBolt4_SpellID);
		weenie->m_Qualities.AddSpell(ForceArc4_SpellID);
		weenie->m_Qualities.AddSpell(ForceStreak4_SpellID);
		weenie->m_Qualities.AddSpell(WhirlingBlade4_SpellID);
		weenie->m_Qualities.AddSpell(BladeArc4_SpellID);
		weenie->m_Qualities.AddSpell(WhirlingBladeStreak4_SpellID);
		weenie->m_Qualities.AddSpell(ShockWave4_SpellID);
		weenie->m_Qualities.AddSpell(ShockArc4_SpellID);
		weenie->m_Qualities.AddSpell(ShockwaveStreak4_SpellID);
		weenie->m_Qualities.AddSpell(FlameBolt4_SpellID);
		weenie->m_Qualities.AddSpell(FlameArc4_SpellID);
		weenie->m_Qualities.AddSpell(FlameStreak4_SpellID);
		weenie->m_Qualities.AddSpell(FrostBolt4_SpellID);
		weenie->m_Qualities.AddSpell(FrostArc4_SpellID);
		weenie->m_Qualities.AddSpell(FrostStreak4_SpellID);
		weenie->m_Qualities.AddSpell(AcidStream4_SpellID);
		weenie->m_Qualities.AddSpell(AcidArc4_SpellID);
		weenie->m_Qualities.AddSpell(AcidStreak4_SpellID);
		weenie->m_Qualities.AddSpell(LightningBolt4_SpellID);
		weenie->m_Qualities.AddSpell(LightningArc4_SpellID);
		weenie->m_Qualities.AddSpell(LightningStreak4_SpellID);

		weenie->m_Qualities.AddSpell(SummonPortal1_SpellID);
		weenie->m_Qualities.AddSpell(SummonSecondPortal1_SpellID);
	}
	if (g_pConfig->PrimeNewCharacterSpellbook() > 4)
	{
		weenie->SpawnInContainer(W_SCARABGOLD_CLASS, 5);

		weenie->m_Qualities.AddSpell(ArmorSelf5_SpellID);
		weenie->m_Qualities.AddSpell(BladeProtectionSelf5_SpellID);
		weenie->m_Qualities.AddSpell(PiercingProtectionSelf5_SpellID);
		weenie->m_Qualities.AddSpell(BludgeonProtectionSelf5_SpellID);
		weenie->m_Qualities.AddSpell(FireProtectionSelf5_SpellID);
		weenie->m_Qualities.AddSpell(ColdProtectionSelf5_SpellID);
		weenie->m_Qualities.AddSpell(AcidProtectionSelf5_SpellID);
		weenie->m_Qualities.AddSpell(LightningProtectionSelf5_SpellID);

		weenie->m_Qualities.AddSpell(ImperilOther5_SpellID);
		weenie->m_Qualities.AddSpell(BladeVulnerabilityOther5_SpellID);
		weenie->m_Qualities.AddSpell(PiercingVulnerabilityOther5_SpellID);
		weenie->m_Qualities.AddSpell(BludgeonVulnerabilityOther5_SpellID);
		weenie->m_Qualities.AddSpell(FireVulnerabilityOther5_SpellID);
		weenie->m_Qualities.AddSpell(ColdVulnerabilityOther5_SpellID);
		weenie->m_Qualities.AddSpell(AcidVulnerabilityOther5_SpellID);
		weenie->m_Qualities.AddSpell(LightningVulnerabilityOther5_SpellID);

		weenie->m_Qualities.AddSpell(HealSelf5_SpellID);
		weenie->m_Qualities.AddSpell(HealOther5_SpellID);
		weenie->m_Qualities.AddSpell(RevitalizeSelf5_SpellID);
		weenie->m_Qualities.AddSpell(RevitalizeOther5_SpellID);
		weenie->m_Qualities.AddSpell(ManaBoostSelf5_SpellID);
		weenie->m_Qualities.AddSpell(ManaBoostOther5_SpellID);

		weenie->m_Qualities.AddSpell(StaminatoManaSelf5_SpellID);
		weenie->m_Qualities.AddSpell(HealthtoManaSelf5_SpellID);
		weenie->m_Qualities.AddSpell(StaminatoHealthSelf5_SpellID);

		weenie->m_Qualities.AddSpell(DrainHealth5_SpellID);
		weenie->m_Qualities.AddSpell(DrainStamina5_SpellID);
		weenie->m_Qualities.AddSpell(DrainMana5_SpellID);

		weenie->m_Qualities.AddSpell(HarmOther5_SpellID);
		weenie->m_Qualities.AddSpell(EnfeebleOther5_SpellID);
		weenie->m_Qualities.AddSpell(ManaDrainOther5_SpellID);

		weenie->m_Qualities.AddSpell(RegenerationSelf5_SpellID);
		weenie->m_Qualities.AddSpell(RejuvenationSelf5_SpellID);
		weenie->m_Qualities.AddSpell(ManaRenewalSelf5_SpellID);

		weenie->m_Qualities.AddSpell(MagicYieldOther5_SpellID);
		weenie->m_Qualities.AddSpell(VulnerabilityOther5_SpellID);
		weenie->m_Qualities.AddSpell(DefenselessnessOther5_SpellID);

		weenie->m_Qualities.AddSpell(StrengthSelf5_SpellID);
		weenie->m_Qualities.AddSpell(EnduranceSelf5_SpellID);
		weenie->m_Qualities.AddSpell(CoordinationSelf5_SpellID);
		weenie->m_Qualities.AddSpell(QuicknessSelf5_SpellID);
		weenie->m_Qualities.AddSpell(FocusSelf5_SpellID);
		weenie->m_Qualities.AddSpell(WillpowerSelf5_SpellID);

		weenie->m_Qualities.AddSpell(CreatureEnchantmentMasterySelf5_SpellID);
		weenie->m_Qualities.AddSpell(LifeMagicMasterySelf5_SpellID);
		weenie->m_Qualities.AddSpell(ItemEnchantmentMasterySelf5_SpellID);
		weenie->m_Qualities.AddSpell(WarMagicMasterySelf5_SpellID);
		weenie->m_Qualities.AddSpell(ManaMasterySelf5_SpellID);

		weenie->m_Qualities.AddSpell(ArcaneEnlightenmentSelf5_SpellID);
		weenie->m_Qualities.AddSpell(HealingMasterySelf5_SpellID);
		weenie->m_Qualities.AddSpell(SprintSelf5_SpellID);
		weenie->m_Qualities.AddSpell(JumpingMasterySelf5_SpellID);

		weenie->m_Qualities.AddSpell(ImpregnabilitySelf5_SpellID);
		weenie->m_Qualities.AddSpell(InvulnerabilitySelf5_SpellID);
		weenie->m_Qualities.AddSpell(MagicResistanceSelf5_SpellID);

		weenie->m_Qualities.AddSpell(FealtySelf5_SpellID);
		weenie->m_Qualities.AddSpell(LeadershipMasterySelf5_SpellID);

		weenie->m_Qualities.AddSpell(SwordMasterySelf5_SpellID);
		weenie->m_Qualities.AddSpell(AxeMasterySelf5_SpellID);
		weenie->m_Qualities.AddSpell(MaceMasterySelf5_SpellID);
		weenie->m_Qualities.AddSpell(SpearMasterySelf5_SpellID);
		weenie->m_Qualities.AddSpell(StaffMasterySelf5_SpellID);
		weenie->m_Qualities.AddSpell(DaggerMasterySelf5_SpellID);
		weenie->m_Qualities.AddSpell(UnarmedCombatMasterySelf5_SpellID);

		weenie->m_Qualities.AddSpell(CrossBowMasterySelf5_SpellID);
		weenie->m_Qualities.AddSpell(BowMasterySelf5_SpellID);
		weenie->m_Qualities.AddSpell(ThrownWeaponMasterySelf5_SpellID);

		weenie->m_Qualities.AddSpell(BloodDrinker5_SpellID);
		weenie->m_Qualities.AddSpell(HeartSeeker5_SpellID);
		weenie->m_Qualities.AddSpell(Defender5_SpellID);
		weenie->m_Qualities.AddSpell(SwiftKiller5_SpellID);

		weenie->m_Qualities.AddSpell(Impenetrability5_SpellID);
		weenie->m_Qualities.AddSpell(BladeBane5_SpellID);
		weenie->m_Qualities.AddSpell(BludgeonBane5_SpellID);
		weenie->m_Qualities.AddSpell(PiercingBane5_SpellID);
		weenie->m_Qualities.AddSpell(FlameBane5_SpellID);
		weenie->m_Qualities.AddSpell(FrostBane5_SpellID);
		weenie->m_Qualities.AddSpell(LightningBane5_SpellID);
		weenie->m_Qualities.AddSpell(AcidBane5_SpellID);

		weenie->m_Qualities.AddSpell(ForceBolt5_SpellID);
		weenie->m_Qualities.AddSpell(ForceArc5_SpellID);
		weenie->m_Qualities.AddSpell(ForceStreak5_SpellID);
		weenie->m_Qualities.AddSpell(WhirlingBlade5_SpellID);
		weenie->m_Qualities.AddSpell(BladeArc5_SpellID);
		weenie->m_Qualities.AddSpell(WhirlingBladeStreak5_SpellID);
		weenie->m_Qualities.AddSpell(ShockWave5_SpellID);
		weenie->m_Qualities.AddSpell(ShockArc5_SpellID);
		weenie->m_Qualities.AddSpell(ShockwaveStreak5_SpellID);
		weenie->m_Qualities.AddSpell(FlameBolt5_SpellID);
		weenie->m_Qualities.AddSpell(FlameArc5_SpellID);
		weenie->m_Qualities.AddSpell(FlameStreak5_SpellID);
		weenie->m_Qualities.AddSpell(FrostBolt5_SpellID);
		weenie->m_Qualities.AddSpell(FrostArc5_SpellID);
		weenie->m_Qualities.AddSpell(FrostStreak5_SpellID);
		weenie->m_Qualities.AddSpell(AcidStream5_SpellID);
		weenie->m_Qualities.AddSpell(AcidArc5_SpellID);
		weenie->m_Qualities.AddSpell(AcidStreak5_SpellID);
		weenie->m_Qualities.AddSpell(LightningBolt5_SpellID);
		weenie->m_Qualities.AddSpell(LightningArc5_SpellID);
		weenie->m_Qualities.AddSpell(LightningStreak5_SpellID);
	}
	if (g_pConfig->PrimeNewCharacterSpellbook() > 5)
	{
		weenie->SpawnInContainer(W_SCARABPYREAL_CLASS, 5);

		weenie->m_Qualities.AddSpell(ArmorSelf6_SpellID);
		weenie->m_Qualities.AddSpell(BladeProtectionSelf6_SpellID);
		weenie->m_Qualities.AddSpell(PiercingProtectionSelf6_SpellID);
		weenie->m_Qualities.AddSpell(BludgeonProtectionSelf6_SpellID);
		weenie->m_Qualities.AddSpell(FireProtectionSelf6_SpellID);
		weenie->m_Qualities.AddSpell(ColdProtectionSelf6_SpellID);
		weenie->m_Qualities.AddSpell(AcidProtectionSelf6_SpellID);
		weenie->m_Qualities.AddSpell(LightningProtectionSelf6_SpellID);

		weenie->m_Qualities.AddSpell(ImperilOther6_SpellID);
		weenie->m_Qualities.AddSpell(BladeVulnerabilityOther6_SpellID);
		weenie->m_Qualities.AddSpell(PiercingVulnerabilityOther6_SpellID);
		weenie->m_Qualities.AddSpell(BludgeonVulnerabilityOther6_SpellID);
		weenie->m_Qualities.AddSpell(FireVulnerabilityOther6_SpellID);
		weenie->m_Qualities.AddSpell(ColdVulnerabilityOther6_SpellID);
		weenie->m_Qualities.AddSpell(AcidVulnerabilityOther6_SpellID);
		weenie->m_Qualities.AddSpell(LightningVulnerabilityOther6_SpellID);

		weenie->m_Qualities.AddSpell(HealSelf6_SpellID);
		weenie->m_Qualities.AddSpell(HealOther6_SpellID);
		weenie->m_Qualities.AddSpell(RevitalizeSelf6_SpellID);
		weenie->m_Qualities.AddSpell(RevitalizeOther6_SpellID);
		weenie->m_Qualities.AddSpell(ManaBoostSelf6_SpellID);
		weenie->m_Qualities.AddSpell(ManaBoostOther6_SpellID);

		weenie->m_Qualities.AddSpell(StaminatoManaSelf6_SpellID);
		weenie->m_Qualities.AddSpell(HealthtoManaSelf6_SpellID);
		weenie->m_Qualities.AddSpell(StaminatoHealthSelf6_SpellID);

		weenie->m_Qualities.AddSpell(DrainHealth6_SpellID);
		weenie->m_Qualities.AddSpell(DrainStamina6_SpellID);
		weenie->m_Qualities.AddSpell(DrainMana6_SpellID);

		weenie->m_Qualities.AddSpell(HarmOther6_SpellID);
		weenie->m_Qualities.AddSpell(EnfeebleOther6_SpellID);
		weenie->m_Qualities.AddSpell(ManaDrainOther6_SpellID);

		weenie->m_Qualities.AddSpell(RegenerationSelf6_SpellID);
		weenie->m_Qualities.AddSpell(RejuvenationSelf6_SpellID);
		weenie->m_Qualities.AddSpell(ManaRenewalSelf6_SpellID);

		weenie->m_Qualities.AddSpell(MagicYieldOther6_SpellID);
		weenie->m_Qualities.AddSpell(VulnerabilityOther6_SpellID);
		weenie->m_Qualities.AddSpell(DefenselessnessOther6_SpellID);

		weenie->m_Qualities.AddSpell(StrengthSelf6_SpellID);
		weenie->m_Qualities.AddSpell(EnduranceSelf6_SpellID);
		weenie->m_Qualities.AddSpell(CoordinationSelf6_SpellID);
		weenie->m_Qualities.AddSpell(QuicknessSelf6_SpellID);
		weenie->m_Qualities.AddSpell(FocusSelf6_SpellID);
		weenie->m_Qualities.AddSpell(WillpowerSelf6_SpellID);

		weenie->m_Qualities.AddSpell(CreatureEnchantmentMasterySelf6_SpellID);
		weenie->m_Qualities.AddSpell(LifeMagicMasterySelf6_SpellID);
		weenie->m_Qualities.AddSpell(ItemEnchantmentMasterySelf6_SpellID);
		weenie->m_Qualities.AddSpell(WarMagicMasterySelf6_SpellID);
		weenie->m_Qualities.AddSpell(ManaMasterySelf6_SpellID);

		weenie->m_Qualities.AddSpell(ArcaneEnlightenmentSelf6_SpellID);
		weenie->m_Qualities.AddSpell(HealingMasterySelf6_SpellID);
		weenie->m_Qualities.AddSpell(SprintSelf6_SpellID);
		weenie->m_Qualities.AddSpell(JumpingMasterySelf6_SpellID);

		weenie->m_Qualities.AddSpell(ImpregnabilitySelf6_SpellID);
		weenie->m_Qualities.AddSpell(InvulnerabilitySelf6_SpellID);
		weenie->m_Qualities.AddSpell(MagicResistanceSelf6_SpellID);

		weenie->m_Qualities.AddSpell(FealtySelf6_SpellID);
		weenie->m_Qualities.AddSpell(LeadershipMasterySelf6_SpellID);

		weenie->m_Qualities.AddSpell(SwordMasterySelf6_SpellID);
		weenie->m_Qualities.AddSpell(AxeMasterySelf6_SpellID);
		weenie->m_Qualities.AddSpell(MaceMasterySelf6_SpellID);
		weenie->m_Qualities.AddSpell(SpearMasterySelf6_SpellID);
		weenie->m_Qualities.AddSpell(StaffMasterySelf6_SpellID);
		weenie->m_Qualities.AddSpell(DaggerMasterySelf6_SpellID);
		weenie->m_Qualities.AddSpell(UnarmedCombatMasterySelf6_SpellID);

		weenie->m_Qualities.AddSpell(CrossBowMasterySelf6_SpellID);
		weenie->m_Qualities.AddSpell(BowMasterySelf6_SpellID);
		weenie->m_Qualities.AddSpell(ThrownWeaponMasterySelf6_SpellID);

		weenie->m_Qualities.AddSpell(BloodDrinker6_SpellID);
		weenie->m_Qualities.AddSpell(HeartSeeker6_SpellID);
		weenie->m_Qualities.AddSpell(Defender6_SpellID);
		weenie->m_Qualities.AddSpell(SwiftKiller6_SpellID);

		weenie->m_Qualities.AddSpell(Impenetrability6_SpellID);
		weenie->m_Qualities.AddSpell(BladeBane6_SpellID);
		weenie->m_Qualities.AddSpell(BludgeonBane6_SpellID);
		weenie->m_Qualities.AddSpell(PiercingBane6_SpellID);
		weenie->m_Qualities.AddSpell(FlameBane6_SpellID);
		weenie->m_Qualities.AddSpell(FrostBane6_SpellID);
		weenie->m_Qualities.AddSpell(LightningBane6_SpellID);
		weenie->m_Qualities.AddSpell(AcidBane6_SpellID);

		weenie->m_Qualities.AddSpell(ForceBolt6_SpellID);
		weenie->m_Qualities.AddSpell(ForceArc6_SpellID);
		weenie->m_Qualities.AddSpell(ForceStreak6_SpellID);
		weenie->m_Qualities.AddSpell(WhirlingBlade6_SpellID);
		weenie->m_Qualities.AddSpell(BladeArc6_SpellID);
		weenie->m_Qualities.AddSpell(WhirlingBladeStreak6_SpellID);
		weenie->m_Qualities.AddSpell(ShockWave6_SpellID);
		weenie->m_Qualities.AddSpell(ShockArc6_SpellID);
		weenie->m_Qualities.AddSpell(ShockwaveStreak6_SpellID);
		weenie->m_Qualities.AddSpell(FlameBolt6_SpellID);
		weenie->m_Qualities.AddSpell(FlameArc6_SpellID);
		weenie->m_Qualities.AddSpell(FlameStreak6_SpellID);
		weenie->m_Qualities.AddSpell(FrostBolt6_SpellID);
		weenie->m_Qualities.AddSpell(FrostArc6_SpellID);
		weenie->m_Qualities.AddSpell(FrostStreak6_SpellID);
		weenie->m_Qualities.AddSpell(AcidStream6_SpellID);
		weenie->m_Qualities.AddSpell(AcidArc6_SpellID);
		weenie->m_Qualities.AddSpell(AcidStreak6_SpellID);
		weenie->m_Qualities.AddSpell(LightningBolt6_SpellID);
		weenie->m_Qualities.AddSpell(LightningArc6_SpellID);
		weenie->m_Qualities.AddSpell(LightningStreak6_SpellID);
	}
	if (g_pConfig->PrimeNewCharacterSpellbook() > 6)
	{
		weenie->SpawnInContainer(W_SCARABPLATINUM_CLASS, 5);

		weenie->m_Qualities.AddSpell(ArmorSelf7_SpellID);
		weenie->m_Qualities.AddSpell(BladeProtectionSelf7_SpellID);
		weenie->m_Qualities.AddSpell(PiercingProtectionSelf7_SpellID);
		weenie->m_Qualities.AddSpell(BludgeonProtectionSelf7_SpellID);
		weenie->m_Qualities.AddSpell(FireProtectionSelf7_SpellID);
		weenie->m_Qualities.AddSpell(ColdProtectionSelf7_SpellID);
		weenie->m_Qualities.AddSpell(AcidProtectionSelf7_SpellID);
		weenie->m_Qualities.AddSpell(LightningProtectionSelf7_SpellID);

		weenie->m_Qualities.AddSpell(ImperilOther7_SpellID);
		weenie->m_Qualities.AddSpell(BladeVulnerabilityOther7_SpellID);
		weenie->m_Qualities.AddSpell(PiercingVulnerabilityOther7_SpellID);
		weenie->m_Qualities.AddSpell(BludgeonVulnerabilityOther7_SpellID);
		weenie->m_Qualities.AddSpell(FireVulnerabilityOther7_SpellID);
		weenie->m_Qualities.AddSpell(ColdVulnerabilityOther7_SpellID);
		weenie->m_Qualities.AddSpell(AcidVulnerabilityOther7_SpellID);
		weenie->m_Qualities.AddSpell(LightningVulnerabilityOther7_SpellID);

		weenie->m_Qualities.AddSpell(healself7_SpellID);
		weenie->m_Qualities.AddSpell(healother7_SpellID);
		weenie->m_Qualities.AddSpell(RevitalizeSelf7_SpellID);
		weenie->m_Qualities.AddSpell(Revitalizeother7_SpellID);
		weenie->m_Qualities.AddSpell(ManaBoostSelf7_SpellID);
		weenie->m_Qualities.AddSpell(ManaBoostOther7_SpellID);

		weenie->m_Qualities.AddSpell(StaminatoManaSelf7_SpellID);
		weenie->m_Qualities.AddSpell(HealthtoManaSelf7_SpellID);
		weenie->m_Qualities.AddSpell(StaminatoHealthSelf7_SpellID);

		weenie->m_Qualities.AddSpell(DrainHealth7_SpellID);
		weenie->m_Qualities.AddSpell(DrainStamina7_SpellID);
		weenie->m_Qualities.AddSpell(DrainMana7_SpellID);

		weenie->m_Qualities.AddSpell(HarmOther7_SpellID);
		weenie->m_Qualities.AddSpell(EnfeebleOther7_SpellID);
		weenie->m_Qualities.AddSpell(ManaDrainOther7_SpellID);

		weenie->m_Qualities.AddSpell(RegenerationSelf7_SpellID);
		weenie->m_Qualities.AddSpell(RejuvenationSelf7_SpellID);
		weenie->m_Qualities.AddSpell(ManaRenewalSelf7_SpellID);

		weenie->m_Qualities.AddSpell(MagicYieldOther7_SpellID);
		weenie->m_Qualities.AddSpell(VulnerabilityOther7_SpellID);
		weenie->m_Qualities.AddSpell(DefenselessnessOther7_SpellID);

		weenie->m_Qualities.AddSpell(StrengthSelf7_SpellID);
		weenie->m_Qualities.AddSpell(EnduranceSelf7_SpellID);
		weenie->m_Qualities.AddSpell(CoordinationSelf7_SpellID);
		weenie->m_Qualities.AddSpell(QuicknessSelf7_SpellID);
		weenie->m_Qualities.AddSpell(FocusSelf7_SpellID);
		weenie->m_Qualities.AddSpell(WillpowerSelf7_SpellID);

		weenie->m_Qualities.AddSpell(CreatureEnchantmentMasterySelf7_SpellID);
		weenie->m_Qualities.AddSpell(LifeMagicMasterySelf7_SpellID);
		weenie->m_Qualities.AddSpell(ItemEnchantmentMasterySelf7_SpellID);
		weenie->m_Qualities.AddSpell(WarMagicMasterySelf7_SpellID);
		weenie->m_Qualities.AddSpell(ManaMasterySelf7_SpellID);

		weenie->m_Qualities.AddSpell(ArcaneEnlightenmentSelf7_SpellID);
		weenie->m_Qualities.AddSpell(HealingMasterySelf7_SpellID);
		weenie->m_Qualities.AddSpell(SprintSelf7_SpellID);
		weenie->m_Qualities.AddSpell(JumpingMasterySelf7_SpellID);

		weenie->m_Qualities.AddSpell(ImpregnabilitySelf7_SpellID);
		weenie->m_Qualities.AddSpell(InvulnerabilitySelf7_SpellID);
		weenie->m_Qualities.AddSpell(MagicResistanceSelf7_SpellID);

		weenie->m_Qualities.AddSpell(FealtySelf7_SpellID);
		weenie->m_Qualities.AddSpell(LeadershipMasterySelf7_SpellID);

		weenie->m_Qualities.AddSpell(SwordMasterySelf7_SpellID);
		weenie->m_Qualities.AddSpell(AxeMasterySelf7_SpellID);
		weenie->m_Qualities.AddSpell(MaceMasterySelf7_SpellID);
		weenie->m_Qualities.AddSpell(SpearMasterySelf7_SpellID);
		weenie->m_Qualities.AddSpell(StaffMasterySelf7_SpellID);
		weenie->m_Qualities.AddSpell(DaggerMasterySelf7_SpellID);
		weenie->m_Qualities.AddSpell(UnarmedCombatMasterySelf7_SpellID);

		weenie->m_Qualities.AddSpell(CrossbowMasterySelf7_SpellID);
		weenie->m_Qualities.AddSpell(BowMasterySelf7_SpellID);
		weenie->m_Qualities.AddSpell(ThrownWeaponMasterySelf7_SpellID);

		weenie->m_Qualities.AddSpell(BloodDrinker7_SpellID);
		weenie->m_Qualities.AddSpell(Heartseeker7_SpellID);
		weenie->m_Qualities.AddSpell(Defender7_SpellID);
		weenie->m_Qualities.AddSpell(Swiftkiller7_SpellID);

		weenie->m_Qualities.AddSpell(Impenetrability7_SpellID);
		weenie->m_Qualities.AddSpell(BladeBane7_SpellID);
		weenie->m_Qualities.AddSpell(BludgeonBane7_SpellID);
		weenie->m_Qualities.AddSpell(PiercingBane7_SpellID);
		weenie->m_Qualities.AddSpell(FlameBane7_SpellID);
		weenie->m_Qualities.AddSpell(FrostBane7_SpellID);
		weenie->m_Qualities.AddSpell(LightningBane7_SpellID);
		weenie->m_Qualities.AddSpell(AcidBane7_SpellID);

		weenie->m_Qualities.AddSpell(ForceBolt7_SpellID);
		weenie->m_Qualities.AddSpell(ForceArc7_SpellID);
		weenie->m_Qualities.AddSpell(ForceStreak7_SpellID);
		weenie->m_Qualities.AddSpell(Whirlingblade7_SpellID);
		weenie->m_Qualities.AddSpell(BladeArc7_SpellID);
		weenie->m_Qualities.AddSpell(WhirlingBladeStreak7_SpellID);
		weenie->m_Qualities.AddSpell(Shockwave7_SpellID);
		weenie->m_Qualities.AddSpell(ShockArc7_SpellID);
		weenie->m_Qualities.AddSpell(ShockwaveStreak7_SpellID);
		weenie->m_Qualities.AddSpell(FlameBolt7_SpellID);
		weenie->m_Qualities.AddSpell(FlameArc7_SpellID);
		weenie->m_Qualities.AddSpell(FlameStreak7_SpellID);
		weenie->m_Qualities.AddSpell(FrostBolt7_SpellID);
		weenie->m_Qualities.AddSpell(FrostArc7_SpellID);
		weenie->m_Qualities.AddSpell(FrostStreak7_SpellID);
		weenie->m_Qualities.AddSpell(AcidStream7_SpellID);
		weenie->m_Qualities.AddSpell(AcidArc7_SpellID);
		weenie->m_Qualities.AddSpell(AcidStreak7_SpellID);
		weenie->m_Qualities.AddSpell(Lightningbolt7_SpellID);
		weenie->m_Qualities.AddSpell(LightningArc7_SpellID);
		weenie->m_Qualities.AddSpell(LightningStreak7_SpellID);
	}

	weenie->RecalculateCoinAmount();
}

void CClient::SendLandblock(DWORD dwFileID)
{
	TURBINEFILE* pLandData = g_pCell->GetFile(dwFileID);
	if (!pLandData)
	{
		if (m_pEvents)
		{
			m_pEvents->SendText(csprintf("Your client is requesting cell data (0x%08X) that this server does not have!", dwFileID), LTT_DEFAULT);
			m_pEvents->SendText("If you are stuck in portal mode, type /render radius 5 to escape. The server administrator should reconfigure the server with a FULL cell.dat file!", LTT_DEFAULT);
		}
		return;
	}

	if ((dwFileID & 0xFFFF) != 0xFFFF)
	{
		LOG(Client, Warning, "Client requested Landblock 0x%08X - should end pReader 0xFFFF\n", dwFileID);
	}

	if (pLandData)
	{
		BinaryWriter BlockPackage;

		BlockPackage.Write<DWORD>(0xF7E2);

		DWORD dwFileSize = pLandData->GetLength();
		BYTE* pbFileData = pLandData->GetData();

		DWORD dwPackageSize = (DWORD)((dwFileSize * 1.02f) + 12 + 1);
		BYTE* pbPackageData = new BYTE[dwPackageSize];

		if (Z_OK != compress2(pbPackageData, &dwPackageSize, pbFileData, dwFileSize, Z_BEST_COMPRESSION))
		{
			LOG(Client, Error, "Error compressing LandBlock package!\n");
		}

		BlockPackage.Write<DWORD>(1); //the resource type: 1 for 0xFFFF, 2 for 0xFFFE, 3 for 0x100
		BlockPackage.Write<DWORD>(2); //1 = client_portal / 2 = client_cell_1 / 3 = client_local_English
		BlockPackage.Write<DWORD>(1); //?
		BlockPackage.Write<DWORD>(dwFileID); //the resource ID number
		BlockPackage.Write<DWORD>(1); //the file version number
		BlockPackage.Write<BYTE>(1); // 0 = uncompressed / 1 = compressed
		BlockPackage.Write<DWORD>(2); //?
		BlockPackage.Write<DWORD>(dwPackageSize + sizeof(DWORD) * 2); //the number of bytes required for the remainder of this message, including this DWORD
		BlockPackage.Write<DWORD>(dwFileSize); //the size of the uncompressed file
		BlockPackage.Write(pbPackageData, dwPackageSize); //bytes of zlib compressed file data
		BlockPackage.Align();

		delete[] pbPackageData;
		delete pLandData;

		SendNetMessage(BlockPackage.GetData(), BlockPackage.GetSize(), EVENT_MSG, FALSE);
	}
}

void CClient::SendLandblockInfo(DWORD dwFileID)
{
	if ((dwFileID & 0xFFFF) != 0xFFFE)
	{
		LOG(Client, Warning, "Client requested LandblockInfo 0x%08X - should end pReader 0xFFFE\n", dwFileID);
		return;
	}

	TURBINEFILE *pObjData = g_pCell->GetFile(dwFileID);
	if (!pObjData)
	{
		return;
	}

	if (pObjData)
	{
		BinaryWriter BlockInfoPackage;
		BlockInfoPackage.Write<DWORD>(0xF7E2);

		DWORD dwFileSize = pObjData->GetLength();
		BYTE* pbFileData = pObjData->GetData();

		DWORD dwPackageSize = (DWORD)((dwFileSize * 1.02f) + 12 + 1);
		BYTE* pbPackageData = new BYTE[dwPackageSize];

		if (Z_OK != compress2(pbPackageData, &dwPackageSize, pbFileData, dwFileSize, Z_BEST_COMPRESSION))
		{
			LOG(Client, Error, "Error compressing LandBlockInfo package!\n");
		}

		BlockInfoPackage.Write<DWORD>(2); // 1 for 0xFFFF, 2 for 0xFFFE, 3 for 0x100
		BlockInfoPackage.Write<DWORD>(2);
		BlockInfoPackage.Write<DWORD>(1);
		BlockInfoPackage.Write<DWORD>(dwFileID);
		BlockInfoPackage.Write<DWORD>(1);
		BlockInfoPackage.Write<BYTE>(1);
		BlockInfoPackage.Write<DWORD>(2);
		BlockInfoPackage.Write<DWORD>(dwPackageSize + sizeof(DWORD) * 2);
		BlockInfoPackage.Write<DWORD>(dwFileSize);
		BlockInfoPackage.Write(pbPackageData, dwPackageSize);
		BlockInfoPackage.Align();

		delete[] pbPackageData;
		delete pObjData;

		SendNetMessage(BlockInfoPackage.GetData(), BlockInfoPackage.GetSize(), EVENT_MSG, FALSE);
	}
}

void CClient::SendLandcell(DWORD dwFileID)
{
	TURBINEFILE* pCellData = g_pCell->GetFile(dwFileID);
	if (!pCellData)
	{
		if (m_pEvents)
		{
			m_pEvents->SendText(csprintf("Your client is requesting cell data (#%08X) that this server does not have!", dwFileID), LTT_DEFAULT);
			m_pEvents->SendText("If you are stuck in portal mode, type /render radius 5 to escape. The server administrator should reconfigure the server with a FULL cell.dat file!", LTT_DEFAULT);
		}
		return;
	}

	if (pCellData)
	{
		BinaryWriter CellPackage;

		CellPackage.Write<DWORD>(0xF7E2);

		DWORD dwFileSize = pCellData->GetLength();
		BYTE* pbFileData = pCellData->GetData();

		DWORD dwPackageSize = (DWORD)((dwFileSize * 1.02f) + 12 + 1);
		BYTE* pbPackageData = new BYTE[dwPackageSize];

		if (Z_OK != compress2(pbPackageData, &dwPackageSize, pbFileData, dwFileSize, Z_BEST_COMPRESSION))
		{
			// These are CEnvCell if I recall correctly
			LOG(Client, Error, "Error compressing landcell package!\n");
		}

		CellPackage.Write<DWORD>(3); // 1 for 0xFFFF, 2 for 0xFFFE, 3 for 0x100
		CellPackage.Write<DWORD>(2);
		CellPackage.Write<DWORD>(1);
		CellPackage.Write<DWORD>(dwFileID);
		CellPackage.Write<DWORD>(1);
		CellPackage.Write<BYTE>(1);
		CellPackage.Write<DWORD>(2);
		CellPackage.Write<DWORD>(dwPackageSize + sizeof(DWORD) * 2);
		CellPackage.Write<DWORD>(dwFileSize);
		CellPackage.Write(pbPackageData, dwPackageSize);
		CellPackage.Align();

		delete[] pbPackageData;
		delete pCellData;

		//LOG(Temp, Normal, "Sent cell %08X ..\n", dwFileID);

		SendNetMessage(CellPackage.GetData(), CellPackage.GetSize(), EVENT_MSG, FALSE);
	}

	//if (m_pEvents)
	//	m_pEvents->SendText(csprintf("The server has sent you cell #%04X!", dwFileID >> 16), LTT_DEFAULT);
}

void CClient::ProcessMessage(BYTE *data, DWORD length, WORD group)
{
	if (g_bDebugToggle)
	{
		LOG(Network, Normal, "%.03f Received response (group %u):\n", g_pGlobals->Time(), group);
		LOG_BYTES(Network, Normal, data, length);
	}

	BinaryReader in(data, length);

	DWORD dwMessageCode = in.ReadDWORD();

#ifdef _DEBUG
	// LOG(Client, Normal, "Processing response 0x%X (size %d):\n", dwMessageCode, length);
#endif

	if (in.GetLastError())
	{
		LOG(Client, Warning, "Error processing response.\n");
		return;
	}
	switch (dwMessageCode)
	{
		case 0xF653:
		{
			if (m_vars.bInWorld)
			{
				m_pEvents->BeginLogout();
			}

			break;
		}

		case 0xF655: // Delete Character
			if (!m_vars.bInWorld)
			{
				DeleteCharacter(&in);
			}

			break;

		case 0xF656: // Create Character
		{
			if (!m_vars.bInWorld)
			{
				CreateCharacter(&in);
			}

			break;
		}

		case 0xF6EA: // Request Object
		{
			if (m_vars.bInWorld)
			{
				DWORD dwEID = in.ReadDWORD();
				if (in.GetLastError()) break;

				CPlayerWeenie *pPlayer;

				if ((m_pEvents) && (pPlayer = m_pEvents->GetPlayer()))
				{
					CWeenieObject *pTarget = g_pWorld->FindWithinPVS(pPlayer, dwEID);
									
					if (pTarget)
						pPlayer->MakeAware(pTarget);
					else
					{
						pTarget = g_pWorld->FindObject(dwEID);

						if (pTarget)
						{
							// LOG(Temp, Normal, "Player %s is requesting info on %s it shouldn't know about.\n", pPlayer->GetName().c_str(), pTarget->GetName().c_str());
						}
					}
				}
			}
			break;
		}

		case 0xF7E6://DDD_InterrogationResponseMessage
		{
			DWORD clientLanguage = in.ReadDWORD();
			PackableList<DWORD> fileIds;
			fileIds.UnPack(&in);
				
			if (in.GetLastError())
				break;

			BinaryWriter EndDDD;
			EndDDD.Write<long>(0xF7EA);
			SendNetMessage(EndDDD.GetData(), EndDDD.GetSize(), EVENT_MSG);
			break;
		}

		case 0xF7EA: //DDD_OnEndDDD
		{
			//BinaryWriter EndDDD;
			//EndDDD.Write<DWORD>(0xF7EA);
			//SendNetMessage(EndDDD.GetData(), EndDDD.GetSize(), EVENT_MSG);

			break;
		}

		case 0xF7E3: //DDD_RequestDataMessage
		{
			if (m_vars.bInWorld)
			{
				DWORD dwFileClass = in.ReadDWORD();
				DWORD dwFileID = in.ReadDWORD();
				if (in.GetLastError()) break;

				//disabled downloading files.
				m_pEvents->SendText("Your client is requesting map data from the server. This feature is disabled to preserve bandwidth", LTT_DEFAULT);
				m_pEvents->SendText("If you are stuck in portal mode, type /render radius 5 to escape. Make sure you are using the approppriate cell.dat file!", LTT_DEFAULT);

				//switch (dwFileClass)
				//{
				//default:
				//	LOG(Client, Warning, "Unknown download request: %08X %d\n", dwFileID, dwFileClass);
				//	break;

				//case 1:
				//{
				//	SendLandblock(dwFileID); // 1 is landblock 0xFFFF
				//	break;
				//}
				//case 2:
				//	SendLandblockInfo(dwFileID); // 2 is landblock environemnt 0xFFFE
				//	break;

				//case 3:
				//	SendLandcell(dwFileID); // 0x100+
				//	break;
				//}
			}

			break;
		}

		case 0xF7B1:
		{
			//should check the sequence
			if (m_vars.bInWorld)
			{
				m_pEvents->ProcessEvent(&in);
			}

			break;
		}
		case 0xF7C8:
		{
			if (!m_vars.bInWorld)
			{
				EnterWorld();
			}

			break;
		}

		case 0xF657:
		{
			if (m_vars.bInWorld)
			{
				DWORD dwGUID = in.ReadDWORD();
				char* szName = in.ReadString();
				if (in.GetLastError()) break;

				m_pEvents->LoginCharacter(dwGUID, /*szName*/GetAccount());
			}

			break;
		}

		// modified cmoski code
		case 0xF7DE:
		{ //Gets data from /general here to rebroadcast as a world message below \o/
			DWORD size = in.ReadDWORD();
			DWORD TurbineChatType = in.ReadDWORD(); //0x1 Inbound, 0x3 Outbound, 0x5 Outbound Ack
			DWORD unk1 = in.ReadDWORD();
			DWORD unk2 = in.ReadDWORD();
			DWORD unk3 = in.ReadDWORD();
			DWORD unk4 = in.ReadDWORD();
			DWORD unk5 = in.ReadDWORD();
			DWORD unk6 = in.ReadDWORD();
			DWORD payload = in.ReadDWORD();

			if (in.GetLastError())
				break;

			if (TurbineChatType == 0x3) //0x3 Outbound
			{
				if (!m_pEvents->GetPlayer() || !m_pEvents->CheckForChatSpam())
					break;
				
				DWORD serial = in.ReadDWORD(); // serial of this character's chat
				DWORD channel_unk = in.ReadDWORD();
				DWORD channel_unk2 = in.ReadDWORD();
				DWORD listening_channel = in.ReadDWORD(); // ListeningChannel in SetTurbineChatChannels (0x000BEEF0-9)
				std::string message = in.ReadWStringToString();
				DWORD payloadSize = in.ReadDWORD();
				DWORD playerGUID = in.ReadDWORD();
				DWORD ob_unknown = in.ReadDWORD(); //Always 0?
				DWORD ob_unknown2 = in.ReadDWORD();

				std::string filteredText = FilterBadChatCharacters(message.c_str());

				if (in.GetLastError() || !filteredText.size())
					break;

				BinaryWriter chatAck;
				chatAck.Write<DWORD>(0xF7DE);
				chatAck.Write<DWORD>(0x30); // Size
				chatAck.Write<DWORD>(0x05); // Type 05 - ACK
				chatAck.Write<DWORD>(0x02); // ??
				chatAck.Write<DWORD>(0x01); // ??
				chatAck.Write<DWORD>(0xB0045); // bitfield?
				chatAck.Write<DWORD>(0x01);
				chatAck.Write<DWORD>(0xB0045); // bitfield?
				chatAck.Write<DWORD>(0x00);
				chatAck.Write<DWORD>(0x10);   // Payload size
				chatAck.Write<DWORD>(serial); // serial
				chatAck.Write<DWORD>(0x02);
				chatAck.Write<DWORD>(0x02);
				chatAck.Write<DWORD>(0x00);
				SendNetMessage(&chatAck, 0x04, FALSE, FALSE);

				if (filteredText.c_str()[0] == '!' || filteredText.c_str()[0] == '@' || filteredText.c_str()[0] == '/')
				{
					CommandBase::Execute((char*)filteredText.erase(0, 1).c_str(), m_pEvents->GetPlayer()->GetClient());
				}
				else
				{
					switch (listening_channel)
					{
					case General_ChatChannel:
					case Trade_ChatChannel:
					case LFG_ChatChannel:
					case Roleplay_ChatChannel:
					case Allegiance_ChatChannel:
					// case Olthoi_ChatChannel:
					// case Society_ChatChannel:
						g_pWorld->BroadcastChatChannel(listening_channel, m_pEvents->GetPlayer(), filteredText);
						break;
					}
				}
			}

			break;
		}
		default:
#ifdef _DEBUG
			LOG(Client, Warning, "Unhandled response %08X from the client.\n", dwMessageCode);
#endif
			break;
	}

	//if ( dwMessageCode != 0xF7B1 )
	//	LOG(Temp, Normal, "Received response %04X\n", dwMessageCode);
}

BOOL CClient::CheckAccount(const char* cmp)
{
	return !_stricmp(m_vars.account.c_str(), cmp);
}

int CClient::GetAccessLevel()
{
	return m_AccountInfo.access;
}

void CClient::SetAccessLevel(unsigned int access)
{
	m_AccountInfo.access = access;
}

BOOL CClient::CheckAddress(SOCKADDR_IN *peer)
{
	return !memcmp(GetHostAddress(), peer, sizeof(SOCKADDR_IN));
}

WORD CClient::GetSlot()
{
	return m_vars.slot;
}

const char *CClient::GetAccount()
{
	return m_vars.account.c_str();
}

const char *CClient::GetDescription()
{
	// Must lead with index or the kick/ban feature won't function.
	return csprintf("#%u %s \"%s\"", m_vars.slot, inet_ntoa(m_vars.addr.sin_addr), m_vars.account.c_str());
}

void CClient::SetLoginData(DWORD dwUnixTime, DWORD dwPortalStamp, DWORD dwCellStamp)
{
	m_vars.fLoginTime = g_pGlobals->Time();

	m_vars.ClientLoginUnixTime = dwUnixTime;
	m_vars.PortalStamp = dwPortalStamp;
	m_vars.CellStamp = dwCellStamp;

	m_vars.bInitDats = FALSE;
}

SOCKADDR_IN *CClient::GetHostAddress()
{
	return &m_vars.addr;
}
