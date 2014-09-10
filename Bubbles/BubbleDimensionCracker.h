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
      float from_this_shapeX, //float from_this_shapeR,
      float universeCenterX, //float universeCenterR,
      TRILATERATION_DATA &new_unit) const
   {
      //new_unit.rel_coord = from_this_shapeX - universeCenterX;
      if (universeCenterX > from_this_shapeX)
      {
         new_unit.abs_dist = (universeCenterX/* - universeCenterR*/) - (from_this_shapeX /*+ from_this_shapeR*/);
      }
      else
      {
         new_unit.abs_dist = (from_this_shapeX/* - from_this_shapeR*/) - (universeCenterX/* + universeCenterR*/);
      }
      mDistanceList.push_back(new_unit);
   }

   /* find the distance units from 'me' to everything else in the universe  */
   inline result_type operator () (const first_argument_type &universeCenter, const second_argument_type &from_this_shape) const
   {
      if (mAbort) throw -999;

      // ignore our own shadow
      if (from_this_shape.ptr->GetID() == universeCenter.ptr->GetID()) 
         return;

      // we can't see etheral bubbles
      if (from_this_shape.ptr->GetEtherealness()) 
         return;

      // ignore anything associated with a deleted thing
      if (from_this_shape.ptr->GetIsDeleted()) 
         return;  

      if (universeCenter.ptr->GetIsDeleted()) 
         return;

      float from_this_shapeX, from_this_shapeY, from_this_shapeZ, from_this_shapeR;
      float universeCenterX, universeCenterY, universeCenterZ;//, universeCenterR;
      unsigned int newId = from_this_shape.ptr->GetID();

      from_this_shape.ptr->GetCachedCenter(from_this_shapeX, from_this_shapeY, from_this_shapeZ);
      from_this_shapeR = from_this_shape.ptr->GetRadius();
      universeCenter.ptr->GetCollisionCenter(universeCenterX, universeCenterY, universeCenterZ);
      //universeCenterR = from_this_shape.ptr->GetRadius();

      {
         TRILATERATION_DATA new_unit = {0};
         new_unit.id = newId;
         new_unit.axis = cAxisSplitterAXIS::X;
         new_unit.radius = from_this_shapeR;

         MakeAxisUnit(from_this_shape, universeCenter, 
            from_this_shapeX,
            universeCenterX,
            new_unit);
     
         /*new_unit.rel_coord = (from_this_shapeX + from_this_shapeR) - (universeCenterX + universeCenterR);
         if (universeCenterX > from_this_shapeX)
         {
            new_unit.abs_dist = (universeCenterX - universeCenterR) - (from_this_shapeX + from_this_shapeR);
         }
         else
         {
            new_unit.abs_dist = (from_this_shapeX - from_this_shapeR) - (universeCenterX + universeCenterR);
         }
         mDistanceList.push_back(new_unit);*/
      }


      {
         TRILATERATION_DATA new_unit = {0};
         new_unit.id = newId;
         new_unit.axis = cAxisSplitterAXIS::Y;
         new_unit.radius = from_this_shapeR;

         MakeAxisUnit(from_this_shape, universeCenter, 
            from_this_shapeY, //from_this_shapeR, 
            universeCenterY, //universeCenterR, 
            new_unit);

         /*new_unit.rel_coord = from_this_shapeY - universeCenterY;
         if (universeCenterY > from_this_shapeY)
         {
            new_unit.abs_dist = universeCenterY - from_this_shapeY;
         }
         else
         {
            new_unit.abs_dist = from_this_shapeY - universeCenterY;
         }*/
         //mDistanceList.push_back(new_unit);
      }


      {
         TRILATERATION_DATA new_unit = {0};
         new_unit.id = newId;
         new_unit.axis = cAxisSplitterAXIS::Z;
         new_unit.radius = from_this_shapeR;

         MakeAxisUnit(from_this_shape, universeCenter, 
            from_this_shapeZ, //from_this_shapeR, 
            universeCenterZ, //universeCenterR, 
            new_unit);
         /*new_unit.rel_coord = from_this_shapeZ - universeCenterZ;
         if (universeCenterZ > from_this_shapeZ)
         {
            new_unit.abs_dist = universeCenterZ - from_this_shapeZ;
         }
         else
         {
            new_unit.abs_dist = from_this_shapeZ - universeCenterZ;
         }
         mDistanceList.push_back(new_unit);*/
      }
   };
};

}

#endif