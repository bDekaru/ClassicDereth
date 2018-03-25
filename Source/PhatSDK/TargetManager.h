
#pragma once

class TargettedVoyeurInfo
{
public:
	unsigned int object_id;
	long double quantum;
	float radius;
	Position last_sent_position;
};


class TargetManager
{
public:
	TargetManager(CPhysicsObj *object);
	~TargetManager();

	void SetTargetQuantum(double new_quantum);
	void AddVoyeur(DWORD object_id, float radius, double quantum);
	void SendVoyeurUpdate(TargettedVoyeurInfo *voyeur, Position *p, TargetStatus status);
	BOOL RemoveVoyeur(DWORD object_id);
	void ReceiveUpdate(class TargetInfo *target_update);
	void ClearTarget();
	void SetTarget(DWORD context_id, DWORD object_id, float radius, double quantum);
	void HandleTargetting();
	void CheckAndUpdateVoyeur(TargettedVoyeurInfo *voyeur);
	void GetInterpolatedPosition(double quantum, Position *p);
	void NotifyVoyeurOfEvent(TargetStatus _event);

	CPhysicsObj *physobj;
	TargetInfo *target_info;
	LongNIHash<TargettedVoyeurInfo> *voyeur_table;
	long double last_update_time;	
};
