--==============================================================================
-- Simple small round knob
--==============================================================================

local params = ... or {}

local bh  = params.bh  or 0.01 -- Height of the base.
local br  = params.br  or 0.02 -- Radius of the base.
local ch  = params.ch  or 0.04 -- Height of the cylinder.
local cr  = params.cr  or 0.01 -- Radius of the cylinder.
local kh  = params.kh  or 0.05 -- Height of the knob.
local kr1 = params.kr1 or 0.05 -- Radius of the knob at the base.
local kr2 = params.kr2 or 0.05 -- Radius of the knob at its tip.

local Mat = execute( "architecture/common_materials" )
local mat = params.mat or Mat.DOORKNOB1

local r = br
if r <  cr then r = cr  end
if r < kr1 then r = kr1 end
if r < kr2 then r = kr2 end
local r2 = r*2

local h = bh+ch+kh

local c = component{ id="doorknob", size={r2,r2,h}, connectorPosition={r,r,0} }
compositeBegin(c,150)
   blocksBegin()
      -- Base.
      translate( r, r, 0 )
      block{
         {-br,-br,  0},
         { br, br, bh},
         c=0x0FF, id=mat,
      }

      -- Cylinder.
      translate( 0, 0, bh )
      block{
         {-cr,-cr,  0},
         { cr, cr, ch},
         c=0x0FF, id=mat,
      }

      -- Knob.
      translate( 0, 0, ch-kh*0.5 )
      block{
         {-kr1,-kr1,  0},
         { kr1,-kr1,  0},
         {-kr1, kr1,  0},
         { kr1, kr1,  0},
         {-kr2,-kr2, kh},
         { kr2,-kr2, kh},
         {-kr2, kr2, kh},
         { kr2, kr2, kh},
         c=0x000, id=mat,
      }
   blocksEnd()
compositeEnd()
return c
