
#include <StdAfx.h>
#include "SoundDesc.h"

CSoundDesc::CSoundDesc()
{
}

CSoundDesc::~CSoundDesc()
{
	Destroy();
}

void CSoundDesc::Destroy()
{
	for (int i = 0; i < stb_desc.num_used; i++)
		delete stb_desc.array_data[i];

	stb_desc.num_used = 0;
}

DEFINE_PACK(CSoundDesc)
{
	UNFINISHED();
}

DEFINE_UNPACK(CSoundDesc)
{
	uint32_t numAmbientSTB = pReader->Read<uint32_t>();
	stb_desc.grow(numAmbientSTB);

	Destroy();
	for (uint32_t i = 0; i < numAmbientSTB; i++)
	{
		AmbientSTBDesc *ambient = new AmbientSTBDesc();
		ambient->UnPack(pReader);
		stb_desc.add(&ambient);
	}

	return true;
}

AmbientSTBDesc::AmbientSTBDesc()
{
}

AmbientSTBDesc::~AmbientSTBDesc()
{
	Destroy();
}

void AmbientSTBDesc::Destroy()
{
	for (int i = 0; i < ambient_sounds.num_used; i++)
		delete ambient_sounds.array_data[i];

	ambient_sounds.num_used = 0;
}

DEFINE_PACK(AmbientSTBDesc)
{
	UNFINISHED();
}

DEFINE_UNPACK(AmbientSTBDesc)
{
	stb_id = pReader->Read<uint32_t>();

	uint32_t num_sounds = pReader->Read<uint32_t>();
	ambient_sounds.grow(num_sounds);

	for (uint32_t i = 0; i < num_sounds; i++)
	{
		AmbientSoundDesc *sound = new AmbientSoundDesc();
		sound->stype = (SoundType) pReader->Read<int>();
		sound->volume = pReader->Read<float>();
		sound->base_chance = pReader->Read<float>();
		sound->is_continuous = sound->base_chance == 0.0f;
		sound->min_rate = pReader->Read<float>();
		sound->max_rate = pReader->Read<float>();
		ambient_sounds.add(&sound);
	}

	return true;
}

