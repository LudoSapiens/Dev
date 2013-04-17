--==============================================================================
-- Simple Couch.
--==============================================================================
local params = ... or {}

local Mat = execute( "architecture/common_materials" )

-- Couch dimensions.
local w = params.w or 2.0
local h = params.h or 1.0
local d = params.d or 0.9

-- Arms dimensions.
local aw1 = params.aw1 or 0.2
local aw2 = params.aw2 or 0.3
local ah1 = params.ah1 or h*0.3
local ah2 = params.ah2 or h*0.7
local ad  = d

-- Back dimensions.
local bw  = w - aw2*2
local bhl = params.bhl or h*0.9
local bhc = params.bhc or h
local bhr = params.bhr or bhl
local bd  = params.bd  or d*0.3

-- Seat dimensions.
local cw = params.cw or 0.8
local sn = params.sn or math.ceil(bw/cw)
local cw = bw / sn
local ch = params.ch or 0.18
local cd = d

-- Base dimensions.
local uw = bw
local uh = params.uh or 0.3
local ud = d

-- Materials.
local mat  = params.mat or { Mat.FURNITURE2 }
local mat1 = mat[1]

local c = component{ id="couch", size={w,h,d}, connectorPosition={0,0,0} }
compositeBegin(c,200)
   -- Arms, back, and base.
   local ax0 = 0
   local ax1 = aw2 - aw1
   local ax2 = ax1 + aw1
   local bx1 = aw2
   local bx2 = bx1 + bw*0.5
   local bx3 = bx1 + bw

   blocksBegin()
      --attraction(1,2)
      attraction(1,3)
      attraction(4,4)
      --attraction(3,4)

      -- Right arm.
      block{
         {ax1, 0 , 0},
         {ax2,ah1,ad},
         c=0xCFF, id=mat1, g=1, s=0x555555,
      }
      block{
         {ax0,ah1, 0},
         {ax2,ah2,ad},
         c=0x2F9, id=mat1, g=2, s=0xAAAAAA,
      }
      -- Left arm.
      block{
         {w-ax2, 0 , 0},
         {w-ax1,ah1,ad},
         c=0xCFF, id=mat1, g=1, s=0x555555,
      }
      block{
         {w-ax2,ah1, 0},
         {w-ax0,ah2,ad},
         c=0x1F9, id=mat1, g=2, s=0xAAAAAA,
      }
      -- Base.
      block{
         {bx1, 0,bd*0.5},
         {bx3,uh,ud    },
         c=0x000, id=mat1, g=3, s=0xFFFFFF,
      }
   blocksEnd()

   blocksBegin()
      attraction(4,4)
      -- Back.
      block{
         {bx1, 0 , 0},
         {bx2, 0 , 0},
         {bx1,bhl, 0},
         {bx2,bhc, 0},
         {bx1, 0 ,bd},
         {bx2, 0 ,bd},
         {bx1,bhl,bd},
         {bx2,bhc,bd},
         c=0x000, id=mat1, g=4, s=0xFFFF0F,
      }
      block{
         {bx2, 0 , 0},
         {bx3, 0 , 0},
         {bx2,bhc, 0},
         {bx3,bhr, 0},
         {bx2, 0 ,bd},
         {bx3, 0 ,bd},
         {bx2,bhc,bd},
         {bx3,bhr,bd},
         c=0x000, id=mat1, g=4, s=0xFFFFF0,
      }
   blocksEnd()

   -- Seat cushions.
   blocksBegin()
      local x   = bx1
      local y   = uh
      local cw_ = cw * 1.0
      local ch_ = ch * 1.0
      local cd_ = cd * 1.0
      for i=1,sn do
         block{
            {x    ,y    ,bd },
            {x+cw_,y+ch_,cd_},
            c=0x000, id=mat1, g=2, s=0xFFFFFF,
         }
         x = x + cw
      end
   blocksEnd()
compositeEnd()
return c
