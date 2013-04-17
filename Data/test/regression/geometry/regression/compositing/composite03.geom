compositeBegin()

--------------------------------------------------------------------------------
-- Building frame
--------------------------------------------------------------------------------

   -- Main volume.
   --component{ id="main", size={10,8, 5} }
   component{ 
      id="main", size={16,8,10}, 
      boundary={
         {0,0,0},{0,0,0.8},{0.3,0,0.8},{0.3,0,1},{0.7,0,1},{0.7,0,0.8},{1,0,0.8},{1,0,0},direction={0,1,0},
         id={"b","t","f","f","f","c","f","f","f","f"}
      }
   }

   -- Level.
   for c in query( "main" ) do
      slice( c, "Y", { id="level", 2, level=counter() } )
   end

   -- Rooms.
   for c in query( "level" ) do
      split( c, "X", { id="room", rel=1 }, { id="corridor", 2 }, { id="room", rel=1 } )
   end

   -- Staircase.
   local sc = component{ id="staircase", size={3,8,3}, position={6.5,0,0} }
   for c in query( "staircase" ) do
      execute( "architecture/stair/staircase01", { c, lh=2, ew=0.5, rn=true, sth=0.4 } )
   end

   -- Subtract space.
   for c in query( "room", "corridor" ) do
      subtract( c, sc, { id="room_final" } )
   end

   -- Walls
   for c in query( "room_final", "staircase" ) do
      local s = {}
      for f in fquery( c, "SIDE", "B" ) do
         s[#s+1] = component{ c, id="wall", boundary=f }
      end
      extrude( s, -0.1, { id="ewall" } )
   end

--==============================================================================
-- Connection
--==============================================================================


--==============================================================================
-- Geometry
--==============================================================================

   blocksBegin()
      for c in query( "ewall" ) do blocks{ c } end
   blocksEnd()

compositeEnd()
