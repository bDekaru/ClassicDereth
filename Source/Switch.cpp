
#include "StdAfx.h"
#include "Switch.h"
#include "World.h"
#include "Player.h"
#include "SpellcastingManager.h"
#include "WeenieFactory.h"

CSwitchWeenie::CSwitchWeenie()
{
}

CSwitchWeenie::~CSwitchWeenie()
{
}

void CSwitchWeenie::ApplyQualityOverrides()
{
}

void CSwitchWeenie::PlaySwitchMotion()
{
	if (DWORD use_target_animation_did = InqDIDQuality(USE_TARGET_ANIMATION_DID, 0))
	{
		last_move_was_autonomous = false;

		_server_control_timestamp++;

		MovementStruct mvs;
		MovementParameters params;
		mvs.type = RawCommand;
		mvs.motion = use_target_animation_did;
		mvs.params = &params;
		params.autonomous = 0;
		params.action_stamp = ++m_wAnimSequence;
		get_movement_manager()->PerformMovement(mvs);
		Animation_Update();
	}
}

int CSwitchWeenie::Activate(DWORD activator_id)
{
	if (get_minterp()->interpreted_state.GetNumActions())
		return WERROR_NONE;
	
	int activationResponse = InqIntQuality(ACTIVATION_RESPONSE_INT, 0);

	if (m_fNextSwitchActivation <= Timer::cur_time)
	{		
		if (DWORD activation_target_id = InqIIDQuality(ACTIVATION_TARGET_IID, 0))
		{
			CWeenieObject *activation_target = g_pWorld->FindObject(activation_target_id);
			if (activation_target)
				activation_target->Activate(activator_id);
		}

		PlaySwitchMotion();

		//if (activationResponse & Activation_CastSpell)
		{
			if (DWORD spell_did = InqDIDQuality(SPELL_DID, 0))
			{
				MakeSpellcastingManager()->CastSpellInstant(activator_id, spell_did);
			}
		}

		if (activationResponse & Generate_ActivationResponse)
		{
			g_pWeenieFactory->AddFromGeneratorTable(this, false);
		}

		if (activationResponse & Talk_ActivationResponse)
		{
			std::string talkText;
			if (m_Qualities.InqString(ACTIVATION_TALK_STRING, talkText))
			{
				CPlayerWeenie *player = g_pWorld->FindPlayer(activator_id);
				if (player)
					player->SendText(talkText.c_str(), LTT_DEFAULT);
			}
		}

		DoActivationEmote(activator_id);

		m_fNextSwitchActivation = Timer::cur_time + InqFloatQuality(RESET_INTERVAL_FLOAT, 0.0);
	}
	else
	{
		std::string failText;
		if (m_Qualities.InqString(ACTIVATION_FAILURE_STRING, failText))
		{
			CPlayerWeenie *player = g_pWorld->FindPlayer(activator_id);
			if (player)
				player->SendText(failText.c_str(), LTT_DEFAULT);
		}
	}

	return WERROR_NONE;
}

int CSwitchWeenie::Use(CPlayerWeenie *other)
{
	CActivationUseEvent *useEvent = new CActivationUseEvent();
	useEvent->_target_id = GetID();
	other->ExecuteUseEvent(useEvent);

	return WERROR_NONE;
}

