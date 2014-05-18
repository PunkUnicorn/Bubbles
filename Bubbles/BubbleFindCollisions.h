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

namespace Bubbles
{

class cBubbleFindCollisions : public std::unary_function<cBubbleBubble::PTR, void> 
{
private:
   class OutsideRadius : public std::binary_function<TRILATERATION_DATA, float, bool>
   {
      public:
         inline result_type operator () (const first_argument_type& otherThing, const second_argument_type& radius) const
         {
            return radius < otherThing.abs_dist;
         };
   };

   /* take distance units and judge if a collision has been made with the given radius */
   static void GetCollisionResults(std::vector<COLLISION_RESULT> &results, 
      std::vector<TRILATERATION_DATA> &distanceList, unsigned int id, float radius)
   {
      std::vector<TRILATERATION_DATA>::iterator new_last = 
         std::remove_if(distanceList.begin(), distanceList.end(), std::bind2nd(OutsideRadius(), radius));

      COLLISION_RESULT found_result;
      unsigned int push_count = 0;
      if (new_last != distanceList.begin())
      {
         /* how many collisions .. it's only a collision if a point in all of it's three dimensions hit
            so discount those that only have one or two dimension collisions */

         // sort in order of ID value to cause grouping
         std::sort(distanceList.begin(), new_last, 
                  cBubbleBubble::TRILATERATION_DATA_id_LessThan);
            
         // find a sequence of 3 of the same ID's in a row
         std::vector<TRILATERATION_DATA>::iterator unit_iterator;
            
         unsigned int last_id;
         unsigned int id_count;
         last_id = 0;
         id_count = 1;

         for (unit_iterator = distanceList.begin(); unit_iterator != new_last; unit_iterator++)
         {
            if (last_id == unit_iterator->id)
            {
               id_count++;
            }
            else
            {
               last_id = unit_iterator->id;
               id_count = 1;
            }

            found_result.mDistanceUnits[id_count-1] = *unit_iterator;

            if (id_count == 3)
            {
               //an ID has occured three times, one for each axis so this is a collision
               found_result.mCenterID = id; 
               results.push_back(found_result);
               push_count++;
            }
         }      
      }
   };
      
   const std::vector<cBubbleBubble::PTR> &mCollisionList;
   std::vector<COLLISION_RESULT> &mCollisionResults;
   std::vector<TRILATERATION_DATA> &mDistanceList;

public:
   cBubbleFindCollisions(
         const std::vector<cBubbleBubble::PTR> &collisionList, 
         std::vector<TRILATERATION_DATA> &dlist, 
         std::vector<COLLISION_RESULT> &results) : mCollisionList(collisionList), mCollisionResults(results), mDistanceList(dlist) {};

   inline result_type operator () (const argument_type& center) const
   { 
      mDistanceList.clear();
      std::for_each(mCollisionList.begin(), mCollisionList.end(),                   
         std::bind1st(cBubbleDimensionCracker(mDistanceList), center));

      GetCollisionResults(mCollisionResults, mDistanceList, center.ptr->GetID(), center.ptr->GetRadius());
      center.ptr->DistanceListUpdated(mDistanceList, mCollisionResults);
   };
};

}

#endif