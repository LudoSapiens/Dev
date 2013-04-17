local params = ... or {}

local w  = params.w  or 0.36
local h1 = params.h1 or 0.15
local h2 = params.h2 or 0.40
local h3 = params.h2 or 0.80
local d  = params.d  or 0.65
local bw = params.bw or 0.20

local x1 = 0
local x2 = (w - bw) * 0.5
local x3 = w - x2
local x4 = w

local y1 = 0
local y2 = h1
local y3 = h2
local y4 = h3

local z1 = 0
local z2 = d * 0.30
local z3 = d * 0.85
local z4 = d

local rh  = h2 * 1.5 -- - h1
local rd  = z4 - z2
local rx1 = params.rx1 or (w * 0.4)
local rx2 = params.rx2 or (w * 0.5)
local ry1 = params.ry1 or (rh * 0.4)
local ry2 = params.ry2 or (rh * 0.5)
local rz1 = params.rz1 or (rd * 0.4)
local rz2 = params.rz2 or (rd * 0.5)

local cx = rx2
local cy = h2
local cz = z2 + rz2*0.5

local c = component{ id="toilet", size={w,h,d}, connectorPosition={0,0,0} }
compositeBegin(c,200)

   -- Base.
   --blocksBegin()
   --blocksEnd()

   -- Bowl.
   differenceBegin()
      -- Exterior.
      unionBegin()
         blocksBegin()
            local cz1 = z2*0.4
            local cz2 = z1
            local cz3 = cz
            local cz4 = cz+rz1*0.8
            block{
               {x2, y1, cz1},
               {x3, y1, cz1},
               {x2, y3, cz2},
               {x3, y3, cz2},
               {x2, y1, cz3},
               {x3, y1, cz3},
               {x2, cy, cz4},
               {x3, cy, cz4},
               c=0xF0F, id=1, g=1, s=0x055000,
            }
         blocksEnd()
         translate( cx, cy, cz )
         blocksBegin()
            block{
               {-rx2,-ry2,-rz2},
               { rx2, ry2, rz2},
               c=0x000, id=1, g=1, s=0x000000,
            }
         blocksEnd()
      unionEnd()
      translate( cx, cy, cz )
      -- Interior.
      blocksBegin()
         block{
            {-rx1,-ry1,-rz1},
            { rx1, ry1, rz1},
            c=0x000, id=1, g=1, s=0x000000,
         }
      blocksEnd()
      blocksBegin()
         block{
            {-rx2, 0 ,-rz2},
            { rx2,ry2, rz2},
            c=0xFFF, id=1, g=1, s=0x000000,
         }
      blocksEnd()
   differenceEnd()

   -- Tank.
   blocksBegin()
      block{
         {x1,y3,z1},
         {x4,y4,z2},
         c=0x000, id=1, g=1, s=0x555555,
      }
      local hx = x1 + (x4-x1)*0.2
      local hy = y3 + (y4-y3)*0.7
      local hz = z2 - 0.005
      block{
         {hx     , hy     , hz     },
         {hx+0.05, hy+0.01, hz+0.01},
         c=0x000, id=2, g=1, s=0x555555,
      }
   blocksEnd()
compositeEnd()
return c
