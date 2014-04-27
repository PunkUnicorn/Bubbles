// BubbleTimer.h
// Copyright (c) 2014
// Author: Matthew Cocks
// License: Attribution 4.0 International (CC BY 4.0)

#ifndef BUBBLETIMER_H
#define BUBBLETIMER_H
#include <SDL.h>

class cMutexWrapper
{
	private:
		SDL_mutex *mMutex;

	public:
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

		~cMutexWrapper(void)
		{
			SDL_DestroyMutex(mMutex);
		}		
};

/*
base class to encapsulate the SDL timer
*/
class cTimerWrapper
{
	private:
		bool mAbort;
		bool mPaused;
        SDL_TimerID mTimerID;
		SDL_Thread *mThreadID;
        unsigned int mEventCallbackDelay;

    public:
		void FactorySetDelay(unsigned int delay) { mEventCallbackDelay = delay; };
		void SetPause(bool pause) 
		{
			if (mPaused == pause) return;
			mPaused = pause;
		}

		void Abort(void) { mAbort = true; }

        virtual void EventTimer(void) = 0;
        virtual bool IsExpired(void) = 0;
        cTimerWrapper(void);
        ~cTimerWrapper(void);
        
        static void AddTimer(cTimerWrapper *my_timer_instance);

		static void AddThread(cTimerWrapper *my_timer_instance);

        static void RemoveTimer(cTimerWrapper *my_timer_instance);

        static unsigned int timer_callback(unsigned int interval, void *pParam);

		static int thread_function(void *data);
};

#endif