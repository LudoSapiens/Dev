--==============================================================================
-- Simple balcony.
--==============================================================================
local params = ... or {}
local h  = params.h  or 1.2
local w  = params.w  or 1
local d  = params.d  or 1
local h1 = params.h1 or 0.4

local of = 0.04

local c = component{ id="balcony", size={w,h,d}, connectorPosition={w/2,h1,0} }
compositeBegin(c,150)
   -- Frame
   blocksBegin()
      block{
         {-0.1,h1-0.2,0}, {w+0.1,h1-0.2,0}, {0,h1,0}, {w,h1,0},
         {-0.1,h1-0.2,d+0.1}, {w+0.1,h1-0.2,d+0.1}, {0,h1,d}, {w,h1,d},
         c=0xfff
      }
      block{ {0,h,0}, {of+0.04,h+0.02,d}, c=0xfff }
      block{ {of+0.04,h,d-0.04-of}, {w-of-0.04,h+0.02,d}, c=0xfff }
      block{ {w-of-0.04,h,0}, {w,h+0.02,d}, c=0xfff }
   blocksEnd()

   blocksBegin()
      block{ {0.1,0,0}, {w-0.1,h1-0.2,d-0.1}, c=0xfff }
   blocksEnd()

   local bh  = h-h1
   local bn1 = 5
   local bn2 = 10
   local bd1 = (d-of-0.14)/(bn1-1)
   local bd2 = (w-of*2)/(bn2-1)
   blocksBegin()
      translate( of, h1, 0.14 )
      for i=0,bn1-2 do
         block{ {-0.02,0,-0.02}, {0.02,bh,0.02}, c=0xf0f }
         translate( 0,0, bd1 )
      end
      for i=0,bn2-2 do
         block{ {-0.02,0,-0.02}, {0.02,bh,0.02}, c=0xf0f }
         translate( bd2,0,0 )
      end
      for i=0,bn1-1 do
         block{ {-0.02,0,-0.02}, {0.02,bh,0.02}, c=0xf0f }
         translate( 0,0, -bd1 )
      end
   blocksEnd()
compositeEnd()
return c
