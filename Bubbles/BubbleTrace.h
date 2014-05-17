// BubbleTrace.h
// Copyright (c) 2014
// Author: Matthew Cocks
// License: Attribution 4.0 International (CC BY 4.0)
#include "Bubbles.h"

#ifndef BUBBLETRACE_H
#define BUBBLETRACE_H

namespace Bubbles
{

//#include "BubbleSTDCALL.h"

#ifndef NULL
#	define NULL (0x0)
#	define IDEFINEDNULLSUCKMYBALLS
#endif

class cBubbleTrace
{
	public:
		//typedef int STDCALL TraceFunc(int);
		static int STDCALL DummyTraceFunc(unsigned int engineId, int i) { return 0; };
		inline int Trace (unsigned int engineId, int i)
		{
			//if (mMode < i) return 0;
			//if (mTraceFunc == NULL) throw "mTraceFunc is NULL";
			//(*mTraceFunc) (42);
			return (*mTraceFunc)(engineId, i);
		}
		cBubbleTrace(int mode, TraceFunc *newFunc = DummyTraceFunc) : mMode(mode), mTraceFunc(newFunc) { };

	private:
		int mMode;
		TraceFunc *mTraceFunc;
};

#ifdef IDEFINEDNULLSUCKMYBALLS
#	undef NULL
#	undef IDEFINEDNULLSUCKMYBALLS
#endif

}

#endif