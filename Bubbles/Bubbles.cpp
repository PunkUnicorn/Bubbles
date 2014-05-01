// Bubbles.cpp
// Copyright (c) 2014
// Author: Matthew Cocks
// License: Attribution 4.0 International (CC BY 4.0)
#include "BubbleTrace.h"
#include <SDL.h>
#include <string>
#include <iostream>

#define BUILDING_DLL
#include "BubbleDLL_PUBLIC.h"

#ifdef _WINDLL //there is a #else. #else a console
#include <Windows.h>

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



#else //console


#include <vector>
#include <map>
#include "BubbleBubble.h"
#include "Bubbles.h"

extern "C" void STDCALL GetCoordsCallback(unsigned int engineId, unsigned int bubbleId, float &X, float &Y, float &Z)
{
    X=+(float)bubbleId;
    Y=+(float)bubbleId;
    Z=0;

    if (X > 50) X=1;
    if (Y > 50) Y=1;

    std::cerr << "{ ""getCoords"":{" << std::endl;

        std::cerr << "\t" << "{ ""engineId"":""" << engineId << """" << "}" << std::endl;
        std::cerr << "\t" << "{ ""bubbleId"":""" << bubbleId << """" << "}" << std::endl;
        std::cerr << "\t" << "{ ""x"":""" << X << """" << "}" << std::endl;
        std::cerr << "\t" << "{ ""y"":""" << Y << """" << "}" << std::endl;
        std::cerr << "\t" << "{ ""z"":""" << Z << """" << "}" << std::endl;	

    std::cerr << "}" << "}" << std::endl;
}

extern "C" void STDCALL GetCollisionReportCallback(unsigned int groupId, unsigned int engineId, cBubbleBubble::COLLISION_RESULT* collisions, unsigned int size)
{
    if (size == 0) return;

    std::cerr << "{ ""collisionReport"":{" << std::endl;

        std::cerr << "{ ""groupId"":""" << groupId << """" << "}" << std::endl;
        std::cerr << "{ ""engineId"":""" << groupId << """" << "}" << std::endl;

        std::cerr << "{ ""collisions"":{" << std::endl;
	    
        for (unsigned int i=0; i < size; i++)
        {
            cBubbleBubble::COLLISION_RESULT *oneBang = &(collisions[i]);

            std::cerr << "\t" << "{ ""centerId"":""" << oneBang->mCenterID << """" << "}" << std::endl;
            std::cerr << "\t" << "{ ""otherId"":""" << oneBang->mDistanceUnits[0].id << """" << "}" << std::endl;
  
            std::cerr << "\t" << "{ ""x"":""" << oneBang->mDistanceUnits[0].rel_coord << """" << "}" << std::endl;
            std::cerr << "\t" << "{ ""y"":""" << oneBang->mDistanceUnits[1].rel_coord << """" << "}" << std::endl;
            std::cerr << "\t" << "{ ""x"":""" << oneBang->mDistanceUnits[2].rel_coord << """" << "}" << std::endl;
  
            std::cerr << "\t" << "{ ""xDist"":""" << oneBang->mDistanceUnits[0].abs_dist << """" << "}" << std::endl;
            std::cerr << "\t" << "{ ""yDist"":""" << oneBang->mDistanceUnits[1].abs_dist << """" << "}" << std::endl;
            std::cerr << "\t" << "{ ""zDist"":""" << oneBang->mDistanceUnits[2].abs_dist << """" << "}" << std::endl;
        }
        
        std::cerr << "}" << "}" << std::endl;

    std::cerr << "}" << "}" << std::endl;
}

static std::vector<std::string> history;
static std::map<std::string, unsigned int> engines;
static std::vector<unsigned int> bubbles;
static unsigned int groupId;
static bool ConAddBubble(std::string engineKey, float radius)
{
    const static std::string winning(" winning");
    const static std::string fail(" fail");

    unsigned int id = (unsigned int)bubbles.size()+1;
    std::cout << id << (AddBubble(engines[engineKey], id, radius, GetCoordsCallback) ? winning : fail) << std::endl;
    bubbles.push_back(id);
    return true;
}
static bool ConAddEngine(std::string key)
{
    unsigned int id = AddEngine();
    std::cout << id << std::endl;
    if (engines.size() == 0)
            groupId = AddEngineGroup(id);
    
    engines[key]=id;
    return true;
}
static bool ConStartEngine(std::string key, unsigned int intervalMS)
{
    StartEngine(engines[key], GetCollisionReportCallback, intervalMS);
    std::cout << "started" << std::endl;
    return true;
}
static bool ConHelp(std::string param)
{ 
    std::cout << "AddEngine <name you provide>" << std::endl; 
    std::cout << "-creates a new engine with your given name" << std::endl;
    std::cout << std::endl;
    std::cout << "AddBubble <engine name>" << std::endl; 
    std::cout << "  -adds a bubble to engine" << std::endl;
    std::cout << std::endl;
    std::cout << "StartEngine <engine name>" << std::endl; 
    std::cout << "  -starts the engine with 200 ms interval" << std::endl;
    std::cout << std::endl;
    std::cout << "Bubble coordinates and collision results reported through stderr" << std::endl;

    return true;
}

static void ProcessCommand(std::string &command)
{
    const static std::string addEngine("AddEngine");
    const static std::string addBubble("AddBubble");
    const static std::string startEngine("StartEngine");
    const static std::string help("help");

    int spaceMan = command.find_first_of(' ');
    std::string param = command.substr(spaceMan+1);
    bool commanded = false;

    if (command.substr(0, spaceMan) == addEngine) commanded = ConAddEngine(param);
    if (command.substr(0, spaceMan) == addBubble) commanded = ConAddBubble(param, 5.0f);
    if (command.substr(0, spaceMan) == startEngine) commanded = ConStartEngine(param, 200);
    if (command.substr(0, spaceMan) == help) commanded = ConHelp(param);

    if (commanded == false)
        std::cout << command << "<-- error with what this is" << std::endl;
}

int main(void)
{
    Init();
    std::cerr << "{ ""consoleStart"":{" << std::endl;

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

    std::cout << "Bubbles 3D Collision Detection Engine 0.2";
    std::cout << exit << std::endl;
    
    std::string line;
    while (std::getline(std::cin, line))
    {
	history.push_back(line);
	if (line == exit) break;
        ProcessCommand(line);
    }

    std::cerr << "}" << "}" << std::endl;
    UnInit();

    return 0;
}

#endif 