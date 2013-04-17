--==============================================================================
-- Simple Doorset.
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
-- Can also look at:
--   http://en.wikipedia.org/wiki/Door#Doorway_components

local params  = ... or {}
local dparams = params.door  or {}
local fparams = params.frame or {}
local kparams = params.knob  or {}
local door    = dparams[1] or "architecture/door/door01"
local frame   = fparams[1] or "architecture/door/frame01"
local knob    = kparams[1] or "architecture/door/knob01"
local mat     = params.mat or {}
dparams.mat = dparams.mat or mat[1]
fparams.mat = fparams.mat or mat[2]
kparams.mat = kparams.mat or mat[3]

local d   = execute( door, dparams )
fparams.w = d.size.x
fparams.h = d.size.y
local c   = execute( frame, fparams )

-- Position the door in the frame.
if params.flipped then
   for r in rquery( c, "doorHinge_right" ) do
      connect( d, r )
   end
else
   for r in rquery( c, "doorHinge_left" ) do
      connect( d, r )
   end
end
-- Place a knob on the door.
for r in rquery( d, "doorknob_front" ) do
   connect( execute( knob, kparams ), r )
end
for r in rquery( d, "doorknob_back" ) do
   connect( execute( knob, kparams ), r )
end

return c
