
#include "StdAfx.h"
#include "PhysicsObj.h"
#include "Physics.h"

SmartArray<CPhysicsObj *> CPhysics::static_animating_objects(8);

double PhysicsTimer::curr_time = INVALID_TIME;
double PhysicsGlobals::floor_z = cos(3437.746770784939);
double PhysicsGlobals::ceiling_z;
double PhysicsGlobals::gravity = -9.8000002;

CPhysics::CPhysics(CObjectMaint *_ObjMaint, SmartBox *_SmartBox)
{
    m_Player = NULL;
    m_ObjMaint = _ObjMaint;
    m_SmartBox = _SmartBox;

    // m_0C = new HashBaseIter<CPhysicsObj *>(m_ObjMaint->m_Objects);

    PhysicsTimer::curr_time = Timer::cur_time; // Timer::m_timeCurrent
}

CPhysics::~CPhysics()
{
    // Missing TexVelGid code here

    // delete m_0C;
}

void CPhysics::AddStaticAnimatingObject(CPhysicsObj *pObject)
{
    static_animating_objects.RemoveUnOrdered(&pObject);
    static_animating_objects.add(&pObject);
}

void CPhysics::RemoveStaticAnimatingObject(CPhysicsObj *pObject)
{
    static_animating_objects.RemoveUnOrdered(&pObject);
}

void CPhysics::SetPlayer(CPhysicsObj *Player)
{
    m_Player = Player;
}

void CPhysics::UseTime()
{
    static const double MinUpdateDelay = 1.0 / 30.0; 
    static double LastUpdate = 0.0; // dbl_5F1658;

    double FrameTime = Timer::cur_time - LastUpdate;

    if (FrameTime < 0.0)
    {
        LastUpdate = Timer::cur_time;
        return;
    }

    if (FrameTime < MinUpdateDelay)
        return;

    // MISSING CODE HERE
    // Update objectmaint objects

   /*
   if (m_Iter)
   {
      m_Iter->SetBegin();

      while (!m_Iter->EndReached())
      {
         CPhysicsObj *pObject = m_Iter->GetCurrent()->GetID();

         pObject->update_object();
         if (m_Player == pObject)
         {
            // m_SmartBox->PlayerPhysicsUpdatedCallback(pObject);
         }

         m_Iter->Next();
      }
   }
   */
	/*
    for (long i = 0; i < static_animating_objects.num_used; i++)
    {
      static_animating_objects.array_data[i]->update_object();
    }
	*/

   if (m_Player)
   {
      // m_Player->update_position();
      m_Player->update_object();
   }

    LastUpdate = Timer::cur_time;

    for (long i = 0; i < static_animating_objects.num_used; i++)
    {
      static_animating_objects.array_data[i]->animate_static_object();
    }

    UpdateTexVelocity(FrameTime);
}

void CPhysics::UpdateTexVelocity(float FrameTime)
{
    // Missing code here..
}



