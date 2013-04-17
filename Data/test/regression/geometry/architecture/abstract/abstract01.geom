local s   = { 10, 5, 10 }
--local s   = { 10, 5, 12 }
local p   = { 0, 0, 0 }
local wdi = 0.10
local wde = wdi * 2

--==============================================================================
-- Frame
--==============================================================================

compositeBegin()

   component{ id="main", size=s, position=p, mat=13 }
   --component{ id="main", size=s, boundary={{0,0,0},{0,0,0.6},{0.3,0,0.8},{0.3,0,1},{0.8,0,1},{0.8,0,0.8},{1,0,0.8},{1,0,0},direction={0,1,0},},position=p, mat=13 }

   for c in query( "main" ) do
      slice( c, "Y", { id="story", 2.5, level=counter() } )
   end

   for c in query( "story" ) do
      split( c, "Z", { id="living space", 4 }, { id="corridor", 2, mat=12 }, { id="living space", rel=2 } )
   end

   for c in query( "living space" ) do
      split( c, "X", { id="apartment", rel=1 }, { id="apartment", rel=1 } )
   end

   local e = component{ id={"elevator", "room"}, ex=-0.06, size={2,s[2],2}, position={4,0,2}, mat=11 }

   for c in query( "apartment", "corridor" ) do
      subtract( c, e, { id="room" } )
   end
--[[
   for c in query( "apartment", "corridor", "elevator" ) do
      local s = {}
      for f in fquery( c, "SIDE", "BOTTOM" ) do
         s[#s+1] = component{ c, id="ext", boundary=f }
      end
      extrude( s, c.ex or -0.05, { id="exti" } )
   end
   blocksBegin()
      for c in query( "exti" ) do
         blocks{ c, id=c.mat }
      end
   blocksEnd()
compositeEnd()
--]]
--==============================================================================
-- Interior
--==============================================================================

   -- Walls
   for c in query( "room" ) do
      local s = {}
      for f in fquery( c, "SIDE" ) do
         s[#s+1] = component{ c, id="wall", boundary=f }
      end
      for f in fquery( c, "BOTTOM" ) do
         s[#s+1] = component{ c, id="floor", boundary=f }
      end
      extrude( s, -wdi, { id="iwall" } )
   end

--[[
   for c in query( "story" ) do
      local s = {}
      for f in fquery( c, "SIDE" ) do
         s[#s+1] = component{ c, id="facade", boundary=f }
      end
      extrude( s, wde, { id="ewall", mat=1 } )
   end
--]]
---[[
   for c in query( "story" ) do
      for f in fquery( c, "SIDE" ) do
         component{ c, id="facade", boundary=f }
      end
   end
   for c in query( "facade" ) do
      split( c, "Y", { id="f1", rel=1 }, { id="ledge", 0.2 } )
   end
   local s = {}
   for c in query( "f1" ) do s[#s+1] = c end
   extrude( s, wde, { id="ewall", mat=1 } )
   local s = {}
   for c in query( "ledge" ) do s[#s+1] = c end
   extrude( s, wde+0.1, { id="ewall", mat=1 } )
   for c in query( "f1" ) do
      slice( c, "X", { id="f2", 4 }, 2 )
   end
   for c in query( "f2" ) do
      region{ c, id="window" }
   end
--]]

   for c in query( "wall" ) do
      if hasParentID(c, "apartment" ) and occlusion( c, "corridor" ) > 0.01 then
         region{ c, id="door" }
      end
   end

   for c in query( "wall" ) do
      if hasParentID( c, "elevator" ) then
         slice( c, "Y", { id="elwall", 2.5 } )
      end
   end
   for c in query( "elwall" ) do
      if occlusion( c, "corridor" ) > 0.01 then
         region{ c, id="eldoor" }
      end
   end

   -- Doors
   for r in rquery( "door" ) do
      connect( execute( "architecture/door/set01", {door={w=0.76,h=2, o=-0.1}, frame={d=wdi*2} } ), r, {0.5,0,0}, {0,wdi,-wdi} )
   end
   for r in rquery( "eldoor" ) do
      connect( execute( "architecture/door/frame02", {w=1.2,h=2,d=wdi*2} ), r, {0.5,0,0}, {0,wdi,-wdi} )
   end
---[[
   -- Windows
   for r in rquery( "window" ) do
      if r.component.level >= 0 then
         connect( execute( "architecture/window/window03", {d3=wde+wdi} ), r, {0.5,0.5,0}, {0,wdi,-wdi} )
      else
         connect( execute( "architecture/window/window02", {d=wde+wdi} ), r, {0.5,0.5,0}, {0,wdi,-wdi} )
      end
   end
--]]
--==============================================================================
-- Geometry
--==============================================================================

   blocksBegin()
      for c in query( "iwall" ) do
         blocks{ c, id=c.mat }
      end
      for c in query( "ewall" ) do
         blocks{ c, id=c.mat }
      end
   blocksEnd()

compositeEnd()
--]]
