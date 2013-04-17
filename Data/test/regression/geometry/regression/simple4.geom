--geometricError(0.002)
--detailsError(0.002)

--==============================================================================
-- Test torus with different creasing
--==============================================================================

--==============================================================================
-- Test arch
--==============================================================================


local n    = 4
local sa   = 0.5
local inc  = 0.5/n
local hinc = inc/2

local w = 1
local h = 1
local d = 0.1
---[[
blocksBegin()
   attraction(0,0)
   block{
      {0,0,0},
      {w/2,h/2,d},
      c=0x139,id=0,g=0
   }
   block{
      {w/2,0,0},
      {w,h/2,d},
      c=0x2c9,id=0,g=0
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
         c=0x036,id=0,g=0
      }
      a = a - inc
   end
blocksEnd()
--]]
--[[
local t = 0.1
local d2 = 0.1
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
         c=0x0f0,id=1,g=0
      }
      block{
         {cosa0.y,sina0.y,0},  {cosa1.y,sina1.y,0},
         {cosa0.z,sina0.z,0},  {cosa1.z,sina1.z,0},
         {cosa0.y,sina0.y,d2}, {cosa1.y,sina1.y,d2},
         {cosa0.z,sina0.z,d2}, {cosa1.z,sina1.z,d2},
         c=0x0f0,id=1,g=0
      }
      a = a - inc
   end
   block{
      {0,0,0},{t,h/2,d2},
      c=0x3f9,id=1,g=0
   }
   block{
      {w-t,0,0},{w,h/2,d2},
      c=0x3f9,id=1,g=0
   }
--]]
blocksEnd()

blocksBegin()
   attraction(0,0)
   block{ 
      {-3,0,0}, {-2,0,0}, {0,3,0}, {0,2,0},
      {-3,0,1}, {-2,0,1}, {0,3,1}, {0,2,1},
      c=0x0f0
   }
   block{ 
      {0,3,0}, {0,2,0}, {3,0,0}, {2,0,0},
      {0,3,1}, {0,2,1}, {3,0,1}, {2,0,1},
      c=0x0f0
   }
   block{ 
      {3,0,0}, {2,0,0}, {0,-3,0}, {0,-2,0},
      {3,0,1}, {2,0,1}, {0,-3,1}, {0,-2,1},
      c=0x0f0
   }
   block{ 
      {0,-3,0}, {0,-2,0}, {-3,0,0}, {-2,0,0},
      {0,-3,1}, {0,-2,1}, {-3,0,1}, {-2,0,1},
      c=0x0f0
   }
blocksEnd()
