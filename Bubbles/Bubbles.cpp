// Bubble.cpp
// Copyright (c) 2014
// Author: Matthew Cocks
// License: Attribution 4.0 International (CC BY 4.0)
#define BUILDING_DLL
#include "BubbleEngine.h"
#include "BubbleVariables.h"
#include <vector>
#include <SDL.h>
#include "STDCALL.h"

#include "DLL_PUBLIC.h"

using namespace Bubbles;

extern "C" DLL_PUBLIC bool STDCALL InitBubbles(void)
{
   Uint32 flags = SDL_INIT_TIMER;

   if (SDL_WasInit(0) != flags)
   {
      return false;
   }
   return true;
}

namespace UnInitBubblesClasses
{
   class StopEngine : std::unary_function<cBubbleEngine::PTR, void>
   {
   private:
      bool mAbort;
   public:
      StopEngine(bool abort) : mAbort(abort) { }
      inline result_type operator () (const argument_type& engine) const
      {
         if (mAbort) 
         {
            engine.ptr->SetPause(true);
            engine.ptr->Abort();
         }
         else 
            delete engine.ptr;
      };
   };

   class CheckEngineState : std::unary_function<cBubbleEngine::PTR, void>
   {
   private:
      bool &mAborted;
   public:
      CheckEngineState(bool &aborted) : mAborted(aborted) { }
      inline result_type operator () (const argument_type& engine) const
      {
         mAborted = mAborted || engine.ptr->HasAborted();
      };
   };
}

extern "C" DLL_PUBLIC void STDCALL UnInitBubbles(void)
{
   try
   {
      if (engines.size() == 0) return;

      std::for_each(engines.begin(), engines.end(),
         UnInitBubblesClasses::StopEngine(true));

      bool hasAborted;
      do 
      {
         hasAborted = false;
         SDL_Delay(10);
         std::for_each(engines.begin(), engines.end(), 
            UnInitBubblesClasses::CheckEngineState(hasAborted));
      }
      while (hasAborted == false);

      std::for_each(engines.begin(), engines.end(), UnInitBubblesClasses::StopEngine(false));
   }
   catch (...)
   { }
}

// this function works but is shit, click here to re-write it
// passed to the engine so it can call us when it needs to refresh the groups bubbles X, Y and Z cached values
extern "C"  static  void STDCALL ClearCache(unsigned int groupId)
{
   try
   {
      static Uint32 noMatterWhatClearCacheAt;
      Uint32 now = SDL_GetTicks();
      static const Uint32 AllOverridingCacheAge = 200; //ms
      static std::map<unsigned int /*groupId*/, unsigned int /*call per group count*/> callPerGroup;

      if (noMatterWhatClearCacheAt < now)
      {
         // clear the cache when the last engine of the group calls ClearCache
         if (++callPerGroup[groupId] != engineGroups[groupId].size()) 
            return;
      }

      noMatterWhatClearCacheAt = now + AllOverridingCacheAge;
      // since we have not exited the function it's time to clear the cache
      for (std::map<unsigned int, BUBBLE_COORDS>::iterator it=bubbleCoords.begin(); it != bubbleCoords.end(); ++it)
         if (groupId == it->second.groupId) 
            it->second.cached = false;

      callPerGroup[groupId] = 0;
   }
   catch (...)
   { }
}

// passed to the engine for it to call to get a bubbles X, Y and Z
extern "C"  static  void STDCALL GetBubbleXYZ(unsigned int engineId, unsigned int id, float &px, float &py, float &pz)
{
   try
   {
      BUBBLE_COORDS &bubbleChords = *(&(bubbleCoords[id]));

      if (bubbleChords.cached == false)
      {
         bubbleChords.cached = true;
         // get the coordinates by calling the functio
         (*bubbleChords.getCoords)(engineId, id, bubbleChords.x, bubbleChords.y, bubbleChords.z);         
      }

      px = bubbleChords.x;
      py = bubbleChords.y;
      pz = bubbleChords.z;
   }
   catch (...)
   { }       
}

extern "C" DLL_PUBLIC unsigned int STDCALL AddEngine(void)
{
   try
   {
      unsigned int id = engines.size();
      cBubbleEngine *engine1 = new cBubbleEngine(id, (cBubbleEngine::ClearCacheFunc *) ClearCache);
      cBubbleEngine::PTR pimp = { engine1 };
      engines.push_back(pimp);      
      return id;
   }
   catch (...)
   {
      return 0;
   }
}

static cBubbleEngine::PTR GetEngine(unsigned int engineId)
{
   if (engineId+1 > engines.size()) throw -1; // Invalid engine ID
   return engines[engineId];
}

extern "C" DLL_PUBLIC void STDCALL SetEngineTimerTrace(unsigned int engineId, TraceFunc *timerTrace)
{
   try
   {
      cBubbleEngine::PTR found = GetEngine(engineId);
      found.ptr->SetTimerTraceFunc(timerTrace);
   }
   catch (...)
   { }
}

extern "C" DLL_PUBLIC unsigned int STDCALL AddEngineGroup(unsigned int engineId)
{
   try
   {
      int engineGroupId = engineGroups.size();
      cBubbleEngine::PTR found = GetEngine(engineId);
      found.ptr->SetGroup(engineGroupId);

      engineGroups[engineGroupId].push_back(engineId);
      engineToGroup[engineId] = engineGroupId;
      return engineGroupId;
   }
   catch (...)
   {
      return 0;
   }
}

extern "C" DLL_PUBLIC void STDCALL AddEngineToGroup(unsigned int engineGroupId, unsigned int engineId)
{
   try
   {
      cBubbleEngine::PTR found = GetEngine(engineId);
      found.ptr->SetGroup(engineGroupId);
      engineGroups[engineGroupId].push_back(engineId);
      engineToGroup[engineId] = engineGroupId;
   }
   catch (...)
   { }
}

extern "C" DLL_PUBLIC unsigned int STDCALL GetGroupCount(void)
{
   try
   {
      return engineGroups.size();
   }
   catch (...)
   {
      return 0;
   }
}

extern "C" DLL_PUBLIC unsigned int STDCALL GetEngineCount(void)
{
   try
   {
      return engines.size();
   }
   catch (...)
   {
      return 0;
   }
}

static std::vector<unsigned int /*engine Id*/> *GetGroup(unsigned int groupId)
{
   if (groupId+1 > engineGroups.size()) throw -2; // Invalid group ID
   return &(engineGroups[groupId]);
}

static std::vector<unsigned int /*engine Id*/> *GetEngineGroup(unsigned int engineId)
{   
   std::map<unsigned int, unsigned int>::iterator found = engineToGroup.find(engineId);
   return found == engineToGroup.end() 
      ? NULL : &( engineGroups[(unsigned int)( engineToGroup[engineId] )] );
}

extern "C" DLL_PUBLIC bool STDCALL AddBubble(unsigned int engineId, unsigned int bubbleId, float radius, GetCoordsFunc *getCoords)
{
   cBubbleEngine::PTR found = {0};
   try
   {
      found = GetEngine(engineId);
      if (found.ptr == NULL)
      {
         return false;
      }
   }
   catch (...)
   { 
      return false;
   }

   try
   {
      TimerWrapper::cMutexWrapper::Lock lock(found.ptr->GetLock());
      BUBBLE_COORDS &bubbleChords = *(&(bubbleCoords[bubbleId]));

      bubbleChords.getCoords = getCoords;
      bubbleChords.cached = false;
      cBubbleBubble *newOne = new cBubbleBubble(engineId, bubbleId, radius, GetBubbleXYZ);//getCoords);

      cBubbleBubble::PTR pimp = { newOne };

      found.ptr->FactoryAddWorkList(pimp);

      std::vector<unsigned int> *enginesInGroup = GetEngineGroup(found.ptr->GetID());
      if (enginesInGroup == NULL)
      {
         // add it to its own collision list and get out of here
         found.ptr->FactoryAddGroupList(pimp);
      }
      else // add the new bubble to each engine collision list in its group      
         for (int i=enginesInGroup->size()-1; i > -1; i--)
            GetEngine((*enginesInGroup)[i]).ptr->FactoryAddGroupList(pimp);

      return true;
   }
   catch (...)
   {
      return false;
   }
}

extern "C" DLL_PUBLIC void STDCALL RemoveBubble(unsigned int engineId, unsigned int bubbleId)
{
   cBubbleEngine::PTR found = {0};
   try
   {
      found = GetEngine(engineId);
      if (found.ptr == NULL) return;
   }
   catch (...)
   { }


   try
   {
      std::vector<unsigned int> *enginesInGroup = GetEngineGroup(found.ptr->GetID());
      if (enginesInGroup == NULL)
      {
         // remove it from its own collision list and get out of here
         found.ptr->FactoryRemove(bubbleId);
      }
      else // remove the new bubble from each engine collision list in its group      
         for (int i=enginesInGroup->size()-1; i > -1; i--)
            GetEngine((*enginesInGroup)[i]).ptr->FactoryRemove(bubbleId);
   }
   catch (...)
   { }
}

extern "C" DLL_PUBLIC void STDCALL SetEtheralness(unsigned int engineId, unsigned int bubbleId, bool etheralness)
{
   try
   {
      cBubbleEngine::PTR found = GetEngine(engineId);
      if (found.ptr == NULL) return;
      cBubbleBubble *bubble = found.ptr->FactoryGetBubble(bubbleId).ptr;
      if (bubble == NULL) return;
      bubble->FactorySetEtherealness(etheralness);
   }
   catch (...)
   { }
}

extern "C" DLL_PUBLIC unsigned int STDCALL GetBubbleCount(unsigned int engineId)
{
   try
   {
      cBubbleEngine::PTR found = GetEngine(engineId);
      return found.ptr->GetWorkList().size();
   }
   catch (...)
   {
      return 0;
   }
}

extern "C" DLL_PUBLIC void STDCALL StartEngine(unsigned int engineId, CollisionReportFunc *callback, unsigned int intervalMS) 
{
   try
   {
      cBubbleEngine::PTR found = GetEngine(engineId);
      found.ptr->Start(callback, intervalMS);
   }
   catch (...)
   { }
}

//typedef void GroupIdleCallback(void);
//static std::map<unsigned int /*groupId*/, GroupIdleCallback*> idleCallback;
//extern "C" DLL_PUBLIC void STDCALL SetGroupOnIdle(unsigned int groupId, GroupIdleCallback *callbackWhenIdle)
//{
//   idleCallback[groupId] = callbackWhenIdle;
//}
//
//extern "C" DLL_PUBLIC bool STDCALL IsGroupIdle(unsigned int groupId)
//{
//   TimerWrapper::cMutexWrapper mut;
//   TimerWrapper::cMutexWrapper::Lock lock(&mut);
//
//   std::vector<unsigned int> *enginesInGroup = GetEngineGroup(groupId);
//   int ret = 0;
//   for (int i=enginesInGroup->size()-1; i > -1; i--)
//   {
//      ret = SDL_CondWaitTimeout(GetEngine((*enginesInGroup)[i]).ptr->GetEngineIdleCond(), 
//                                 mut.GetMutex(), 0);
//      if (ret != 0)
//         break;
//   }
//   if (ret == 0)
//   {
//      // all have returned 0 meaning signalled
//      return true;
//   }
//   return false;
//}
//
//static int CheckEngineIdle(void *data)
//{
//   if (engineGroups.size() == 0) throw -2; // Invalid group ID
//   for (engineGroups[groupId]);
//   if (ISGroupIdle(
//}

extern "C" DLL_PUBLIC void STDCALL PauseEngine(unsigned int engineId, bool pause)
{
   try
   {
      cBubbleEngine::PTR found = GetEngine(engineId);
      found.ptr->SetPause(pause);
   }
   catch (...)
   { }
}

extern "C" DLL_PUBLIC void STDCALL PauseGroup(unsigned int groupId, bool pause)
{
   try
   {
      std::vector<unsigned int> *enginesInGroup = GetGroup(groupId);
      if (enginesInGroup == NULL) return;

      for (int i=enginesInGroup->size()-1; i > -1; i--)
         GetEngine((*enginesInGroup)[i]).ptr->SetPause(pause);
   }
   catch (...)
   { }
}