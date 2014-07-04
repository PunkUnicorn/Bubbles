// BubbleEngine.h
// Copyright (c) 2014
// Author: Matthew Cocks
// License: Attribution 4.0 International (CC BY 4.0)

#ifndef BUBBLECOLLISIONREPORTTHREAD_H
#define BUBBLECOLLISIONREPORTTHREAD_H
#include <SDL.h>
#include <allocators>

namespace Bubbles
{

// looks after delivering the collision report to the application callback on a dedicated thread
class cBubbleCollisionReportThread 
{
private:
   typedef struct tagREPORT_PAYLOAD
   {
      unsigned int groupID; 
      unsigned int engineId; 
      COLLISION_RESULT* list; 
      unsigned int size;
      CollisionReportFunc *reportFunc;
      std::allocator<tagREPORT_PAYLOAD> *allocator;
   } REPORT_PAYLOAD;

   std::allocator<REPORT_PAYLOAD> mPayloadAllocator;
   SDL_Thread *mThreadID;

   static int thread_function(void *p)
   {
      try
      {
         if (p == NULL) return 0;
         REPORT_PAYLOAD &payload = *(REPORT_PAYLOAD *)p;
        
         payload.reportFunc(payload.groupID, payload.engineId, payload.list, payload.size);
         payload.allocator->deallocate(&payload, 1);

         //                
         //             ,\ ! /, 
         //            -- POP --
         //             '/ ! \' 
         //
      }
      catch (...)
      {
         return -1;
      }
      return 0;
   };

public:
   cBubbleCollisionReportThread(void) : mPayloadAllocator(), mThreadID(NULL) { };
   void Init(void) { mThreadID = SDL_CreateThread(thread_function, NULL); };
   static void Start(cBubbleCollisionReportThread &me, CollisionReportFunc *func, unsigned int pgroupID, unsigned int pengineId, COLLISION_RESULT* plist, unsigned int psize)
   {
      REPORT_PAYLOAD *p = me.mPayloadAllocator.allocate(1);
      REPORT_PAYLOAD &payload = *p;

      payload.reportFunc = func;
      payload.groupID = pgroupID;
      payload.engineId = pengineId;
      payload.list = plist;
      payload.size = psize;
      payload.allocator = &me.mPayloadAllocator;

      int threadReturnValue;
      SDL_WaitThread(me.mThreadID, &threadReturnValue);
      me.mThreadID = SDL_CreateThread(thread_function, p);
   };
   virtual ~cBubbleCollisionReportThread() { int threadReturnValue; SDL_WaitThread(mThreadID, &threadReturnValue); };
};

}

#endif