--==============================================================================
-- Function to build a BB
--==============================================================================
local function line( a, b, r, m )
   scopeBegin()
      local ab = b - a
      local l  = length( ab )
      local q  = quat( vec3(0,0,1), ab )
      translate( a[1], a[2], a[3] )
      rotate( q )
      block{
         { -r, -r,  0 },
         {  r, -r,  0 },
         { -r,  r,  0 },
         {  r,  r,  0 },
         { -r, -r,  l },
         {  r, -r,  l },
         { -r,  r,  l },
         {  r,  r,  l },
         c=0x0FF, id=m,
      }
      --print(a, b, r)
      --print(ab, l, q)
   scopeEnd()
end

local function blockWire( b, r )
   r = r or 6.0/128.0
   local corners = {}
   if #b == 8 then
      for i=1,8 do
         corners[i] = vec3( unpack(b[i]) )
      end
   elseif #b == 2 then
      local bl   = vec3( unpack(b[1]) )
      local tr   = vec3( unpack(b[2]) )
      local minV = min( bl, tr )
      local maxV = max( bl, tr )
      corners[1] = vec3( minV.x, minV.y, minV.z )
      corners[2] = vec3( maxV.x, minV.y, minV.z )
      corners[3] = vec3( minV.x, maxV.y, minV.z )
      corners[4] = vec3( maxV.x, maxV.y, minV.z )
      corners[5] = vec3( minV.x, minV.y, maxV.z )
      corners[6] = vec3( maxV.x, minV.y, maxV.z )
      corners[7] = vec3( minV.x, maxV.y, maxV.z )
      corners[8] = vec3( maxV.x, maxV.y, maxV.z )
   else
      print( "Missing support" )
   end
   --print("Corners:")
   --for i,v in ipairs(corners) do
   --   print(i,v)
   --end

   -- Set per-edge crease information.
   local cr = {}
   local c = b.c or 0x0
   for i=1,12 do
      local ci = c % 2
      cr[i] = ci + 2
      c = (c - ci) * 0.5
   end
   --print("Creases:")
   --for i,v in ipairs(cr) do
   --   print(i,v)
   --end

   -- Create cylinders for every edge.
   scopeBegin()
      -- Around X axis.
      line( corners[1], corners[2], r, cr[ 1] )
      line( corners[3], corners[4], r, cr[ 2] )
      line( corners[7], corners[8], r, cr[ 3] )
      line( corners[5], corners[6], r, cr[ 4] )
      -- Around Y axis.
      line( corners[1], corners[3], r, cr[ 5] )
      line( corners[5], corners[7], r, cr[ 6] )
      line( corners[6], corners[8], r, cr[ 7] )
      line( corners[2], corners[4], r, cr[ 8] )
      -- Around Z axis.
      line( corners[1], corners[5], r, cr[ 9] )
      line( corners[2], corners[6], r, cr[10] )
      line( corners[4], corners[8], r, cr[11] )
      line( corners[3], corners[7], r, cr[12] )
   scopeEnd()
end

--==============================================================================
-- COMPONENTS
--==============================================================================

component{ id="main2", position={0,0,0}, 
   boundary={{0,0,0},{0,0,0.6},{0.3,0,0.6},{0.4,0,1},{1,0,1},{1,0,0.4},{0.8,0,0},direction={0,1,0}},
   size={10,4,10}
}
local mc = component{ id="main", att1="test", boundary={{0,0,0},{0,0,1},{1,0,1},{0.2,0.0,0.8},direction={0,1,0}}, size={10,2,10} }
mc.att2 = 1

--[[
for c in query( "main" ) do
   slice( c, "Y", { id="level", 2 }, 2 )
end

for c in query( "level" ) do
   split( c, "X", { id="room", rel=2 }, { id="room", rel=1 } )
end

for c in query( "room" ) do
   local s = {}
   for f in fquery( c, "SIDE", "B" ) do
      s[#s+1] = component{ c, id="wall", boundary=f }
   end
   extrude( s, -0.1, { id="ewall" } )
   for f in fquery( c, "B" ) do
      component{ c, id="floor", boundary=f }
   end
   for f in fquery( c, "T" ) do
      component{ c, id="ceilling", boundary=f }
   end
end
--]]

-- Walls
for c in query( "main2" ) do
   local s = {}
   --[[
   for f in fquery( c, "SIDE" ) do
      s[#s+1] = component{ c, id="wall", boundary=f, depth=-0.05*f-0.05 }
   end
   for f in fquery( c, "B" ) do
      s[#s+1] = component{ c, id="floor", boundary=f, depth=-0.2 }
   end
   --]]
   for f in fquery( c ) do
      s[#s+1] = component{ c, id="wall", boundary=f, depth=-0.2}
   end
   extrude( s, -0.05, { id="ewall" } )
end

blocksBegin()
for c in query( "ewall" ) do
   --output( c )
   blocks{ c }
end
blockWire{ {0,0,0},{10,4,10},c=0}

blocksEnd()

