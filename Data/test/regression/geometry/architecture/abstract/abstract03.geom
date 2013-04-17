local baseGeom = true
local s   = vec3( 10, 2.5, 10 )
local p   = vec3( 0, 0, 0 )
local p2  = vec3( 5, 0, 5 )
local wdi = (baseGeom and 0.30) or 0.10
local wde = wdi * 2

--==============================================================================
-- Frame
--==============================================================================

compositeBegin()

   local tag = "floor"
   --local tag = "room"
   local c1 = component{ id=tag, size=s, position=p, mat=1, g=1 }
   local c2 = component{ id=tag, size=s, position=p2, mat=1, g=2 }

   --merge( {c1,c2}, { id="room" } )
   --subtract( c1, c2, { id="room" } )
   intersect( {c1,c2}, { id="room" } )

--==============================================================================
-- Interior
--==============================================================================

   -- Walls
   local mat=10
   for c in query( "room" ) do
      mat=mat+1
      local s = {}
      for f in fquery( c, "SIDE", "BOTTOM" ) do
         s[#s+1] = component{ c, id="wall", boundary=f }
      end
      extrude( s, -wdi, { id="iwall" } )
   end

   if baseGeom then
      for c in query( "room" ) do
         mat=mat+1
         local s = {}
         for f in fquery( c, "SIDE", "BOTTOM" ) do
            s[#s+1] = component{ c, id="wall", boundary=f }
         end
         extrude( s, wde, { id="ewall", mat=mat } )
      end
   end

--==============================================================================
-- Geometry
--==============================================================================
---[[
   blocksBegin()
      for c in query( "iwall" ) do
         blocks{ c, id=12 }
      end
      for c in query( "ewall" ) do
         blocks{ c, id=10 }
      end
   blocksEnd()
--]]
--[[
   blocksBegin()
      for c in query( "iwall" ) do
         if c.g == 1 then blocks{ c, id=12 } end
      end
      for c in query( "ewall" ) do
         if c.g == 1 then blocks{ c, id=10 } end
      end
   blocksEnd()

   blocksBegin()
      for c in query( "iwall" ) do
         if c.g == 2 then blocks{ c, id=12 } end
      end
      for c in query( "ewall" ) do
         if c.g == 2 then blocks{ c, id=10 } end
      end
   blocksEnd()
--]]
compositeEnd()
