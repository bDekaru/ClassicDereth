
#include <StdAfx.h>
#include "ObjDesc.h"

BOOL Subpalette::supercedes(Subpalette *change)
{
	return !offset && numcolors == 2048 && (change->offset || change->numcolors != 2048);
}

BOOL Subpalette::replaces(Subpalette *change)
{
	return (offset == change->offset && numcolors == change->numcolors) || (!offset && numcolors == 2048);
}

DEFINE_PACK(Subpalette)
{
	pWriter->Pack_AsDataIDOfKnownType(0x04000000, subID);
	pWriter->Write<BYTE>((BYTE)(offset >> 3));
	pWriter->Write<BYTE>((BYTE)(numcolors >> 3));
}

DEFINE_UNPACK(Subpalette)
{
	subID = pReader->Unpack_AsDataIDOfKnownType(0x04000000);
	offset = pReader->Read<BYTE>() * 8;
	numcolors = pReader->Read<BYTE>();
	if (!numcolors)
		numcolors = 256;
	numcolors *= 8;
	return true;
}

BOOL TextureMapChange::replaces(TextureMapChange *change)
{
	return part_index == change->part_index && old_tex_id == change->old_tex_id;
}

DEFINE_PACK(TextureMapChange)
{
	pWriter->Write<BYTE>(part_index);
	pWriter->Pack_AsDataIDOfKnownType(0x05000000, old_tex_id);
	pWriter->Pack_AsDataIDOfKnownType(0x05000000, new_tex_id);
}

DEFINE_UNPACK(TextureMapChange)
{
	part_index = pReader->Read<BYTE>();
	old_tex_id = pReader->Unpack_AsDataIDOfKnownType(0x05000000);
	new_tex_id = pReader->Unpack_AsDataIDOfKnownType(0x05000000);
	return true;
}

DEFINE_PACK(AnimPartChange)
{
	pWriter->Write<BYTE>(part_index);
	pWriter->Pack_AsDataIDOfKnownType(0x01000000, part_id);
}

DEFINE_UNPACK(AnimPartChange)
{
	part_index = pReader->Read<BYTE>();
	part_id = pReader->Unpack_AsDataIDOfKnownType(0x01000000);
	return true;
}

BOOL AnimPartChange::UnPack(BYTE **ppData, ULONG iSize)
{
	// old style unpacking
	if (iSize < 3)
		return FALSE;

	UNPACK(BYTE, part_index);
	UNPACK(WORD, part_id);
	return TRUE;
}

BOOL AnimPartChange::replaces(AnimPartChange *pChange)
{
	return (part_index == pChange->part_index) ? TRUE : FALSE;
}

void ObjDesc::RemoveSubpalette(Subpalette *toRemove)
{
	if (!toRemove)
		return;

	Subpalette *pNext = toRemove->next;
	if (pNext)
		pNext->prev = toRemove->prev;
	else
		lastSubpal = toRemove->prev;

	Subpalette *pPrev = toRemove->prev;
	if (pPrev)
		pPrev->next = toRemove->next;
	else
		firstSubpal = toRemove->next;

	delete toRemove;
	num_subpalettes--;
}

void ObjDesc::RemoveDuplicateSubpalette(Subpalette *newGuy)
{
	Subpalette *pIterator = firstSubpal;

	while (pIterator)
	{
		Subpalette *pNext = pIterator->next;

		if (newGuy->replaces(pIterator))
		{
			RemoveSubpalette(pIterator);
		}

		pIterator = pNext;
	}
}

BOOL ObjDesc::AddSubpalette(Subpalette *_subpal)
{
	if (!_subpal)
		return FALSE;

	Subpalette *pIterator = firstSubpal;

	while (pIterator)
	{
		if (pIterator->supercedes(_subpal))
		{
			delete _subpal;
			return TRUE;
		}

		pIterator = pIterator->next;
	}

	RemoveDuplicateSubpalette(_subpal);
	if (num_subpalettes == 255)
	{
		delete _subpal;
		return FALSE;
	}

	Subpalette *last = lastSubpal;
	if (last)
	{
		_subpal->prev = last;
		lastSubpal->next = _subpal;
	}
	else
	{
		_subpal->prev = NULL;
		firstSubpal = _subpal;
	}

	_subpal->next = NULL;
	lastSubpal = _subpal;
	num_subpalettes++;

	return TRUE;
}

void ObjDesc::RemoveTextureMapChange(TextureMapChange *toRemove)
{
	if (!toRemove)
		return;

	TextureMapChange *pNext = toRemove->next;
	if (pNext)
		pNext->prev = toRemove->prev;
	else
		lastTMChange = toRemove->prev;

	TextureMapChange *pPrev = toRemove->prev;
	if (pPrev)
		pPrev->next = toRemove->next;
	else
		firstTMChange = toRemove->next;

	delete toRemove;
	num_texture_map_changes--;
}

void ObjDesc::RemoveDuplicateTextureMapChange(TextureMapChange *newGuy)
{
	TextureMapChange *pIterator = firstTMChange;

	while (pIterator)
	{
		TextureMapChange *next = pIterator->next;
		if (newGuy->replaces(pIterator))
			RemoveTextureMapChange(pIterator);

		pIterator = next;
	}
}

BOOL ObjDesc::AddTextureMapChange(TextureMapChange *_texChange)
{
	if (!_texChange)
		return FALSE;

	RemoveDuplicateTextureMapChange(_texChange);
	if (num_texture_map_changes == 255)
	{
		delete _texChange;
		return FALSE;
	}

	if (lastTMChange)
	{
		_texChange->prev = lastTMChange;
		lastTMChange->next = _texChange;
	}
	else
	{
		_texChange->prev = NULL;
		firstTMChange = _texChange;
	}

	_texChange->next = NULL;
	lastTMChange = _texChange;
	num_texture_map_changes++;
	return TRUE;
}


void ObjDesc::RemoveAnimPartChange(AnimPartChange *toRemove)
{
	if (!toRemove)
		return;

	AnimPartChange *pNext = toRemove->next;
	if (pNext)
		pNext->prev = toRemove->prev;
	else
		lastAPChange = toRemove->prev;

	AnimPartChange *pPrev = toRemove->prev;
	if (pPrev)
		pPrev->next = toRemove->next;
	else
		firstAPChange = toRemove->next;

	delete toRemove;
	num_anim_part_changes--;
}

void ObjDesc::RemoveDuplicateAnimPartChange(AnimPartChange *newGuy)
{
	AnimPartChange *pIterator = firstAPChange;

	while (pIterator)
	{
		AnimPartChange *next = pIterator->next;
		if (pIterator->part_index == newGuy->part_index)
			RemoveAnimPartChange(pIterator);

		pIterator = next;

	}
}

AnimPartChange *ObjDesc::GetAnimPartChange(int part_index) // custom
{
	AnimPartChange *partChange = firstAPChange;

	while (partChange)
	{
		if (partChange->part_index == part_index)
			return partChange;

		partChange = partChange->next;
	}

	return NULL;
}

std::list<Subpalette *> ObjDesc::GetSubpalettes(unsigned int rangeStart, unsigned int rangeLength) // custom
{
	std::list<Subpalette *> results;

	uint32_t rangeEnd = rangeLength;

	Subpalette *subpal = firstSubpal;
	while (subpal)
	{		
		uint32_t palStart = subpal->offset >> 3;
		uint32_t palEnd = palStart + (subpal->numcolors >> 3);

		if (palStart < rangeEnd && palEnd > rangeStart) // get anything that intersects the range
			results.push_back(subpal);

		subpal = subpal->next;
	}

	return results;
}

std::list<TextureMapChange *> ObjDesc::GetTextureMapChanges(int part_index) // custom
{
	std::list<TextureMapChange *> results;

	TextureMapChange *tmChange = firstTMChange;

	while (tmChange)
	{
		if (tmChange->part_index == part_index)
			results.push_back(tmChange);

		tmChange = tmChange->next;
	}

	return results;
}

bool ObjDesc::ContainsAnimPartChange(int part_index) // custom
{
	AnimPartChange *partChange = firstAPChange;

	while (partChange)
	{
		if (partChange->part_index == part_index)
			return true;

		partChange = partChange->next;
	}

	return false;
}

bool ObjDesc::ContainsAnimPartChange(int part_index, uint32_t part_id) // custom
{
	AnimPartChange *partChange = firstAPChange;

	while (partChange)
	{
		if (partChange->part_index == part_index && partChange->part_id == part_id)
			return true;

		partChange = partChange->next;
	}

	return false;
}

bool ObjDesc::ContainsTextureMapChange(int part_index, uint32_t old_tex_id, uint32_t new_tex_id) // custom
{
	TextureMapChange *tmChange = firstTMChange;

	while (tmChange)
	{
		if (tmChange->part_index == part_index)
		{
			if (tmChange->old_tex_id == old_tex_id && tmChange->new_tex_id == new_tex_id)
				return true;
		}

		tmChange = tmChange->next;
	}

	return false;
}


bool ObjDesc::ContainsTextureMapChange(int part_index, uint32_t new_tex_id) // custom
{
	TextureMapChange *tmChange = firstTMChange;

	while (tmChange)
	{
		if (tmChange->part_index == part_index)
		{
			if (tmChange->new_tex_id == new_tex_id)
				return true;
		}

		tmChange = tmChange->next;
	}

	return false;
}


bool ObjDesc::ContainsSubpalette(uint32_t subid, uint32_t offset, uint32_t numcolors) // custom
{
	Subpalette *subpal = firstSubpal;

	uint32_t rangeEnd = offset + numcolors;

	while (subpal)
	{
		uint32_t subpalStart = subpal->offset;
		uint32_t subpalEnd = subpalStart + subpal->numcolors;

		if (subpal->subID == subid && subpalStart <= offset && subpalEnd >= rangeEnd)
			return true;

		subpal = subpal->next;
	}

	return false;
}

void ObjDesc::ClearSubpalettes() // custom
{
	while (firstSubpal)
	{
		Subpalette *pDelete = firstSubpal;
		firstSubpal = pDelete->next;
		delete pDelete;
	}

	firstSubpal = NULL;
	lastSubpal = NULL;
	num_subpalettes = 0;
}

BOOL ObjDesc::AddAnimPartChange(AnimPartChange *_partChange)
{
	if (!_partChange)
		return FALSE;

	//RemoveDuplicateAnimPartChange(_partChange);
	if (num_anim_part_changes == 255)
	{
		delete _partChange;
		return FALSE;
	}

	AnimPartChange *change = firstAPChange;
	while (change != nullptr)
	{
		if (change->part_index == _partChange->part_index)
		{
			change->part_id = _partChange->part_id;
			delete _partChange;
			return TRUE;
		}
		change = change->next;
	}

	if (lastAPChange)
	{
		_partChange->prev = lastAPChange;
		lastAPChange->next = _partChange;
	}
	else
	{
		_partChange->prev = NULL;
		firstAPChange = _partChange;
	}

	_partChange->next = NULL;
	lastAPChange = _partChange;
	num_anim_part_changes++;
	return TRUE;
}

ObjDesc::ObjDesc(const ObjDesc &other)
{
	*this += other;
}

ObjDesc::ObjDesc()
{
}

ObjDesc::~ObjDesc()
{
	Clear();
}

void ObjDesc::Clear()
{
	while (firstSubpal)
	{
		Subpalette *pDelete = firstSubpal;
		firstSubpal = pDelete->next;
		delete pDelete;
	}

	while (firstTMChange)
	{
		TextureMapChange *pDelete = firstTMChange;
		firstTMChange = pDelete->next;
		delete pDelete;
	}

	while (firstAPChange)
	{
		AnimPartChange *pDelete = firstAPChange;
		firstAPChange = pDelete->next;
		delete pDelete;
	}

	lastSubpal = NULL;
	firstSubpal = NULL;
	lastTMChange = NULL;
	firstTMChange = NULL;
	lastAPChange = NULL;
	firstAPChange = NULL;
	num_subpalettes = 0;
	num_texture_map_changes = 0;
	num_anim_part_changes = 0;
}

void ObjDesc::Wipe()
{
	while (firstSubpal)
	{
		RemoveSubpalette(firstSubpal);
	}
	while (firstTMChange)
	{
		RemoveTextureMapChange(firstTMChange);
	}
	while (firstAPChange)
	{
		RemoveAnimPartChange(firstAPChange);
	}
}

DEFINE_PACK(ObjDesc)
{
	pWriter->Write<BYTE>(0x11);
	pWriter->Write<BYTE>(num_subpalettes);
	pWriter->Write<BYTE>(num_texture_map_changes);
	pWriter->Write<BYTE>(num_anim_part_changes);

	if (num_subpalettes > 0) // && firstSubpal
	{
		pWriter->Pack_AsDataIDOfKnownType(0x04000000, paletteID);

		Subpalette *pPal = firstSubpal;
		for (int i = 0; i < num_subpalettes; i++)
		{
			pPal->Pack(pWriter);
			pPal = pPal->next;
		}
	}

	if (num_texture_map_changes > 0) // && firstTMChange
	{
		TextureMapChange *pTex = firstTMChange;
		for (int i = 0; i < num_texture_map_changes; i++)
		{
			pTex->Pack(pWriter);
			pTex = pTex->next;
		}
	}

	if (num_anim_part_changes > 0) // && firstAPChange
	{
		AnimPartChange *pAP = firstAPChange;
		for (int i = 0; i < num_anim_part_changes; i++)
		{
			pAP->Pack(pWriter);
			pAP = pAP->next;
		}
	}

	pWriter->Align();
}

DEFINE_UNPACK(ObjDesc)
{
	Wipe();

	if (pReader->Read<BYTE>() != 0x11)
		return false;

	BYTE numSubpalettes = pReader->Read<BYTE>();
	BYTE numTMCs = pReader->Read<BYTE>();
	BYTE numAPCs = pReader->Read<BYTE>();

	if (numSubpalettes > 0)
	{
		paletteID = pReader->Unpack_AsDataIDOfKnownType(0x04000000);

		for (uint32_t i = 0; i < numSubpalettes; i++)
		{
			Subpalette *pSubpalette = new Subpalette();
			if (!pSubpalette->UnPack(pReader))
			{
				delete pSubpalette;
				return false;
			}

			AddSubpalette(pSubpalette);
		}
	}

	for (uint32_t i = 0; i < numTMCs; i++)
	{
		TextureMapChange *pTMC = new TextureMapChange();
		if (!pTMC->UnPack(pReader))
		{
			delete pTMC;
			return false;
		}

		AddTextureMapChange(pTMC);
	}

	for (uint32_t i = 0; i < numAPCs; i++)
	{
		AnimPartChange *pAPC = new AnimPartChange();
		if (!pAPC->UnPack(pReader))
		{
			delete pAPC;
			return false;
		}

		AddAnimPartChange(pAPC);
	}

	pReader->ReadAlign();
	return true;
}

ObjDesc &ObjDesc::operator=(const ObjDesc& rhs)
{
	Clear();

	paletteID = rhs.paletteID;
	*this += rhs;

	return *this;
}

ObjDesc &ObjDesc::operator+=(const ObjDesc& rhs)
{
	auto rhs_pal = rhs.firstSubpal;
	for (int i = 0; i < rhs.num_subpalettes; i++)
	{
		AddSubpalette(new Subpalette(*rhs_pal));
		rhs_pal = rhs_pal->next;
	}

	auto rhs_tex = rhs.firstTMChange;
	for (int i = 0; i < rhs.num_texture_map_changes; i++)
	{
		AddTextureMapChange(new TextureMapChange(*rhs_tex));
		rhs_tex = rhs_tex->next;
	}

	auto rhs_part = rhs.firstAPChange;
	for (int i = 0; i < rhs.num_anim_part_changes; i++)
	{
		AddAnimPartChange(new AnimPartChange(*rhs_part));
		rhs_part = rhs_part->next;
	}

	return *this;
}

void ObjDesc::AddSubpalettes(std::list<Subpalette *> subpals)
{
	for (auto entry : subpals)
		AddSubpalette(new Subpalette(*entry));
}

void ObjDesc::AddTextureMapChanges(std::list<TextureMapChange *> TMchanges)
{
	for (auto entry : TMchanges)
		AddTextureMapChange(new TextureMapChange(*entry));
}


