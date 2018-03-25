
#include "StdAfx.h"
#include "ObjectMaint.h"
#include "PhysicsObj.h"

CObjectMaint::CObjectMaint() : object_table(128), null_object_table(16), weenie_object_table(128), null_weenie_object_table(16), object_inventory_table(32)
{
}

void CObjectMaint::AddObject(CPhysicsObj *object)
{
	object_table.add(object);
}

void CObjectMaint::GotoLostCell(class CPhysicsObj *, unsigned long)
{
	// UNFINISHED();
}

void CObjectMaint::RemoveFromLostCell(class CPhysicsObj *)
{
	// UNFINISHED();
}

void CObjectMaint::AddObjectToBeDestroyed(unsigned long)
{
	// UNFINISHED();
}

void CObjectMaint::RemoveObjectToBeDestroyed(unsigned long)
{
	// UNFINISHED();
}

CPhysicsObj *CObjectMaint::GetObject(DWORD object_id)
{
	return object_table.lookup(object_id);
}
