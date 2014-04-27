// Bubble.cpp
// Copyright (c) 2014
// Author: Matthew Cocks
// License: Attribution 4.0 International (CC BY 4.0)

#include "BubbleEngine.h"
#include "BubbleVariables.h"
#include <vector>
#include <SDL.h>
#include "BubbleSTDCALL.h"

#define BUILDING_DLL
#include "BubbleDLL_PUBLIC.h"
#include "Bubbles.h" //not needed here actually but include so to check it compiles

extern "C" DLL_PUBLIC bool STDCALL Init(void)
{
	Uint32 flags = SDL_INIT_TIMER;
	if (SDL_Init(flags) < 0)
		return false;

	if (SDL_WasInit(0) != flags)
	{
		SDL_Quit();
		return false;
	}
	return true;
}

class StopEngine : std::unary_function<cBubbleEngine::PTR, void>
{
	public:
		inline result_type operator () (const argument_type& engine) const
		{
			engine.ptr->Abort();
			delete engine.ptr;
		};
};

extern "C" DLL_PUBLIC void STDCALL UnInit(void)
{
	std::for_each(engines.begin(), engines.end(), StopEngine());

	SDL_Quit();
}

extern "C" DLL_PUBLIC unsigned int STDCALL AddEngine(void)
{
	unsigned int id = engines.size();
	cBubbleEngine &engine1 = *(new cBubbleEngine(id));
	cBubbleEngine::PTR pimp = { &engine1 };
	engines.push_back(pimp);
	return id;
}

static cBubbleEngine::PTR GetEngine(unsigned int engineId)
{
	if (engineId+1 > engines.size()) throw new std::bad_exception("Invalid engine ID");
	return engines[engineId];
}

extern "C" DLL_PUBLIC unsigned int STDCALL AddEngineGroup(unsigned int engineId)
{
	int engineGroupId = engineGroups.size();
	cBubbleEngine::PTR found = GetEngine(engineId);
	found.ptr->SetGroup(engineGroupId);

	engineGroups[engineGroupId].push_back(engineId);
	engineToGroup[engineId] = engineGroupId;
	return engineGroupId;
}

extern "C" DLL_PUBLIC void STDCALL AddEngineToGroup(unsigned int engineGroupId, unsigned int engineId)
{
	cBubbleEngine::PTR found = GetEngine(engineId);
	found.ptr->SetGroup(engineGroupId);
	engineGroups[engineGroupId].push_back(engineId);
	engineToGroup[engineId] = engineGroupId;
}

extern "C" DLL_PUBLIC unsigned int STDCALL GetGroupCount(void)
{
	return engineGroups.size();
}

extern "C" DLL_PUBLIC unsigned int STDCALL GetEngineCount(void)
{
	return engines.size();
}

static std::vector<unsigned int /*engine Id*/> *GetGroup(unsigned int groupId)
{
	if (groupId+1 > engineGroups.size()) throw new std::bad_exception("Invalid group ID");
	return &(engineGroups[groupId]);
}

static std::vector<unsigned int /*engine Id*/> *GetEngineGroup(unsigned int engineId)
{	
	std::map<unsigned int, unsigned int>::iterator found = engineToGroup.find(engineId);
	return found == engineToGroup.end() 
		? NULL : &( engineGroups[(unsigned int)( engineToGroup[engineId] )] );
}

extern "C" DLL_PUBLIC bool STDCALL AddBubble(unsigned int engineId, unsigned int bubbleId, float radius, cBubbleBubble::GetCoordsFunc *fptr)
{
	cBubbleEngine::PTR found = GetEngine(engineId);

	if (found.ptr == NULL)
	{
		return false;
	}

	for (cMutexWrapper::Lock lock(found.ptr->GetCollisionLock()) ;;)
	{
		cBubbleBubble::GetCoordsFunc *getCoords = (cBubbleBubble::GetCoordsFunc*)fptr;
		cBubbleBubble *newOne = new cBubbleBubble(engineId, bubbleId, radius, getCoords, false);

		cBubbleBubble::PTR pimp = { newOne };

		found.ptr->FactoryGetWorkList().push_back(pimp);			

		std::vector<unsigned int> *enginesInGroup = GetEngineGroup(found.ptr->GetID());
		if (enginesInGroup == NULL)
		{
			// add it to its own collision list and get out of here
			found.ptr->FactoryGetCollisionList().push_back(pimp);
			break; 
		}

		// add the new bubble to each engine collision list in its group
		for (int i=enginesInGroup->size()-1; i > -1; i--)
			GetEngine((*enginesInGroup)[i]).ptr->FactoryGetCollisionList().push_back(pimp);

		break;
	}
	return true;
}

extern "C" DLL_PUBLIC unsigned int STDCALL GetBubbleCount(unsigned int engineId)
{
	cBubbleEngine::PTR found = GetEngine(engineId);
	return found.ptr->GetWorkList().size();
}

extern "C" DLL_PUBLIC void STDCALL StartEngine(unsigned int engineId, cBubbleEngine::CollisionReportFunc *callback, unsigned int intervalMS) 
{
	cBubbleEngine::PTR found = GetEngine(engineId);
	found.ptr->Start(found.ptr->GetCollisionList(), callback, intervalMS);
}

extern "C" DLL_PUBLIC void STDCALL PauseEngine(unsigned int engineId, bool pause)
{
	cBubbleEngine::PTR found = GetEngine(engineId);
	found.ptr->SetPause(pause);
}

extern "C" DLL_PUBLIC void STDCALL PauseGroup(unsigned int groupId, bool pause)
{
	std::vector<unsigned int> *enginesInGroup = GetGroup(groupId);
	if (enginesInGroup == NULL) return;

	for (int i=enginesInGroup->size()-1; i > -1; i--)
		GetEngine((*enginesInGroup)[i]).ptr->SetPause(pause);
}