
#include <StdAfx.h>
#include "PhysicsObj.h"
#include "Scripts.h"
#include "AnimHooks.h"

BOOL ScriptAndModData::UnPack(BYTE **ppData, ULONG iSize)
{
    UNPACK(float, m_Mod);
    UNPACK(uint32_t, m_ScriptID);

    return TRUE;
}

ScriptManager::ScriptManager(CPhysicsObj *pObject)
{
    m_pOwner = pObject;

    m_pScriptsBegin = NULL;
    m_pScriptsEnd = NULL;
    m_0C = -1;
    m_EndTime = 0;
}

ScriptManager::~ScriptManager()
{
    if (m_pScriptsBegin)
        ClearScripts();
}

void ScriptManager::ClearScripts()
{
    while (m_pScriptsBegin)
    {
        ScriptManagerNode *pNode = m_pScriptsBegin;
        m_pScriptsBegin = pNode->m_pNext;
        m_0C = -1;

        if (m_pScriptsBegin)
        {
            m_EndTime =
                m_pScriptsBegin->m_Time +
                m_pScriptsBegin->m_pScript->m_ScriptData.array_data[0]->m_Time;
        }
        else
        {
            m_EndTime = INVALID_TIME;
            m_pScriptsEnd = NULL;
        }

        if (pNode)
        {
            PhysicsScript::Release(pNode->m_pScript);

            delete pNode;
        }
    }
}

BOOL ScriptManager::AddScript(uint32_t ScriptID)
{
    PhysicsScript *pScript = PhysicsScript::Get(ScriptID);

    if (!pScript)
        return FALSE;

    if (!AddScriptInternal(pScript))
        return FALSE;

    return TRUE;
}

void ScriptManager::AddScriptNode(ScriptManagerNode *pNode)
{
    if (m_pScriptsEnd)
    {
        m_pScriptsEnd->m_pNext = pNode;
        m_pScriptsEnd = pNode;
    }
    else
    {
        m_pScriptsBegin = pNode;
        m_pScriptsEnd = pNode;
        m_0C = -1;
        m_EndTime = pNode->m_Time + pNode->m_pScript->m_ScriptData.array_data[0]->m_Time;
    }
}

BOOL ScriptManager::AddScriptInternal(PhysicsScript *pScript)
{
    ScriptManagerNode *pNode = new ScriptManagerNode;

    if (m_pScriptsEnd)
        pNode->m_Time = m_pScriptsEnd->m_Time + m_pScriptsEnd->m_pScript->m_EndTime;
    else
        pNode->m_Time = Timer::cur_time;
    
    pNode->m_pScript = pScript;

    AddScriptNode(pNode);

    return TRUE;
}

CAnimHook *ScriptManager::NextHook()
{
    PhysicsScript *pCurrentScript = m_pScriptsBegin->m_pScript;

    int32_t HookIndex = ++m_0C;
    int32_t TotalHooks = pCurrentScript->m_ScriptData.num_used;

    if (HookIndex >= TotalHooks)
        return NULL; // Now beyond current script.

    if ((HookIndex + 1) >= TotalHooks)
    {
        // This is the LAST hook?
        ScriptManagerNode *pNextNode = m_pScriptsBegin->m_pNext;

        if (pNextNode)
        {
            m_EndTime = pNextNode->m_Time + pNextNode->m_pScript->m_ScriptData.array_data[0]->m_Time;
            return m_pScriptsBegin->m_pScript->m_ScriptData.array_data[0]->m_Event;
        }
        else
        {
            m_EndTime = INVALID_TIME;
            return m_pScriptsBegin->m_pScript->m_ScriptData.array_data[0]->m_Event;
        }
    }
    else
    {
        // Our next hook needs to be executed at *THIS* time..
        m_EndTime = m_pScriptsBegin->m_Time + pCurrentScript->m_ScriptData.array_data[HookIndex+1]->m_Time;

        // Now, we are at THIS one..
        return pCurrentScript->m_ScriptData.array_data[HookIndex]->m_Event;
    }
}

void ScriptManager::UpdateScripts()
{
    if (!m_pScriptsBegin)
        return;

    while (Timer::cur_time >= m_EndTime)
    {
        CAnimHook *pNextHook = NextHook();

        if (pNextHook)
            pNextHook->Execute(m_pOwner);
        else
        {
            ScriptManagerNode *pOldNode = m_pScriptsBegin;

            m_pScriptsBegin = pOldNode->m_pNext;
            m_0C = -1;

            if (m_pScriptsBegin)
                m_EndTime = m_pScriptsBegin->m_Time + m_pScriptsBegin->m_pScript->m_ScriptData.array_data[0]->m_Time;
            else
            {
                m_pScriptsEnd = NULL;
                m_EndTime = INVALID_TIME;
            }

            PhysicsScript::Release(pOldNode->m_pScript);
            delete pOldNode;
        }

        if (!m_pScriptsBegin)
            return;
    }
}

ScriptManagerNode::ScriptManagerNode()
{
    m_Time = 0;
    m_pScript = NULL;
    m_pNext = NULL;
}

PhysicsScriptData::PhysicsScriptData()
{
    m_Time = 0;
    m_Event = NULL;
}

int PhysicsScriptData::Sort(const void *a, const void *b)
{
    // Sort by time.
    return(((*(PhysicsScriptData **)a)->m_Time < (*(PhysicsScriptData **)b)->m_Time) ? -1 : 1);
}

PhysicsScript::PhysicsScript() : m_ScriptData(2)
{
    m_EndTime = 0;
}

PhysicsScript::~PhysicsScript()
{
    Destroy();
}

DBObj* PhysicsScript::Allocator()
{
    return((DBObj *)new PhysicsScript());
}

void PhysicsScript::Destroyer(DBObj *pScript)
{
    delete ((PhysicsScript *)pScript);
}

PhysicsScript *PhysicsScript::Get(uint32_t ID)
{
    return (PhysicsScript *)ObjCaches::PhysicsScripts->Get(ID);
}

void PhysicsScript::Release(PhysicsScript *pScript)
{
    if (pScript)
        ObjCaches::PhysicsScripts->Release(pScript->GetID());
}

void PhysicsScript::Destroy()
{
    for (int32_t i = 0; i < m_ScriptData.num_used; i++)
    {
        PhysicsScriptData *pData = m_ScriptData.array_data[i];

        if (pData) {
            if (pData->m_Event)
                delete pData->m_Event;

            delete pData;
        }

        m_ScriptData.array_data[i] = NULL;
    }
}

BOOL PhysicsScript::UnPack(BYTE** ppData, ULONG iSize)
{
    Destroy();

    UNPACK(uint32_t, id);

    uint32_t ScriptDataCount;
    UNPACK(uint32_t, ScriptDataCount);

    m_ScriptData.Grow(ScriptDataCount);

    for (uint32_t i = 0; i < ScriptDataCount; i++)
    {
        PhysicsScriptData *pScriptData = new PhysicsScriptData;

        UNPACK(double, pScriptData->m_Time);
        pScriptData->m_Event = CAnimHook::UnPackHook(ppData, iSize);

        m_ScriptData.Add(&pScriptData);
    }

    PACK_ALIGN();

    Init();

    return TRUE;
}

void PhysicsScript::Init()
{
    // Sort if necessary.
    if (m_ScriptData.num_used > 0)
    {
        qsort(m_ScriptData.array_data, m_ScriptData.num_used, sizeof(LPVOID), &PhysicsScriptData::Sort);
        m_EndTime = m_ScriptData.array_data[ m_ScriptData.num_used - 1 ]->m_Time;
    }
}

// ****************************
// Physics Script Table (0x34):
// ****************************

PhysicsScriptTable::PhysicsScriptTable() : m_TableData(4)
{
}

PhysicsScriptTable::~PhysicsScriptTable()
{
    Destroy();
}

DBObj* PhysicsScriptTable::Allocator()
{
    return((DBObj *)new PhysicsScriptTable());
}

void PhysicsScriptTable::Destroyer(DBObj *pScriptTable)
{
    delete ((PhysicsScriptTable *)pScriptTable);
}

PhysicsScriptTable *PhysicsScriptTable::Get(uint32_t ID)
{
    return (PhysicsScriptTable *)ObjCaches::PhysicsScriptTables->Get(ID);
}

void PhysicsScriptTable::Release(PhysicsScriptTable *pScriptTable)
{
    if (pScriptTable)
        ObjCaches::PhysicsScripts->Release(pScriptTable->GetID());
}

void PhysicsScriptTable::Destroy()
{
    LongNIValHashIter<PhysicsScriptTableData *> it(&m_TableData);

    while(!it.EndReached())
    {
        PhysicsScriptTableData *pData = it.GetCurrent()->m_Data;
        it.DeleteCurrent();
        delete pData;
    }
}

BOOL PhysicsScriptTable::UnPack(BYTE** ppData, ULONG iSize)
{
    Destroy();

    int32_t EntryCount;

    UNPACK(uint32_t, id);    
    UNPACK(int32_t, EntryCount);

    for (int32_t i = 0; i < EntryCount; i++)
    {
        // The key the data will be referenced by.
        uint32_t DataKey;
        UNPACK(uint32_t, DataKey);

        // Unpack the data.
        PhysicsScriptTableData *pData = new PhysicsScriptTableData;

        UNPACK_POBJ(pData);

        // Add it to the table.
        m_TableData.add(pData, DataKey);
    }

    PACK_ALIGN();

    return TRUE;
}

uint32_t PhysicsScriptTable::GetScript(uint32_t Index, float Mod)
{
    PhysicsScriptTableData *pEntry = NULL;

    if (!m_TableData.lookup(Index, &pEntry))
        return 0;

    return pEntry->GetScript(Mod);
}

PhysicsScriptTableData::PhysicsScriptTableData() : m_SAMDataArray(4)
{
}

PhysicsScriptTableData::~PhysicsScriptTableData()
{
}

BOOL PhysicsScriptTableData::UnPack(BYTE** ppData, ULONG iSize)
{
    int32_t SAMCount;

    UNPACK(int32_t, SAMCount);
    m_SAMDataArray.grow(SAMCount);

    for (int32_t i = 0; i < SAMCount; i++)
    {
        ScriptAndModData SAMData;
        UNPACK_OBJ(SAMData);

        m_SAMDataArray.add(&SAMData);
    }

    return TRUE;
}

uint32_t PhysicsScriptTableData::GetScript(float Mod)
{
    for (int32_t i = 0; i < m_SAMDataArray.num_used; i++)
    {
        if (Mod <= m_SAMDataArray.array_data[i].m_Mod)
            return m_SAMDataArray.array_data[i].m_ScriptID;
    }

    return 0;
}






