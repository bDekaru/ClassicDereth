
void InitPhysicsTemporary();
void CleanupPhysicsTemporary();

void Send_StateChangeEvent();
void EmitEffect(uint32_t dwIndex, float flScale);
void EmitSound(uint32_t dwIndex, float fSpeed, bool bLocalClientOnly = false);

ObjDesc m_WornObjDesc;

bool InValidCell() { return cell ? true : false; }
uint32_t GetSoundTableID();
uint32_t GetMotionTableID();
uint32_t GetPETableID();
float DistanceTo(CPhysicsObj *pOther);
float DistanceSquared(CPhysicsObj *pOther);
void EnterPortal(uint32_t old_cell_id);
void ExitPortal();
class CWeenieObject *GetWeenie();

void SendNetMessage(void *_data, uint32_t _len, WORD _group, BOOL _event = 0);
void SendNetMessage(BinaryWriter *_food, WORD _group, BOOL _event = 0, BOOL del = 1);

virtual void Tick();

uint32_t GetLandcell();
std::string GetName();

int GetPlacementFrameID();
int GetActivePlacementFrameID();

uint32_t last_tick_cell_id = 0;
CPhysicsObj *last_tick_parent = NULL;

