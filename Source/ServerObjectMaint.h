#pragma once

class CPhysicsObj;

class CServerObjectMaint
{
public:
	void GotoLostCell(CPhysicsObj *, unsigned long);
	void RemoveFromLostCell(CPhysicsObj *);
	void AddObjectToBeDestroyed(unsigned long);
	void RemoveObjectToBeDestroyed(unsigned long);
	CPhysicsObj *GetObject(DWORD object_id);
};