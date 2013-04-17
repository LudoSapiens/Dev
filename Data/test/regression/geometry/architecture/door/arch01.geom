--==============================================================================
-- Arch doorway
--==============================================================================

local params = ... or {}
local h  = params.h or 2
local w  = params.w or 1
local d  = params.d or 0.1
local d2 = params.d2 or 0.1
local t  = params.t or 0.1
local r  = params.r or 0.5

local n    = 4
local sa   = 0.5
local inc  = 0.5/n
local hinc = inc/2

local h1 = h*r
local h2 = h-h1

local Mat = execute( "architecture/common_materials" )
local mat = params.mat or Mat.DOORFRAME1

local c = component{ id="window", size={w,h,d}, connectorPosition={w/2,0,0} }
compositeBegin(c,100)

   differenceBegin()
      inputNode()
      -- Hole.
      blocksBegin()
         attraction(0,0)
         block{
            {0,0,0},
            {w/2,h1,d},
            c=0x139,id=mat,g=0
         }
         block{
            {w/2,0,0},
            {w,h1,d},
            c=0x2c9,id=mat,g=0
         }
         local a = sa
         for i=0,n-1 do
            local sina = sin( vec3(a,a-hinc,a-inc) )
            local cosa = cos( vec3(a,a-hinc,a-inc) )
            sina = sina*h2 + h1
            cosa = cosa*(w/2) + (w/2)
            block{
               {cosa.x,sina.x,0}, {w/2,h1,0},
               {cosa.y,sina.y,0}, {cosa.z,sina.z,0},
               {cosa.x,sina.x,d}, {w/2,h1,d},
               {cosa.y,sina.y,d}, {cosa.z,sina.z,d},
               c=0x036,id=mat,g=0
            }
            a = a - inc
         end
      blocksEnd()
   differenceEnd()

   if t > 0 then
      -- Frame
      blocksBegin()
         attraction(0,0)
         local a = sa
         for i=0,n-1 do
            local sina  = sin( vec3(a,a-hinc,a-inc) )
            local cosa  = cos( vec3(a,a-hinc,a-inc) )
            local sina0 = sina*h2 + h1
            local sina1 = sina*(h2-t) + h1
            local cosa0 = cosa*(w/2) + (w/2)
            local cosa1 = cosa*(w/2-t) + (w/2)
            block{
               {cosa0.x,sina0.x,0},  {cosa1.x,sina1.x,0},
               {cosa0.y,sina0.y,0},  {cosa1.y,sina1.y,0},
               {cosa0.x,sina0.x,d2}, {cosa1.x,sina1.x,d2},
               {cosa0.y,sina0.y,d2}, {cosa1.y,sina1.y,d2},
               c=0x0f0,id=mat,g=0
            }
            block{
               {cosa0.y,sina0.y,0},  {cosa1.y,sina1.y,0},
               {cosa0.z,sina0.z,0},  {cosa1.z,sina1.z,0},
               {cosa0.y,sina0.y,d2}, {cosa1.y,sina1.y,d2},
               {cosa0.z,sina0.z,d2}, {cosa1.z,sina1.z,d2},
               c=0x0f0,id=mat,g=0
            }
            a = a - inc
         end
         block{
            {0,0,0},{t,h1,d2},
            c=0x3f9,id=mat,g=0
         }
         block{
            {w-t,0,0},{w,h1,d2},
            c=0x3f9,id=mat,g=0
         }
      blocksEnd()
   end
compositeEnd()

return c
