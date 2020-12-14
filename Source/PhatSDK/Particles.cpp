
#include <StdAfx.h>
#include "Particles.h"

void Particle::Init(CPhysicsObj *pOwner, uint32_t ParticleID,
	Frame *pFrame, CPhysicsPart *pPart, Vector *Offset,
	uint32_t Info2C, BOOL Persistant,
	Vector *RandomA, Vector *RandomB, Vector *RandomC,
	float StartScale, float FinalScale, float StartTrans, float FinalTrans,
	double Lifespan)
{

	if (Persistant)
		m_LastUpdate = PhysicsTimer::curr_time; // Huh?
	else
		m_LastUpdate = PhysicsTimer::curr_time;

	m_08 = Lifespan;
	m_FrameTime = 0.0;

	if (ParticleID == (uint32_t)-1)
		m_Frame = pOwner->m_Position.frame;
	else
		m_Frame = pOwner->part_array->parts[ParticleID]->pos.frame;

	// This is surely inlined, but from what?
	Vector translated_point = pFrame->m_origin + (*Offset);
	m_58.x = (m_Frame.m00 * translated_point.x) + (m_Frame.m10 * translated_point.y) + (m_Frame.m20 * translated_point.z);
	m_58.y = (m_Frame.m01 * translated_point.x) + (m_Frame.m11 * translated_point.y) + (m_Frame.m21 * translated_point.z);
	m_58.z = (m_Frame.m02 * translated_point.x) + (m_Frame.m12 * translated_point.y) + (m_Frame.m22 * translated_point.z);

	switch (Info2C)
	{
	case 1:
		// Do nothing.
		break;
	case 4:
		m_7C = *RandomC;
	case 3:
		m_70 = *RandomB;
	case 2:
		m_64 = m_Frame.localtoglobalvec(*RandomA);
		break;
	case 9:
		m_7C = m_Frame.localtoglobalvec(*RandomC);
	case 8:
		m_70 = m_Frame.localtoglobalvec(*RandomB);
		m_64 = m_Frame.localtoglobalvec(*RandomA);
		break;
	case 11:
		m_7C = *RandomC;
	case 10:
		m_70 = *RandomB;
	case 12:
		m_64 = *RandomA;
		break;
	case 5:
		m_64 = m_Frame.localtoglobalvec(*RandomA);
		m_70 = *RandomB;
		m_7C = *RandomC;
		break;
	case 7:
		m_64 = *RandomA;
		m_70 = *RandomB;
		m_58 *= *RandomC;
		m_7C = m_58;
		break;
	case 6:
		{
			m_64 = *RandomA;
			m_70 = *RandomB;

			float RA = (float) Random::RollDice(-3.1415927f, 3.1415927f);
			float PO = (float) Random::RollDice(-3.1415927f, 3.1415927f);
			float RB = cosf(PO);

			m_7C.x = cosf(RA) * RandomC->x * RB;
			m_7C.y = sinf(RA) * RandomC->y * RB;
			m_7C.z = sinf(PO) * RandomC->z * RB;

			if (m_7C.normalize_check_small())
				m_7C = Vector(0, 0, 0);
			break;
		}
	default:
		m_64 = *RandomA;
		m_70 = *RandomB;
		m_7C = *RandomC;
		break;
	}

	m_StartScale = StartScale;
	m_FinalScale = FinalScale;
	m_StartTrans = StartTrans;
	m_FinalTrans = FinalTrans;

	// Init the part scale to mine.
	pPart->gfxobj_scale = Vector(StartScale, StartScale, StartScale);

	// Init the part translucency to mine.
	pPart->SetTranslucency(StartTrans);

	Update(Info2C, Persistant, pPart, &m_Frame);
}

void Particle::Update(uint32_t Info2C, BOOL Persistant, CPhysicsPart *pPartObj, Frame *pFrame)
{
	float TimeSinceUpdate = PhysicsTimer::curr_time - m_LastUpdate;

	if (Persistant)
	{
		m_FrameTime += TimeSinceUpdate;
		m_LastUpdate = PhysicsTimer::curr_time;
	}
	else
	{
		m_FrameTime = TimeSinceUpdate;
	}

	switch (Info2C)
	{
	case 1:
		pPartObj->pos.frame.m_origin = pFrame->m_origin + m_58;
		break;
	case 2:
	case 12:
		pPartObj->pos.frame.m_origin.x = (m_FrameTime * m_64.x) + (pFrame->m_origin.x + m_58.x);
		pPartObj->pos.frame.m_origin.y = (m_FrameTime * m_64.y) + (pFrame->m_origin.y + m_58.y);
		pPartObj->pos.frame.m_origin.z = (m_FrameTime * m_64.z) + (pFrame->m_origin.z + m_58.z);
		break;
	case 3:
	case 8:
	case 10:
		pPartObj->pos.frame.m_origin.x = (m_FrameTime * m_FrameTime * (m_70.x / 2)) + (m_FrameTime * m_64.x) + (m_58.x + pFrame->m_origin.x);
		pPartObj->pos.frame.m_origin.y = (m_FrameTime * m_FrameTime * (m_70.y / 2)) + (m_FrameTime * m_64.y) + (m_58.y + pFrame->m_origin.y);
		pPartObj->pos.frame.m_origin.z = (m_FrameTime * m_FrameTime * (m_70.z / 2)) + (m_FrameTime * m_64.z) + (m_58.z + pFrame->m_origin.z);
		break;
	case 4:
	case 9:
	case 11:
		{
			Frame TempFrame = *pFrame; // var_40
			TempFrame.m_origin.x += (m_58.x + (m_FrameTime * m_64.x)) + (m_FrameTime * (m_FrameTime * (m_70.x / 2)));
			TempFrame.m_origin.y += (m_58.y + (m_FrameTime * m_64.y)) + (m_FrameTime * (m_FrameTime * (m_70.y / 2)));
			TempFrame.m_origin.z += (m_58.z + (m_FrameTime * m_64.z)) + (m_FrameTime * (m_FrameTime * (m_70.z / 2)));

			TempFrame.rotate(m_7C * m_FrameTime);
			pPartObj->pos.frame = TempFrame;
			break;
		}
	case 5:
		pPartObj->pos.frame.m_origin.x = pFrame->m_origin.x + m_58.x + (m_FrameTime * m_64.x) + (m_7C.x * cosf(m_70.x * m_FrameTime));
		pPartObj->pos.frame.m_origin.y = pFrame->m_origin.y + m_58.y + (m_FrameTime * m_64.y) + (m_7C.y * sinf(m_70.y * m_FrameTime));
		pPartObj->pos.frame.m_origin.z = pFrame->m_origin.z + m_58.z + (m_FrameTime * m_64.z) + (m_7C.z * cosf(m_70.z * m_FrameTime));
		break;
	case 7:
		pPartObj->pos.frame.m_origin.x = pFrame->m_origin.x + m_58.x + (m_70.x * m_FrameTime * m_FrameTime) + (m_7C.x * cosf(m_64.x * m_FrameTime));
		pPartObj->pos.frame.m_origin.y = pFrame->m_origin.y + m_58.y + (m_70.y * m_FrameTime * m_FrameTime) + (m_7C.y * cosf(m_64.x * m_FrameTime));
		pPartObj->pos.frame.m_origin.z = pFrame->m_origin.z + m_58.z + (m_70.z * m_FrameTime * m_FrameTime) + (m_7C.z * cosf(m_64.x * m_FrameTime));
		break;
	case 6:
		pPartObj->pos.frame.m_origin.x = pFrame->m_origin.x + m_58.x + (m_FrameTime * (m_64.x * m_7C.x) + (m_FrameTime * m_70.x));
		pPartObj->pos.frame.m_origin.y = pFrame->m_origin.y + m_58.y + (m_FrameTime * (m_64.y * m_7C.y) + (m_FrameTime * m_70.y));
		pPartObj->pos.frame.m_origin.z = pFrame->m_origin.z + m_58.z + (m_FrameTime * (m_64.z * m_7C.z) + (m_FrameTime * m_70.z));
		break;
	}

	double DeltaTime;

	if (m_FrameTime >= m_08)
		DeltaTime = 1.0f;
	else
		DeltaTime = m_FrameTime / m_08;

	float CurrentScale = m_StartScale + ((m_FinalScale - m_StartScale) * DeltaTime);
	float CurrentTrans = m_StartTrans + ((m_FinalTrans - m_StartTrans) * DeltaTime);

	pPartObj->gfxobj_scale = Vector(CurrentScale, CurrentScale, CurrentScale);
	pPartObj->SetTranslucency(CurrentTrans);
}

BOOL ParticleEmitter::always_use_software_particles = FALSE;

ParticleEmitter::ParticleEmitter(CPhysicsObj *pOwner)
	: m_Owner(pOwner), m_EmitterID(0), m_08(-1)
{
	m_EmitterObj = NULL;
	m_EmitterInfo = NULL;

	m_Particles = NULL;
	m_Parts58 = NULL;
	m_Parts5C = NULL;
	m_60 = 0;
	m_MaxDegradeDist = 0.0f;
	m_70 = 0;
	m_74 = 0;
	m_bStopEmitting = FALSE;
	m_90 = Timer::cur_time; // Timer::m_timeCurrent;
	m_78 = Timer::cur_time; // Timer::m_timeCurrent;
}

ParticleEmitter::~ParticleEmitter()
{
	Destroy();
}

void ParticleEmitter::Destroy()
{
	if (m_EmitterObj)
	{
		m_EmitterObj->unset_parent();
		m_EmitterObj->leave_world();
	}

	if (m_Parts58)
	{
		for (int32_t i = 0; i < m_EmitterInfo->m_48; i++)
		{
			CPhysicsPart *pPart = m_Parts58[i];

			if (pPart)
				delete pPart;

			m_Parts58[i] = NULL;
			m_Parts5C[i] = NULL;
		}

		delete[] m_Parts58;
	}

	if (m_Particles)
	{
		delete[] m_Particles;
		m_Particles = NULL;
	}

	if (m_EmitterInfo)
	{
		ParticleEmitterInfo::Release(m_EmitterInfo);
		m_EmitterInfo = NULL;
	}

	if (m_EmitterObj)
	{
		delete m_EmitterObj;
		m_EmitterObj = NULL;
	}

	m_Parts5C = NULL;
	m_70 = 0;
	m_90 = Timer::cur_time; // Timer::m_timeCurrent;
}

ParticleEmitter *ParticleEmitter::makeParticleEmitter(CPhysicsObj *pOwner)
{
	if (!pOwner)
		return NULL;

	return(new ParticleEmitter(pOwner));
}

BOOL ParticleEmitter::SetInfo(uint32_t InfoID)
{
	ParticleEmitterInfo *pInfo = ParticleEmitterInfo::Get(InfoID);

	if (!pInfo)
		return FALSE;

	return SetInfo(pInfo);
}

BOOL ParticleEmitter::SetInfo(ParticleEmitterInfo *pInfo)
{
	Destroy();

	m_EmitterInfo = pInfo;

	uint32_t GfxID;

	if (!always_use_software_particles)
		GfxID = pInfo->m_38;
	else
		GfxID = pInfo->m_34;

	if (!GfxID || !(m_EmitterObj = CPhysicsObj::makeParticleObject(pInfo->m_48, &pInfo->m_70)))
	{
		Destroy();
		return FALSE;
	}

	m_Origin = m_EmitterObj->m_Position.frame.m_origin;
	m_Parts5C = m_EmitterObj->part_array->parts;

	m_Parts58 = new CPhysicsPart*[m_EmitterInfo->m_48];

	if (!m_Parts58)
	{
		Destroy();
		return FALSE;
	}

	for (int32_t i = 0; i < m_EmitterInfo->m_48; i++)
		m_Parts58[i] = NULL;

	for (int32_t i = 0; i < m_EmitterInfo->m_48; i++)
	{
		m_Parts58[i] = CPhysicsPart::makePhysicsPart(GfxID);

		if (!m_Parts58[i])
		{
			Destroy();
			return FALSE;
		}

		if (ImgTex::DoChunkification())
		{
			UNFINISHED_LEGACY("m_Parts58[i]->NotifySurfaceTiles() call");
			// m_Parts58[i]->NotifySurfaceTiles();
		}
	}

	m_MaxDegradeDist = m_Parts58[0]->GetMaxDegradeDistance();
	m_Particles = new Particle[m_EmitterInfo->m_48];

	if (!m_Particles)
	{
		Destroy();
		return FALSE;
	}

	return TRUE;
}

BOOL ParticleEmitter::SetParenting(uint32_t b, Frame *c)
{
	if (!m_EmitterObj)
		return FALSE;

	if (!m_EmitterObj->set_parent(m_Owner, b, c))
		return FALSE;

	m_08 = b;
	m_Frame = *c;

	return TRUE;
}

BOOL ParticleEmitter::ShouldEmitParticle()
{
	Vector Offset;

	if (m_EmitterInfo->m_28 & 2)
		Offset = m_EmitterObj->m_Position.frame.m_origin - m_Origin;

	return m_EmitterInfo->ShouldEmitParticle(m_70, m_74, &Offset, m_78);
}

BOOL ParticleEmitter::StopEmitter()
{
	if (!m_bStopEmitting)
	{
		if ((m_EmitterInfo->m_58 > 0.0) && ((m_EmitterInfo->m_58 + m_68) < PhysicsTimer::curr_time))
			m_bStopEmitting = TRUE;

		if ((m_EmitterInfo->m_50) && (m_74 >= m_EmitterInfo->m_50))
			m_bStopEmitting = TRUE;
	}

	// We might not have stopped..
	return m_bStopEmitting;
}

BOOL ParticleEmitter::KillParticle(int32_t Index)
{
	Particle *pParticle = m_Particles + Index;

	if (pParticle->m_FrameTime < pParticle->m_08)
		return FALSE;

	m_EmitterObj->RemovePartFromShadowCells(m_Parts5C[Index]);
	m_Parts5C[Index] = NULL;
	m_70--;

	return TRUE;
}

void ParticleEmitter::RecordParticleEmission()
{
	m_70++;
	m_74++;

	m_Origin = m_EmitterObj->m_Position.frame.m_origin;
	m_78 = PhysicsTimer::curr_time;
}

BOOL ParticleEmitter::UpdateParticles()
{
	if (!m_EmitterInfo || !m_EmitterObj)
		return FALSE;

	//if ((m_EmitterObj->m_28 > m_MaxDegradeDist) || !m_EmitterObj->m_ObjCell /*|| !m_EmitterObj->m_ObjCell->UNKNOWNVIRUALFUNCTHATSHOULDRETURNTRUE()*/)
	if ((m_EmitterObj->CYpt > m_MaxDegradeDist))
	{
		if (!m_60)
		{
			m_EmitterObj->SetNoDraw(TRUE);
			m_60 = 1;
		}

		m_90 = PhysicsTimer::curr_time;

		if (m_EmitterInfo->IsPersistant())
		{
			for (int32_t i = 0; i < m_EmitterInfo->m_48; i++)
				m_Particles[i].m_LastUpdate = PhysicsTimer::curr_time;

			return TRUE;
		}
		else
		{
			for (int32_t i = 0; i < m_EmitterInfo->m_48; i++)
			{
				if (m_Parts5C[i])
				{
					m_Particles[i].m_FrameTime = PhysicsTimer::curr_time - m_Particles[i].m_LastUpdate;

					KillParticle(i);
				}
			}

			if (!m_bStopEmitting)
			{
				if (ShouldEmitParticle())
					RecordParticleEmission();

				StopEmitter();

				return TRUE;
			}
			else
				return (m_70 ? TRUE : FALSE);
		}
	}
	else
	{
		if (m_60)
		{
			m_60 = 0;
			m_EmitterObj->SetNoDraw(FALSE);
		}

		for (uint32_t i = 0; i < (unsigned)m_EmitterInfo->m_48; i++)
		{
			if (m_Parts5C[i])
			{
				Frame *pFrame;

				if (m_EmitterInfo->m_30)
				{
					if (m_08 == (uint32_t)-1)
						pFrame = &m_Owner->m_Position.frame;
					else
						pFrame = &m_Owner->part_array->parts[m_08]->pos.frame;
				}
				else
					pFrame = &m_Particles[i].m_Frame;

				m_Particles[i].Update(
					m_EmitterInfo->m_2C, m_EmitterInfo->IsPersistant(),
					m_Parts5C[i], pFrame);

				KillParticle(i);
			}
		}

		if (!m_bStopEmitting)
		{
			if (ShouldEmitParticle())
				EmitParticle();

			StopEmitter();
			m_90 = PhysicsTimer::curr_time;

			return TRUE;
		}
		else
		{
			m_90 = PhysicsTimer::curr_time;
			return (m_70 ? TRUE : FALSE);
		}
	}
}

ParticleManager::ParticleManager() : m_NoIDEmitters(0xFFFF0000), m_Emitters(2)
{
}

ParticleManager::~ParticleManager()
{
}

uint32_t ParticleManager::CreateParticleEmitter
(CPhysicsObj *pOwner, uint32_t a, int32_t b, Frame *c, uint32_t EmitterID)
{

	if (EmitterID)
	{
		ParticleEmitter *pOldEmitter = m_Emitters.remove(EmitterID);

		if (pOldEmitter)
			delete pOldEmitter;
	}

	ParticleEmitter *pEmitter = ParticleEmitter::makeParticleEmitter(pOwner);

	if (!pEmitter)
		return NULL;

	if (!pEmitter->SetInfo(a) || !pEmitter->SetParenting(b, c) || !pEmitter->InitEnd())
	{
		delete pEmitter;
		return NULL;
	}

	if (!EmitterID)
		EmitterID = m_NoIDEmitters++;

	// This ID can be used to reference this emitter.
	pEmitter->m_EmitterID = EmitterID;

	// Add emitter to collection.
	m_Emitters.add(pEmitter, EmitterID);

	return EmitterID;
}

void ParticleManager::UpdateParticles()
{
	LongNIHashIter<ParticleEmitter> it(&m_Emitters);

	while (!it.EndReached())
	{
		try
		{
			ParticleEmitter *pEmitter = it.GetCurrentData();
			it.Next();

			if (!pEmitter->UpdateParticles())
			{
				m_Emitters.remove(pEmitter->m_EmitterID);
				delete pEmitter;
			}
		}
		catch (...)
		{
			SERVER_ERROR << "Error in Update Particles";
		}
	}
}

BOOL ParticleEmitter::InitEnd()
{
	m_68 = Timer::cur_time; // Timer::m_timeCurrent

	for (int32_t i = 0; i < m_EmitterInfo->m_4C; i++)
		EmitParticle();

	return TRUE;
}

void ParticleEmitter::EmitParticle()
{
	int32_t i;
	for (i = 0; i < m_EmitterInfo->m_48; i++)
	{
		if (!m_Parts5C[i])
			break;
	}

	// All objects parts are allocated.
	if (i == m_EmitterInfo->m_48)
		return;

	// The part index 'i' is empty.
	m_Parts5C[i] = m_Parts58[i];

	// Nothing available to touch.
	if (!m_Parts5C[i])
		return;

	Vector RandomOffset, RandomA, RandomB, RandomC;

	BOOL Persistant = m_EmitterInfo->IsPersistant();

	m_Particles[i].Init(
		m_Owner, m_08, &m_Frame, m_Parts5C[i],
		m_EmitterInfo->GetRandomOffset(&RandomOffset),
		m_EmitterInfo->m_2C,
		Persistant, // This was inlined
		m_EmitterInfo->GetRandomA(&RandomA),
		m_EmitterInfo->GetRandomB(&RandomB),
		m_EmitterInfo->GetRandomC(&RandomC),
		m_EmitterInfo->GetRandomStartScale(),
		m_EmitterInfo->GetRandomFinalScale(),
		m_EmitterInfo->GetRandomStartTrans(),
		m_EmitterInfo->GetRandomFinalTrans(),
		m_EmitterInfo->GetRandomLifespan());

	m_EmitterObj->AddPartToShadowCells(m_Parts5C[i]);

	// This was inlined.
	RecordParticleEmission();
}

ParticleEmitterInfo::ParticleEmitterInfo()
{
}

ParticleEmitterInfo::~ParticleEmitterInfo()
{
}

DBObj* ParticleEmitterInfo::Allocator()
{
	return((DBObj *)new ParticleEmitterInfo());
}

void ParticleEmitterInfo::Destroyer(DBObj *pEmitterInfo)
{
	delete ((ParticleEmitterInfo *)pEmitterInfo);
}

ParticleEmitterInfo* ParticleEmitterInfo::Get(uint32_t ID)
{
	return (ParticleEmitterInfo *)ObjCaches::ParticleEmitterInfos->Get(ID);
}

void ParticleEmitterInfo::Release(ParticleEmitterInfo *pEmitterInfo)
{
	if (pEmitterInfo)
		ObjCaches::ParticleEmitterInfos->Release(pEmitterInfo->GetID());
}

BOOL ParticleEmitterInfo::UnPack(BYTE **ppData, ULONG iSize)
{
	UNPACK(uint32_t, id);

	uint32_t Reserved; // Skips 4 bytes.
	UNPACK(uint32_t, Reserved);

	UNPACK(uint32_t, m_28);
	UNPACK(uint32_t, m_2C);

	UNPACK(uint32_t, m_34);
	UNPACK(uint32_t, m_38);

	UNPACK(double, m_40);

	UNPACK(int32_t, m_48);
	UNPACK(int32_t, m_4C);
	UNPACK(int32_t, m_50);

	UNPACK(double, m_58);
	UNPACK(double, m_68);
	UNPACK(double, m_60);

	UNPACK_OBJ(m_80);

	UNPACK(float, m_8C);
	UNPACK(float, m_90);

	UNPACK_OBJ(m_94);

	UNPACK(float, m_B8);
	UNPACK(float, m_BC);

	UNPACK_OBJ(m_A0);

	UNPACK(float, m_C0);
	UNPACK(float, m_C4);

	UNPACK_OBJ(m_AC);

	UNPACK(float, m_C8);
	UNPACK(float, m_CC);

	UNPACK(float, m_D4);
	UNPACK(float, m_D8);

	UNPACK(float, m_D0);

	UNPACK(float, m_E0);
	UNPACK(float, m_E4);
	UNPACK(float, m_DC);
	UNPACK(uint32_t, m_30);

	float maxVal = (m_BC * m_68);

	// Sphere?
	m_70 = CSphere(Vector(0, 0, 0), ((m_90 > maxVal) ? m_90 : maxVal));

	return TRUE;
}

float ParticleEmitterInfo::GetRandomStartScale()
{
	float Scale = m_D4 + (Random::RollDice(-1.0, 1.0) * m_D0);

	if (Scale < MINIMUM_SCALE)
		return MINIMUM_SCALE;
	if (Scale > MAXIMUM_SCALE)
		return MAXIMUM_SCALE;

	return Scale;
}

float ParticleEmitterInfo::GetRandomFinalScale()
{
	float Scale = m_D8 + (Random::RollDice(-1.0, 1.0) * m_D0);

	if (Scale < MINIMUM_SCALE)
		return MINIMUM_SCALE;
	if (Scale > MAXIMUM_SCALE)
		return MAXIMUM_SCALE;

	return Scale;
}

float ParticleEmitterInfo::GetRandomStartTrans()
{
	float Trans = m_E0 + (Random::RollDice(-1.0, 1.0) * m_DC);

	if (Trans < MINIMUM_TRANS)
		return MINIMUM_TRANS;
	if (Trans > MAXIMUM_TRANS)
		return MAXIMUM_TRANS;

	return Trans;
}

float ParticleEmitterInfo::GetRandomFinalTrans()
{
	float Trans = m_E4 + (Random::RollDice(-1.0, 1.0) * m_DC);

	if (Trans < MINIMUM_TRANS)
		return MINIMUM_TRANS;
	if (Trans > MAXIMUM_TRANS)
		return MAXIMUM_TRANS;

	return Trans;
}

float ParticleEmitterInfo::GetRandomLifespan()
{
	float Lifespan = m_68 + (Random::RollDice(-1.0, 1.0) * m_60);

	if (Lifespan < MINUMUM_LIFESPAN)
		Lifespan = MINUMUM_LIFESPAN;

	return Lifespan;
}

Vector *ParticleEmitterInfo::GetRandomOffset(Vector *pVector)
{
	Vector RandomNormal(
		Random::RollDice(-1.0, 1.0),
		Random::RollDice(-1.0, 1.0),
		Random::RollDice(-1.0, 1.0));

	Vector RandomAngles = m_80 * m_80.dot_product(RandomNormal);
	Vector RandomOffset = RandomNormal - RandomAngles;

	// Scale the random offset.
	Vector RandomOffsetScaled;

	if (!RandomOffset.normalize_check_small())
		RandomOffsetScaled = RandomOffset * (m_8C + ((m_90 - m_8C) * Random::RollDice(0, 1.0)));
	else
		RandomOffsetScaled = Vector(0, 0, 0);

	*pVector = RandomOffsetScaled;
	return pVector;
}

Vector *ParticleEmitterInfo::GetRandomA(Vector *pVector)
{
	*pVector = m_94 * (m_B8 + ((m_BC - m_B8) * Random::RollDice(0, 1.0)));

	return pVector;
}

Vector *ParticleEmitterInfo::GetRandomB(Vector *pVector)
{
	*pVector = m_A0 * (m_C0 + ((m_C4 - m_C0) * Random::RollDice(0, 1.0)));

	return pVector;
}

Vector *ParticleEmitterInfo::GetRandomC(Vector *pVector)
{
	*pVector = m_AC * (m_C8 + ((m_CC - m_C8) * Random::RollDice(0, 1.0)));

	return pVector;
}

BOOL ParticleEmitterInfo::IsPersistant()
{
	if (!m_50 && (m_58 == 0.0))
		return TRUE;

	return FALSE;
}

BOOL ParticleEmitterInfo::ShouldEmitParticle(int32_t a, int32_t b, Vector *c, double d)
{
	if ((m_50 > 0) && (b >= m_50))
		return FALSE;

	if (a >= m_48)
		return FALSE;

	if (m_28 & 1)
	{
		if ((Timer::cur_time - d) > m_40) // Timer::m_timeCurrent
			return TRUE;
	}
	else
		if (m_28 & 2)
		{
			if ((m_40 * m_40) < (c->x*c->x + c->y*c->y + c->z*c->z))
				return TRUE;
		}

	return FALSE;
}

