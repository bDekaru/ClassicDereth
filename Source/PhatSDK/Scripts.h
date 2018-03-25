
#pragma once

#include "ObjCache.h"
#include "SmartArray.h"
#include "HashData.h"

// The following are declared elsewhere
// class CPhysicsObj;
class CAnimHook;

// The following are declared here
class ScriptAndModData;
class PhysicsScriptTable;
class PhysicsScriptTableData;
class PhysicsScript;
class PhysicsScriptData;
class ScriptManager;
class ScriptManagerNode;

// Represents the script Index and Mod float (scale/speed for example)
class ScriptAndModData
{
public:
    BOOL UnPack(BYTE **ppData, ULONG iSize);

    float m_Mod;
    DWORD m_ScriptID;
};

class PhysicsScriptTable : public DBObj
{
public:
    PhysicsScriptTable();
    ~PhysicsScriptTable();

    static DBObj* Allocator();
    static void Destroyer(DBObj*);
    static PhysicsScriptTable* Get(DWORD ID);
    static void Release(PhysicsScriptTable *);

    void Destroy();
    BOOL UnPack(BYTE** ppData, ULONG iSize);

    DWORD GetScript(DWORD Index, float Mod);

    LongNIValHash<PhysicsScriptTableData *> m_TableData; // 0x20 / 0x2C
};

class PhysicsScriptTableData
{
public:
    PhysicsScriptTableData();
    ~PhysicsScriptTableData();

    DWORD GetScript(float Mod);
    BOOL UnPack(BYTE** ppData, ULONG iSize);

    // DWORD/float representing the iIndex/fMod for the effect.
    SmartArray<ScriptAndModData> m_SAMDataArray;
};

class PhysicsScript : public DBObj
{
public:
    PhysicsScript();
    ~PhysicsScript();

    static DBObj* Allocator();
    static void Destroyer(DBObj*);
    static PhysicsScript* Get(DWORD ID);
    static void Release(PhysicsScript *);

    void Destroy();
    void Init();

    BOOL UnPack(BYTE** ppData, ULONG iSize);

    OldSmartArray<PhysicsScriptData *> m_ScriptData; // 0x1C / 0x28 - size: 0x10
    double m_EndTime;
};

class PhysicsScriptData
{
public:
    PhysicsScriptData();

    static int Sort(const void *a, const void *b);

    double        m_Time;    // 0x00
    CAnimHook*    m_Event;    // 0x08
    LPVOID        m_Next;    // 0x0C - Unused?
};

class ScriptManager
{
public:
    ScriptManager(CPhysicsObj *pObject);
    ~ScriptManager();

    BOOL AddScript(DWORD ScriptID);
    BOOL AddScriptInternal(PhysicsScript *pScript);
    void AddScriptNode(ScriptManagerNode *pNode);
    void ClearScripts();
    void UpdateScripts();

    CAnimHook *NextHook();


    CPhysicsObj *m_pOwner; // 0x00
    ScriptManagerNode *m_pScriptsBegin; // 0x04
    ScriptManagerNode *m_pScriptsEnd; // 0x08
    long m_0C; // 0x0C
    double m_EndTime; // 0x10
};

class ScriptManagerNode
{
public:
    ScriptManagerNode();

    double m_Time; // 0x00
    PhysicsScript *m_pScript; // 0x08
    ScriptManagerNode *m_pNext; // 0x0C
};

