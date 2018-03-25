

// These are all members of CWeenieObject

void Movement_Init();
void Movement_Shutdown();
void Movement_Think();

void Movement_UpdatePos();
void Movement_UpdateVector();
void Movement_SendUpdate(DWORD dwCell);

double m_fMoveThink;

Position m_LastMovePosition;

double _last_update_pos = 0.0;