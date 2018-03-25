
#include "StdAfx.h"
#include "ServerObjectMaint.h"
#include "PhysicsObj.h"
#include "World.h"

void CServerObjectMaint::GotoLostCell(class CPhysicsObj *, unsigned long)
{
}

void CServerObjectMaint::RemoveFromLostCell(class CPhysicsObj *)
{
}

void CServerObjectMaint::AddObjectToBeDestroyed(unsigned long)
{
}

void CServerObjectMaint::RemoveObjectToBeDestroyed(unsigned long)
{
}

CPhysicsObj *CServerObjectMaint::GetObject(DWORD object_id)
{
	return g_pWorld->FindObject(object_id);
}
