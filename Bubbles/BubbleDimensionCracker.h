// BubbleDimensionCracker.h
// Copyright (c) 2014
// Author: Matthew Cocks
// License: Attribution 4.0 International (CC BY 4.0)

#ifndef BUBBLEDIMENSIONCRACKER_H
#define BUBBLEDIMENSIONCRACKER_H
#include <vector>
#include <algorithm>
#include <map>
#include "BubbleBubble.h"
#include "BubbleTrace.h"

namespace Bubbles 
{

/* The purpose of this class is to take a game object with an ID and use subtraction and the less than operator to discover distances of items from each other.
   Find the distance for each of items x, y and z dimensions: each axis on their own individual plane taking a 3D object and reducing it to a series of 1D intersections */
class cBubbleDimensionCracker : public std::binary_function<cBubbleBubble::PTR, cBubbleBubble::PTR, void>
{
private:
   TraceFunc *mTraceFunc;
   std::vector<TRILATERATION_DATA> &mDistanceList;
   bool &mAbort;

public:
   cBubbleDimensionCracker(TraceFunc *traceFunc, std::vector<TRILATERATION_DATA> &data, bool &abort) 
      : mTraceFunc(traceFunc), mDistanceList(data), mAbort(abort)
   { };

   inline void MakeAxisUnit(const first_argument_type &universeCenter, const second_argument_type &from_this_shape, 
						float from_this_shapeX, float universeCenterX, TRILATERATION_DATA &new_unit) const
   {
      if (universeCenterX > from_this_shapeX)
         new_unit.abs_dist = (universeCenterX) - (from_this_shapeX);
      else
         new_unit.abs_dist = (from_this_shapeX) - (universeCenterX);

	  mDistanceList.push_back(new_unit);
   }

   /* find the distance units from 'me' to everything else in the universe  */
   inline result_type operator () (const first_argument_type &universeCenter, const second_argument_type &from_this_shape) const
   {
      if (mAbort) throw -999;

      // ignore our own shadow
      if (from_this_shape.ptr->GetID() == universeCenter.ptr->GetID()) 
         return;

      // we can't see the other bubble if it's etheral
      if (from_this_shape.ptr->GetEtherealness()) 
         return;

      // ignore anything associated with a deleted thing
      if (from_this_shape.ptr->GetIsDeleted()) 
         return;  

      if (universeCenter.ptr->GetIsDeleted()) 
         return;

	  Uint32 now = SDL_GetTicks();

      float from_this_shapeX, from_this_shapeY, from_this_shapeZ, from_this_shapeR;
      float universeCenterX, universeCenterY, universeCenterZ;
      unsigned int newId = from_this_shape.ptr->GetID();

      from_this_shape.ptr->GetCachedCenter(now, from_this_shapeX, from_this_shapeY, from_this_shapeZ);
      from_this_shapeR = from_this_shape.ptr->GetRadius();
      universeCenter.ptr->GetCollisionCenter(now, universeCenterX, universeCenterY, universeCenterZ);

	  /* scope block */
      {
         TRILATERATION_DATA new_unit = {0};
         new_unit.id = newId;
         new_unit.axis = cAxisSplitterAXIS::X;
         new_unit.width = from_this_shapeR;

         MakeAxisUnit(from_this_shape, universeCenter, 
            from_this_shapeX,
            universeCenterX,
            new_unit);
      }

	  /* scope block attack of the clones*/
      {
         TRILATERATION_DATA new_unit = {0};
         new_unit.id = newId;
         new_unit.axis = cAxisSplitterAXIS::Y; // difference
         new_unit.width = from_this_shapeR;

         MakeAxisUnit(from_this_shape, universeCenter, 
            from_this_shapeY, // difference
            universeCenterY, // difference
            new_unit); // difference
      }


	  /* scope block revenge of the Ctrl-Cth*/
      {
         TRILATERATION_DATA new_unit = {0};
         new_unit.id = newId;
         new_unit.axis = cAxisSplitterAXIS::Z; // difference
         new_unit.width = from_this_shapeR;

         MakeAxisUnit(from_this_shape, universeCenter, 
            from_this_shapeZ, // difference
            universeCenterZ, // difference
            new_unit); // difference
      }
   };
};

}

#endif