// BubbleBubble.h
// Copyright (c) 2014
// Author: Matthew Cocks
// License: Attribution 4.0 International (CC BY 4.0)

#ifndef BUBBLEBUBBLE_H
#define BUBBLEBUBBLE_H
#include <vector>
#include "BubbleSTDCALL.h"
#include "Bubbles.h"

namespace Bubbles
{

#ifndef NULL
#   define NULL (0x0)
#   define IDEFINEDNULL
#endif

class cBubbleBubble
{
public:
	typedef struct
	{
		cBubbleBubble *ptr;
	} PTR;

	typedef void DistanceListUpdatedFunc(TRILATERATION_DATA*, unsigned int, COLLISION_RESULT*, unsigned int); 

private:
	unsigned int mEngineID;
	unsigned int mID;
	float mRadius;
	bool mEtherealness;

	GetCoordsFunc *mGetCoordsFunc;
	DistanceListUpdatedFunc *mDistanceListUpdatedFunc;

	//float x, y, z;
   //bool mCached;

public:
   //inline void ClearCache(void) { mCached = false; };
	inline void GetCollisionCenter(float &x, float &y, float &z) 
	{
        //if (mCached == false)
        //{
		  (*mGetCoordsFunc)(mEngineID, mID, x, y, z);
        //    mCached = true;
        //}

        //px = x;
        //py = y;
        //pz = z;
	};

	inline void FactorySetRadius(float radius) { mRadius = radius; };
	inline void FactorySetID(unsigned int id) { mID = id; };
	inline void FactorySetEtherealness(bool etherealness) { mEtherealness = etherealness; };

	inline bool GetEtherealness() const { return mEtherealness; };
	inline float GetRadius() const { return mRadius; };
	inline unsigned int const GetID() const { return mID; };

	cBubbleBubble(unsigned int engineId, unsigned int id, float radius, GetCoordsFunc *getCoordsFunc) 
		: mEngineID(engineId), 
		mID(id), 
		mRadius(radius), 
		mEtherealness(false), 
		mGetCoordsFunc(getCoordsFunc), 
		mDistanceListUpdatedFunc(NULL) 
		//x(0.0f), 
		//y(0.0f), 
		//z(0.0f),
      //  mCached(false)
	{};

	~cBubbleBubble(void) {};

	inline void DistanceListUpdated(std::vector<TRILATERATION_DATA> &list, std::vector<COLLISION_RESULT> &bangs) const
	{
		if (mDistanceListUpdatedFunc != NULL)
		{
			void * trilateration_list = NULL;
			void * collision_results = NULL;
				 
			unsigned int list_size = list.size();
			if (list_size > 0)
				trilateration_list = &(list.front());

			unsigned int bangs_size = bangs.size();
			if (bangs_size > 0)
				collision_results = &(bangs.front());

			(*mDistanceListUpdatedFunc)(
				(TRILATERATION_DATA*) trilateration_list, // holds all TRILATERATION_DATA accumulated at this point, nine times out of ten this function will hold an incomplete set of data. The tenth time it will be complete
				list_size, 
				(COLLISION_RESULT*) collision_results, // note it's only the known collision results SO FAR, it's a collection in progress
				bangs_size );
		}
	}

	inline static bool TRILATERATION_DATA_abs_dist_LessThan(TRILATERATION_DATA &lhs, TRILATERATION_DATA &rhs)
	{
		return lhs.abs_dist < rhs.abs_dist ||
            (lhs.id == rhs.id && lhs.axis < rhs.axis /*ensure result order of X, Y, Z*/);
	}

	inline static bool TRILATERATION_DATA_id_LessThan(const TRILATERATION_DATA &lhs, const TRILATERATION_DATA &rhs)
	{
		return lhs.id < rhs.id || 
            (lhs.id == rhs.id && lhs.axis < rhs.axis /*ensure result order of X, Y, Z*/);
	}
		
	inline static bool TRILATERATION_DATA_id_Equals(TRILATERATION_DATA &lhs, TRILATERATION_DATA &rhs)
	{
		return lhs.id == rhs.id;
	}
};

static inline bool operator ==(const cBubbleBubble::PTR & p_lhs, const cBubbleBubble::PTR & p_rhs)
{
	return p_lhs.ptr->GetID() == p_rhs.ptr->GetID();
}


#ifdef IDEFINEDNULL
#   undef IDEFINEDNULL
#   undef NULL
#endif

}

#endif