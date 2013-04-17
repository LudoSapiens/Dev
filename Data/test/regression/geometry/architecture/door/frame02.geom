--==============================================================================
-- Simple Door Frame.
--==============================================================================
-- Standard door frames are:
--   2.3m x 0.95m x <depth>
local params = ... or {}
local jamb   = params.jamb   or 0.08
local lintel = params.lintel or 0.08
local d      = params.d      or 0.2
local w      = params.w      or (0.90 + jamb*2)
local h      = params.h      or (2.20 + lintel)

local Mat  = execute( "architecture/common_materials" )
local mat  = params.mat or Mat.DOORFRAME1
local hmat = params.hmat or 1
local c = component{ params.parent, id="doorframe", size={w,h,d}, connectorPosition={w/2,0,0} }
compositeBegin(c,100)

   -- Set a region for a door.
   local d_2 = d * 0.5
   --region{ c, id="doorHinge", { {0,0,d_2}, {0,0,d_2} } }

   -- Hole
   differenceBegin()
      inputNode()
      blocksBegin()
         block{
            {0,0,-0.01},
            {w,h,d+0.01},
            c=0xfff, id=hmat,
         }
      blocksEnd()
   differenceEnd()

   -- Frame.
   local h1 = h - lintel
   local w1 = w - jamb
   blocksBegin()
      -- Left jamb.
      block{
         {0,0,0},
         {jamb,h1,d},
         c=0xfff, g=1, id=mat,
      }
      -- Right jamb.
      block{
         {w1,0,0},
         {w,h1,d},
         c=0xfff, g=1, id=mat,
      }
      -- Lintel.
      block{
         {0,h1,0},
         {w,h,d},
         c=0xfff, g=2, id=mat,
      }
   blocksEnd()
compositeEnd()
return c
