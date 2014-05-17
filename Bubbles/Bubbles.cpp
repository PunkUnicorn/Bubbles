// Bubbles.cpp
// Copyright (c) 2014
// Author: Matthew Cocks
// License: Attribution 4.0 International (CC BY 4.0)
#include "BubbleTrace.h"
#include "BubbleConsole.h"
#include <SDL.h>
//#include <string>
#include <iostream>
#include "BubbleSTDCALL.h"

#define BUILDING_DLL
#include "BubbleDLL_PUBLIC.h"
#include "Bubbles.h"

#ifdef _WINDLL // <-------- there is a #else
#include <Windows.h>

using namespace Bubbles;

static cBubbleTrace *trace;
extern "C" DLL_PUBLIC bool STDCALL InitWithTrace(int traceMode, TraceFunc *traceFunc)
{
	trace = new cBubbleTrace(traceMode, traceFunc);
	trace->Trace(0, 9000);
	
	HINSTANCE sdlHinst = LoadLibrary(L"SDL.dll");
	if (sdlHinst == NULL)
	{
		trace->Trace(0, 9993);
		return false;
	}
	FreeLibrary(sdlHinst);

	Uint32 flags = SDL_INIT_TIMER;
	if (SDL_Init(SDL_INIT_TIMER) < 0)
	{
		trace->Trace(0, 9996);
		if (SDL_Init(SDL_INIT_EVERYTHING) < 0)
		{
			trace->Trace(0, 9997);
			return false;
		}
	}

	if (SDL_WasInit(SDL_INIT_TIMER) != SDL_INIT_TIMER)
	{
		trace->Trace(0, 9998);
		SDL_Quit();
		return false;
	}

	trace->Trace(0, 9999);
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

//#else //console

int main(void)
{
    Init();
    std::cerr << "{ ""bubblesEngine"":{" << std::endl;

    std::string exit;
    std::cout << "        .---.         .-----------" << std::endl;
    std::cout << "       /     \\  __  /    ------   bubble console bird bows and greets you" << std::endl;
    std::cout << "      / /     \\(--)/    -----" << std::endl;
    std::cout << "     //////   ' \\/ `   ---" << std::endl;
    std::cout << "    //// / // :    : ---" << std::endl;
    std::cout << "   // /   /  /`    '--" << std::endl;
    std::cout << "  //          //..\\\\" << std::endl;
    std::cout << "=============UU====UU====" << std::endl;
    std::cout << "             '//||\\\\`" << std::endl;
    std::cout << "               ''``" << std::endl;
    std::cout << "ASCII Art by Daron Brewood (http://www.chris.com/ascii/index.php)" << std::endl;

    std::cout << "Bubbles 3D Collision Detection Engine 0.3";
    std::cout << exit << std::endl;
    
    std::string line;
    while (std::getline(std::cin, line))
    {
	    if (line == exit) break;
        ProcessCommand(line);
    }

    std::cerr << "}" << "}" << std::endl;
    UnInit();

    return 0;
}

#endif 
