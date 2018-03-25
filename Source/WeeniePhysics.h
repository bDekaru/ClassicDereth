
// included as part of CWeenieObject

// These should probably be defined elsewhere
static const float DEFAULT_FRICTION;
static const float DEFAULT_TRANSLUCENCY;
static const float DEFAULT_ELASTICITY;

// from PhysicsObj

class CPartArray *part_array = NULL; // 0x10

class ScriptManager *script_manager = NULL; // 0x30
class PhysicsScriptTable *physics_script_table = NULL; // 0x34

DWORD m_DefaultScript = 0; // 0x38
float m_DefaultScriptIntensity = 0.0f; // 0x3C

CPhysicsObj *parent = NULL; // !!!! this is actually a CPhysicsObj!!!!!!

class CHILDLIST *children = NULL;
Position m_Position; // 0x48

class CObjCell *cell = NULL; // 0x90

DWORD num_shadow_objects = 0;
DArray<CShadowObj> shadow_objects;

DWORD m_PhysicsState = 0; // 0xA8
DWORD transient_state = 0; // 0xAC

float m_fElasticity = DEFAULT_ELASTICITY; // 0xB0
float m_fTranslucency = DEFAULT_TRANSLUCENCY; // 0xB4
float translucencyOriginal = DEFAULT_TRANSLUCENCY; // 0xB8
float m_fFriction = DEFAULT_FRICTION; // 0xBC

class MovementManager *movement_manager = NULL; // 0xC4
class PositionManager *position_manager = NULL; // 0xC8
int last_move_was_autonomous = 1; // 0xCC
int jumped_this_frame = 0; // 0xD0
double update_time = 0.0; // 0xD8

Vector m_velocityVector; // 0xE0
Vector m_Acceleration; // 0xE0
Vector m_Omega; // 0xE0

class PhysicsObjHook *hooks = NULL; // 0x104
std::list<class CAnimHook *> anim_hooks; // 0x108
float m_scale = 1.0f; // 0x114

class CWeenieObject *weenie_obj = NULL; // 0x12C
Plane contact_plane; // 0x130
DWORD contact_plane_cell_id = 0; // 0x140

Vector sliding_normal; // 0x144
Vector cached_velocity; // 0x150

int colliding_with_environment = 0; // 0x160
WORD _position_timestamp = 0; // 0x164
WORD m_wNumAnimInteracts = 1; // 0x166
WORD m_wNumBubbleModes = 1; // 0x168
WORD m_wNumJumps = 0; // 0x16A
WORD _teleport_timestamp = 0; // 0x16C
WORD m_wAnimCount = 0; // 0x16E
WORD _force_position_timestamp = 0; // 0x170
WORD _vdesc_timestamp = 0; // 0x172
WORD _instance_timestamp = 1; // 0x174

BOOL set_active(BOOL active);

void clear_target();
void unstick_from_object();
void stick_to_object(DWORD target);
void interrupt_current_movement();
void RemoveLinkAnimations();
float get_heading();
void set_heading(float degrees, int send_event);
void set_frame(class Frame &frame);

BOOL IsInterpolating();
BOOL motions_pending();
DWORD StopInterpretedMotion(DWORD motion, class MovementParameters *params);
DWORD DoInterpretedMotion(DWORD motion, class MovementParameters *params);
BOOL movement_is_autonomous();

class CMotionInterp *get_minterp();
void InitializeMotionTables();
void StopCompletely(int send_event);

void set_local_velocity(const Vector &new_velocity, int send_event);
void set_on_walkable(BOOL is_on_walkable);

void StopCompletely_Internal();
void CheckForCompletedMotions();
bool IsFullyConstrained();

void UpdateChildrenInternal();
void UpdateChild(CPhysicsObj *child_obj, unsigned int part_index, Frame *child_frame);

void MakeMovementManager(BOOL init_motion);

void leave_world();
BOOL is_completely_visible();
void update_object();
void animate_static_object();
void set_nodraw(BOOL NoDraw, BOOL Unused);
void Hook_AnimDone();
void calc_cross_cells_static();
void add_obj_to_cell(CObjCell *pCell, Frame *pFrame);
void MotionDone(DWORD motion, BOOL success);
static CPhysicsObj *makeObject(DWORD data_did, DWORD object_iid, BOOL bDynamic);
void add_anim_hook(class CAnimHook *pHook);
void InitDefaults(class CSetup *pSetup);
void report_collision_end(const int force_end);
void remove_shadows_from_cells();
void leave_cell(BOOL Unknown);
void set_cell_id(DWORD CellID);
void calc_acceleration();
void enter_cell(CObjCell *pCell);
void set_initial_frame(Frame *Pos);

BOOL InitObjectBegin(DWORD object_iid, BOOL bDynamic);
BOOL InitPartArrayObject(DWORD data_did, BOOL bCreateParts);
BOOL CacheHasPhysicsBSP();
void SetTranslucencyInternal(float Amount);

BOOL InitObjectEnd();
BOOL SetPlacementFrameInternal(DWORD PlacementID);
BOOL play_script_internal(DWORD ScriptID);

DWORD SetMotionTableID(DWORD ID);
void set_stable_id(DWORD ID);
void set_phstable_id(DWORD ID);

BOOL makeAnimObject(DWORD setup_id, BOOL bCreateParts);

void UpdatePartsInternal();
void process_hooks();

int check_collisions(CPhysicsObj *);
int get_object_info(class CTransition *transit, int admin_move);

DWORD GetNumSphere(); // Inlined
class CSphere *GetSphere(); // inlined

DWORD GetNumCylsphere(); // Inlined
class CCylSphere *GetCylsphere(); // inlined

float GetStepDownHeight();
float GetStepUpHeight();

int check_contact(int contact);

TransitionState FindObjCollisions(CTransition *transition);

float get_walkable_z();
void set_velocity(const Vector &new_velocity, int send_event);

CTransition *transition(Position *old_pos, Position *new_pos, int admin_move);

DWORD DoMotion(DWORD motion, MovementParameters *params, int send_event);


