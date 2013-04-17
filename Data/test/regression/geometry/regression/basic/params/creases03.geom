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
   r = r or 3.0/128.0
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
--[[
local rows = {
   {   nil, 0xFFB, 0xBBF, 0xf0f },
   { 0xFFF, 0xFBF, 0xBFB, 0xBBB },
   {   nil, 0xBFF, 0xFBB, 0x000 },
}
--]]
local rows2 = {
   { 0xFFF, 0xBFF, 0xFBB },
   { 0xBBB, 0xF0F, 0x309 },
   { 0x00F, 0xA00, 0x000 },
}


local rows = { {rows2[3][3]} }

local xs = 5
local ys = 5
local yn  = #rows
local xn  = 0
for _,row in ipairs(rows) do
   local n = #row
   if xn < n then xn = n end
end
blocksBegin()
   translate( (xn-1)*-0.5*xs, (yn-1)*0.5*ys, 0 )
   for y=1,yn do
      local row = rows[y] or {}
      for x=1,xn do
         local c = row[x]
         if c then
            local t = { {-1,-1,-1}, {1,1,1}, c=c }
            block( t )
            blockWire( t )
         end
         translate( xs, 0, 0 )
      end
      translate( -xn*xs, -ys, 0 )
   end
blocksEnd()
