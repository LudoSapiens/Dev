--detailsError( 1.0 )
local w = 15
local h = 15
local d = 2
local hd = h + d
local wd = w + d
blocksBegin()

   -- Floor.
   local s = {w-d, d, h-d}
   translate( 0, -d, 0 )
   block{ s, c=0xFFF }
   collisionBox{ size=s }
   translate( 0, d, 0 )

   --translate( 0, s[2], 0 )

   -- Front and back walls.
   local s = {w,2*d,d}
   translate( 0, 0, -h )
   block{ s, c=0xFFF }
   collisionBox{ size=s }
   translate( 0, 0, 2*h )
   block{ s, c=0xFFF }
   collisionBox{ size=s }
   translate( 0, 0, -h )

   -- Left and right walls.
   local s = {d,2*d,h-d} -- Remove the -d to reproduce crash.
   translate( -w, 0, 0 )
   block{ s, c=0xFFF }
   collisionBox{ size=s }
   translate( 2*w, 0, 0 )
   block{ s, c=0xFFF }
   collisionBox{ size=s }
blocksEnd()
