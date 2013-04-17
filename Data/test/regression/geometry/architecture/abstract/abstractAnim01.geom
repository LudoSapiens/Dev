local Mat = execute( "architecture/common_materials" )

local params = ... or {}

local on = params.on or { default = true }
local function isOn( name )
   local v = on[name]
   if v == nil then v = on.default end
   --print( name .. " is " .. tostring(v) )
   return v
end

local baseGeom = false

local s   = { params.w or 10, params.h or 3.0, params.d or 20 }
local p   = { 0, 0, 0 }
local wdi = params.wdi or 0.10
local wde = params.wde or wdi * 2
local wd  = wdi+wde
local h   = params.h or 3.0

local scpos = params.stairPos or vec3(4,0,6)
local cpos  = params.cpos or 0

local mat = {
   corridor = { floor=Mat.FLOOR1, wall=Mat.WALL1 },
   room     = { floor=Mat.FLOOR2, wall=Mat.WALL2 },
}

local bn = params.bn or 1
local boundaries = {
   {{0,0,0},{0,0,1},{1,0,1},{1,0,0},id={"b","t","s","f","s","bs"}},
   {{0,0,0},{0,0,1},{1,0,1},{1,0,0.1},{0.5,0,0.1},{0.5,0,0},id={"b","t","s","f","s","bs","s","bs"}}
}

--==============================================================================
-- Frame
--==============================================================================

compositeBegin()

   component{ id="level", size=s, position=p, mat=Mat.WALL1, matf=Mat.FLOOR2, boundary=boundaries[bn] }

   if isOn( "facade" ) then
      for  c in query( "level" ) do
         local s = {}
         for f in fquery( c, "SIDE" ) do
            s[#s+1] = component{ c, id="fwall", boundary=f, mat=Mat.EXT_WALL1 }
         end
         extrude( s, wde, { id="ewall" } )
      end
   end

   if isOn( "windows" ) then
      local wr = params.wr or 4
      for c in query( "fwall" ) do
         if hasFaceID( c, "f" ) or hasFaceID( c, "bs" ) then slice( c, "X", { id="fA", wr }, 2 ) end
      end
      for c in query( "fA" ) do
         region{ c, id="window" }
      end

      local wp = params.wp or 1
      for r in rquery( "window" ) do
         if wp == 4 then
            connect( execute( "architecture/window/window03", { mat=m, w=1.6, h=1.4, d2=wd, d3=wd } ), r, {0.5,0.5,0}, {0.0,0,-wdi} )
         elseif wp == 3 then
            connect( execute( "architecture/window/window03", { mat=m, w=0.8, h=1.5, d2=wd, d3=wd } ), r, {0.5,0.5,0}, {0.0,0,-wdi} )
         elseif wp == 2 then
            connect( execute( "architecture/window/window01", { mat=m, w=0.8, h=1.5, d2=wd, d3=wd } ), r, {0.5,0.5,0}, {0.0,0,-wdi} )
         else
            connect( execute( "architecture/window/window03", { mat=m, w=0.70, h=1.5, d2=0.2, d3=wd } ), r, {0.0,0.5,0}, {0.55,0,-wdi} )
            connect( execute( "architecture/window/window03", { mat=m, w=0.70, h=1.5, d2=0.2, d3=wd } ), r, {1.0,0.5,0}, {-0.55,0,-wdi} )
         end
      end
   end

   local cs={}
   for c in query( "window" ) do
      cs[#cs+1] = volumeConstraint{ c, offset={0.1,0,0}, repulse=true }
   end

   if isOn( "livingSpaceCorridors" ) then
      for c in query( "level" ) do
         local cp = cpos
         if cp+2 > scpos.x and cp < scpos.x+2 then cp = scpos.x-2 end
         split( c, "Z", { id="space", rel=2, cp=cp }, { id="corridor space", 2 }, { id="space", rel=2 } )
      end
      for c in query( "space" ) do
         split( c, "X", { id="living space", c.cp or cpos }, { id="corridor space", 2 }, { id="living space", rel=2 }, cs )
      end
      local s = {}
      for c in query( "corridor space" ) do s[#s+1] = c end
      merge( s, { id="corridor", matf=Mat.FLOOR1, matw=Mat.WALL1 } )
   else
      for c in query( "level" ) do
         component{ c, id="area" }
      end
   end

   if isOn( "apartment" ) then
      for c in query( "living space" ) do
         slice( c, "X", { id="apartment", 4, matf=Mat.FLOOR2, matw=Mat.WALL2 }, 2, cs )
      end
   else
      for c in query( "living space" ) do
         component{ c, id="area", matf=Mat.FLOOR2, matw=Mat.WALL2 }
      end
   end

   local e
   if isOn( "staircase" ) then
      e = component{ id={"stairwell"}, size={2,s[2]*2,3}, position=scpos }
      execute( "architecture/stair/staircase01", 
         { e, lh=h, ew=0.2, rn=true, sth=0, mat={Mat.DETAIL2,Mat.WALL1,Mat.WALL1,Mat.WALL1} }
      )
      e = component{ id={"staircase", "area"}, size={2,s[2],3}, boundary=boundaries[1], position=scpos, matf=Mat.FLOOR3, matw=Mat.WALL3 }
   end

   if e then
      for c in query( "apartment", "corridor" ) do
         subtract( c, e, { id="area" } )
      end
   else
      for c in query( "apartment", "corridor" ) do
         component{ c, id="area" }
      end
   end

--==============================================================================
-- Interior
--==============================================================================

   -- Walls
   for c in query( "area" ) do
      local s = {}
      for f in fquery( c, "SIDE" ) do
         s[#s+1] = component{ c, id="wall", boundary=f, mat=c.matw }
      end
      for f in fquery( c, "BOTTOM" ) do
         s[#s+1] = component{ c, id="floor", boundary=f, mat=c.matf }
      end
      extrude( s, -wdi, { id="iwall" } )
   end

   for c in query( "wall" ) do
      if hasParentID(c, "apartment") and occlusion( c, "corridor" ) > 0.01 and not c.parent.hasDoor then
         if c.size.x > 0.8 and c.parent.size.x > 0.5 then
            region{ c, id="door" }
            c.parent.hasDoor = true
         end
      elseif hasParentID(c,"staircase") and hasFaceID(c, "f" ) then
         region{ c, id="door" }
      end
   end

   -- Doors
   if isOn( "door" ) then
      local doorOpening = params.doorOpening or 0.1
      local doorType    = params.doorType or 2
      local doorFlipped = params.doorFlipped
      if doorFlipped then doorOpening = -doorOpening end
      local doors = {
         { "architecture/door/door01", o=doorOpening },
         { "architecture/door/door02", o=doorOpening, mat=Mat.DOOR2 },
      }
      local door = doors[doorType]
      for r in rquery( "door" ) do
         connect( execute( "architecture/door/set01", { d=wdi*2, door=door, flipped=doorFlipped } ), r, {0.6,0,0}, {0,wdi,-wdi} )
      end
   end

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
