geometricError(0.04)
--detailsError(0.02)
detailsError(100)
--detailsError(0.02)


--==============================================================================
-- Parameters
--==============================================================================

local w  = 100
local h  =  50
local d  =  80

local wc = w*0.20  -- Width of the center section.
local wr = 8       -- Width of every room.

local hb = 15      -- Height of the bottom section.
local ht = h - hb  -- Height for the top section
local hl = 2.5     -- Height for levels.

local dt  = d * 0.45  -- Depth of the top section.
local dbr = 0.80      -- Depth ratio of the bottom center section.

local ew = 12  -- The width of the elevator shaft.
local eh =  h  -- The height of the elevator shaft.
local ed =  8  -- The depth of the elevator shaft.

local sw =  8  -- The width of the stairwells.
local sh =  h  -- The height of the stairwells.
local sd =  8  -- The depth of the stairwells.

local tw = wr*0.6  -- Width of the bathrooms.
local td = 2       -- Depth of the bathrooms.

local sectionInset = 2    -- Distance by which the top section insets the bottom section.
local corrW        = 4    -- Width of corridors.
local wallD        = 0.05 -- Wall thickness.

--==============================================================================
-- Variables
--==============================================================================

local nLevels  = floor( h/hl )
local midLevel = floor( hb/hl )

local frameO = wallD + 0.01
local frameD = frameO * 2

--==============================================================================
-- Utility Functions
--==============================================================================

local perpTable = { X="Z", Z="X" }
local function perp( ori )
   return perpTable[ori]
end


--==============================================================================
-- Building shell
--==============================================================================

compositeBegin()

   component{ id="hull", size={w,h,d} }

   for c in query( "hull" ) do
      split( c, "Y", { id="bottom hull", hb }, { id="top hull", rel=1, startLevel=midLevel } )
   end

   -- Bottom section.
   for c in query( "bottom hull" ) do
      split( c, "X", { id="wing hull", side="left", rel=1 },
                     { id="bottom center hull", side="center", wc },
                     { id="wing hull", side="right", rel=1 } )
   end

   for c in query( "wing hull" ) do
      component{ c, id="wing",
                 size=c.size, boundary={
                    {0,0,0},{0,0,dbr},{0.3,0,1},{0.7,0,1},{1,0,dbr},{1,0,0},
                    direction={0,1,0},
               } }
   end

   for c in query( "bottom center hull" ) do
      split( c, "Z", { id="wing", side="center", rel=dbr }, { id="empty", rel=1-dbr } )
   end

   -- Top section.
   local s = { w - 2*sectionInset, ht, dt }
   local p = { sectionInset, 0, sectionInset }
   for c in query( "top hull" ) do
      local bx1 = 0.1
      local bz1 = bx1 * s[1] / s[2]
      local fx1 = bx1
      local fx2 = 0.3
      local fx3 = 0.5 - (wc / s[1])*0.5
      component{ c, id="wing", side="top",
                 size=s, position=p, boundary={
                    {0,0,bz1},{0,0,1},
                    {fx1,0,1.3},{fx2,0,1.3},{fx3,0,1},{1-fx3,0,1},{1-fx2,0,1.3},{1-fx1,0,1.3},{1,0,1},
                    {1,0,bz1},
                    {1-bx1,0,0},{bx1,0,0},
                    direction={0,1,0},
                    --id={"s","fo","f","fo","s","b"}
                 } }
   end

   -- Level.
   --for c in query( "main" ) do
   --   split( c, "Y", { id="hull", rel=1 }, { id="roof", 0.8 } )
   --end
   for c in query( "wing" ) do
      local levelCounter = counter( c.startLevel )
      slice( c, "Y", { id="wing level", hl, level=levelCounter } )
   end
   --for c in query( "roof" ) do
   --   c.level = levelCounter.current
   --end

--==============================================================================
-- Interior
--==============================================================================

   --[[
   -- Elevator shaft.
   local s  = { 8, h, 4 }
   local es = component{ id="elevator shaft", size=s, position={(w-s[1])*0.5,0,1} }

   -- Stairwells.
   local s   = { 4, h, 4 }
   local p   = { 1, 0, 6 }
   local sw1 = component{ id="stairwell", size=s, position=p }
   p[1] = w - p[1] - s[1]
   local sw2 = component{ id="stairwell", size=s, position=p }
   --]]

   local t = { left=1, right=4, center=0, top=0 }
   for c in query( "wing level" ) do
      if c.level % 3 == 0 then
         if c.side == "center" then
            --component{ c, id="rooms area" }
         else
            component{ c, id="living area", size=c.size, orientation=t[c.side] }
         end
      end
   end

   local t = { left="X", right="X", center="Z", top="Z" }
   for c in query( "living area" ) do
      split( c, "Z", { id="rooms area", rel=1, orientation=5 },
                     { id="corridor space", corrW },
                     { id="rooms area", rel=1 } )
   end

   for c in query( "rooms area" ) do
      slice( c, "X", { id="room area", wr } )
   end

   for c in query( "room area" ) do
      split( c, "Z", { id="room entrance area", 5 }, { id="bedroom space", rel=1 } )
      --local b = component{ c, id="bathroom", size=c.size,
      --                     boundary={
      --                        {0,0,0},{0,0,bd},{bw,0,bd},{bw,0,0},
      --                        direction={0,1,0},
      --                     } }
      --subtract( c, b, { id="room" } )
   end

   for c in query( "room entrance area" ) do
      split( c, "X", { id="room entrance space", rel=1 }, { id="bathroom space", tw } )
   end

   -- Corridor Location.
   local cz
   for c in query( "corridor space" ) do
      if c.side == "top" then
         cz = c.position[3] - sd
      end
   end

   -- Elevator Shaft.
   local s = {ew,eh,ed}
   local p = {(w-ew)*0.5,0,cz}
   local es = component{ id="elevator", size=s, position=p }

   -- Stairwells.
   local s = {sw,sh,sd}
   local sw1, sw2
   for c in query( "corridor space" ) do
      if c.side == "left" then
         p = {c.position[1]-corrW-sd,0,cz}
         sw1 = component{ id="stairwell", size=s, position=p }
      elseif c.side == "right" then
         p = {c.position[1]+corrW,0,cz}
         sw2 = component{ id="stairwell", size=s, position=p }
      end
   end
   --local s = {sw,sh,sd}
   --local p = {15,0,15}
   --local sw1 = component{ id="stairwell", size=s, position=p }
   --local p = {w-p[1],0,15}
   --local sw2 = component{ id="stairwell", size=s, position=p }

   -- Cut spaces with elevator and stairwells.
   for c in query( "bathroom space" ) do
      subtract( c, { es, sw1, sw2 }, { id="bathroom" } )
      --component{ c, id="bathroom" }
   end
   for c in query( "bedroom space" ) do
      subtract( c, { es, sw1, sw2 }, { id="bedroom" } )
      --component{ c, id="bedroom" }
   end
   for c in query( "room entrance space" ) do
      subtract( c, { es, sw1, sw2 }, { id="room entrance" } )
      --component{ c, id="room entrance" }
   end
   for c in query( "corridor space" ) do
      subtract( c, { es, sw1, sw2 }, { id="corridor" } )
   end

   -- Walls
   for c in query( "bathroom", "bedroom", "closet", "corridor", "elevator", "hall", "room", "stairwell" ) do
      local s = {}
      for f in fquery( c, "SIDE", "B" ) do
         s[#s+1] = component{ c, id="wall", boundary=f }
      end
      extrude( s, -wallD, { id="iwall" } )
   end

   local t = {
      { "room entrance", { "SIDE" } },
   }
   for i,v in ipairs( t ) do
      for c in query( v[1] ) do
         local s = {}
         for f in fquery( c, unpack( v[2] ) ) do
            s[#s+1] = component{ c, id="wall", boundary=f }
         end
         extrude( s, -wallD, { id="iwall" } )
      end
   end

   -- Regions for door placement.
   for c in query( "room entrance" ) do
      for f in fquery( c, "-Z" ) do
         region{ c, f, id="door" }
      end
   end

   --[[
   -- Regions for window placement.
   for c in query( "room" ) do
      for f in fquery( c, "Z" ) do
         if faceID( c, f ) then region{ c, f, id="window" } end
      end
   end
   --]]

--==============================================================================
-- Exterior
--==============================================================================

   --[[
   for c in query( "level", "roof" ) do
      for f in fquery( c, "SIDE" ) do
         component{ c, id="lfacade", boundary=f }
      end
   end
   for c in query( "lfacade" ) do
      split( c, "Y", { id="wallFacade", rel=1 }, { id="ledge1", 0.1 }, { id="ledge2", 0.1 } )
   end
   for c in query( "wallFacade" ) do
      if c.level == 0 then
         split( c, "X", { id="wallFacade1", 0.3 }, { id="wallFacade2", rel=1 }, { id="wallFacade1", 0.3 } )
      elseif c.level < nLevels then
         split( c, "X", { id="wallFacade1", 0.3 }, { id="wallFacade2", rel=0.4 }, { id="wallFacade1", rel=1 }, { id="wallFacade2", rel=0.4 }, { id="wallFacade1", 0.3 } )
      else
         split( c, "X", { id="wallFacade1", 0.3 }, { id="wallFacade2", rel=1 },  { id="wallFacade1", 0.3 } )
      end
   end

   local s = {}
   for c in query( "wallFacade2" ) do
      s[#s+1] = c
   end
   extrude( s, 0.15, { id="ewall", mat=20 } )
   local s = {}
   for c in query( "wallFacade1" ) do
      s[#s+1] = c
   end
   extrude( s, 0.1, { id="ewall", mat=21 } )

   local s = {}
   for c in query( "ledge1" ) do
      s[#s+1] = c
   end
   extrude( s, 0.2, { id="eledge", mat=30 } )

   local s = {}
   for c in query( "ledge2" ) do
      s[#s+1] = c
   end
   extrude( s, 0.3, { id="eledge", mat=30 } )
   --]]

--==============================================================================
-- Furnishing
--==============================================================================


--==============================================================================
-- Geometry
--==============================================================================

   blocksBegin()
      for c in query( "iwall" ) do blocks{ c, id=0 } end
      for c in query( "ewall" ) do blocks{ c, id=c.mat } end
      for c in query( "eledge" ) do blocks{ c, id=c.mat } end
   blocksEnd()

--==============================================================================
-- Connections
--==============================================================================

   print("Connecting doors")
   for r in rquery( "door" ) do
      local s = r.size
      local w = 0.7
      connect( execute( "architecture/door/frame02", {w=w,h=2,d=frameD} ), r, {0.5,0,0}, {0,wallD,-frameO} )
   end

   for r in rquery( "window" ) do
      if r.component.level == lastLevel then
         connect( execute( "architecture/window/window01", { w=1.6, h=1.6, d2=0.3, d3=0.26 } ), r, {0.5,0.5,0}, {0,-0.2,-0.1} )
      else
         connect( execute( "architecture/window/window01", { w=1, h=1.6, d2=0.3, d3=0.26 } ), r, {0.5,0.5,0}, {0,0,-0.1} )
      end
   end

compositeEnd()
