--==============================================================================
-- Simple Boxspring.
--==============================================================================
local params = ... or {}

local w = params.w
local h = params.h
local d = params.d

-- Boxspring.
blocksBegin()
   block{
      {0,0,0},
      {w,h,d},
      c=0x309, id=1, g=1, s=0xFFFFFF,
   }
blocksEnd()
