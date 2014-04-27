Bubbles
=======

Collision detection engine, Bubbles

This is a 3D collision detection engine. Written in C++ < 11 it uses SDL 1.2.

It expects a unique int Id and radius for each collidable item, and then two application callbacks: 
- One for it to ask for a game objects x, y and z coordinate given the unique Id. This is the GetCoord callback
- Another for it to tell the application back the collisions. This is the CollisionReport callback

How to use this library:
- Init()
-- to start it all up
- AddEngine()
-- to add an engine, it returns the engine Id
- AddBubble() 
-- to add each collidable item. Provide an Id, radius, GetCoord callback and the engine Id to add it to
- StartEngine() 
-- enables the engine. Takes engine Id, CollisionReport and the frequency to check for collisions in millisecons
- AddBubble() 
-- add more bubbles while the engines going, thats fine
- UnInit() 
-- to clean it all up

There is also the ability to split the workload over two threads by adding two engines, both sharing the same 
GetCoord and CollisionReport. This means one engine can produce collision reports for half the collidable items 
and the other engine produce the collision report for the other half. They both share the same list of what is 
collideable. This is done using groups, and the sequence isn't much more complicated

- Init()
- AddEngine() 
-- for the first
- AddEngine() 
-- for the second
- AddEngineGroup() 
-- which takes an engine Id
- AddEngineToGroup() 
-- to associate the other engine Id with the same group
- AddBubble() 
-- etc add to either the first engine or the second
- StartEngine() 
-- enables the engine. Takes engine Id, CollisionReport and the frequency to check for collisions in milliseconds
- AddBubble() 
-- continue to add to either the first engine or the second
- UnInit() 
-- to clean it all up
