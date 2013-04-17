--==============================================================================
-- Simple U-shape staircase with 2 landing
--==============================================================================

local params = ... or {}
local lh     = params.lh  or 2
local ldd    = params.ldd or 0.5
local ew     = params.ew  or 0.1

-- stair parameters.
local th      = params.th or 0.03
local ns      = params.ns or 0.03
local rn      = params.rn
local riser   = params.riser or "linear"
local stw     = params.stw or 0.05
local sth     = params.sth or 0

-- Materials.
local mat     = params.mat or {}

-- Main component
local comp = params[1]
if comp == nil then return end
queryBegin(comp)

-- Derived parameters.
local stepNum = 8
local sw      = (comp.size[1]-ew)/2
local fh      = 0.1


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
      split( c, "Z", { id="emptyB", rel=1 }, { id="frontB", ldd } ) 
   else
      split( c, "Y", { id="bottomHalfLevel", rel=1 }, { id="topHalfLevel", rel=1, first=false } ) 
   end
end

for c in query( "bottomHalfLevel" ) do
   split( c, "Z", { id="backB", ldd }, { id="centerB", rel=1 }, { id="frontB", ldd } )
   c.first = (c.level == 0)
end

for c in query( "topHalfLevel" ) do
   split( c, "Z", { id="backT", ldd }, { id="centerT", rel=1 }, { id="frontT", ldd } )
end

for c in query( "centerB" ) do
   split( c, "X", { id="empty2B", sw }, { id="emptyB", rel=1 }, { id="stairB", sw } )
end

for c in query( "centerT" ) do
   split( c, "X", { id="stairT", sw, orientation=5 }, { id="emptyT", rel=1 }, { id="empty2T", sw } )
end

-- Stringer.
if stw > 0 then
   for c in query( "backT" ) do
      for f in fquery( c, "Z" ) do
         component{ c, id="landingWA", boundary=f }
      end
      for f in fquery( c, "-X", "X" ) do
         component{ c, id="landingW", boundary=f }
      end
   end
   for c in query( "frontB" ) do
      if c.level > 0 then
         for f in fquery( c, "-Z" ) do
            component{ c, id="landingWA", boundary=f }
         end
         for f in fquery( c, "-X", "X" ) do
            component{ c, id="landingW", boundary=f }
         end
      end
   end
   for c in query( "landingWA" ) do
      if c.level == lastLevel then
         split( c, "X", { id="landingW", rel=1 }, { id="landingWS", sw-stw } )
      else
         split( c, "X", { id="landingWS", sw-stw }, { id="landingW", rel=1 }, { id="landingWS", sw-stw } )
      end
   end
   for c in query( "landingW" ) do
      local st2h = c.size[2]/stepNum
      if c.level == lastLevel then st2h = st2h/2 end
      split( c, "Y", { id="landingWF", fh }, { id="landingST", st2h+sth }, { id="landingWS", rel=1 } )
   end
   local s={}
   for c in query( "landingST" ) do s[#s+1]=c end
   extrude( s, -stw, { id="eLandingST" } )
end

--==============================================================================
-- 
--==============================================================================

for c in query( "backB", "frontT" ) do
   for f in fquery( c, "T" ) do
      extrude( component{ c, id="landingCeiling", boundary=f }, -fh, { id="eLandingCeiling" } )
   end
end

for c in query( "backT", "frontB" ) do
   if (c.level > 0) or hasID( c, "backT" ) then
      for f in fquery( c, "B" ) do
         extrude( component{ c, id="landingFloor", boundary=f }, -fh, { id="eLandingFloor" } )
      end
   end
end

--==============================================================================
-- Connection
--==============================================================================

for c in query( "stairB" ) do
   region{ c, id="stair_r" }
end

for c in query( "stairT" ) do
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
   connect( 
      execute( "architecture/stair/stair01", 
         { w=size[1], h=size[2], d=size[3], stepNum=stepNum, th=th, ns=ns, rn=rn, riser=riser, stw=stw, sth=sth, trimmed=r.component.first, mat=mat } 
      ),
      r, {0,0,0}, {0,fh,0} )
end

queryEnd()
return c
