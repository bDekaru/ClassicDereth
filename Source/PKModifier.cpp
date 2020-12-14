
#include <StdAfx.h>
#include "PKModifier.h"
#include "UseManager.h"
#include "Player.h"
#include "Config.h"

CPKModifierWeenie::CPKModifierWeenie()
{
}

CPKModifierWeenie::~CPKModifierWeenie()
{
}

int CPKModifierWeenie::Use(CPlayerWeenie *player)
{
	CGenericUseEvent *useEvent = new CGenericUseEvent;
	useEvent->_target_id = GetID();
	useEvent->_do_use_message = false;
	player->ExecuteUseEvent(useEvent);

	return WERROR_NONE;
}

int CPKModifierWeenie::DoUseResponse(CWeenieObject *player)
{
	if (atoi(g_pConfig->GetValue("player_killer_only", "0")) != 0)
	{
		std::string useMessage;
		if (m_Qualities.InqString(USE_PK_SERVER_ERROR_STRING, useMessage))
		{
			player->SendText(useMessage.c_str(), LTT_MAGIC);
		}

		return WERROR_PK_INVALID_STATUS;
	}

	if (int pkmod = InqIntQuality(PK_LEVEL_MODIFIER_INT, 0))
	{
		// -1 = NPK
		// 1 = PK

		if (pkmod > 0)
		{
			double pkTimestamp;
			if (player->IsPK() || player->m_Qualities.InqFloat(PK_TIMESTAMP_FLOAT, pkTimestamp, TRUE))
			{
				std::string useMessage;
				if (m_Qualities.InqString(ACTIVATION_FAILURE_STRING, useMessage))
				{
					player->SendText(useMessage.c_str(), LTT_MAGIC);
				}

				return WERROR_PK_INVALID_STATUS;
			}

			std::string useMessage;
			if (m_Qualities.InqString(USE_MESSAGE_STRING, useMessage))
			{
				player->SendText(useMessage.c_str(), LTT_MAGIC);
			}

			player->m_Qualities.SetInt(PLAYER_KILLER_STATUS_INT, PKStatusEnum::PK_PKStatus);
			player->NotifyIntStatUpdated(PLAYER_KILLER_STATUS_INT, false);
		}
		else if (pkmod < 0)
		{
			double pkTimestamp;
			if (!player->IsPK() && !player->m_Qualities.InqFloat(PK_TIMESTAMP_FLOAT, pkTimestamp, TRUE))
			{
				std::string useMessage;
				if (m_Qualities.InqString(ACTIVATION_FAILURE_STRING, useMessage))
				{
					player->SendText(useMessage.c_str(), LTT_MAGIC);
				}

				return WERROR_PK_INVALID_STATUS;
			}

			std::string useMessage;
			if (m_Qualities.InqString(USE_MESSAGE_STRING, useMessage))
			{
				player->SendText(useMessage.c_str(), LTT_MAGIC);
			}

			player->m_Qualities.SetInt(PLAYER_KILLER_STATUS_INT, PKStatusEnum::NPK_PKStatus);
			player->NotifyIntStatUpdated(PLAYER_KILLER_STATUS_INT, false);

			player->m_Qualities.RemoveFloat(PK_TIMESTAMP_FLOAT);
			player->NotifyFloatStatUpdated(PK_TIMESTAMP_FLOAT);
		}
	}

	return CWeenieObject::DoUseResponse(player);
}

