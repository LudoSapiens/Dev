local p = { c=0x000, s=0x000000 }

local sx = 20
local sy =  5

local rangex = { n=5, min=0.0, max=1.5 }
local rangey = { n=5, min=0.0, max=1.5 }
for i,r in ipairs( { rangex, rangey } ) do
   r.delta = r.max - r.min
   r.n_1   = r.n - 1
end

local function drawPair( dx, dy )
   print( "Pair", dx, dy )
   dx = dx * 0.5
   dy = dy * 0.5
   local dxp1 = dx + 1
   blocksBegin()
      scopeBegin()
         translate(-dxp1,-dy, 0 )
         p.id = 1
         block( p )
      scopeEnd()
      scopeBegin()
         translate( dxp1, dy, 0 )
         p.id = 2
         block( p )
      scopeEnd()
      attraction( 0, 0 )
   blocksEnd()
end

local function toHalf( n )
   if n % 2 == 0 then
      return (n-1) * 0.5
   else
      return (n-1) * 0.5
   end
end

translate( sx*(rangey.n_1)*-0.5, sy*(rangex.n_1)*0.5, 0.0 )
for y=1,rangey.n do
   local fy = (y-1)/rangey.n_1
   local dy = rangey.delta * fy + rangey.min
   scopeBegin()
   for x=1,rangex.n do
      local fx = (x-1)/rangex.n_1
      local dx = rangex.delta * fx + rangex.min
      drawPair( dx, dy )
      translate( 0, -sy, 0 )
   end
   scopeEnd()
   translate( sx, 0, 0 )
end
