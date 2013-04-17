compositeBegin()

--==============================================================================
-- Frame/space
--==============================================================================

   -- Main volume.
   --component{ id="main", size={10,8, 5} }
   component{ 
      id="main", size={10,8,5}, 
      --id="main", size={10,2,5}, 
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
      split( c, "X", { id="room", rel=1 }, { id="room", rel=1 } )
   end

   -- Walls
   for c in query( "room" ) do
      local s = {}
      for f in fquery( c, "SIDE", "B" ) do
         s[#s+1] = component{ c, id="wall", boundary=f }
      end
      extrude( s, -0.1, { id="ewall" } )
   end

   -- Regions for window placement.
   for c in query( "room" ) do
      for f in fquery( c, "Z" ) do
         if faceID( c, f ) == "f" then region{ c, f, id="window" } end
      end
   end

--==============================================================================
-- Geometry
--==============================================================================

   -- Walls geometry.
   blocksBegin()
      for c in query( "ewall" ) do blocks{ c } end
   blocksEnd()

--==============================================================================
-- Connection
--==============================================================================

   -- Window connection.
   local ws  = { 0.3, 1.2 }
   for r in rquery( "window" ) do
      local w = ws[ math.mod(r.component.level, 2)+1 ]
      connect( execute( "architecture/window/window01", {w=w} ), r, {0.5,0.5,0}, {0,0,-0.1} )
   end

compositeEnd()
