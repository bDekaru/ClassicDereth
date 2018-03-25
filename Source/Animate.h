
#define MAX_MOTION_QUEUE 6
#define MAX_EMOTE_QUEUE 6

void Animation_Init();
void Animation_Shutdown();
DWORD Animation_GetAnimationSet();

void Animation_Jump(float fPower, Vector &jumpVelocity);

BinaryWriter *Animation_GetAnimationInfo(bool bMoveToUpdate = false);
void Animation_Update();
void Animation_MoveToUpdate();

BOOL m_bAnimUpdate;
WORD m_wAnimSequence;



