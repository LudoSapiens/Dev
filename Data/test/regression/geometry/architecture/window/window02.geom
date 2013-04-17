--==============================================================================
-- Arch window
--==============================================================================
local params = ... or {}
local h  = params.h or 1
local w  = params.w or 1
local d  = params.d or 0.1
local d2 = params.d2 or 0.1
local t  = params.t or 0.1
local f  = params.f or 0.04

local n    = 4
local sa   = 0.5
local inc  = 0.5/n
local hinc = inc/2

local Mat = execute( "architecture/common_materials" )
local mat = params.mat or { Mat.EXT_DETAIL1, Mat.EXT_DETAIL2, Mat.EXT_DETAIL2 }
local mat1 = mat[1]
local mat2 = mat[2]
local mat3 = mat[3]

local c = component{ id="window", size={w,h,d}, connectorPosition={w/2,h/2,0} }
compositeBegin(c,100)
   differenceBegin()
      inputNode()
      -- Hole.
      blocksBegin()
         attraction(0,0)
         block{
            {0,0,0},
            {w/2,h/2,d},
            c=0x139,id=mat1,g=0
         }
         block{
            {w/2,0,0},
            {w,h/2,d},
            c=0x2c9,id=mat1,g=0
         }
         local a = sa
         for i=0,n-1 do
            local sina = sin( vec3(a,a-hinc,a-inc) )
            local cosa = cos( vec3(a,a-hinc,a-inc) )
            sina = sina*(h/2) + (h/2)
            cosa = cosa*(w/2) + (w/2)
            block{
               {cosa.x,sina.x,0}, {w/2,h/2,0},
               {cosa.y,sina.y,0}, {cosa.z,sina.z,0},
               {cosa.x,sina.x,d}, {w/2,h/2,d},
               {cosa.y,sina.y,d}, {cosa.z,sina.z,d},
               c=0x036,id=mat1,g=0
            }
            a = a - inc
         end
      blocksEnd()
   differenceEnd()
   -- Frame
   if params.frame then
   blocksBegin()
      attraction(0,0)
      local a = sa
      for i=0,n-1 do
         local sina  = sin( vec3(a,a-hinc,a-inc) )
         local cosa  = cos( vec3(a,a-hinc,a-inc) )
         local sina0 = sina*(h/2) + (h/2)
         local sina1 = sina*(h/2-t) + (h/2)
         local cosa0 = cosa*(w/2) + (w/2)
         local cosa1 = cosa*(w/2-t) + (w/2)
         block{
            {cosa0.x,sina0.x,0},  {cosa1.x,sina1.x,0},
            {cosa0.y,sina0.y,0},  {cosa1.y,sina1.y,0},
            {cosa0.x,sina0.x,d2}, {cosa1.x,sina1.x,d2},
            {cosa0.y,sina0.y,d2}, {cosa1.y,sina1.y,d2},
            c=0x0f0,id=mat2,g=0
         }
         block{
            {cosa0.y,sina0.y,0},  {cosa1.y,sina1.y,0},
            {cosa0.z,sina0.z,0},  {cosa1.z,sina1.z,0},
            {cosa0.y,sina0.y,d2}, {cosa1.y,sina1.y,d2},
            {cosa0.z,sina0.z,d2}, {cosa1.z,sina1.z,d2},
            c=0x0f0,id=mat2,g=0
         }
         a = a - inc
      end
      block{
         {0,0,0},{t,h/2,d2},
         c=0x3f9,id=mat2,g=0
      }
      block{
         {w-t,0,0},{w,h/2,d2},
         c=0x3f9,id=mat2,g=0
      }
      -- inside cross.
      block{
         {(w-f)/2,0,f},{(w+f)/2,h-t,f*2},
         c=0xfff,id=mat3,g=1
      }
      block{
         {t,(h-f)/2,f},{(w-f)/2,(h+f)/2,f*2},
         c=0xfff,id=mat3,g=1
      }
      block{
         {(w+f)/2,(h-f)/2,f},{w-t,(h+f)/2,f*2},
         c=0xfff,id=mat3,g=1
      }
   blocksEnd()
   end
compositeEnd()
return c
