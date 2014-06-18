// BubbleEngine.h
// Copyright (c) 2014
// Author: Matthew Cocks
// License: Attribution 4.0 International (CC BY 4.0)

#ifndef BUBBLEENGINE_H
#define BUBBLEENGINE_H
#include <vector>
#include <algorithm>
#include <map>
#include "BubbleSTDCALL.h"
#include "BubbleBubble.h"
#include "..\Shared\TimerWrapper.h"
#include "BubbleDimensionCracker.h"
#include "BubbleFindCollisions.h"
#include "BubbleCollisionReportThread.h"

namespace Bubbles
{


class cRefreshWorklistCoords : public std::unary_function<cBubbleBubble::PTR, void>
{
private:
   bool &mAbort;

public:
   cRefreshWorklistCoords(bool &abort) 
      : mAbort(abort)
   { }


   /* find the distance units from 'me' to everything else in the universe  */
   inline result_type operator () (const argument_type &workListItem) const
   {
      workListItem.ptr->ClearCache();
      //float x, y, z;
      //workListItem.ptr->GetCollisionCenter(x, y, z);
   }
};



class cBubbleEngine : public TimerWrapper::cTimerWrapper
{
public:
   static const unsigned int RESERVE_COLLISIONRESULTS = 1024;
   typedef void STDCALL ClearCacheFunc(unsigned int groupId);

   typedef struct
   {
      cBubbleEngine *ptr;
   } PTR;

   static const unsigned int FOUR_A_SECOND = 250;
   static const unsigned int FIVE_A_SECOND = 200;
   static const unsigned int SIX_A_SECOND = 166;

   cBubbleEngine(unsigned int ID, ClearCacheFunc *clearCache, unsigned int reserveAmount = RESERVE_COLLISIONRESULTS) 
      : mTimerTrace(NULL),
      mClearCacheFunc(clearCache),
      mGroupID(0),
      mID(ID),
      mReserveAmount(),
      mFlipFlop(false),
      mCollisionList(),
      mCollisionListLock(),
      mWorkList(),
      mDistanceList(),
      mCollisionResults(),
      mCollisionReportFunc(NULL),
      mThreadRun1(),
      mThreadRun2(),
      mThread1Or2(false) 
   { 
      mDistanceList.reserve(reserveAmount);
      mCollisionResults.reserve(reserveAmount);
   }

   inline void FactoryAddWorkList(cBubbleBubble::PTR &addMe) 
   { 
      TimerWrapper::cMutexWrapper::Lock lock(GetCollisionLock());
      mWorkList.push_back(addMe);
   }
   inline void FactoryAddCollisionList(cBubbleBubble::PTR &addMe) 
   { 
      TimerWrapper::cMutexWrapper::Lock lock(GetCollisionLock());
      mCollisionList.push_back(addMe);
   }

   inline const std::vector<cBubbleBubble::PTR> &GetWorkList(void) const { return mWorkList; };
   inline const std::vector<cBubbleBubble::PTR> &GetCollisionList() const { return mCollisionList; }

   inline void SetTimerTraceFunc(TraceFunc *func) { mTimerTrace = func; };
   inline void SetGroup(unsigned int groupID) { mGroupID = groupID; }
   inline void SetPause(bool pause) { this->TimerWrapper::cTimerWrapper::SetPause(pause); }
   inline void Abort(void) { this->TimerWrapper::cTimerWrapper::Abort(); };
   inline bool HasAborted(void) { return this->TimerWrapper::cTimerWrapper::HasAborted(); }

   inline cBubbleBubble::PTR FactoryGetBubble(unsigned int Id)
   {
      TimerWrapper::cMutexWrapper::Lock lock(GetCollisionLock());
      
      std::vector<cBubbleBubble::PTR>::iterator it = mCollisionList.begin();
      for (; it != mCollisionList.end(); it++)
      {
         if (it->ptr->GetID() != Id) continue;
         return *it;
      }
      throw 99;
   }

   inline TimerWrapper::cMutexWrapper *GetCollisionLock(void) { return &mCollisionListLock; }
   inline unsigned int GetID(void) const { return mID; }

   void Start(CollisionReportFunc *collisionReportFunc, unsigned int interval=FIVE_A_SECOND)
   {
      mThreadRun1.Init();
      mThreadRun2.Init();
      mCollisionReportFunc = collisionReportFunc;
      TimerWrapper::cTimerWrapper::FactorySetDelay(interval);
      TimerWrapper::cTimerWrapper::AddThread(this); // Milder on threads -> cTimerWrapper::AddTimer(this);      
   };

   virtual ~cBubbleEngine(void)
   {
      TimerWrapper::cTimerWrapper::RemoveTimer(this);
   }

private:
   TraceFunc *mTimerTrace;
   ClearCacheFunc *mClearCacheFunc;
   unsigned int mGroupID;
   unsigned int mID;
   unsigned int mReserveAmount;
   bool mFlipFlop;

   std::vector<cBubbleBubble::PTR> mCollisionList; // total list of everything that can collide
   TimerWrapper::cMutexWrapper mCollisionListLock;
   std::vector<cBubbleBubble::PTR> mWorkList; // list of what this engine compares and reports on, because work can be split over two engines

   std::vector<TRILATERATION_DATA> mDistanceList; // results of relative distances
   std::vector<COLLISION_RESULT> mCollisionResults; // results of found collisions
   CollisionReportFunc *mCollisionReportFunc; // collision callback to inform of collisions
   cBubbleCollisionReportThread mThreadRun1;
   cBubbleCollisionReportThread mThreadRun2;
   bool mThread1Or2;

   void EventTimer(void)
   {
      if (mFlipFlop) return;
      mFlipFlop = true;        
      
      SDL_Delay(30);
      if (this->TimerWrapper::cTimerWrapper::IsAborting()) throw -999;

      // this is where collisions are deduced
      try
      {
         TimerWrapper::cMutexWrapper::Lock lock(GetCollisionLock());

         Uint32 start = SDL_GetTicks();
         mCollisionResults.clear();

         if (mClearCacheFunc != NULL)
            (*mClearCacheFunc)(mGroupID);

         // Get fresh Worklist coordinates
         std::for_each(mWorkList.begin(), mWorkList.end(), cRefreshWorklistCoords(this->TimerWrapper::cTimerWrapper::mAbort));

         // Calculate collisions
         if (this->TimerWrapper::cTimerWrapper::IsAborting()) throw -999;
         std::for_each(mWorkList.begin(), mWorkList.end(), 
            cBubbleFindCollisions(mCollisionList, mDistanceList, mCollisionResults, this->TimerWrapper::cTimerWrapper::mAbort));

         unsigned int size;
         size = mCollisionResults.size();
         void *list = NULL;

         if (size > 0)
         {
            list = &( mCollisionResults.front() );
                    
            cBubbleCollisionReportThread::Start(mThread1Or2 ? mThreadRun1 : mThreadRun2, mCollisionReportFunc, mGroupID, mID, (COLLISION_RESULT*) list, size); 
            mThread1Or2 = !mThread1Or2;
            // Milder on threads -> (*mCollisionReportFunc)(mGroupID, mID, (COLLISION_RESULT*) list, size);            
         }

         if (mTimerTrace != NULL)
         {
            Uint32 duration = (SDL_GetTicks() - start);
            (*mTimerTrace)(GetID(), duration);
         }
      }
      catch (int code)
      {
         if (code != -999) throw;
      }

      mFlipFlop = false;
   };
};

static inline bool operator ==(const cBubbleEngine::PTR & p_lhs, const cBubbleEngine::PTR & p_rhs)
{
   return p_lhs.ptr->GetID() == p_rhs.ptr->GetID();
}

}
#endif