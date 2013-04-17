-- Main volume.
local w = 16
local h =  2
local d = 10

local wd = 0.1


compositeBegin()

--==============================================================================
-- Frame
--==============================================================================

   component{ id="main", size={w,h,d} }

   for c in query( "main" ) do
      split( c, "Z", { id="wing", rel=1 },
                     { id="corridor", 0 },
                     { id="wing", rel=1, constraints=true, orientation=5 } )
   end

   for c in query( "wing" ) do
      for f in fquery( c, "-Z" ) do
         component{ c, id="front", boundary=f }
      end
   end

   for c in query( "front" ) do
      slice( c, "X", { id="sfront", w/4 }, 2 )
   end

   for c in query( "sfront" ) do
      region{ c, id="windows" }
   end

   for r in rquery( "windows" ) do
      connect( execute( "architecture/window/window01",
                        { w=2.5, h=1, d=nil, d2=nil, d3=nil } ),
               r, {0.5,0.5,0}, {0,0,-wd} )
   end

   local cs={}
   for c in query( "window" ) do
      if c.constraints then
         cs[#cs+1] = volumeConstraint{ c, offset={0.2,0,0}, repulse=true }
      end
   end

   for c in query( "wing" ) do
      local cons = (c.constraints and cs)
      slice( c, "X", { id="room", w/3 }, 2, cons )
   end

   -- Walls
   for c in query( "corridor", "room" ) do
      local s = {}
      for f in fquery( c, "SIDE" ) do
         s[#s+1] = component{ c, id="wall", boundary=f, mat=0 }
      end
      for f in fquery( c, "B" ) do
         s[#s+1] = component{ c, id="wall", boundary=f, mat=3 }
      end
      extrude( s, -wd, { id="ewall" } )
   end

--==============================================================================
-- Geometry
--==============================================================================

   blocksBegin()
      for c in query( "ewall" ) do blocks{ c, id=c.mat } end
   blocksEnd()

compositeEnd()
