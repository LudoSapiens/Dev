--==============================================================================
-- Simple Door.
--==============================================================================
-- According to this link:
--   http://www.dimensionsguide.com/dimensions-of-a-door-frame/
-- a standard door is:
--   2.2m x 0.9m x 0.05m
-- while its jamb (frame) reaches:
--   2.3m x 0.95m x <whatever the wall is>
-- (basically, it's 10cm higher and 5cm wider).
-- Bathroom door can be a little smaller, to a minimum of:
--   2.0m x 0.7m x 0.05m
-- Note:
--  * Should we add space between elements to avoid friction?
local params = ... or {}
local h = params.h or 2.2
local w = params.w or 0.9
local d = params.d or 0.05
local o = params.o or 0
local q = { {0,1,0}, o }

--[[
    +-------------+
    |             |
    |  +--+ +--+  |
    |  |  | |  |  |
    |  +--+ +--+  |
    |             |
    |  +--+ +--+  |
    |  |  | |  |  |
    |  +--+ +--+  |
    |             |
    +-------------+

gx1 <-->
gx2 <----->
  w <------------->
--]]
local gd  = d * 0.20
local gx1 = w * 0.20
local gx2 = w * 0.45
local gx3 = w - gx2
local gx4 = w - gx1
local gy1 = h * 0.20
local gy2 = h * 0.45
local gy3 = h * 0.55
local gy4 = h * 0.90

local Mat  = execute( "architecture/common_materials" )
local mat  = params.mat or Mat.DOOR1

local c = component{ id="door", size={w,h,d}, connectorPosition={0,0,d/2}, connectorOrientation=q }
compositeBegin(c,150)

   local kx = 0.07 -- Distance from side.
   local ky = 0.91 -- 36" from bottom = 91 cm
   local bb = { {0,0,0}, {0,0,0} }
   local kp = { w-kx, ky, 0 }
   local q  = { { 0, 1, 0 }, 0.5 }
   region{ c, id="doorknob_back" , rel=bb, position=kp, orientation=q }
   kp[3] = d
   q[2] = 0.0
   region{ c, id="doorknob_front", rel=bb, position=kp, orientation=q }

   differenceBegin()
      blocksBegin()
         block{
            {0,0,0},
            {w,h,d},
            c=0xfff, id=mat,
         }
      blocksEnd()
      differenceBegin()
         -- Carve out bounding box.
         blocksBegin()
            -- Front.
            block{
               {gx1,gy1, 0},
               {gx4,gy4,gd},
               c=0xfff, id=mat,
            }
            -- Back.
            block{
               {gx1,gy1,d-gd},
               {gx4,gy4,d   },
               c=0xfff, id=mat,
            }
         blocksEnd()
         -- Reinsert center cross.
         blocksBegin()
            -- Front.
            block{
               {gx2,gy1, 0},
               {gx3,gy4,gd},
               c=0xfff, id=mat,
            }
            block{
               {gx1,gy2, 0},
               {gx4,gy3,gd},
               c=0xfff, id=mat,
            }
            -- Back.
            block{
               {gx2,gy1,d-gd},
               {gx3,gy4,d   },
               c=0xfff, id=mat,
            }
            block{
               {gx1,gy2,d-gd},
               {gx4,gy3,d   },
               c=0xfff, id=mat,
            }
         blocksEnd()
      differenceEnd()
   differenceEnd()
compositeEnd()
return c
