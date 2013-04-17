local params = ... or {}

local w = params.w or 0.70
local h = params.h or 0.85
local d = params.d or 0.40

local h1  = params.h1 or (h * 0.80)

local rx1 = params.rx1 or (w * 0.5)
local rx2 = params.rx2 or (rx1 * 1.3)
local ry1 = params.ry1 or (w * 0.3)
local ry2 = params.ry2 or (w * 0.5)
local rz1 = params.rz1 or (d * 0.8)
local rz2 = params.rz2 or (rz1 * 1.3)

local br1 = params.br1 or (w * 0.20)
local br2 = params.br2 or (w * 0.10)
local bh  = h - ry1

local frx = 0.03
local fry = frx
local frz = frx
local fh  = frx * 4

local c = component{ id="sink", size={w,h,d}, connectorPosition={0,0,0} }
compositeBegin(c,200)
   translate(h/2,0,d)
   differenceBegin()
      -- Sink exterior.
      blocksBegin()
         block{
            {-rx2,h-ry2,-rz2},
            { rx2,h+ry2, rz2},
            c=0x000, id=1, g=1, s=0x000000,
         }
      blocksEnd()
      -- Sink interior.
      blocksBegin()
         block{
            {-rx1,h-ry1,-rz1},
            { rx1,h+ry1, rz1},
            c=0x000, id=1, g=1, s=0x000000,
         }
      blocksEnd()
      -- Sink surface (cutting the spheres above).
      blocksBegin()
         block{
            {-rx2,h    ,-rz2},
            { rx2,h+ry2, rz2},
            c=0xFFF, id=1, g=1, s=0x000000,
         }
      blocksEnd()
   differenceEnd()

   -- Base.
   blocksBegin()
      block{
         {-br1, 0,-br1    },
         { br1, 0,-br1    },
         {-br2,bh,-br2-0.1},
         { br2,bh,-br2-0.1},
         {-br1, 0, br1    },
         { br1, 0, br1    },
         {-br2,bh, br2    },
         { br2,bh, br2    },
         c=0xF0F, id=1, g=1, s=0x000000,
      }
   blocksEnd()

   -- Faucet.
   scopeBegin()
      local f = 0.5681818181 -- Special factor to get closest to a sphere.
      local d = rz1 + ((rz2-rz1)*0.5)
      d = d * 0.75
      translate( 0, h, -d )
      -- Faucet.
      blocksBegin()
         block{
            {-frx,0 ,-frz},
            { frx,fh, frz},
            c=0xFFF, id=2, g=1, s=0x000000,
         }
         block{
            {-frx,fh-fry,-frz   },
            { frx,fh    , fh*0.7},
            c=0xFFF, id=2, g=1, s=0x000000,
         }
      blocksEnd()
      -- Hot water.
      translate( -3*frx, 0, 0 )
      blocksBegin()
         block{
            {-frx, 0 ,-frz},
            { frx,fry, frz},
            c=0xF0F, id=2, g=1, s=0x000000,
         }
      blocksEnd()
      -- Cold water.
      translate( 6*frx, 0, 0 )
      blocksBegin()
         block{
            {-frx, 0 ,-frz},
            { frx,fry, frz},
            c=0xF0F, id=2, g=1, s=0x000000,
         }
      blocksEnd()
   scopeEnd()
compositeEnd()
return c
