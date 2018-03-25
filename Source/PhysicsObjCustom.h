
void InitPhysicsTemporary();
void CleanupPhysicsTemporary();

void Send_StateChangeEvent();
void EmitEffect(DWORD dwIndex, float flScale);
void EmitSound(DWORD dwIndex, float fSpeed, bool bLocalClientOnly = false);

ObjDesc m_WornObjDesc;

bool InValidCell() { return cell ? true : false; }
DWORD GetSoundTableID();
DWORD GetMotionTableID();
DWORD GetPETableID();
float DistanceTo(CPhysicsObj *pOther);
float DistanceSquared(CPhysicsObj *pOther);
void EnterPortal(DWORD old_cell_id);
void ExitPortal();
class CWeenieObject *GetWeenie();

void SendNetMessage(void *_data, DWORD _len, WORD _group, BOOL _event = 0);
void SendNetMessage(BinaryWriter *_food, WORD _group, BOOL _event = 0, BOOL del = 1);

virtual void Tick();

DWORD GetLandcell();
std::string GetName();

int GetPlacementFrameID();
int GetActivePlacementFrameID();

DWORD last_tick_cell_id = 0;
CPhysicsObj *last_tick_parent = NULL;

