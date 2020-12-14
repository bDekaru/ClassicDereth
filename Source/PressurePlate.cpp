
#include <StdAfx.h>
#include "PressurePlate.h"
#include "World.h"
#include "SpellcastingManager.h"
#include "WeenieFactory.h"

CPressurePlateWeenie::CPressurePlateWeenie()
{
}

CPressurePlateWeenie::~CPressurePlateWeenie()
{
}

void CPressurePlateWeenie::ApplyQualityOverrides()
{
}

void CPressurePlateWeenie::PostSpawn()
{
	CWeenieObject::PostSpawn();
}

void CPressurePlateWeenie::Tick()
{
}

int CPressurePlateWeenie::DoCollision(const class ObjCollisionProfile &prof)
{
	if (prof._bitfield & Player_OCPB && next_activation_time <= Timer::cur_time)
	{
		Activate(prof.id);
		next_activation_time = Timer::cur_time + 1.0;
	}

	return 1;
}

int CPressurePlateWeenie::Activate(uint32_t activator_id)
{
	//CWeenieObject::Activate(activator_id);

	CWeenieObject *activator_weenie = g_pWorld->FindObject(activator_id);
	if (activator_weenie && activator_weenie->_IsPlayer())
	{
		if (uint32_t use_sound_did = InqDIDQuality(USE_SOUND_DID, 0))
		{
			CWeenieObject *weenie = g_pWorld->FindObject(activator_id);

			if (weenie)
			{			
				weenie->EmitSound(use_sound_did, 1.0f);
			}
		}

		if (uint32_t activation_target_id = InqIIDQuality(ACTIVATION_TARGET_IID, 0))
		{
			CWeenieObject *activation_target = g_pWorld->FindObject(activation_target_id);
			if (activation_target)
				activation_target->Activate(activator_id);
		}

		if (InqIntQuality(ACTIVATION_RESPONSE_INT, 0) & CastSpell_ActivationResponse)
		{
			if (uint32_t spell_did = InqDIDQuality(SPELL_DID, 0))
			{
				MakeSpellcastingManager()->CastSpellInstant(activator_id, spell_did);
			}
		}

		if (InqIntQuality(ACTIVATION_RESPONSE_INT, 0) & Generate_ActivationResponse)
		{
			// spawn generated objects

			if (m_Qualities._generator_table)
			{
				for (auto &entry : m_Qualities._generator_table->_profile_list)
				{
					if (!entry.maxNum)
						continue;

					int numToSpawn = entry.maxNum;

					if (m_Qualities._generator_registry)
					{
						for (auto &registered : m_Qualities._generator_registry->_registry)
						{
							if (entry.slot == registered.second.slot)
							{
								numToSpawn--;

								if (!numToSpawn)
									break;
							}
						}
					}

					if (m_Qualities._generator_queue)
					{
						for (auto &queued : m_Qualities._generator_queue->_queue)
						{
							if (entry.slot == queued.slot)
							{
								numToSpawn--;

								if (!numToSpawn)
									break;
							}
						}
					}

					for (int i = 0; i < numToSpawn; i++)
					{
						g_pWeenieFactory->GenerateFromTypeOrWcid(this, &entry);
					}
				}
			}
		}
	}

	return 1;
}