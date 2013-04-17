--==============================================================================
-- Simple O-shape staircase with 4 landing
--==============================================================================

local params = ... or {}
local lh     = params.lh  or 2
local ldd    = params.ldd or 1

-- stair parameters.
local th      = params.th or 0.03
local ns      = params.ns or 0.03
local rn      = params.rn
local riser   = params.riser or "linear"
local stw     = params.stw or 0.05
local sth     = params.sth or 0

-- Materials.
local mat     = params.mat or {}

-- Derived parameters.
local stepNum = 4
local sw      = ldd
local fh      = 0.1

-- Main component
local comp = params[1]
if comp == nil then return end
queryBegin(comp)

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
   if c.level == lastLevel then
      split( c, "X", { id="bottomLanding", sw }, { id="emptyB", rel=1 } ) 
   else
      split( c, "Y", { id="qLevel1", rel=1, sub=0 }, { id="qLevel2", rel=1, sub=1 }, { id="qLevel3", rel=1, sub=2 }, { id="qLevel4", rel=1, sub=3 } ) 
   end
end

for c in query( "qLevel1" ) do
   split( c, "X", { id="lstairl", sw }, { id="emptyB", rel=1 } )
end
for c in query( "qLevel2" ) do
   split( c, "Z", { id="lstairl", sw, orientation=1 }, { id="emptyB", rel=1 } )
end
for c in query( "qLevel3" ) do
   split( c, "X", { id="emptyB", rel=1 }, { id="lstairl", sw, orientation=5 } )
end
for c in query( "qLevel4" ) do
   split( c, "Z", { id="emptyB", rel=1 }, { id="lstairl", sw, orientation=4 } )
end

for c in query( "lstairl" ) do
   split( c, "Z", { id="topLanding", sw }, { id="stair", rel=1 }, { id="bottomLanding", sw } )
end

-- Stringer.
if stw > 0 then
   for c in query( "bottomLanding" ) do
      if (c.level > 0) or (c.sub > 0) then
         for f in fquery( c, "-X", "Z" ) do
            component{ c, id="landingW", boundary=f }
         end
      end
   end
   for c in query( "landingW" ) do
      local st2h = c.size[2]/stepNum
      if c.level == lastLevel then st2h = st2h/4 end
      split( c, "Y", { id="landingWF", fh }, { id="landingST", st2h+sth }, { id="landingWS", rel=1 } )
   end
   local s={}
   for c in query( "landingST" ) do s[#s+1]=c end
   extrude( s, -stw, { id="eLandingST" } )
end

--==============================================================================
-- 
--==============================================================================

for c in query( "topLanding" ) do
   for f in fquery( c, "T" ) do
      extrude( component{ c, id="landingCeiling", boundary=f }, -fh, { id="eLandingCeiling" } )
   end
end

for c in query( "bottomLanding" ) do
   if (c.level > 0) or (c.sub > 0) then
      for f in fquery( c, "B" ) do
         extrude( component{ c, id="landingFloor", boundary=f }, -fh, { id="eLandingFloor" } )
      end
   end
end

--==============================================================================
-- Connection
--==============================================================================

for c in query( "stair" ) do
   region{ c, id="stair_r" }
end

--==============================================================================
-- Geometry
--==============================================================================

blocksBegin()
   for c in query( "eLandingFloor", "eLandingCeiling", "eLandingST" ) do
      blocks{ c, id=mat[1] }
   end
blocksEnd()

for r in rquery( "stair_r" ) do
   local size = r.size
   local first = (r.component.level==0) and (r.component.sub==0)
   connect( 
      execute( "architecture/stair/stair01", 
         { w=size[1], h=size[2], d=size[3], stepNum=stepNum, th=th, ns=ns, rn=rn, riser=riser, stw=stw, sth=sth, trimmed=first, mat=mat } 
      ),
      r, {0,0,0}, {0,fh,0} )
end

queryEnd()
