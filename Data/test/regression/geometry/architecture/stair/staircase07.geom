--==============================================================================
-- Simple staircase
--==============================================================================

local params = ... or {}
local lh     = params.lh  or 2
local ldd    = params.ldd or 0.5

-- stair parameters.
local th      = params.th or 0.03
local ns      = params.ns or 0.03
local rn      = params.rn
local riser   = params.riser or "linear"
local stw     = params.stw or 0.05
local sth     = params.sth or 0

-- Materials.
local mat     = params.mat

-- Main component
local comp = params[1]
if comp == nil then return end
queryBegin(comp)

-- Derived parameters.
local stepNum = 16
local fh      = 0.05
local wd      = 0.05

--==============================================================================
-- Interior volume
--==============================================================================

local lastLevel = -1
for c in query( "staircaseLevel" ) do
   lastLevel = max( lastLevel, c.level )
end

-- Do we need to split?
if lastLevel == -1 then
   local levelCounter = counter()
   slice( comp, "Y", { id="staircaseLevel", lh, level=levelCounter } )
   lastLevel = levelCounter.current-1
end

for c in query( "staircaseLevel" ) do
   if c.level ~= lastLevel then
      component{ c, id="stairB" } 
   end
end

--==============================================================================
-- Connection
--==============================================================================

for c in query( "stairB" ) do
   region{ c, id="stair_r" }
end

--==============================================================================
-- Geometry
--==============================================================================

for r in rquery( "stair_r" ) do
   local size = r.size
   connect( 
      execute( "architecture/stair/stair01", 
         { w=size[1]-wd, h=size[2], d=size[3], stepNum=stepNum, th=th, ns=ns, rn=rn, riser=riser, stw=stw, sth=sth, trimmed=true, railing=true, mat=mat } 
      ),
      r, {0,0,0}, {wd,fh,0} )
end

queryEnd()

