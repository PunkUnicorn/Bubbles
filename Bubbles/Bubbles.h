// Bubbles.h
// Copyright (c) 2014
// Author: Matthew Cocks
// License: Attribution 4.0 International (CC BY 4.0)

#ifndef BUBBLES_H
#define BUBBLES_H
#include "BubbleTrace.h"
#include "BubbleEngine.h"
#include "BubbleBubble.h"
#include "BubbleSTDCALL.h"
#include "BubbleDLL_PUBLIC.h"

/* 
useage:
add an engine
if using two engines to share the workload of bubbles then add a group passing in the first engine, 
then add subsiquent engines and call AddEngineToGroup to associate the new engines with the group
then add the bubbles /after/ assigning engines to groups
*/

extern "C" DLL_PUBLIC bool STDCALL InitWithTrace(int traceMode, cBubbleTrace::TraceFunc *traceFunc); //<--platform specific
extern "C" DLL_PUBLIC bool STDCALL Init(void);
extern "C" DLL_PUBLIC void STDCALL UnInit(void);
extern "C" DLL_PUBLIC unsigned int STDCALL AddEngine(void);
extern "C" DLL_PUBLIC unsigned int STDCALL GetEngineCount(void);
extern "C" DLL_PUBLIC unsigned int STDCALL AddEngineGroup(unsigned int engineId);
extern "C" DLL_PUBLIC void STDCALL AddEngineToGroup(unsigned int engineGroupId, unsigned int engineId);
extern "C" DLL_PUBLIC unsigned int STDCALL GetGroupCount(void);
extern "C" DLL_PUBLIC bool STDCALL AddBubble(unsigned int engineId, unsigned int bubbleId, float radius, cBubbleBubble::GetCoordsFunc *fptr);
extern "C" DLL_PUBLIC unsigned int STDCALL GetBubbleCount(unsigned int engineId);
extern "C" DLL_PUBLIC void STDCALL StartEngine(unsigned int engineId, cBubbleEngine::CollisionReportFunc *callback, unsigned int intervalMS);
extern "C" DLL_PUBLIC void STDCALL PauseEngine(unsigned int engineId, bool pause);
extern "C" DLL_PUBLIC void STDCALL PauseGroup(unsigned int groupId, bool pause);

#endif