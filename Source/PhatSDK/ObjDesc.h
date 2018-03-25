
#pragma once

#include "Packable.h"

class AnimPartChange : public PackObj, public LegacyPackObj
{
public:
	DECLARE_PACKABLE();

	AnimPartChange() { }
	AnimPartChange(unsigned int part_index_, DWORD part_id_) {
		part_index = part_index_;
		part_id = part_id_;
	}

	BOOL UnPack(BYTE **ppData, ULONG iSize);
	BOOL replaces(AnimPartChange *pChange);

	unsigned int part_index = 0;
	DWORD part_id = 0;
	AnimPartChange *prev = NULL;
	AnimPartChange *next = NULL;
};

class TextureMapChange : public PackObj
{
public:
	DECLARE_PACKABLE();

	TextureMapChange() { }
	TextureMapChange(unsigned int part_index_, DWORD old_tex_id_, DWORD new_tex_id_) {
		part_index = part_index_;
		old_tex_id = old_tex_id_;
		new_tex_id = new_tex_id_;
	}

	BOOL replaces(TextureMapChange *change);

	unsigned int part_index = 0;
	DWORD old_tex_id = 0;
	DWORD new_tex_id = 0;
	TextureMapChange *prev = NULL;
	TextureMapChange *next = NULL;
};

class Subpalette : public PackObj
{
public:
	DECLARE_PACKABLE();

	Subpalette() { }
	Subpalette(DWORD subID_, unsigned int offset_, unsigned int numcolors_) {
		subID = subID_;
		offset = offset_;
		numcolors = numcolors_;
	}

	BOOL supercedes(Subpalette *change);
	BOOL replaces(Subpalette *change);

	DWORD subID = 0;
	unsigned int offset = 0;
	unsigned int numcolors = 0;
	Subpalette *prev = NULL;
	Subpalette *next = NULL;
};

class VisualDesc : public PackObj
{
public:
	VisualDesc() { }
	virtual ~VisualDesc() { }
};

class ObjDesc : public VisualDesc
{
public:
	ObjDesc(const ObjDesc &other);
	ObjDesc();
	virtual ~ObjDesc();
	
	void Clear();
	void Wipe();

	DECLARE_PACKABLE();
	
	ObjDesc &operator=(const ObjDesc& rhs);
	ObjDesc &operator+=(const ObjDesc& rhs);

	void RemoveSubpalette(Subpalette *toRemove);
	void RemoveDuplicateSubpalette(Subpalette *newGuy);
	BOOL AddSubpalette(Subpalette *_subpal);

	void RemoveTextureMapChange(TextureMapChange *toRemove);
	void RemoveDuplicateTextureMapChange(TextureMapChange *newGuy);
	BOOL AddTextureMapChange(TextureMapChange *_texChange);

	void RemoveAnimPartChange(AnimPartChange *toRemove);
	void RemoveDuplicateAnimPartChange(AnimPartChange *newGuy);
	BOOL AddAnimPartChange(AnimPartChange *_partChange);

	void AddSubpalettes(std::list<Subpalette *> subpals); // custom
	std::list<Subpalette *> GetSubpalettes(unsigned int rangeStart, unsigned int rangeLength); // custom
	void AddTextureMapChanges(std::list<TextureMapChange *> TMchanges); // custom
	std::list<TextureMapChange *> GetTextureMapChanges(int part_index); // custom
	AnimPartChange *GetAnimPartChange(int part_index); // custom
	bool ContainsAnimPartChange(int part_index); // custom
	bool ContainsAnimPartChange(int part_index, DWORD part_id); // custom
	bool ContainsTextureMapChange(int part_index, DWORD new_tex_id); // custom
	bool ContainsTextureMapChange(int part_index, DWORD old_tex_id, DWORD new_tex_id); // custom
	bool ContainsSubpalette(DWORD subid, DWORD offset, DWORD numcolors); // custom
	void ClearSubpalettes(); // custom

	DWORD paletteID = 0;
	Subpalette *firstSubpal = NULL;
	Subpalette *lastSubpal = NULL;
	int num_subpalettes = 0;
	TextureMapChange *firstTMChange = NULL;
	TextureMapChange *lastTMChange = NULL;
	int num_texture_map_changes = 0;
	AnimPartChange *firstAPChange = NULL;
	AnimPartChange *lastAPChange = NULL;
	int num_anim_part_changes = 0;
};
