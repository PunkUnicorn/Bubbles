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
#include "BubbleTimer.h"
#include "BubbleDimensionCracker.h"
#include "BubbleFindCollisions.h"
#include "BubbleCollisionReportThread.h"

namespace Bubbles
{

class cBubbleEngine : public cTimerWrapper
{
public:
   static const unsigned int RESERVE_COLLISIONRESULTS = 400;
   typedef void STDCALL ClearCacheFunc(unsigned int groupID);

   typedef struct
   {
      cBubbleEngine *ptr;
   } PTR;

   static const unsigned int FOUR_A_SECOND = 250;
   static const unsigned int FIVE_A_SECOND = 200;
   static const unsigned int SIX_A_SECOND = 166;

   cBubbleEngine(unsigned int ID, GetCoordsFunc *getCoords, ClearCacheFunc *clearCache, unsigned int reserveAmount = RESERVE_COLLISIONRESULTS) 
      : mTimerTrace(NULL),
      mGetCoordsFunc(getCoords),
      mClearCacheFunc(clearCache),
      mGroupID(0),
      mID(ID),
      mReserveAmount(reserveAmount),
      mFlipFlop(false),
      mCollisionList(),
      mCollisionListLock(),
      mWorkList(),
      mDistanceList(reserveAmount * 3),
      mCollisionResults(reserveAmount),
      mCollisionReportFunc(NULL),
      mThreadRun() { }

   inline std::vector<cBubbleBubble::PTR> &FactoryGetWorkList(void) { return mWorkList; }
   inline std::vector<cBubbleBubble::PTR> &FactoryGetCollisionList(void) { return mCollisionList; }

   inline const std::vector<cBubbleBubble::PTR> &GetWorkList(void) const { return mWorkList; };
   inline const std::vector<cBubbleBubble::PTR> &GetCollisionList() const { return mCollisionList; }

   inline void SetTimerTraceFunc(TraceFunc *func) { mTimerTrace = func; };
   inline void SetGroup(unsigned int groupID) { mGroupID = groupID; }
   inline void SetPause(bool pause) { this->cTimerWrapper::SetPause(pause); }
   inline void Abort(void) { this->cTimerWrapper::Abort(); };

   inline cMutexWrapper *GetCollisionLock(void) { return &mCollisionListLock; }
   inline unsigned int GetID(void) const { return mID; }

   void Start(CollisionReportFunc *collisionReportFunc, unsigned int interval=FIVE_A_SECOND)
   {
      mThreadRun.Init();
      mCollisionReportFunc = collisionReportFunc;
      cTimerWrapper::FactorySetDelay(interval);
      cTimerWrapper::AddThread(this); // Milder on threads -> cTimerWrapper::AddTimer(this);      
   };

   virtual ~cBubbleEngine(void)
   {
      cTimerWrapper::RemoveTimer(this);
   }

private:
   TraceFunc *mTimerTrace;
   GetCoordsFunc *mGetCoordsFunc;
   ClearCacheFunc *mClearCacheFunc;
   unsigned int mGroupID;
   unsigned int mID;
   unsigned int mReserveAmount;
   bool mFlipFlop;

   std::vector<cBubbleBubble::PTR> mCollisionList; // total list of everything that can collide
   cMutexWrapper mCollisionListLock;
   std::vector<cBubbleBubble::PTR> mWorkList; // list of what this engine compares and reports on, because work can be split over two engines

   std::vector<TRILATERATION_DATA> mDistanceList; // results of relative distances
   std::vector<COLLISION_RESULT> mCollisionResults; // results of found collisions
   CollisionReportFunc *mCollisionReportFunc; // collision callback to inform of collisions
   cBubbleCollisionReportThread mThreadRun;

   bool IsExpired(void) { return false; };

   void EventTimer(void)
   {
      if (mFlipFlop) return;
      mFlipFlop = true;
      Uint32 start = SDL_GetTicks();

      // this is where collisions are deduced
      for (cMutexWrapper::Lock lock(GetCollisionLock()) ;;)
      {
         mCollisionResults.clear();

         (*mClearCacheFunc)(mGroupID);

         std::for_each(mWorkList.begin(), mWorkList.end(), 
            cBubbleFindCollisions(mCollisionList, mDistanceList, mCollisionResults));

         unsigned int size = mCollisionResults.size();
         void *list = NULL;

         if (size > 0)
            list = &( mCollisionResults.front() );
                     
         mThreadRun.Start(mThreadRun, mCollisionReportFunc, mGroupID, mID, (COLLISION_RESULT*) list, size); // Milder on threads -> (*mCollisionReportFunc)(mGroupID, mID, (COLLISION_RESULT*) list, size);

         if (mTimerTrace != NULL)
         {
               Uint32 duration = (SDL_GetTicks() - start);
               (*mTimerTrace)(GetID(), duration);
         }
         break;
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