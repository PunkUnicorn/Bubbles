// BubbleBubble.h
// Copyright (c) 2014
// Author: Matthew Cocks
// License: Attribution 4.0 International (CC BY 4.0)

#ifndef BUBBLEBUBBLE_H
#define BUBBLEBUBBLE_H
#include <vector>
#include "BubbleSTDCALL.h"

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

	enum cAxisSplitterAXIS /* -gcc : int */
	{
		X = 0,
		Y,
		Z,
		NoOfDimensions
	};

	typedef struct 
	{
		cAxisSplitterAXIS axis; //which axis this represents either the x, y or z
		float abs_dist; //absolute distance from id
		float rel_coord; //value of the axis (value of x, y or z)
		unsigned int id;
	} TRILATERATION_DATA;

	typedef struct 
	{
		unsigned int mCenterID;
		cBubbleBubble::TRILATERATION_DATA mDistanceUnits[NoOfDimensions];
	} COLLISION_RESULT;

	typedef void STDCALL GetCoordsFunc(unsigned int /*engineId*/, unsigned int /*bubbleId*/, float &/*X*/, float &/*Y*/, float &/*Z*/);
	typedef void DistanceListUpdatedFunc(TRILATERATION_DATA*, unsigned int, COLLISION_RESULT*, unsigned int); // Note COLLISION_RESULTS** only holds those collisions deduced SO FAR. It's a collection in progress

private:
	unsigned int mEngineID;
	unsigned int mID;
	float mRadius;
	bool mEtherealness;

	GetCoordsFunc *mGetCoordsFunc;
	DistanceListUpdatedFunc *mDistanceListUpdatedFunc; //cheeky function to get a preview of all the data to be used in the collision detection

	float x, y, z;

public:
	inline void GetCollisionCenter(float &x, float &y, float &z) 
	{
		(*mGetCoordsFunc)(mEngineID, mID, x, y, z);
	};

	inline void FactorySetRadius(float radius) { mRadius = radius; };
	inline void FactorySetID(unsigned int id) { mID = id; };
	inline void FactorySetEtherealness(bool etherealness) { mEtherealness = etherealness; };

	inline bool GetEtherealness() const { return mEtherealness; };
	inline float GetRadius() const { return mRadius; };
	inline unsigned int const GetID() const { return mID; };

	cBubbleBubble(unsigned int engineId, unsigned int id, float radius, GetCoordsFunc *getCoordsFunc, bool isEthereal = false) 
		: mEngineID(engineId), 
		mID(id), 
		mRadius(radius), 
		mEtherealness(isEthereal), 
		mGetCoordsFunc(getCoordsFunc), 
		mDistanceListUpdatedFunc(NULL), 
		x(0.0f), 
		y(0.0f), 
		z(0.0f) 
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

#endif