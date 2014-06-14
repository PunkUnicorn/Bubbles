// BubbleConsole.cpp
// Copyright (c) 2014
// Author: Matthew Cocks
// License: Attribution 4.0 International (CC BY 4.0)
#define BUILDING_DLL

#include <vector>
#include <map>
#include <string>
#include <iostream>
#include <ctime>
#include "BubbleBubble.h"
#include "Bubbles.h"
#include "BubbleTimer.h"
#include "BubbleConsole.h"

using namespace Bubbles;

static cMutexWrapper stderrLock;
extern "C" void STDCALL TimerTrace(int duration)
{
    time_t curTime;
    time(&curTime);
    tm timeinfo;
    localtime_s(&timeinfo, &curTime);

    // lock scope
    {
       cMutexWrapper::Lock lock(&stderrLock) ;
       std::cerr << "{ ""duration"":{" << std::endl;

         static char buffer[256];
         std::cerr << "\t" << "{ ""time"":""" << asctime_s(buffer, sizeof(buffer), &timeinfo) << """" << "}" << std::endl;
         std::cerr << "\t" << "{ ""duration"":""" << duration << """" << "}" << std::endl;

       std::cerr << "}" << "}" << std::endl;
    }
}

extern "C" static void STDCALL ConsoleGetCoordsCallback(unsigned int engineId, unsigned int bubbleId, float &X, float &Y, float &Z)
{
    X=+(float)bubbleId;
    Y=+(float)bubbleId;
    Z=0;

    if (X > 50) X=1;
    if (Y > 50) Y=1;

    // lock scope
    {
        cMutexWrapper::Lock lock(&stderrLock) ;
        std::cerr << "{ ""getCoords"":{" << std::endl;

            std::cerr << "\t" << "{ ""engineId"":""" << engineId << """" << "}" << std::endl;
            std::cerr << "\t" << "{ ""bubbleId"":""" << bubbleId << """" << "}" << std::endl;
            std::cerr << "\t" << "{ ""x"":""" << X << """" << "}" << std::endl;
            std::cerr << "\t" << "{ ""y"":""" << Y << """" << "}" << std::endl;
            std::cerr << "\t" << "{ ""z"":""" << Z << """" << "}" << std::endl;   

        std::cerr << "}" << "}" << std::endl;
    }
}

extern "C" static void STDCALL ConsoleGetCollisionReportCallback(unsigned int groupId, unsigned int engineId, COLLISION_RESULT* collisions, unsigned int size)
{
    if (size == 0) return;

    // lock scope
    {
        cMutexWrapper::Lock lock(&stderrLock);
        std::cerr << "{ ""collisionReport"":{" << std::endl;

            std::cerr << "{ ""groupId"":""" << groupId << """" << "}" << std::endl;
            std::cerr << "{ ""engineId"":""" << groupId << """" << "}" << std::endl;

            std::cerr << "{ ""collisions"":{" << std::endl;
       
            for (unsigned int i=0; i < size; i++)
            {
                COLLISION_RESULT *oneBang = &(collisions[i]);

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
}

static std::map<std::string, unsigned int> engines;
static std::vector<unsigned int> bubbles;
static unsigned int groupId;
static bool ConAddBubble(std::string engineKey, float radius)
{
    const static std::string winning(" winning");
    const static std::string fail(" fail");

    unsigned int id = (unsigned int)bubbles.size()+1;
    std::cout << id << (AddBubble(engines[engineKey], id, radius, ConsoleGetCoordsCallback) ? winning : fail) << std::endl;
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
    StartEngine(engines[key], ConsoleGetCollisionReportCallback, intervalMS);
    std::cout << "started" << std::endl;
    return true;
}
static bool ConHelp(std::string param)
{ 
    std::cout << "AddEngine <name you provide> e.g. AddEngine fred" << std::endl; 
    std::cout << "-creates a new engine with your given name" << std::endl;
    std::cout << std::endl;
    std::cout << "AddBubble <engine name> e.g. AddBubble fred" << std::endl; 
    std::cout << "  -adds a bubble to engine" << std::endl;
    std::cout << std::endl;
    std::cout << "StartEngine <engine name> e.g. StartEngine fred" << std::endl; 
    std::cout << "  -starts the engine with 200 ms interval" << std::endl;
    std::cout << std::endl;
    std::cout << "Bubble coordinates and collision results reported through stderr" << std::endl;

    return true;
}

void ProcessCommand(std::string &command)
{
    const static std::string addEngine("AddEngine");
    const static std::string addBubble("AddBubble");
    const static std::string startEngine("StartEngine");
    const static std::string help("help");

    int spaceMan = command.find_first_of(' ');
    std::string param1;
    int param2 = 200;

    bool oneTimeOnly = true;
    if  (spaceMan == std::string::npos) command = help;
    else for (; oneTimeOnly; oneTimeOnly = false)
    {
        int secondSpaceMan;
        param1 = command.substr(spaceMan);
        if (param1.size() == 0) break;
        secondSpaceMan = param1.find_first_of(' ');
        if (secondSpaceMan == std::string::npos) break;
        try 
        {
            param2 = atoi(param1.substr(secondSpaceMan).c_str());
        }
        catch (...)
        {
            param2 = 200;
        }
        param1 = param1.substr(0, secondSpaceMan);
    }

    bool commanded = false;
    if (command.substr(0, spaceMan) == addEngine) commanded = ConAddEngine(param1);
    if (command.substr(0, spaceMan) == addBubble) commanded = ConAddBubble(param1, (float)param2);
    if (command.substr(0, spaceMan) == startEngine) commanded = ConStartEngine(param1, param2);
    if (command.substr(0, spaceMan) == help) commanded = ConHelp(param1);

    if (commanded == false)
        std::cout << command << "<-- error with what this is, try 'help'" << std::endl;
}
