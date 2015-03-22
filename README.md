Bubbles
=======

Dependancies:
-------------
SDL 1.2
http://www.libsdl.org/download-1.2.php


Collision detection engine, Bubbles

This is a 3D collision detection engine. Written in C++ with the STL. It uses SDL 1.2.

Cubic bounding bubbles with collisions detected at a user set rate typically four, five or six times a second.

Developed for use with both or eather Visual Studio (2010) and or g++ (4.8.2 on slackware)

It expects a unique int Id and width for each collidable item, and then two application callbacks: 
- One to ask an objects x, y and z coordinate given the unique Id. This is the GetCoord callback
- Another to tell the application back the collisions. This is the CollisionReport callback

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

There is also the ability to split the workload over two or more threads by adding engines, all sharing the
same GetCoord and CollisionReport. This means one engine can deduce collision for half the collidable items
and the other engine deduces the collisions for the rest. They both share the same total list of collideable
items. But they only process results for their half of the workload avoiding a count * count spike. Splitting
the load over multiple engines becomes (count / no of engines) * count

This is done using 'engine groups', and sequence the isn't much more complicated.

- Init()
- AddEngine() 
-- for the first
- AddEngine() 
-- for the second
- AddEngineGroup() 
-- which takes an existing engine Id. Add groups first /before/ adding bubbles
- AddEngineToGroup() 
-- to associate the other engine Id with the group created above
- AddBubble() 
-- etc add to either the first engine or the second. Note: groups have been set up before adding bubbles
- StartEngine()
-- to start the first engine
- StartEngine()
-- to start the second engine
- AddBubble() 
-- continue to add to either the first or second engine
- UnInit() 
-- to clean it all up
