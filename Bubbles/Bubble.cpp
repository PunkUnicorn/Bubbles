// Bubble.cpp
// Copyright (c) 2014
// Author: Matthew Cocks
// License: Attribution 4.0 International (CC BY 4.0)
#define BUILDING_DLL
#include "BubbleEngine.h"
#include "BubbleVariables.h"
#include <vector>
#include <SDL.h>
#include "BubbleSTDCALL.h"

#include "BubbleDLL_PUBLIC.h"

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
         if (mAbort) engine.ptr->Abort();
         else delete engine.ptr;
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
   std::for_each(engines.begin(), engines.end(), UnInitBubblesClasses::StopEngine(true));

   bool hasAborted;
   do 
   {
      hasAborted = false;
      SDL_Delay(10);
      std::for_each(engines.begin(), engines.end(), UnInitBubblesClasses::CheckEngineState(hasAborted));
   }
   while (hasAborted == false);
   std::for_each(engines.begin(), engines.end(), UnInitBubblesClasses::StopEngine(false));
}

// this function works but is shit, click here to re-write it
// passed to the engine so it can clear the groups bubbles X, Y and Z cached values
extern "C" static void STDCALL ClearCache(unsigned int groupId)
{
   // clear the cache when the last engine of the group calls ClearCache
   static std::map<unsigned int /*groupId*/, unsigned int /*call per group count*/> callPerGroup;
   if (++callPerGroup[groupId] != engineGroups[groupId].size()) 
      return;

   for (std::map<unsigned int, BUBBLE_COORDS>::iterator it=bubbleCoords.begin(); it != bubbleCoords.end(); ++it)
      if (groupId == it->second.groupId) it->second.cached = false;

   callPerGroup[groupId] = 0;
}

// passed to the engine for it to get a bubbles X, Y and Z
extern "C" static void STDCALL GetBubbleXYZ(unsigned int engineId, unsigned int id, float &px, float &py, float &pz)
{
   BUBBLE_COORDS &bubbleChords = *(&(bubbleCoords[id]));

   if (bubbleChords.cached == false)
   {
      bubbleChords.cached = true;
      (*bubbleChords.getCoords)(engineId, id, bubbleChords.x, bubbleChords.y, bubbleChords.z);
   }

   px = bubbleChords.x;
   py = bubbleChords.y;
   pz = bubbleChords.z;
}

extern "C" DLL_PUBLIC unsigned int STDCALL AddEngine(void)
{
   unsigned int id = engines.size();
   cBubbleEngine *engine1 = new cBubbleEngine(id, GetBubbleXYZ, (cBubbleEngine::ClearCacheFunc *) ClearCache);
   cBubbleEngine::PTR pimp = { engine1 };
   engines.push_back(pimp);      
   return id;
}

static cBubbleEngine::PTR GetEngine(unsigned int engineId)
{
   if (engineId+1 > engines.size()) throw -1; // Invalid engine ID
   return engines[engineId];
}

extern "C" DLL_PUBLIC void STDCALL SetEngineTimerTrace(unsigned int engineId, TraceFunc *timerTrace)
{
   cBubbleEngine::PTR found = GetEngine(engineId);
   found.ptr->SetTimerTraceFunc(timerTrace);
}

extern "C" DLL_PUBLIC unsigned int STDCALL AddEngineGroup(unsigned int engineId)
{
   int engineGroupId = engineGroups.size();
   cBubbleEngine::PTR found = GetEngine(engineId);
   found.ptr->SetGroup(engineGroupId);

   engineGroups[engineGroupId].push_back(engineId);
   engineToGroup[engineId] = engineGroupId;
   return engineGroupId;
}

extern "C" DLL_PUBLIC void STDCALL AddEngineToGroup(unsigned int engineGroupId, unsigned int engineId)
{
   cBubbleEngine::PTR found = GetEngine(engineId);
   found.ptr->SetGroup(engineGroupId);
   engineGroups[engineGroupId].push_back(engineId);
   engineToGroup[engineId] = engineGroupId;
}

extern "C" DLL_PUBLIC unsigned int STDCALL GetGroupCount(void)
{
   return engineGroups.size();
}

extern "C" DLL_PUBLIC unsigned int STDCALL GetEngineCount(void)
{
   return engines.size();
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
   cBubbleEngine::PTR found = GetEngine(engineId);

   if (found.ptr == NULL)
   {
      return false;
   }

   try
   {
      cMutexWrapper::Lock lock(found.ptr->GetCollisionLock());
      BUBBLE_COORDS &bubbleChords = *(&(bubbleCoords[bubbleId]));

      bubbleChords.getCoords = getCoords;
      bubbleChords.cached = false;
      cBubbleBubble *newOne = new cBubbleBubble(engineId, bubbleId, radius, GetBubbleXYZ);

      cBubbleBubble::PTR pimp = { newOne };

      found.ptr->FactoryGetWorkList().push_back(pimp);         

      std::vector<unsigned int> *enginesInGroup = GetEngineGroup(found.ptr->GetID());
      if (enginesInGroup == NULL)
      {
         // add it to its own collision list and get out of here
         found.ptr->FactoryGetCollisionList().push_back(pimp);
         //break; 
      }
      else // add the new bubble to each engine collision list in its group      
         for (int i=enginesInGroup->size()-1; i > -1; i--)
            GetEngine((*enginesInGroup)[i]).ptr->FactoryGetCollisionList().push_back(pimp);
      return true;
   }
   catch (...)
   {
      return false;
   }
}

extern "C" DLL_PUBLIC void STDCALL SetEtheralness(unsigned int engineId, unsigned int bubbleId, bool etheralness)
{
    cBubbleEngine::PTR found = GetEngine(engineId);
    found.ptr->FactoryGetBubble(bubbleId).ptr->FactorySetEtherealness(etheralness);
}

extern "C" DLL_PUBLIC unsigned int STDCALL GetBubbleCount(unsigned int engineId)
{
   cBubbleEngine::PTR found = GetEngine(engineId);
   return found.ptr->GetWorkList().size();
}

extern "C" DLL_PUBLIC void STDCALL StartEngine(unsigned int engineId, CollisionReportFunc *callback, unsigned int intervalMS) 
{
   cBubbleEngine::PTR found = GetEngine(engineId);
   found.ptr->Start(callback, intervalMS);
}

extern "C" DLL_PUBLIC void STDCALL PauseEngine(unsigned int engineId, bool pause)
{
   cBubbleEngine::PTR found = GetEngine(engineId);
   found.ptr->SetPause(pause);
}

extern "C" DLL_PUBLIC void STDCALL PauseGroup(unsigned int groupId, bool pause)
{
   std::vector<unsigned int> *enginesInGroup = GetGroup(groupId);
   if (enginesInGroup == NULL) return;

   for (int i=enginesInGroup->size()-1; i > -1; i--)
      GetEngine((*enginesInGroup)[i]).ptr->SetPause(pause);
}