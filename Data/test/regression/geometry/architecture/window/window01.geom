--==============================================================================
-- Simple window.
--==============================================================================
local params = ... or {}
local h  = params.h  or 1.2
local w  = params.w  or 1
local w2 = params.w2 or 0
local d  = params.d  or 0.04
local d2 = params.d2 or 0.2
local d3 = params.d3 or 0.16
local t  = params.t  or 0.04
local t2 = params.t2 or 0.1

local Mat = execute( "architecture/common_materials" )
local mat = params.mat or { Mat.DETAIL1, Mat.DETAIL2, Mat.DETAIL2 }
local mat1 = mat[1]
local mat2 = mat[2]
local mat3 = mat[3]

local c = component{ id="window", size={w,h,d}, connectorPosition={w/2,h/2,0} }
compositeBegin(c,100)
   differenceBegin()
      inputNode()
      -- Hole
      blocksBegin()
         block{
            {0,0,0},
            {w,h,d3},
            c=0xfff, id=mat1
         }
      blocksEnd()
   differenceEnd()
   -- Frame
   translate(0,0,d3/2)
   blocksBegin()
      attraction(1,1)
      block{
         {-w2,0,0},
         {w+w2,t2,d2},
         c=0xfff, id=mat2
      }
      block{
         {0,t2,0},
         {t,t2,0},
         {0,h,0},
         {t,h-t,0},
         {0,t2,d},
         {t,t2,d},
         {0,h,d},
         {t,h-t,d},
         c=0xfff, g=1, id=mat3
      }
      block{
         {w-t,t2,0},
         {w,t2,0},
         {w-t,h-t,0},
         {w,h,0},
         {w-t,t2,d},
         {w,t2,d},
         {w-t,h-t,d},
         {w,h,d},
         c=0xfff, g=1, id=mat3
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
         c=0xfff, g=1, id=mat3
      }
      block{
         {w/2-t/2,t2,0}, {w/2+t/2,h-t,d}, c=0xfff, id=mat3
      }
      block{
         {t,h/3-t/2,0}, {w/2-t/2,h/3+t/2,d}, c=0xfff, id=mat3
      }
      block{
         {w/2+t/2,h/3-t/2,0}, {w-t,h/3+t/2,d}, c=0xfff, id=mat3
      }
      block{
         {t,2*h/3-t/2,0}, {w/2-t/2,2*h/3+t/2,d}, c=0xfff, id=mat3
      }
      block{
         {w/2+t/2,2*h/3-t/2,0}, {w-t,2*h/3+t/2,d}, c=0xfff, id=mat3
      }
      --[[
      block{
         {0,0,0.01},
         {w,h,0.02},
         c=0xfff, id=mat1
      }
      --]]
   blocksEnd()

   local bw = params.bw or 0.2
   local bh = params.bh or 0.2
   local bd = params.bd or d+0.1
   local bn = params.bn or 0

   if bn > 0 then
      local bs = (h-bh-t2)/(bn-1)
      blocksBegin()
         for i=0,bn-1 do
            block{ {-bw,h-bh-bs*i,0},{0,h-bs*i,bd}, c=0xfff }
            block{ {w,h-bh-bs*i,0},{w+bw,h-bs*i,bd}, c=0xfff }
         end
      blocksEnd()
   end
compositeEnd()
return c
