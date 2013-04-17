--==============================================================================
-- Simple Mattress.
--==============================================================================
local params = ... or {}

local w = params.w
local h = params.h
local d = params.d

-- Mattress.
blocksBegin()
   block{
      {0,0,0},
      {w,h,d},
      c=0x000, id=1, g=2, s=0xFFFFFF,
   }
blocksEnd()
