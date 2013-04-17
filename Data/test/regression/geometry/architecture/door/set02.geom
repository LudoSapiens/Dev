--==============================================================================
-- Simple double door doorset.
--==============================================================================

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

local d0  = execute( door, dparams )
local d1  = execute( door, dparams )
fparams.w = d0.size.x*2
fparams.h = d0.size.y
local c   = execute( frame, fparams )

-- Position the doors in the frame.
for r in rquery( c, "doorHinge_left" ) do
   connect( d0, r )
end
for r in rquery( c, "doorHinge_right" ) do
   connect( d1, r )
end
-- Place a knobs on the door.
for r in rquery( f, "doorknob_front" ) do
   connect( execute( knob, kparams ), r )
end
for r in rquery( f, "doorknob_back" ) do
   connect( execute( knob, kparams ), r )
end

return c
