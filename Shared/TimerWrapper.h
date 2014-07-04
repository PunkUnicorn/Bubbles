// TimerWrapper.h
// Copyright (c) 2014
// Author: Matthew Cocks
// License: Attribution 4.0 International (CC BY 4.0)

#ifndef TIMERWRAPPER_H
#define TIMERWRAPPER_H
#include <SDL.h>

namespace TimerWrapper
{

class cMutexWrapper
{
private:
   SDL_mutex *mMutex;

public:
   inline SDL_mutex *GetMutex(void) { return mMutex; }
   class Lock
   {
      private: cMutexWrapper *mMutex;
      public: Lock(cMutexWrapper *mutex) : mMutex(mutex) { SDL_mutexP(mMutex->mMutex); }
      ~Lock(void) { SDL_mutexV(mMutex->mMutex); }
   };

   cMutexWrapper(void)
   {
      mMutex = SDL_CreateMutex();
   }

   virtual ~cMutexWrapper(void)
   {
      SDL_DestroyMutex(mMutex);
   }      
};

/*
base class to encapsulate the SDL timer
*/
class cTimerWrapper
{
protected:
   bool mAbort;

private:
   bool mThisTribbleIsDead; //set when the timer has successfully aborted
   bool mPaused;
   SDL_TimerID mTimerID;
   SDL_Thread *mThreadID;
   unsigned int mEventCallbackDelay;

public:
   inline void FactorySetDelay(unsigned int delay) { mEventCallbackDelay = delay; };
   inline unsigned int GetDelay(void) const { return mEventCallbackDelay; }
   inline void SetPause(bool pause) 
   {
      if (mPaused == pause) return;
      mPaused = pause;
   }

   inline void Abort(void) { mAbort = true; }
   inline bool IsAborting(void) { return mAbort; }
   inline bool HasAborted(void) { return mThisTribbleIsDead; }

   virtual void EventTimer(void) = 0;
   cTimerWrapper(void);
   virtual ~cTimerWrapper(void);
        
   static void AddTimer(cTimerWrapper *my_timer_instance);

   static void AddThread(cTimerWrapper *my_timer_instance);

   static void RemoveTimer(cTimerWrapper *my_timer_instance);

   static unsigned int timer_callback(unsigned int interval, void *pParam);

   static int thread_function(void *data);
};

}

#endif

