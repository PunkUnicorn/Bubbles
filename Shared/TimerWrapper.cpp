// TimerWrapper.cpp
// Copyright (c) 2014
// Author: Matthew Cocks
// License: Attribution 4.0 International (CC BY 4.0)

#include "TimerWrapper.h"

namespace TimerWrapper
{

/*
base class to encapsulate the SDL timer
*/

cTimerWrapper::cTimerWrapper(void) : mAbort(false), mThisTribbleIsDead(false), mPaused(false), mTimerID(0), mThreadID(NULL), mEventCallbackDelay(120) {};
cTimerWrapper::~cTimerWrapper(void)
{
   mAbort = true;
   if (mThreadID != NULL)
   {
      SDL_WaitThread(mThreadID, NULL);
   }
}

// static class function
void cTimerWrapper::AddTimer(cTimerWrapper *my_timer_instance)
{
   my_timer_instance->mTimerID = SDL_AddTimer( 
           my_timer_instance->mEventCallbackDelay, 
           cTimerWrapper::timer_callback, 
           (void *) (my_timer_instance) );
};

//static class function
void cTimerWrapper::AddThread(cTimerWrapper *my_timer_instance)
{
   my_timer_instance->mThreadID = SDL_CreateThread(thread_function, my_timer_instance);
}

// static class function
void cTimerWrapper::RemoveTimer(cTimerWrapper *my_timer_instance)
{
   SDL_RemoveTimer(my_timer_instance->mTimerID);
    my_timer_instance->mTimerID = 0;
    my_timer_instance->mAbort = true;
}

// static class function
unsigned int cTimerWrapper::timer_callback(unsigned int interval, void *pParam)
{
   cTimerWrapper *envoke = (cTimerWrapper *) pParam;
   if (envoke->mAbort)
   {
      const SDL_TimerID swapId = envoke->mTimerID;
		if (envoke->mTimerID != 0) 
      {
         envoke->mTimerID = 0;
		   SDL_RemoveTimer(swapId);
         envoke->mThisTribbleIsDead = true;
      }
      return 0;
   }   
   if (envoke->mPaused) return envoke->mEventCallbackDelay;
   try
   {
      envoke->EventTimer();
   }
   catch (...)
   {
      if (envoke->mAbort) throw -999;
   }

   return envoke->mEventCallbackDelay;
}

inline static Uint32 DecelerateTowardsEvent(Uint32 delayTime)
{
   static const int MINIMUM_DELAY = 0;
   Uint32 newDelayTime = delayTime >> 1;
   if (newDelayTime < MINIMUM_DELAY) newDelayTime = MINIMUM_DELAY;
   return newDelayTime;
}

// static class function
int cTimerWrapper::thread_function(void *data)
{
   cTimerWrapper *timer = (cTimerWrapper *)data;

   Uint32 startTime = SDL_GetTicks();
   Uint32 delayTime = DecelerateTowardsEvent(timer->mEventCallbackDelay);

   while (timer->mAbort == false)
   {
      if (timer->mPaused)
      {
         static const Uint32 GIVE_IT_A_MONKEY = 500;
         SDL_Delay(GIVE_IT_A_MONKEY);
         continue;
      }

      if (timer->mEventCallbackDelay != 0)
      {
         while ((SDL_GetTicks() - startTime) < timer->mEventCallbackDelay) 
         {
            SDL_Delay(delayTime);
            if (timer->mAbort) break;
            delayTime = DecelerateTowardsEvent(delayTime);
         }
         if (timer->mAbort) break;
         if (timer->mPaused) continue;
      }

      try
      {
         timer->EventTimer();
      }
      catch (...)
      {
         if (timer->mAbort == false) throw;
      }

      startTime = SDL_GetTicks();
      delayTime = DecelerateTowardsEvent(timer->mEventCallbackDelay);
   }

   try
   {
      timer->mThisTribbleIsDead = true;
   }
   catch (...) {}
   return 0;
}

}