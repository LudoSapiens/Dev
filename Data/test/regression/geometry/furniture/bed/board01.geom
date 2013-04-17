--==============================================================================
-- Simple Headboard.
--==============================================================================
local params = ... or {}

local w   = params.w
local h1  = params.h1
local h2  = params.h2
local h3  = params.h3
local d   = params.d
local mat = params.mat

local x1 = 0
local x2 = d
local x3 = w-x2
local x4 = w

blocksBegin()
   block{
      {x1, 0,0},
      {x2,h3,d},
      c=0x309, id=mat, g=1, s=0xFFFFFF,
   }
blocksEnd()

blocksBegin()
   block{
      {x2,h1,0},
      {x3,h2,d},
      c=0x309, id=mat, g=1, s=0xFFFFFF,
   }
blocksEnd()

blocksBegin()
   block{
      {x3, 0,0},
      {x4,h3,d},
      c=0x309, id=mat, g=1, s=0xFFFFFF,
   }
blocksEnd()
