--==============================================================================
-- Simple Pillow.
--==============================================================================
local params = ... or {}

local w = params.w
local h = params.h
local d = params.d

local rx = params.rx or 0.1
local ry = params.ry or 0.49

local x1  = 0
local x2  = w*rx
local x3  = w-x2
local x4  = w
local y1  = 0
local y2  = h*ry
local y3  = h-y2
local y4  = h
local z1  = 0
local z2  = d

blocksBegin()
   attraction( 3, 3 )
   block{
      {x1,y2,z1},
      {x2,y3,z2},
      c=0x000, id=1, g=3, s=0x000000,
   }
   block{
      {x2,y1,z1},
      {x3,y4,z2},
      c=0x000, id=1, g=3, s=0x000000,
   }
   block{
      {x3,y2,z1},
      {x4,y3,z2},
      c=0x000, id=1, g=3, s=0x000000,
   }
blocksEnd()
