detailsError(1)

--------------------------------------------------------------------------------
-- Utility routines
--------------------------------------------------------------------------------

local function componentBlocks( c, width )
   width = width or 5.1
   local s = {}
   for f in fquery( c ) do
      s[#s+1] = component{ c, boundary=f }
   end
   extrude( s, width, { id="__componentBlocks" } )
   for cb in query( "__componentBlocks" ) do
      --output( cb )
      blocks{ cb }
   end
end

local function createCircle( radius, nSides )
   local s = {}
   for i=1,nSides do
      local p = {}
      local a = math.pi*2*(i-1)/nSides
      p[1] =  math.cos(a)*radius
      p[2] = 0
      p[3] = -math.sin(a)*radius
      s[i] = p
   end
   return s
end


--------------------------------------------------------------------------------
-- Building frame
--------------------------------------------------------------------------------

-- Main volume.
local ro  = 200
local h   = 10
local extrusion = { 0, h, 0 }
extrusion = { 0, 20, 0 }  -- Use { 10, 20, 0 } to make the CSG burp (calling fquery( c, "s", "t" ) is sufficient).

local pts = createCircle( ro, 5 )
pts.direction = extrusion
local outer = component{ id="main", boundary=pts, position={-ro,0,-ro} }
--output( outer )

--local ri = ro * 0.5
--local pts = createCircle( ri, 5 )
--pts.direction = extrusion
--local inner = component{ id="main", boundary=pts, position={-ri,0,-ri} }

blocksBegin()
   componentBlocks( outer )
blocksEnd()
