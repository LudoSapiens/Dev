--==============================================================================
-- Simple Door Frame.
--==============================================================================
-- Standard door frames are:
--   2.3m x 0.95m x <depth>
local params = ... or {}
local jamb   = params.jamb   or 0.05
local lintel = params.lintel or 0.05
local d      = params.d      or 0.2
local w      = params.w      or 0.90
local h      = params.h      or 2.20

local ht     = h + lintel
local wt     = w + jamb*2

local Mat  = execute( "architecture/common_materials" )
local mat  = params.mat or Mat.DOORFRAME1
local hmat = params.hmat or 1

local c = component{ id="doorframe", size={wt,ht,d}, connectorPosition={wt/2,0,0} }
compositeBegin(c,100)

   -- Set a region for a door.
   local d_2 = d * 0.5
   region{ c, id="doorHinge_left",  rel={{0,0,0},{0,0,0}}, position={jamb,0,d} }
   region{ c, id="doorHinge_right", rel={{0,0,0},{0,0,0}}, position={wt-jamb,0,d}, orientation={{0,1,0},0.5} }

   -- Hole
   differenceBegin()
      inputNode()
      blocksBegin()
         block{
            {0,0,0},
            {wt,ht,d},
            c=0xfff, id=hmat,
         }
      blocksEnd()
   differenceEnd()
   -- Frame.
   blocksBegin()
      attraction(1,1)
      -- Left jamb.
      block{
         {0,0,0},
         {jamb,h,d},
         c=0xfff, g=1, id=mat,
      }
      -- Right jamb.
      block{
         {wt-jamb,0,0},
         {wt,h,d},
         c=0xfff, g=1, id=mat,
      }
      -- Lintel.
      block{
         {0,h,0},
         {wt,ht,d},
         c=0xfff, g=2, id=mat,
      }
   blocksEnd()
compositeEnd()
return c
