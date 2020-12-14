
#include <StdAfx.h>
#include "ServerObjectMaint.h"
#include "PhysicsObj.h"
#include "World.h"

void CServerObjectMaint::GotoLostCell(class CPhysicsObj *, uint32_t)
{
}

void CServerObjectMaint::RemoveFromLostCell(class CPhysicsObj *)
{
}

void CServerObjectMaint::AddObjectToBeDestroyed(uint32_t)
{
}

void CServerObjectMaint::RemoveObjectToBeDestroyed(uint32_t)
{
}

CPhysicsObj *CServerObjectMaint::GetObject(uint32_t object_id)
{
	return g_pWorld->FindObject(object_id);
}
