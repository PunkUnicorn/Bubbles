// BubbleEngine.h
// Copyright (c) 2014
// Author: Matthew Cocks
// License: Attribution 4.0 International (CC BY 4.0)

#ifndef BUBBLEENGINE_H
#define BUBBLEENGINE_H

#include <vector>
#include <algorithm>
#include <map>
#include "STDCALL.h"
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
	Uint32 mNow;
   bool &mAbort;

public:
   cRefreshWorklistCoords(Uint32 now, bool &abort) : mNow(now), mAbort(abort) { }
   inline result_type operator () (const argument_type &workListItem) const { workListItem.ptr->ClearCache(mNow); } 
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
      mBubbleRunningLock(),
      mGroupList(),
      mListLock(),
      mWorkList(),
      mRecycleBin(),
      mDistanceList(),
      mCollisionResults1(),
      mCollisionResults2(),
      mCollisionReportFunc(NULL),
      mThreadRun1(),
      mThreadRun2(),
      mThread1Or2(false) 
   { 
      mDistanceList.reserve(reserveAmount);
      mCollisionResults1.reserve(reserveAmount);
      mCollisionResults2.reserve(reserveAmount);
   }

   inline void Purge(void)
   {
      // TODO: take over the world and recruit minions to write this
   }

   inline void FactoryAddWorkList(cBubbleBubble::PTR &addMe) 
   { 
      TimerWrapper::cMutexWrapper::Lock lock(GetLock());
      mWorkList.push_back(addMe);

      Purge();
   }
   inline void FactoryAddGroupList(cBubbleBubble::PTR &addMe) 
   { 
      TimerWrapper::cMutexWrapper::Lock lock(GetLock());
      mGroupList.push_back(addMe);
   }

   inline TimerWrapper::cMutexWrapper *GetEngineCycleLock(void) { return &mBubbleRunningLock; }

   inline void FactoryRemove(unsigned int bubbleId)
   {
      TimerWrapper::cMutexWrapper::Lock lock(GetLock());
      cBubbleBubble::PTR findMe = FactoryGetBubble(bubbleId);
      if (findMe.ptr == NULL) return;

      std::vector<cBubbleBubble::PTR>::iterator collisionListIt = std::find(mGroupList.begin(), mGroupList.end(), findMe);
      if (collisionListIt != mGroupList.end())
      {
         std::swap(*collisionListIt, mGroupList.back());
	      mGroupList.pop_back();
      }

      std::vector<cBubbleBubble::PTR>::iterator workListIt = std::find(mWorkList.begin(), mWorkList.end(), findMe);
      if (workListIt != mWorkList.end())
      {
         findMe.ptr->FactorySetDeleted(true);
         mRecycleBin.push_back(findMe);
         std::swap(*workListIt, mWorkList.back());
	      mWorkList.pop_back();
      }
   }

   inline const std::vector<cBubbleBubble::PTR> &GetWorkList(void) const { return mWorkList; };
   inline const std::vector<cBubbleBubble::PTR> &GetGroupList(void) const { return mGroupList; }

   inline void SetTimerTraceFunc(TraceFunc *func) { mTimerTrace = func; };
   inline unsigned int GetDelay(void) { return this->cTimerWrapper::GetDelay(); }
   inline void SetGroup(unsigned int groupID) { mGroupID = groupID; }
   inline void SetPause(bool pause) { this->TimerWrapper::cTimerWrapper::SetPause(pause); }
   inline void Abort(void) { this->TimerWrapper::cTimerWrapper::Abort(); };
   inline bool HasAborted(void) { return this->TimerWrapper::cTimerWrapper::HasAborted(); }

   inline cBubbleBubble::PTR FactoryGetBubble(unsigned int Id)
   {
      static cBubbleBubble::PTR dud = { NULL };
      std::vector<cBubbleBubble::PTR>::iterator it = mGroupList.begin();
      for (; it != mGroupList.end(); it++)
      {
         if (it->ptr->GetID() != Id) continue;
         if (it->ptr->GetIsDeleted()) 
            break;
         return *it;
      }
      return dud;
   }

   inline TimerWrapper::cMutexWrapper *GetLock(void) { return &mListLock; }
   inline unsigned int GetID(void) const { return mID; }
   inline unsigned int GetGroup(void) const { return mGroupID; }

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
   TimerWrapper::cMutexWrapper mBubbleRunningLock;

   std::vector<cBubbleBubble::PTR> mGroupList; // total list of everything that can collide
   TimerWrapper::cMutexWrapper mListLock;
   std::vector<cBubbleBubble::PTR> mWorkList; // list of what this engine compares and reports on, because work can be split over two engines
   std::vector<cBubbleBubble::PTR> mRecycleBin; // deleted bubbles

   std::vector<TRILATERATION_DATA> mDistanceList; // results of relative distances
   // second bank of these? oops

   std::vector<COLLISION_RESULT> mCollisionResults1; // results of found collisions
   std::vector<COLLISION_RESULT> mCollisionResults2; // results of found collisions
   CollisionReportFunc *mCollisionReportFunc; // collision callback to inform of collisions
   cBubbleCollisionReportThread mThreadRun1;
   cBubbleCollisionReportThread mThreadRun2;
   bool mThread1Or2;

   void EventTimer(void)
   {
      EventTimerLocked();

	  if (mClearCacheFunc != NULL)
		 (*mClearCacheFunc)(mGroupID);
   }

   void EventTimerLocked(void)
   {
      Uint32 start = 0;
      if (mTimerTrace != NULL)
         start = SDL_GetTicks();

      TimerWrapper::cMutexWrapper::Lock lock(GetEngineCycleLock());

      if (this->TimerWrapper::cTimerWrapper::IsAborting()) return;

      try
      {
         TimerWrapper::cMutexWrapper::Lock lock(GetLock());         
         bool useThread1 
               = mThread1Or2 = !mThread1Or2;

         std::vector<COLLISION_RESULT> &useResultsBuffer = (useThread1 ? mCollisionResults1 : mCollisionResults2);
         useResultsBuffer.clear();

		 Uint32 now = SDL_GetTicks();
         std::for_each(mWorkList.begin(), mWorkList.end(), 
            cRefreshWorklistCoords(now, this->TimerWrapper::cTimerWrapper::mAbort));

         if (this->TimerWrapper::cTimerWrapper::IsAborting()) throw -999;

         // Calculate collisions
         if (mWorkList.size() > 0)
            std::for_each(mWorkList.begin(), mWorkList.end(), 
               cBubbleFindCollisions(mTimerTrace, mGroupList, mDistanceList, useResultsBuffer, this->TimerWrapper::cTimerWrapper::mAbort));

         unsigned int size;
         size = useResultsBuffer.size();
         void *list = NULL;
         if (size > 0)
         {
            list = &( useResultsBuffer.front() );
            cBubbleCollisionReportThread::Start(mTimerTrace, ( mThread1Or2 ? mThreadRun1 : mThreadRun2 ), mCollisionReportFunc, mGroupID, mID, (COLLISION_RESULT*) list, size); 
            // Milder on threads -> (*mCollisionReportFunc)(mGroupID, mID, (COLLISION_RESULT*) list, size);            
         }

         if (mTimerTrace != NULL)
         {
            Uint32 duration = (SDL_GetTicks() - start);
            (*mTimerTrace)(GetID(), duration);
         }
      }
      catch (...)
      {
         if (this->TimerWrapper::cTimerWrapper::IsAborting() == false) 
            throw;
         else
            /*eat exception and move on*/;
      }
   };
};

static inline bool operator ==(const cBubbleEngine::PTR & p_lhs, const cBubbleEngine::PTR & p_rhs)
{
   return p_lhs.ptr->GetID() == p_rhs.ptr->GetID();
}

}
#endif