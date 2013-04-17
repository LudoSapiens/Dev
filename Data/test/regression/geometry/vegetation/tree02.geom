local function evalCatmullRom( p0, p1, p2, p3, t )
   local t_sq = t * t
   local t_cb = t * t_sq
   return (
               (p1*2)
             + (p2 - p0) * t
             + (p0*2 - p1*5 + p2*4 - p3) * t_sq
             + (p1*3 - p0   - p2*3 + p3) * t_cb
          ) * 0.5
end

local function evalCatmullRomDeriv( p0, p1, p2, p3, t )
   local t_sq = t * t
   return (
               (p2 - p0)
             + 2*(p0*2 - p1*5 + p2*4 - p3) * t
             + 3*(p1*3 - p0   - p2*3 + p3) * t_sq
          ) * 0.5
end

local function evalCatmullRomMulti( pts, t )
   local n   = #pts
   local s   = n - 3 -- Number of valid segments
   local ts  = t * s
   local i,f = math.modf( ts )
   local p0 = pts[i+1]
   local p1 = pts[i+2]
   local p2 = pts[i+3]
   local p3 = pts[i+4]
   local p,d = evalCatmullRom( p0, p1, p2, p3, f ),
               evalCatmullRomDeriv( p0, p1, p2, p3, f )
   --print( "...", t, ts, i, f, p, d )
   return p,d
end

-----------------------------------------------------------------------------
-- Spins one or multiple points of pts around the line (center, axis).
-- It does so n times, and stores the results in dst.
-- The axis must be normalized.
local function lathe( center, axis, n, pts, dst )
   if type(pts) ~= "table" then
      local dPts = pts - center
      for i=0,n-1 do
         local a = i/n
         local q = quat( axis, a )
         local p = center + q*dPts
         dst[#dst+1] = p
      end
   else
      -- Iterate over all of the values of pts every time.
      local dPts = {}
      for i,v in ipairs( pts ) do
         dPts[i] = v - center
      end
      for i=0,n-1 do
         local a = i/n
         local q = quat( axis, a )
         local d = {}
         for i,v in ipairs( dPts ) do
            local p = center + q*v
            d[i] = p
         end
         dst[#dst+1] = d
      end
   end
end

local function pp( top )
   print( "Center: "..tostring(top.center), top[1], top[2], top[3], top[4] )
end

local function simpleBlock( pts, i, t )
   local s = 0
   if i == 10 then s = 0x001000 end
   block{
      pts[1], pts[2], pts[3], pts[4],
      pts[5], pts[6], pts[7], pts[8],
      m=0, g=1, c=0x0, s=s, id=1,
   }
end

--------------------------------------------------------------------------------
-- Must return newPos, newDir, newOri, newR.
local function simpleTransform( oldPos, oldDir, oldOri, oldR, oldT, newPos, newDir, newT, i )
   local q = quat( oldDir, newDir )
   return newPos, newDir, oldOri, oldR, newT
end

--------------------------------------------------------------------------------
-- Must return newPos, newDir, newOri, newR.
local function twistTransform( oldPos, oldDir, oldOri, oldR, oldT, newPos, newDir, newT, i )
   local q = quat( oldDir, newDir )
   --return newPos, newDir, q * oldOri * quat( vec3(0,0,1), (newT-oldT)*0.5 ), oldR*newT+0.2, newT
   return newPos, newDir, q * oldOri * quat( vec3(0,0,1), (newT-oldT)*0.5 ), abs(0.8-newT)+0.3, newT
end

--------------------------------------------------------------------------------
local function catmullRomBlocks( pts, n, adj, blockFunc, transFunc )
   blockFunc = blockFunc or simpleBlock
   transFunc = transFunc or simpleTransform
   local t0  = 0
   local t1  = 1
   local dt  = t1 - t0
   local ring = { vec3(1,0,0), vec3(0,1,0), vec3(-1,0,0), vec3(0,-1,0) }
   local bot
   local top = {}
   local p,d = evalCatmullRomMulti( pts, t0 )
   local ori = quat( vec3(0,0,1), normalize(d) ) -- * quat( vec3(1,0,0), normalize(adj) )
   local x   = adj - p
   local r   = length( x )
   local xn  = normalize( x )
   local dn  = normalize( d )
   ori = quat( xn, cross(dn,xn), dn )
   top.pos, top.dir, top.ori, top.r, top.t = transFunc( p, d, ori, r, t0, p, d, t0, 0 )
   for i,v in ipairs( ring ) do
      top[i] = top.ori*v*top.r + top.pos
   end
   --pp( top )

   for i=1,n do
      local t = t0 + dt*i/n
      bot = top
      top = {}
      local p,d = evalCatmullRomMulti( pts, t )
      top.pos, top.dir, top.ori, top.r, top.t = transFunc( bot.pos, bot.dir, bot.ori, bot.r, bot.t, p, d, t, i )
      --ori = quat( bot.dir, top.dir ) * ori
      for i,v in ipairs( ring ) do
         --top[i] = ori*v*r + top.center
         top[i] = top.ori*v*top.r + top.pos
      end
      --pp( top )
      blockFunc(
         --{ bot[1], bot[2], bot[4], bot[3], top[1], top[2], top[4], top[3], },
         { bot[2], bot[1], top[2], top[1], bot[3], bot[4], top[3], top[4], },
         i,
         t
      )
   end
end

--[[
local pts = {
   vec3( 0, 0, 0 ),
   vec3( 1, 0, 0 ),
   vec3( 2, 0, 0 ),
   vec3( 3, 1, 0 ),
   vec3( 4, 0, 0 ),
   vec3( 5, 0, 0 ),
   vec3( 6, 0, 0 ),
}
catmullRomBlocks( pts, 10, vec3(1,-0.5,0.5), nil, twistTransform )
--]]

local pts = {
   vec3( 0, 0, 0 ),
   vec3( 0, 1, 0 ),
   vec3( 0, 2, 0 ),
   vec3( 0, 3, 1 ),
   vec3( 0, 4, 0 ),
   vec3( 0, 5, 0 ),
   vec3( 0, 6, 0 ),
}

local pts2 = {
   vec3( 0.1, 4, 0 ),
   vec3( 0.1, 5, 0 ),
   vec3( 1, 6, -1 ),
   vec3( 1, 7, -1 ),
   vec3( 2, 8, 0 ),
}

local pts3 = {
   vec3( -0.1, 4, 0 ),
   vec3( -0.1, 5, 0 ),
   vec3( -1, 6, 1 ),
   vec3( -1, 7, 1 ),
   vec3( -1, 8, 0 ),
}

local pts4 = {
   vec3( 0, 4, 0 ),
   vec3( 0, 4.5, 0 ),
   vec3( 0.5, 6, 1 ),
   vec3( 0.5, 7, 1 ),
   vec3( 0, 8, 2 ),
}

local pts5 = {
   vec3(0,4.8,-0.2),
   vec3(0,4.8,-0.5),
   vec3(0,5,-1.5),
   vec3(0,6,-2),
   vec3(0,7,-3),
}

local pts6 = {
   vec3(0,4.8,0),
   vec3(-0.5,4.8,0),
   vec3(-1.5,5,0),
   vec3(-3,6,0),
   vec3(-3,7,0),
}

-- Trunk and branches.
blocksBegin()
   attraction(1,1)
   catmullRomBlocks( pts, 10, vec3(0.5,1,-0.5), nil, twistTransform )
   catmullRomBlocks( pts2, 7, vec3(0.2,5,-0.2), nil, nil )
   catmullRomBlocks( pts3, 5, vec3(-0.2,5,-0.2), nil, nil )
   --catmullRomBlocks( pts5, 6, vec3(0.2,5.0,-0.5), nil, nil )
   --catmullRomBlocks( pts6, 6, vec3(-0.5,5.0,-0.2), nil, nil )
   translate(0.1,-0.5,0.3)
   catmullRomBlocks( pts4, 6, vec3(0.2,4.5,-0.2), nil, nil )
blocksEnd()

--[[
-- Leaves.
displacement(
   function( IN )
      local p = IN.pos
      return p + IN.n * (perlin1( p*2 )*0.2)
   end
)

blocksBegin()
   translate(2,8.5,-0.5)
   block{ {-3.5,-3,-3.5},{3.5,3,3.5} }
   translate(-3,-0.5,2)
   block{ {-3.5,-3,-3.5},{3.5,3,3.5} }
blocksEnd()
--]]
