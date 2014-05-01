// BubbleEngine.h
// Copyright (c) 2014
// Author: Matthew Cocks
// License: Attribution 4.0 International (CC BY 4.0)

#ifndef BUBBLEENGINE_H
#define BUBBLEENGINE_H
#include <vector>
#include <algorithm>
#include <map>
#include "BubbleSTDCALL.h"
#include "BubbleBubble.h"
#include "BubbleTimer.h"
#include "BubbleDimensionCracker.h"
#include "BubbleFindCollisions.h"

class cBubbleEngine : public cTimerWrapper
{
public:
	static const unsigned int RESERVE_COLLISIONRESULTS = 600;

	typedef struct
	{
		cBubbleEngine *ptr;
	} PTR;

	static const unsigned int FOUR_A_SECOND = 250;
	static const unsigned int FIVE_A_SECOND = 200;
	static const unsigned int SIX_A_SECOND = 166;

	typedef void STDCALL CollisionReportFunc(unsigned int /*group ID*/, unsigned int /*engine ID*/, cBubbleBubble::COLLISION_RESULT*, unsigned int /*size*/);

	cBubbleEngine(unsigned int ID, unsigned int reserveAmount = RESERVE_COLLISIONRESULTS) 
		: mGroupID(0),
        mID(ID), 
		mReserveAmount(reserveAmount), 
		mFlipFlop(false),
		mCollisionList(),
		mCollisionListLock(),
		mWorkList(),
        mDistanceList(reserveAmount * 3),
        mCollisionResults(reserveAmount),
        mCollisionReportFunc(NULL)
	{ 
		//mDistanceList.reserve(reserveAmount * 3); 
		//mCollisionResults.reserve(reserveAmount);
	}

	inline std::vector<cBubbleBubble::PTR> &FactoryGetWorkList(void) { return mWorkList; }
	inline std::vector<cBubbleBubble::PTR> &FactoryGetCollisionList(void) { return mCollisionList; };

	inline const std::vector<cBubbleBubble::PTR> &GetWorkList(void) const { return mWorkList; }; // the list of items this engine finds collisions for
	inline const std::vector<cBubbleBubble::PTR> &GetCollisionList() const { return mCollisionList; }; // the total list of items that can be collided into

	inline void SetGroup(unsigned int groupID) { mGroupID = groupID; };
	inline void SetPause(bool pause) { this->cTimerWrapper::SetPause(pause); };
	inline void Abort(void) { this->cTimerWrapper::Abort(); };

	inline cMutexWrapper *GetCollisionLock(void) { return &mCollisionListLock; };
	inline unsigned int GetID(void) const { return mID; };

	void Start(const std::vector<cBubbleBubble::PTR> &newList, CollisionReportFunc *collisionReportFunc, unsigned int interval=FIVE_A_SECOND)
	{
		if (newList.size() > 0)
		{
			FactoryGetWorkList().clear();
			FactoryGetWorkList().assign(newList.begin(), newList.end());
		}

		mCollisionReportFunc = collisionReportFunc;
		cTimerWrapper::FactorySetDelay(interval);
		cTimerWrapper::AddThread(this);
		// Milder on threads -> cTimerWrapper::AddTimer(this);
	};

	~cBubbleEngine(void)
	{
		cTimerWrapper::RemoveTimer(this);
	}

private:
	unsigned int mGroupID;
	unsigned int mID;
	unsigned int mReserveAmount;
	bool mFlipFlop;

	std::vector<cBubbleBubble::PTR> mCollisionList; // total list of everything that can collide
	cMutexWrapper mCollisionListLock;
	std::vector<cBubbleBubble::PTR> mWorkList; // list of what this engine compares because work can be split over two engines

	std::vector<cBubbleBubble::TRILATERATION_DATA> mDistanceList; // results of relative distances
	std::vector<cBubbleBubble::COLLISION_RESULT> mCollisionResults; // results of found collisions
	CollisionReportFunc *mCollisionReportFunc;

	bool /*-gcc cTimerWrapper::*/IsExpired(void) { return false; };

	void /*-gcc cTimerWrapper::*/EventTimer(void)
	{
		if (mFlipFlop)
			return;
		mFlipFlop = true;

		// this is where collisions are deduced
		for (cMutexWrapper::Lock lock(GetCollisionLock()) ;;)
		{
			mCollisionResults.clear();
			std::for_each(mWorkList.begin(), mWorkList.end(), 
				cBubbleFindCollisions(GetCollisionList(), mDistanceList, mCollisionResults));

			unsigned int size = mCollisionResults.size();
			void *list = NULL;

			if (size > 0)
				list = &( mCollisionResults.front() );

			(*mCollisionReportFunc)(mGroupID, mID, (cBubbleBubble::COLLISION_RESULT*) list, size );

			break;
		}

		mFlipFlop = false;
	};
};

static inline bool operator ==(const cBubbleEngine::PTR & p_lhs, const cBubbleEngine::PTR & p_rhs)
{
	return p_lhs.ptr->GetID() == p_rhs.ptr->GetID();
}

#endif