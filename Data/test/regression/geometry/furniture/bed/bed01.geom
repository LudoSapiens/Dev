--==============================================================================
-- Simple Bed.
--==============================================================================
local params = ... or {}

local Mat = execute( "architecture/common_materials" )

-- single, double, queen, king
local t = params.type or "double"

-- Bed dimensions.
-- From:
--   http://en.wikipedia.org/wiki/Mattress
-- local hhh = { 0.15, 0.46 }
local dims = {
   single = { 0.99, 1.91 },
   double = { 1.37, 1.91 },
   queen  = { 1.52, 2.03 },
   king   = { 1.93, 2.03 },
}
local dim = dims[t]
local w = params.w or dim[1]
local d = params.d or dim[2]

local fh = 0.2

-- Mattress.
local mw = params.mw or w
local mh = params.mh or 0.30 -- Between 0.15 and 0.46.
local md = params.md or d

-- Box spring.
local bw = mw
local bh = mh
local bd = md

-- Pillows.
local pns = {
   single = 1,
   double = 2,
   queen  = 2,
   king   = 2,
}
local pn = params.pns or pns[t]
local pw = params.pw or 0.65
local ph = params.ph or 0.30
local pd = params.pd or 0.50
if pn*pw > mw then
   pw = mw / pn
end

-- Headboard.
local hw = w
local hh = params.hh or ((bh+mh)*2)
local hd = params.hd or 0.08

d = d + hd

-- Materials.
local mat  = params.mat or { Mat.FURNITURE1, Mat.FURNITURE2 }
local mat1 = mat[1]
local mat2 = mat[2]

local c = component{ id="bed", size={w,h,d}, connectorPosition={0,0,0} }
compositeBegin(c,200)
   -- Box spring.
   scopeBegin()
      translate( 0,fh,0)
      execute( "furniture/bed/boxspring01", {w=bw, h=bh, d=bd, mat=mat2} )
   scopeEnd()

   -- Mattress.
   scopeBegin()
      translate( 0, bh+fh, 0 )
      execute( "furniture/bed/mattress01", {w=mw, h=mh, d=md, mat=mat2} )
   scopeEnd()

   -- Pillows.
   scopeBegin()
      translate( 0, bh+mh+fh, 0 )
      local pwt = pn * pw
      local pwd = (mw - pwt)/(pn + 1)
      local pwo = pwd + pw
      local x   = pwd
      local y   = -ph*0.4
      local z   = 0.10
      translate( x, y, z )
      for i=1,pn do
         execute( "furniture/bed/pillow01", {w=pw, h=ph, d=pd, mat=mat2} )
         translate( pwo, 0, 0 )
      end
   scopeEnd()

   -- Headboard.
   scopeBegin()
      translate( 0, 0, -hd )
      local h1 = 0.05
      local h2 = bh + mh + 0.30
      local h3 = hh
      execute( "furniture/bed/board01", {w=w, h1=h1, h2=h2, h3=h3, d=hd, mat=mat1} )
   scopeEnd()

   -- Footboard.
   scopeBegin()
      translate( 0, 0, bd )
      local h1 = 0.05
      local h2 = bh + mh + fh
      local h3 = h2 + 0.10
      execute( "furniture/bed/board01", {w=w, h1=h1, h2=h2, h3=h3, d=hd, mat=mat1} )
   scopeEnd()
compositeEnd()
return c
