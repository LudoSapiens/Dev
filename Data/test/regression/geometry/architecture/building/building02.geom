local Mat = execute( "architecture/common_materials" )

local floorThickness = 0.1

--==============================================================================
-- Frame
--==============================================================================
--[[
compositeBegin()
   component{ id="floor", size={10,2.66,10} }

   for c in query( "floor" ) do
      split( c, "Z", { id="wing", rel=1, side="left" }, { id="corridor", 2 }, { id="wing", rel=1, side="right" } )
   end

   for c in query( "wing" ) do
      if c.side == "left" then
         slice( c, "X", { id="apartment", 5 } )
      else
         slice( c, "X", { id="apartment", 6 } )
      end
   end

   -- Walls and floors.
   for c in query( "apartment", "corridor" ) do
      local s = {}
      for f in fquery( c, "SIDE", "B" ) do
         s[#s+1] = component{ c, id="wall", boundary=f }
      end
      extrude( s, -floorThickness, { id="ewall" } )
   end

   for c in query( "apartment" ) do
      local s = c.side
      if s == "right" then
         -- Put window in +z and door in -z.
         for f in fquery( c, "+Z" ) do
            region{ c, f, id="window" }
         end
         for f in fquery( c, "-Z" ) do
            region{ c, f, id="door" }
         end
      else
         -- Put window in -z and door in +z.
         for f in fquery( c, "-Z" ) do
            region{ c, f, id="window" }
         end
         for f in fquery( c, "+Z" ) do
            region{ c, f, id="door" }
         end
      end
   end
--]]

compositeBegin()
   component{ id="main", size={6,3,10} }
   for c in query( "main" ) do
      local s = {}
      for f in fquery( c, "Z" ) do
         s[#s+1] = component{ c, id="wall", boundary=f }
         region { c, f, id="win" }
      end
      extrude( s , -0.3, { id="ewall" } )
   end

--==============================================================================
-- Geometry
--==============================================================================

   -- Generate wall geometry.
   blocksBegin()
      for c in query( "ewall" ) do blocks{ c, id=Mat.WALL1 } end
   blocksEnd()

--==============================================================================
-- Connections
--==============================================================================

   for r in rquery( "win" ) do
      connect( execute( "architecture/window/window02", { frame=true, h=2.0, w=1.2, d=0.3, d2=0.15 } ), r, {0.5,0.5,0}, {-1.5,0,-0.3} )
      connect( execute( "architecture/window/window02", { frame=true, h=2.0, w=1.2, d=0.3, d2=0.15 } ), r, {0.5,0.5,0}, {1.5,0,-0.3} )
      local doorset = execute( "architecture/door/set01", { door={"architecture/door/door01",o=0.1}, frame={d=0.3} } )
      connect( doorset, r, {0.5,0.0,0}, {0,0,-0.3} )
   end

   -- Generate windows.
   for r in rquery( "window" ) do
      connect( execute( "architecture/window/window02", { h=2.0 } ), r, {0.5,0.5,0}, {-1,0,-0.1} )
      connect( execute( "architecture/window/window02", { h=2.0, d2=0.15 } ), r, {0.5,0.5,0}, {1,0,-0.1} )
      --connect( execute( "architecture/window/window01" ), r, {0.5,0.5,0}, {0,0,-0.1} )
   end
   
   -- Generate doors.
   local doors = {
      left  = "architecture/door/door01",
      right = "architecture/door/door02",
   }
   for r in rquery( "door" ) do
      local doorset = execute( "architecture/door/set01", { door={doors[r.component.side],o=0.1} } )
      connect( doorset, r, {0.5,0.0,0}, {0,floorThickness,-0.1} )
   end

compositeEnd()
