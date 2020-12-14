#pragma once

class CPhysicsObj;

class CServerObjectMaint
{
public:
	void GotoLostCell(CPhysicsObj *, uint32_t);
	void RemoveFromLostCell(CPhysicsObj *);
	void AddObjectToBeDestroyed(uint32_t);
	void RemoveObjectToBeDestroyed(uint32_t);
	CPhysicsObj *GetObject(uint32_t object_id);
};