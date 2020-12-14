
#pragma once

class CPhysicsObj;
class CObjectInventory;

class CObjectMaint
{
public:
	CObjectMaint();

	void AddObject(CPhysicsObj *object);
	void GotoLostCell(CPhysicsObj *, uint32_t);
	void RemoveFromLostCell(CPhysicsObj *);
	void AddObjectToBeDestroyed(uint32_t);
	void RemoveObjectToBeDestroyed(uint32_t);
	CPhysicsObj *GetObject(uint32_t object_id);

	// Turbine_RefCount m_cTurbineRefCount;
	// int is_active;
	std::map<uint32_t, class CLostCell *> lost_cell_table; // IntrusiveHashTable<uint32_t, CLostCell *, 0>
	LongHash<CPhysicsObj> object_table;
	LongHash<CPhysicsObj> null_object_table;
	LongHash<CWeenieObject> weenie_object_table;
	LongHash<CWeenieObject> null_weenie_object_table;
	std::set<uint32_t> visible_object_table; // HashSet<uint32_t>
	std::map<uint32_t, double> destruction_object_table; // HashTable<uint32_t, double, 0>
	LongHash<CObjectInventory> object_inventory_table;
	std::queue<double> object_destruction_queue; // AC1Legacy::PQueueArray<double>
};