--==============================================================================
-- Simple window.
--==============================================================================
local params = ... or {}
local h  = params.h  or 1.2
local w  = params.w  or 1
local d  = params.d  or 0.04
local d2 = params.d2 or 0.2
local d3 = params.d3 or 0.16
local t  = params.t  or 0.06
local t2 = params.t2 or 0.1

local c = component{ id="window", size={w,h,d}, connectorPosition={w/2,h/2,0} }
compositeBegin(c,100)
   differenceBegin()
      inputNode()
      -- Hole
      blocksBegin()
         block{
            {0,0,0},
            {w,h,d3},
            c=0xfff, id=0
         }
      blocksEnd()
   differenceEnd()
   -- Frame
   translate(0,0,d3/2)
   blocksBegin()
      attraction(1,1)
      block{
         {0,0,0},
         {t,t,0},
         {0,h,0},
         {t,h-t,0},
         {0,0,d},
         {t,t,d},
         {0,h,d},
         {t,h-t,d},
         c=0xfff, g=1, id=2
      }
      block{
         {w-t,t,0},
         {w,0,0},
         {w-t,h-t,0},
         {w,h,0},
         {w-t,t,d},
         {w,0,d},
         {w-t,h-t,d},
         {w,h,d},
         c=0xfff, g=1, id=2
      }
      block{
         {t,h-t,0},
         {w-t,h-t,0},
         {0,h,0},
         {w,h,0},
         {t,h-t,d},
         {w-t,h-t,d},
         {0,h,d},
         {w,h,d},
         c=0xfff, g=1, id=2
      }
      block{
         {0,0,0},
         {w,0,0},
         {t,t,0},
         {w-t,t,0},
         {0,0,d},
         {w,0,d},
         {t,t,d},
         {w-t,t,d},
         c=0xfff, g=1, id=2
      }
      block{
         {w/2-t/2,t,0}, {w/2+t/2,h-t,d}, c=0xfff, id=2
      }
   blocksEnd()
compositeEnd()
return c
