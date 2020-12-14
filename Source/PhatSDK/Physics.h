
#pragma once

#include "SmartArray.h"
#include "HashData.h"

class SmartBox;
class CObjectMaint;
// class CPhysicsObj;

class CPhysics
{
public:
    static SmartArray<CPhysicsObj *> static_animating_objects;

    static void AddStaticAnimatingObject(CPhysicsObj *pObject);
    static void RemoveStaticAnimatingObject(CPhysicsObj *pObject);

    CPhysics(CObjectMaint *_ObjMaint, SmartBox *_SmartBox);
    ~CPhysics();

    void SetPlayer(CPhysicsObj *Player);
    void UseTime();
    void UpdateTexVelocity(float FrameTime);

    CObjectMaint *    m_ObjMaint;                  // 0x00
    SmartBox *        m_SmartBox;                  // 0x04
    CPhysicsObj *    m_Player;                   // 0x08
    HashBaseIter<CPhysicsObj *>* m_Iter;    // 0x0C
    uint32_t            m_10;                          // 0x10
};

class PhysicsTimer
{
public:
    static double curr_time;
};

class PhysicsGlobals
{
public:
    static double floor_z;
    static double ceiling_z;
    static double gravity;
};