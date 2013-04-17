local params = ... or {}

local w = params.w or 1.70
local h = params.h or 0.40
local d = params.d or 0.70

local bx = params.bx or 0.05
local by = params.by or bx
local bz = params.bx or bx

local e = 0.01

local x1 = 0
local x2 = bx
local x3 = w - bx
local x4 = w
local y1 = 0
local y2 = by
local y3 = h
local y4 = 2*h
local z1 = 0
local z2 = bz
local z3 = d - bz
local z4 = d

local c = component{ id="bath", size={w,h,d}, connectorPosition={0,0,0} }
compositeBegin(c,200)
   differenceBegin()
      -- Box.
      blocksBegin()
         block{
            {x1,y1,z1},
            {x4,y3,z4},
            c=0xFFB, id=1, s=0xF0F000
         }
      blocksEnd()
      -- Bath interior.
      blocksBegin()
         block{
            {x2,y2,z2},
            {x3,y2,z2},
            {x2,y4,z2},
            {x3,y4,z2},
            {x2,y2,z3},
            {x3,y2,z3},
            {x2,y4,z3},
            {x3,y4,z3},
            c=0x000, id=1, g=1, s=0x000055,
         }
      blocksEnd()
   differenceEnd()
compositeEnd()
return c
