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
   std::vector<TRILATERATION_DATA> &mDistanceList;
   bool &mAbort;

public:
   cBubbleDimensionCracker(std::vector<TRILATERATION_DATA> &data, bool &abort) 
      : mDistanceList(data), mAbort(abort)
   { };


   /* find the distance units from 'me' to everything else in the universe  */
   inline result_type operator () (const first_argument_type &universeCenter, const second_argument_type &from_this_shape) const
   {
      if (mAbort) throw -999;
      if (from_this_shape.ptr->GetID() == universeCenter.ptr->GetID()) return;
      // etheral bubbles can't hit anything themselves. But other things register to them
      if (from_this_shape.ptr->GetEtherealness()) return;

      TRILATERATION_DATA new_unit;
      new_unit.id = from_this_shape.ptr->GetID();

      float from_this_shapeX, from_this_shapeY, from_this_shapeZ;
      from_this_shape.ptr->GetCachedCenter(from_this_shapeX, from_this_shapeY, from_this_shapeZ);

      float universeCenterX, universeCenterY, universeCenterZ;
      universeCenter.ptr->GetCollisionCenter(universeCenterX, universeCenterY, universeCenterZ);

      new_unit.axis = cAxisSplitterAXIS::X;
      new_unit.rel_coord = from_this_shapeX - universeCenterX;
      if (universeCenterX > from_this_shapeX)
      {
         new_unit.abs_dist = universeCenterX - from_this_shapeX;
      }
      else
      {
         new_unit.abs_dist = from_this_shapeX - universeCenterX;
      }
      mDistanceList.push_back(new_unit);


      new_unit.axis = cAxisSplitterAXIS::Y;
      new_unit.rel_coord = from_this_shapeY - universeCenterY;
      if (universeCenterY > from_this_shapeY)
      {
         new_unit.abs_dist = universeCenterY - from_this_shapeY;
      }
      else
      {
         new_unit.abs_dist = from_this_shapeY - universeCenterY;
      }
      mDistanceList.push_back(new_unit);


      new_unit.axis = cAxisSplitterAXIS::Z;
      new_unit.rel_coord = from_this_shapeZ - universeCenterZ;
      if (universeCenterZ > from_this_shapeZ)
      {
         new_unit.abs_dist = universeCenterZ - from_this_shapeZ;
      }
      else
      {
         new_unit.abs_dist = from_this_shapeZ - universeCenterZ;
      }
      mDistanceList.push_back(new_unit);
   };
};

}

#endif