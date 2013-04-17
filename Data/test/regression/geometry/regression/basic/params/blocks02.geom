blocksBegin()
   local n = 32
   translate( -n*2-1, 0, 0 )
   for i=0,n-1 do
      translate( 4, 0, 0 )
      local q = quat( vec3(0, 1, 0), i/n )
      local a = vec3( -1, -2, -1 )
      local b = vec3(  1, -2, -1 )
      local c = q * vec3( -1,  2, -1 )
      local d = q * vec3(  1,  2, -1 )
      local e = vec3( -1, -2,  1 )
      local f = vec3(  1, -2,  1 )
      local g = q * vec3( -1,  2,  1 )
      local h = q * vec3(  1,  2,  1 )
      block{
         a, b, c, d, e, f, g, h,
         c = 0xFFF,
      }
   end
blocksEnd()
