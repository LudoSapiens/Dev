--==============================================================================
-- Simple hole Frame.
--==============================================================================
local params = ... or {}
local d      = params.d      or 0.2
local w      = params.w      or 0.90
local h      = params.h      or 2.20

local Mat  = execute( "architecture/common_materials" )
local mat  = params.mat or Mat.DOORFRAME1

local c = component{ params.parent, id="hole", size={w,h,d}, connectorPosition={w/2,0,0} }
compositeBegin(c,100)

   -- Hole
   differenceBegin()
      inputNode()
      blocksBegin()
         block{
            {0,0,0},
            {w,h,d},
            c=0xfff, id=mat,
         }
      blocksEnd()
   differenceEnd()

compositeEnd()
return c
