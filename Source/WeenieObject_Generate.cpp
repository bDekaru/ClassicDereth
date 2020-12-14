#include <StdAfx.h>
#include "WeenieObject.h"
#include "PhysicsObj.h"
#include "ObjectMsgs.h"
#include "World.h"
#include "Lifestone.h"
#include "UseManager.h"
#include "SpellcastingManager.h"
#include "AttackManager.h"
#include "WeenieFactory.h"

#include "BinaryWriter.h"
#include "ChatMsgs.h"
#include "HouseManager.h"
#include "Player.h"
#include "Client.h"
#include "ClientCommands.h"
#include "MonsterAI.h"
#include "AllegianceManager.h"
#include "SpellcastingManager.h"
#include "GameEventManager.h"
#include "EmoteManager.h"
#include "WorldLandBlock.h"
#include "InferredPortalData.h"
#include "MovementManager.h"
#include "Movement.h"
#include "WClassID.h"
#include "InferredPortalData.h"
#include "Config.h"
#include "House.h"
#include "Ammunition.h"


void CWeenieObject::EnsureLink(CWeenieObject *source)
{
	if (GeneratorTable *table = m_Qualities._generator_table)
	{
		source->m_Qualities.SetInstanceID(GENERATOR_IID, GetID());

		// linkable gens have a placeholder item that defines how long to wait
		// the regen interval on these is faster than the placeholder delay
		// we either need to have the placeholder stored on load or find/copy that node
		// in the table to have a proper delay on linked items
		// the assumption is linkable gens have the placeholder as the first item

		uint32_t length = (uint32_t)table->_profile_list.size();
		GeneratorProfile prof;

		if (length > 0)
		{
			prof = *table->_profile_list.begin();
		}

		prof.slot = length;
		prof.type = source->m_Qualities.id;
		prof.ptid = source->m_Qualities.GetInt(PALETTE_TEMPLATE_INT, 0);
		prof.shade = source->m_Qualities.GetFloat(SHADE_FLOAT, 0.0);
		prof.pos_val = source->GetPosition();

		table->_profile_list.push_back(prof);

		// some of these have defined init/max values that don't make sense
		// so update the max-gen with the length of the table
		m_Qualities.SetInt(MAX_GENERATED_OBJECTS_INT, length + 1);

		GeneratorAddToRegistry(source, prof);
	}
	else
	{
		// probably should check if it's zero or not
		source->m_Qualities.SetInstanceID(ACTIVATION_TARGET_IID, GetID());
	}
}

void CWeenieObject::NotifyGeneratedFailure(CWeenieObject *weenie)
{
	// Don't erase entry from m_GeneratorSpawns here as NotifyGeneratedDeath still runs after this.
	OnGeneratedFailure(weenie);
}

void CWeenieObject::OnGeneratedFailure(CWeenieObject *weenie)
{
	if (!weenie)
		return;

	uint32_t weenie_id = weenie->GetID();

	if (!m_Qualities._generator_registry)
		return;

	GeneratorRegistryNode *node = m_Qualities._generator_registry->_registry.lookup(weenie_id);

	if (node)
	{
		// Remove from the registry here as we don't want OnGeneratedDeath to trigger the respawn delay.
		m_Qualities._generator_registry->_registry.remove(weenie->GetID());

		if (!m_Qualities._generator_queue)
			m_Qualities._generator_queue = new GeneratorQueue();

		// Ensure this node is not stuck in queue
		GeneratorQueueNode queueNode;
		queueNode.slot = node->slot;
		queueNode.when = Timer::cur_time;
		m_Qualities._generator_queue->_queue.push_back(queueNode);
	}

	// Set next regen to try again.
	_nextRegen = Timer::cur_time;
}

void CWeenieObject::NotifyGeneratedDeath(CWeenieObject *weenie)
{
	OnGeneratedDestruction(weenie, (RegenerationType)(Death_RegenerationType | Destruction_RegenerationType));
	m_GeneratorSpawns.erase(weenie->GetID());
}

void CWeenieObject::NotifyGeneratedPickedUp(CWeenieObject *weenie)
{
	OnGeneratedDestruction(weenie, (RegenerationType)(PickUp_RegenerationType | Destruction_RegenerationType));
	weenie->m_Qualities.RemoveInstanceID(GENERATOR_IID);
	m_GeneratorSpawns.erase(weenie->GetID());
}

void CWeenieObject::OnGeneratedDestruction(CWeenieObject *weenie, RegenerationType flags)
{
	if (!weenie || !m_Qualities._generator_registry)
		return;

	GeneratorRemoveNode(weenie->GetID(), flags);

	if (InqIIDQuality(GENERATOR_IID, 0)) // we have a generator
	{
		std::vector<CWeenieObject *> rotList;
		bool hasValidChildren = false;
		if (m_Qualities._generator_registry->_registry.size() > 0)
		{
			for (auto entry : m_Qualities._generator_registry->_registry)
			{
				CWeenieObject *weenie = g_pWorld->FindObject(entry.second.m_objectId);

				if (weenie && (weenie->IsCreature() || !weenie->IsStuck())) //stuck objects(chests and decorations usually) do not prevent the generator from being finished.
					hasValidChildren = true;
				else if (weenie)
					rotList.push_back(weenie);
			}
		}

		if (!hasValidChildren)
		{
			//make the leftovers rot.
			for (auto entry : rotList)
			{
				entry->m_Qualities.SetInstanceID(GENERATOR_IID, 0);
				entry->_timeToRot = Timer::cur_time + 300.0; //in 5 minutes
				entry->_beganRot = false;
				entry->m_Qualities.SetFloat(TIME_TO_ROT_FLOAT, _timeToRot);
			}

			//we're the child of a generator and all our children have been destroyed/picked up.
			//so we're done and should cease to exist.
			if (!IsCreature())
			{
				MarkForDestroy();
			}
		}
	}
}

void CWeenieObject::GeneratorRemoveNode(uint32_t weenie_id, RegenerationType flags)
{
	if (!weenie_id || !m_Qualities._generator_registry)
		return;

	GeneratorRegistryNode *node = m_Qualities._generator_registry->_registry.lookup(weenie_id);

	if (node)
	{
		GeneratorRegistryNode oldNode = *node;
		m_Qualities._generator_registry->_registry.remove(weenie_id);

		double delay = -1.0;
		if (m_Qualities._generator_table)
		{
			// find this profile slot
			for (auto &profile : m_Qualities._generator_table->_profile_list)
			{
				if (profile.slot == oldNode.slot && (profile.whenCreate & flags) != 0)
				{
					delay = profile.delay * g_pConfig->RespawnTimeMultiplier();
					break;
				}
			}
		}

		if (delay >= 0)
		{
			if (!m_Qualities._generator_queue)
				m_Qualities._generator_queue = new GeneratorQueue();

			GeneratorQueueNode queueNode;
			queueNode.slot = oldNode.slot;
			queueNode.when = Timer::cur_time + delay;
			m_Qualities._generator_queue->_queue.push_back(queueNode);

			if (_nextRegen < 0.0)
				_nextRegen = Timer::cur_time + (InqFloatQuality(REGENERATION_INTERVAL_FLOAT, 0.0, TRUE) * g_pConfig->RespawnTimeMultiplier());
		}
	}
}

void CWeenieObject::GeneratorAddToRegistry(CWeenieObject *source, const GeneratorProfile &profile)
{
	if (!m_Qualities._generator_registry)
		m_Qualities._generator_registry = new GeneratorRegistry();

	GeneratorRegistryNode node;
	node.m_wcidOrTtype = profile.type;
	node.slot = profile.slot;
	node.ts = Timer::cur_time;
	node.amount = source->InqIntQuality(STACK_SIZE_INT, 1);
	node.checkpointed = 0;
	node.m_bTreasureType = profile.IsTreasureType();
	node.shop = 0;
	node.m_objectId = source->GetID();

	m_Qualities._generator_registry->_registry.add(source->GetID(), &node);
	m_GeneratorSpawns[source->GetID()] = source->m_Qualities.id;

	source->ChanceExecuteEmoteSet(GetID(), Generation_EmoteCategory);

	//Linkable generators do not have a _nextRegen time when the generator_queue is empty
	//and they have spawns in the world. Regular generators should not have a _nextRegen
	//time if they have more spawns than MAX_GENERATED_OBJECTS_INT.
	int numSpawned = m_Qualities._generator_registry->_registry.size();
	int maxSpawn = InqIntQuality(MAX_GENERATED_OBJECTS_INT, 0, TRUE);

	bool queueEmpty = (!m_Qualities._generator_queue || m_Qualities._generator_queue->_queue.empty());
	if (numSpawned >= maxSpawn && (maxSpawn > 0 || queueEmpty))
		_nextRegen = -1.0;
	else
		_nextRegen = Timer::cur_time + (InqFloatQuality(REGENERATION_INTERVAL_FLOAT, -1.0, TRUE) * g_pConfig->RespawnTimeMultiplier());
}

bool CWeenieObject::IsGeneratorSlotReady(int slot)
{
	if (m_Qualities._generator_registry)
	{
		for (auto &entry : m_Qualities._generator_registry->_registry)
		{
			if (entry.second.slot == slot)
			{
				return false;
			}
		}
	}

	if (m_Qualities._generator_queue)
	{
		for (auto &entry : m_Qualities._generator_queue->_queue)
		{
			if (entry.slot == slot && entry.when > Timer::cur_time)
			{
				return false;
			}
		}
	}

	return true;
}

void CWeenieObject::InitCreateGenerator(double adjust)
{
	double delay = 0.0;
	if (m_Qualities.InqFloat(GENERATOR_INITIAL_DELAY_FLOAT, delay, TRUE))
	{
		// if this event started in the past, we need to adjust the delay to account for
		// the event start time, not this object becoming active
		delay += adjust;
		_nextRegen = Timer::cur_time + (delay * g_pConfig->RespawnTimeMultiplier());
	}
	else
	{
		UpdateGenerator(true);
	}
	//{
	//	if (g_pGameEventManager->IsEventStarted(eventString.c_str()))
	//		g_pWeenieFactory->AddFromGeneratorTable(this, true);
	//}
	//else
	//	g_pWeenieFactory->AddFromGeneratorTable(this, true);
}

void CWeenieObject::UpdateGenerator(bool init)
{
	if (ShouldDestroy()) return;
	if (!m_Qualities._generator_table) return;
	if (InqBoolQuality(GENERATOR_DISABLED_BOOL, FALSE) == TRUE) return;

	GeneratorTimeType timeType = (GeneratorTimeType)InqIntQuality(GENERATOR_TIME_TYPE_INT, 0, TRUE);

	// REGENERATION_TIMESTAMP_FLOAT
	// GENERATOR_UPDATE_TIMESTAMP_FLOAT

	bool reset = false;

	double lastUpdate;
	switch (timeType)
	{
	case Night_GeneratorTimeType:
	case Day_GeneratorTimeType:
		//reset = !GameTime::IsNight();
		reset = GameTime::IsNight();
		if (timeType == Night_GeneratorTimeType)
			reset = !reset;

		lastUpdate = InqFloatQuality(GENERATOR_UPDATE_TIMESTAMP_FLOAT, 0, TRUE);
		if (lastUpdate < GameTime::LastCycleChange())
		{
			// set our nextRegen time a short time in the future
			_nextRegen = Timer::cur_time + Random::GenFloat(0.001, 1.0);
			m_Qualities.SetFloat(GENERATOR_UPDATE_TIMESTAMP_FLOAT, GameTime::LastCycleChange());
		}
		break;
	case RealTime_GeneratorTimeType: //These would run from GeneratorStartTime to GeneratorEndTime, specified as real world Unix epoch times, not implemented yet. For example the eventmadcowgen generator would generate mad cows from April 1, 2003 5:01:00 AM to April 2, 2003 4:59:00 AM.
		return;
	}

	if (reset)
	{
		DestroyGenerated();
		_nextRegen = -1;
		return;
	}

	if (init)
	{
		g_pWeenieFactory->AddFromGeneratorTable(this, true);
		return;
	}

	// not time to spawn more
	if (/*_nextRegen < 0 || */_nextRegen > Timer::cur_time)
		return;

	double regenInterval = 0.0;
	if (!m_Qualities.InqFloat(REGENERATION_INTERVAL_FLOAT, regenInterval) || !(regenInterval > 0.0))
	{
		// we don't regen, so don't do any more checks
		return;
	}

	int maxSpawn = InqIntQuality(MAX_GENERATED_OBJECTS_INT, 0, TRUE);

	int numSpawned = m_GeneratorSpawns.size();

	//check if the number spawned is higher than the max_generated_objects. Linkable generators have a max of 0, so check if there is an existing _generator_queue instead.
	if (numSpawned < maxSpawn || maxSpawn == 0)
	{
		bool emptyQueue = !m_Qualities._generator_queue || m_Qualities._generator_queue->_queue.empty();

		if (!emptyQueue)
		{
			PackableList<GeneratorQueueNode> &queue = m_Qualities._generator_queue->_queue;
			for (auto entry = queue.begin(); entry != queue.end();)
			{
				if (entry->when <= Timer::cur_time)
				{
					entry = queue.erase(entry);
					continue;
				}

				entry++;
			}
		}

		numSpawned = g_pWeenieFactory->AddFromGeneratorTable(this, false);
	}
}

void CWeenieObject::GenerateOnDemand(int amount) //Called by Emote_Type 72 Generator
{
	if (amount != 0)
	{
		for (int i = 0; i < amount; i++)
			g_pWeenieFactory->AddFromGeneratorTable(this, false);
	}
	else if (m_Qualities._generator_registry)
	{
		while (!m_Qualities._generator_registry->_registry.empty())
		{
			uint32_t weenie_id = m_Qualities._generator_registry->_registry.begin()->first;

			if (CWeenieObject *spawned_weenie = g_pWorld->FindObject(weenie_id))
			{
				//erase weenie from generator registry to stop spawning new weenies.
				m_Qualities._generator_registry->_registry.erase(weenie_id);
			}
		}
		if (m_Qualities._generator_queue)
		{
			m_Qualities._generator_queue->_queue.clear();
		}
	}
}

void CWeenieObject::CheckEventState(std::string eventName, GameEventDef *event)
{
	std::string eventString;
	if (!m_Qualities.InqString(GENERATOR_EVENT_STRING, eventString))
		return;

	if (event->IsStarted())
	{
		double start = event->GetStarted();
		TaskSchedule(Random::GenFloat(0.001, 1.0), [&]()
		{
			m_Qualities.SetBool(GENERATOR_DISABLED_BOOL, FALSE);
			InitCreateGenerator(start - g_pGlobals->Time());
		});
	}
	else
	{
		m_Qualities.SetBool(GENERATOR_DISABLED_BOOL, TRUE);
		TaskSchedule(Random::GenFloat(0.001, 1.0), [&]()
		{
			DestroyGenerated();
		});
	}
}

void CWeenieObject::DestroyGenerated()
{
	int destroy = 0;
	m_Qualities.InqInt(GENERATOR_END_DESTRUCTION_TYPE_INT, destroy);

	if (m_Qualities._generator_registry)
	{
		auto reg = m_Qualities._generator_registry->_registry;
		auto itr = reg.begin();
		while (itr != reg.end())
		{
			uint32_t id = itr->first;
			CWeenieObject * weenie = g_pWorld->FindObject(id);
			if (weenie && ((GeneratorDestruct)destroy == Kill_GeneratorDestruct || (GeneratorDestruct)destroy == Destroy_GeneratorDestruct))
			{
				weenie->MarkForDestroy();
			}

			reg.remove(id);
			m_GeneratorSpawns.erase(id);
			itr = reg.begin();
		}
	}

	if (m_Qualities._generator_queue)
	{
		m_Qualities._generator_queue->_queue.clear();
	}
}

void CWeenieObject::HandleEventActive()
{
	// if we have any spawns, we shouldn't be needing activation
	if (m_Qualities._generator_registry && !m_Qualities._generator_registry->_registry.empty())
		return;
	if (m_Qualities._generator_queue && !m_Qualities._generator_queue->_queue.empty())
		return;

	g_pWeenieFactory->AddFromGeneratorTable(this, false);
}

void CWeenieObject::HandleEventInactive()
{
	if (m_Qualities._generator_registry)
	{
		while (!m_Qualities._generator_registry->_registry.empty())
		{
			uint32_t weenie_id = m_Qualities._generator_registry->_registry.begin()->first;

			if (CWeenieObject *spawned_weenie = g_pWorld->FindObject(weenie_id))
			{
				spawned_weenie->MarkForDestroy();
			}

			// make sure it's gone (it should be already.)
			m_Qualities._generator_registry->_registry.erase(weenie_id);
		}
	}

	if (m_Qualities._generator_queue)
	{
		m_Qualities._generator_queue->_queue.clear();
	}
}
