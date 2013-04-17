local n   = 8
local v2d = {}
local r1  = 2
local r2  = 8
local h   = 1
local l   = r2*2
local wh  = r1*0.1
local x   = 0
local y   = x
local xyr = { 0, r2-2.5*r1 }  -- Range of generate X,Y offsets.

-- Set seed.
seed(3)

x,y = 0.5,-1

local rc = {
   n  = false, -- Randomizes the number of corridors.
   xy = false,  -- Randomizes the center offset
}

--------------------------------------------------------------------------------
local function createPolygon( n, r )
   local v = {}
   for i=1,n do
      local f = i/n
      local a = f * 2 * math.pi
      local x = math.cos( a ) * r
      local y = math.sin( a ) * r
      v[#v+1] = { x, y }
      --print(x, y)
   end
   return v
end
--------------------------------------------------------------------------------
local function randomPosition( r1, r2 )
   local a = random( 2 * math.pi )
   local d = random( r1, r2 )
   local x = math.cos( a ) * d
   local y = math.sin( a ) * d
   --print( "random position", x, y, a, d )
   return x, y
end
--------------------------------------------------------------------------------
local function validPosition( x, y, v2d )
   local v1 = v2d[#v2d]
   for i,v2 in ipairs( v2d ) do
      local a = vec2( v2[1]-v1[1], v2[2]-v1[2] )
      local b = vec2(     x-v1[1],     y-v1[2] )
      local c = vec2( v2[1]-x    , v2[2]-y     )
      if dot(a,b) < 0 or dot(a,c) < 0 then
         return false
      end
      v1 = v2
   end
   return true
end

--==============================================================================
-- Frame
--==============================================================================

compositeBegin()

   -- Randomize n.
   if rc.n then
      n = floor( random(3, 10+1) )
   end

   -- Create polygon.
   v2d = createPolygon( n, r2 )

   -- Randomize position.
   if rc.xy then
      repeat
         x, y = randomPosition( xyr[1], xyr[2] )
      until validPosition( x, y, v2d )
   end
   --x, y = 1, 4

   -- Make central section.
   local f  = r1 / r2
   local id = { "b", "t" }
   local b1 = { id=id, direction={0,h,0} }
   local b2 = { id=id, direction={0,h,0} }
   for i,v in ipairs(v2d) do
      b1[#b1+1] = { v[1]*f+x, 0, -v[2]*f-y }
      b2[#b2+1] = { v[1]    , 0, -v[2]     }
      id[#id+1] = i
   end
   local c1 = component{ boundary=b1, id="node"         }
   local c2 = component{ boundary=b2, id="center space" }

   local cs = {}

   -- Make side branches (inside center).
   for f in fquery( c1, "SIDE" ) do
      local s = component{ c1, id="node face", boundary=f }
      local fid = faceID( c1, f )
      extrude( s, 2*r2, { id="branch", n=fid } )
      cs[fid] = faceConstraint{ s, offset={0,0,0} }
   end
   local b = { c1 }
   for c in query( "branch" ) do
      b[#b+1] = c
   end
   subtract( c2, b, { id="center" } )

   -- Make side wings.
   for c in query( "center space" ) do
      for f in fquery( c, "SIDE" ) do
         local s = component{ c, id="wing face", boundary=f }
         local fid = faceID( c, f )
         extrude( s, l, { id="wing", n=fid } )
      end
   end
   --cs = {}
   for c in query( "wing" ) do
      split( c, "X", { id="side", rel=0.1 }, { id="hall", rel=1 }, { id="side", rel=0.1 }, { cs[c.n] } )
   end

--==============================================================================
-- Interior
--==============================================================================

   -- Walls
   for c in query( "center" ) do
      local s = {}
      for f in fquery( c, "TOP", "SIDE" ) do
         s[#s+1] = component{ c, id="wall", boundary=f }
      end
      extrude( s, -0.05, { id="iwall", mat=1 } )
   end
   for c in query( "side" ) do
      local s = {}
      for f in fquery( c, "TOP", "SIDE" ) do
         s[#s+1] = component{ c, id="wall", boundary=f }
      end
      extrude( s, -0.05, { id="iwall", mat=1 } )
   end
   for c in query( "wing" ) do
      local s = {}
      for f in fquery( c, "BOTTOM", "Z" ) do
         s[#s+1] = component{ c, id="wall", boundary=f }
      end
      extrude( s,  0.05, { id="iwall" } )
   end
   for c in query( "center space" ) do
      local s = {}
      for f in fquery( c, "BOTTOM" ) do
         s[#s+1] = component{ c, id="wall", boundary=f }
      end
      extrude( s,  0.05, { id="iwall" } )
   end

--==============================================================================
-- Geometry
--==============================================================================

   blocksBegin()
      for c in query( "iwall" ) do
         blocks{ c, id=c.mat }
      end
   blocksEnd()

compositeEnd()
