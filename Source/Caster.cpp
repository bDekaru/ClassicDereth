
#include "StdAfx.h"
#include "Caster.h"

CCasterWeenie::CCasterWeenie()
{
}

void CCasterWeenie::ApplyQualityOverrides()
{
}

int CCasterWeenie::Use(CPlayerWeenie *other)
{
	return CWeenieObject::Use(other);
}