--==============================================================================
-- Simple Range.
--==============================================================================
local params = ... or {}
local Mat = execute( "architecture/common_materials" )

-- Range information.
local w = params.w or 0.75
local h = params.h or 1.20
local d = params.d or 0.70

-- Some epsilon value.
local e = params.e or 0.05

-- Stove information.
local sh = params.sh or (e*2)

-- Elements informations.
local er = params.er or w*0.16
local eh = params.eh or e*0.5

-- Backsplash informations.
local bh = params.bh or 0.15
local bd = params.bd or e

-- Over information.
local od = d - e - e
local oh  = h - bh - eh

-- Door information.
local dh = params.dh or oh*0.6
local dy = params.dy or (oh - sh - dh)

-- Drawer information.
local rh = params.rh or (dy - 2*e)
local ry = params.ry or e

-- Materials.
local mat  = params.mat or { Mat.METAL1, Mat.METAL2, Mat.METAL3 }
local mat1 = mat[1]
local mat2 = mat[2]
local mat3 = mat[3]

-- Some useful temps.
local dy1 = dy
local dy2 = dy + dh

local c = component{ id="range", size={w,h,d}, connectorPosition={0,0,0} }
compositeBegin(c,200)

   -- Oven block.
   blocksBegin()
      block{
         {0,0 ,0 },
         {w,oh,od},
         c=0xFFF, id=mat3, g=1,
      }
   blocksEnd()

   -- Oven nose.
   blocksBegin()
      block{
         {0,dy2+e,od  },
         {w,oh   ,od+e},
         c=0xFFB, id=mat3, g=1,
      }
   blocksEnd()

   -- Door.
   blocksBegin()
      block{
         {0,dy1,od  },
         {w,dy2,od+e},
         c=0xFFF, id=mat3, g=1,
      }
   blocksEnd()

   -- Door Handle.
   blocksBegin()
      block{
         {w*0.1,dy2*0.9  ,od+e  },
         {w*0.9,dy2*0.9+e,od+e+e},
         c=0xFFF, id=mat1, g=1,
      }
   blocksEnd()

   -- Drawer.
   blocksBegin()
      block{
         {0,ry   ,od  },
         {w,ry+rh,od+e},
         c=0xFFF, id=mat3, g=1,
      }
   blocksEnd()

   -- Backsplash.
   blocksBegin()
      block{
         {0,oh   ,0 },
         {w,oh+bh,bd},
         c=0xFFB, id=mat2, g=1,
      }
   blocksEnd()

   -- Elements.
   blocksBegin()
      local pos = { {0.25, 0.25}, {0.75, 0.25}, {0.25, 0.75}, {0.75, 0.75} }
      --local pos = { {0.25, 0.25, er*0.7}, {0.75, 0.25}, {0.25, 0.75, er*1.2}, {0.75, 0.75, er*0.7} }
      for i,v in ipairs(pos) do
         local x = v[1]*w
         local y = v[2]*(od-bd) + bd
         local r = v[3] or er
         block{
            {x-r,oh   ,y-r},
            {x+r,oh+eh,y+r},
            c=0xF0F, id=mat1, g=1,
         }
      end
   blocksEnd()
compositeEnd()
return c
