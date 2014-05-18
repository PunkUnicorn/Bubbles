// Bubbles.h
// Copyright (c) 2014
// Author: Matthew Cocks
// License: Attribution 4.0 International (CC BY 4.0)

#ifndef BUBBLES_H
#define BUBBLES_H
#include "BubbleSTDCALL.h"
#define BUILDING_DLL
#include "BubbleDLL_PUBLIC.h"

namespace Bubbles 
{

class cAxisSplitterAXIS { public: enum { X = 0, Y, Z, NoOfDimensions }; };

typedef struct 
{
   /*cAxisSplitterAXIS*/int axis; //which axis this represents either the x, y or z
   float abs_dist; //absolute distance from id
   float rel_coord; //value of the axis (value of x, y or z)
   unsigned int id;
} TRILATERATION_DATA;

typedef struct 
{
   unsigned int mCenterID;
   TRILATERATION_DATA mDistanceUnits[cAxisSplitterAXIS::NoOfDimensions];
} COLLISION_RESULT;

typedef int STDCALL TraceFunc(unsigned int /*Id*/, int /*trace code*/);
typedef void STDCALL GetCoordsFunc(unsigned int /*engineId*/, unsigned int /*bubbleId*/, float &/*X*/, float &/*Y*/, float &/*Z*/);
typedef void STDCALL CollisionReportFunc(unsigned int /*group ID*/, unsigned int /*engine ID*/, COLLISION_RESULT*, unsigned int /*size*/);

extern "C" DLL_PUBLIC bool STDCALL InitWithTrace(int traceMode, TraceFunc *traceFunc);
extern "C" DLL_PUBLIC bool STDCALL Init(void);
extern "C" DLL_PUBLIC void STDCALL UnInit(void);
extern "C" DLL_PUBLIC unsigned int STDCALL AddEngine(void);
extern "C" DLL_PUBLIC void STDCALL SetEngineTimerTrace(unsigned int engineId, TraceFunc *timerTrace /*trace code is the duration in milliseconds*/);
extern "C" DLL_PUBLIC unsigned int STDCALL GetEngineCount(void);
extern "C" DLL_PUBLIC unsigned int STDCALL AddEngineGroup(unsigned int engineId);
extern "C" DLL_PUBLIC void STDCALL AddEngineToGroup(unsigned int engineGroupId, unsigned int engineId);
extern "C" DLL_PUBLIC unsigned int STDCALL GetGroupCount(void);
extern "C" DLL_PUBLIC bool STDCALL AddBubble(unsigned int engineId, unsigned int bubbleId, float radius, GetCoordsFunc *fptr);
extern "C" DLL_PUBLIC unsigned int STDCALL GetBubbleCount(unsigned int engineId);
extern "C" DLL_PUBLIC void STDCALL StartEngine(unsigned int engineId, CollisionReportFunc *callback, unsigned int intervalMS);
extern "C" DLL_PUBLIC void STDCALL PauseEngine(unsigned int engineId, bool pause);
extern "C" DLL_PUBLIC void STDCALL PauseGroup(unsigned int groupId, bool pause);

}

#endif