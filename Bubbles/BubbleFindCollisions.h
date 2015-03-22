// BubbleFindCollisions.h
// Copyright (c) 2014
// Author: Matthew Cocks
// License: Attribution 4.0 International (CC BY 4.0)

#ifndef BUBPACTUTILS_H
#define BUBPACTUTILS_H
#include "BubbleBubble.h"
#include "BubbleDimensionCracker.h"
#include "BubbleTrace.h"
#include <vector>
#include <algorithm>
#include <map>
#include <functional>

namespace Bubbles
{

class cBubbleFindCollisions : public std::unary_function<cBubbleBubble::PTR, void> 
{
private:
   class OutsideRadius : public std::binary_function<TRILATERATION_DATA, float, bool>
   {
      public:
         inline result_type operator () (const first_argument_type& otherThing, const second_argument_type& width) const
         {
            return width < (otherThing.abs_dist - otherThing.width);
         };
   };

   /* take distance units and judge if a collision has been made with the given radius */
   static void GetCollisionResults(TraceFunc *traceFunc, std::vector<COLLISION_RESULT> &results, 
      std::vector<TRILATERATION_DATA> &distanceList, unsigned int id, float radius)
   {
      std::vector<TRILATERATION_DATA>::iterator new_last = 
         std::remove_if(distanceList.begin(), distanceList.end(), std::bind2nd(OutsideRadius(), radius));

      COLLISION_RESULT found_result;
      if (new_last != distanceList.begin())
      {
         /* how many collisions .. it's only a collision if a point in all of it's three dimensions hit
            so discount those that only have one or two dimension collisions */

         // sort in order of ID value to cause grouping
         std::sort(distanceList.begin(), new_last, 
                  cBubbleBubble::TRILATERATION_DATA_id_LessThan);
         
         // find a sequence of 3 of the same ID's in a row
         std::vector<TRILATERATION_DATA>::iterator unit_iterator;
            
         unsigned int previous_id = 0;
         bool have_previous_id = false;
         unsigned int id_count = 1;

         for (unit_iterator = distanceList.begin(); unit_iterator != new_last; unit_iterator++)
         {
            if (have_previous_id && previous_id == unit_iterator->id)
            {
               id_count++;
            }
            else
            {
               have_previous_id = true;
               previous_id = unit_iterator->id;
               id_count = 1;
            }

            found_result.mDistanceUnits[id_count-1] = *unit_iterator;

            if (id_count == 3)
            {
               //an ID has occured three times, one for each axis so this is a collision
               found_result.mCenterID = id; 
               results.push_back(found_result);
            }
         } 
      }
   };
  
   TraceFunc *mTraceFunc;
   const std::vector<cBubbleBubble::PTR> &mGroupList;
   std::vector<COLLISION_RESULT> &mCollisionResults;
   std::vector<TRILATERATION_DATA> &mDistanceList;
   bool &mAbort;

public:
   cBubbleFindCollisions(
         TraceFunc *traceFunc,
         const std::vector<cBubbleBubble::PTR> &collisionList, 
         std::vector<TRILATERATION_DATA> &dlist, 
         std::vector<COLLISION_RESULT> &results,
         bool &abort) : mTraceFunc(traceFunc), mGroupList(collisionList), mCollisionResults(results), mDistanceList(dlist), mAbort(abort) {};

   inline result_type operator () (const argument_type& center) const
   { 
      try
      {
         if (mAbort) throw -999;
         mDistanceList.clear();
         if (center.ptr->GetIsDeleted()) 
            return;

         std::for_each(mGroupList.begin(), mGroupList.end(),                   
            std::bind1st(cBubbleDimensionCracker(mTraceFunc, mDistanceList, mAbort), center));

         GetCollisionResults(mTraceFunc, mCollisionResults, mDistanceList, center.ptr->GetID(), center.ptr->GetRadius());
         center.ptr->DistanceListUpdated(mDistanceList, mCollisionResults);
      }
      catch (int code)
      {
         throw code;
      }
   };
};

}

#endif