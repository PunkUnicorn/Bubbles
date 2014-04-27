// BubbleTrace.h
// Copyright (c) 2014
// Author: Matthew Cocks
// License: Attribution 4.0 International (CC BY 4.0)

#ifndef BUBBLETRACE_H
#define BUBBLETRACE_H
#include "BubbleSTDCALL.h"

#ifndef NULL
#	define NULL (0x0)
#	define IDEFINEDNULLSUCKMYBALLS
#endif

class cBubbleTrace
{
	public:
		typedef int STDCALL TraceFunc(int);
		static int STDCALL DummyTraceFunc(int i) { return 0; };
		inline int Trace (int i)
		{
			//if (mMode < i) return 0;
			//if (mTraceFunc == NULL) throw "mTraceFunc is NULL";
			//(*mTraceFunc) (42);
			return (*mTraceFunc)(i);
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

#endif