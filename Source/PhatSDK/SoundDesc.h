
#pragma once

#include "Packable.h"
#include "SmartArray.h"

class AmbientSoundDesc
{
public:
	SoundType stype;
	int is_continuous;
	float volume;
	float base_chance;
	float min_rate;
	float max_rate;
};

class AmbientSTBDesc : public PackObj
{
public:
	AmbientSTBDesc();
	virtual ~AmbientSTBDesc();

	void Destroy();
	DECLARE_PACKABLE();

	DWORD stb_id = 0;
	int stb_not_found = 0;
	SmartArray<AmbientSoundDesc *> ambient_sounds;
	class CSoundTable *sound_table = NULL;
	unsigned int play_count = 0;
};

class CSoundDesc : public PackObj
{
public:
	CSoundDesc();
	virtual ~CSoundDesc();
	
	void Destroy();
	DECLARE_PACKABLE();

	SmartArray<AmbientSTBDesc *> stb_desc;
};
