
#include <StdAfx.h>
#include "ObjectMaint.h"
#include "PhysicsObj.h"

CObjectMaint::CObjectMaint() : object_table(128), null_object_table(16), weenie_object_table(128), null_weenie_object_table(16), object_inventory_table(32)
{
}

void CObjectMaint::AddObject(CPhysicsObj *object)
{
	object_table.add(object);
}

void CObjectMaint::GotoLostCell(class CPhysicsObj *, uint32_t)
{
	// UNFINISHED();
}

void CObjectMaint::RemoveFromLostCell(class CPhysicsObj *)
{
	// UNFINISHED();
}

void CObjectMaint::AddObjectToBeDestroyed(uint32_t)
{
	// UNFINISHED();
}

void CObjectMaint::RemoveObjectToBeDestroyed(uint32_t)
{
	// UNFINISHED();
}

CPhysicsObj *CObjectMaint::GetObject(uint32_t object_id)
{
	return object_table.lookup(object_id);
}
