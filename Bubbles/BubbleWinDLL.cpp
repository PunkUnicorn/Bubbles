// BubbleWinDLL.cpp
// Copyright (c) 2014
// Author: Matthew Cocks
// License: Attribution 4.0 International (CC BY 4.0)

#if _MSC_VER

#include "BubbleTrace.h"
#include <Windows.h>
#include <SDL.h>

#define BUILDING_DLL
#include "BubbleDLL_PUBLIC.h"

static cBubbleTrace *trace;
extern "C" DLL_PUBLIC bool STDCALL InitWithTrace(int traceMode, cBubbleTrace::TraceFunc *traceFunc)
{
	trace = new cBubbleTrace(traceMode, traceFunc);
	trace->Trace(9000);
	
	HINSTANCE sdlHinst = LoadLibrary(L"SDL.dll");
	if (sdlHinst == NULL)
	{
		trace->Trace(9993);
		return false;
	}

	//http://sixserv.org/2010/05/21/dynamic-loading-of-sdl/
	//SDL_INIT *sdl_init = (SDL_INIT *) GetProcAddress(sdlHinst, "SDL_Init");
	//if (sdl_init != NULL)
	//{
	//	trace->Trace(9994);
	//	if ((*sdl_init)(SDL_INIT_TIMER) != 0)
	//	{
	//		trace->Trace(9995);
	//		return false;
	//	}
	//	return true;
	//}
	FreeLibrary(sdlHinst);

	Uint32 flags = SDL_INIT_TIMER;
	if (SDL_Init(SDL_INIT_TIMER) < 0)
	{
		trace->Trace(9996);
		if (SDL_Init(SDL_INIT_EVERYTHING) < 0)
		{
			trace->Trace(9997);
			return false;
		}
	}

	if (SDL_WasInit(SDL_INIT_TIMER) != SDL_INIT_TIMER)
	{
		trace->Trace(9998);
		SDL_Quit();
		return false;
	}

	trace->Trace(9999);
	return true;
}

BOOL APIENTRY DllMain(HANDLE hModule, 
                      DWORD  ul_reason_for_call, 
                      LPVOID lpReserved)
{
	switch( ul_reason_for_call ) 
	{
	    case DLL_PROCESS_ATTACH:
			break;

		case DLL_THREAD_ATTACH:
			break;

		case DLL_THREAD_DETACH:
			break;

		case DLL_PROCESS_DETACH:
			if (trace != NULL)
			{
				delete trace;
			}
			break;
    	}
    	return TRUE;
}

#endif //*#if _MSC_VER