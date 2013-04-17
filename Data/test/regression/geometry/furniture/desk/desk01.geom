local params = ... or {}

local w = params.w or 1.50
local h = params.h or 0.75
local d = params.d or 0.55

-- Top extent.
local wet = params.wet or 0.02
local web = params.web or wet * 0.5
local ht  = params.ht  or 0.02

-- Legs.
local wl1 = params.wl1 or 0.03
local wl2 = params.wl2 or 0.05
local hl  = params.hl  or 0.05
local dl  = params.dl  or 0.02

-- Drawers.
local nd = params.nd or { floor(w/0.60), floor(h/0.30) }
-- Weights to define the space around the drawers.
-- Consider the drawers to have a weight of 1.
local ds = params.ds or 0.10
local di = params.di or 0.20
local db = params.db or 0.20
local dt = params.dt or db
local dd = web

-- Useful variables.
local e = max( wet, web )

local x1 = 0
local x2 = e
local x3 = w - x2
local x4 = w

local y1 = 0
local y2 = hl
local y3 = h - ht
local y4 = h

local z1 = 0
local z2 = d - e
local z3 = d

local c = component{ id="desk", size={w,h,d}, connectorPosition={0,0,0} }
compositeBegin(c,200)
   -- Legs.
   blocksBegin()
      block{
         {x2    ,y1,z1   },
         {x2+wl2,y2,z1+dl},
         c=0xFFF, id=2,
      }
      block{
         {x3-wl2,y1,z1   },
         {x3    ,y2,z1+dl},
         c=0xFFF, id=2,
      }
      block{
         {x2    ,y1,z2-dl},
         {x2+wl2,y2,z2   },
         c=0xFFF, id=2,
      }
      block{
         {x3-wl2,y1,z2-dl},
         {x3    ,y2,z2   },
         c=0xFFF, id=2,
      }
   blocksEnd()

   -- Main block.
   blocksBegin()
      block{
         {x2,y2,z1},
         {x3,y3,z2},
         c=0xFFF, id=1,
      }
   blocksEnd()

   local front = component{ id="desk_front", size={x3-x2,y3-y2}, position={x2,y2,z2} }

   -- Top.
   blocksBegin()
      local x5 = x2 - web
      local x6 = w - x5
      local x7 = x2 - wet
      local x8 = w - x7
      local z4 = z2 + web
      local z5 = z2 + wet
      block{
         {x5,y3,z1},
         {x6,y3,z1},
         {x7,y4,z1},
         {x8,y4,z1},
         {x5,y3,z4},
         {x6,y3,z4},
         {x7,y4,z3},
         {x8,y4,z3},
         c=0xFFF, id=1,
      }
   blocksEnd()

   -- Place drawer locations.

   -- Horizontal split.
   local tmpX = {}
   for i=1,nd[1] do
      tmpX[#tmpX+1] = { id="drawer_hspace", rel=di }
      tmpX[#tmpX+1] = { id="drawer_hregion", rel=1 }
   end
   tmpX[1]       = { id="left_space", rel=ds }
   tmpX[#tmpX+1] = { id="right_space", rel=ds }
   split( front, "X", unpack(tmpX) )

   -- Vertical split.
   local tmpY = {}
   for i=1,nd[2] do
      tmpY[#tmpY+1] = { id="drawer_vspace", rel=di }
      tmpY[#tmpY+1] = { id="drawer_region", rel=1 }
   end
   tmpY[1]       = { id="bottom_space", rel=db }
   tmpY[#tmpY+1] = { id="top_space", rel=dt }
   for f in query( "drawer_hregion" ) do
      split( f, "Y", unpack(tmpY) )
   end

   for f in query( "drawer_region" ) do
      -- Normally, we'd create separate components, but we only need geometry.
      extrude( f, dd, { id="drawer"} )
   end

   blocksBegin()
      for c in query( "drawer" ) do
         blocks{ c }
      end
   blocksEnd()
compositeEnd()
return c
