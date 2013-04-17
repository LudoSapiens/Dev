--[[
displacement(
   function( IN )
      local p = IN.pos

      --return p + IN.n * (perlin1( p*2 )*0.2) * fract(p.x*2)
      --return p + IN.n * (perlin1( p*2 )*0.2)
      --return p + IN.n * tex( "checker", p.xy*0.5 ).x*0.1
      --return p + IN.n * tex( "rockbump", p.xy*0.5 ).x*-0.1
      --return p + IN.n * tex( "terrain/yosemite_hm", (p.xy+vec2(0,1))*0.5 ).x*0.4
      --if IN.f ~= 3 then return p end
      --return p + IN.n * tex( "test05", (p.xz+vec2(1,1))*0.5 ).x*0.2
      if IN.f ~= 3 and p.y < 0 then return p end
      return p + vec3(0,1,0) * tex( "test05", (p.xz+vec2(1,1))*0.5 ).x*0.2
      --return p + IN.n * (1-abs(p.z))*0.4
   end
)
--]]
--[[
mapping(
   function( IN )
      return vec2(0.5)
   end
)--]]
blocksBegin()
--[[
   block{
      {-4,-1,-1},
      {-2,-1,-1},
      {-4, 1,-1},
      {-2, 1,-1},
      {-4,-1, 1},
      {-2,-1, 1},
      {-4, 1, 1},
      {-2, 1, 1},
      m=0, g=0, c=0xf0f, s=0, id=1
   }
   block{
      {-1,-1,-1},
      { 1,-1,-1},
      {-1, 1,-1},
      { 1, 1,-1},
      {-1,-1, 1},
      { 1,-1, 1},
      {-1, 1, 1},
      { 1, 1, 1},
      m=0, g=0, c=0x0, s=0, id=2
   }
   block{
      {2,-1,-1},
      {4,-1,-1},
      {2, 1,-1},
      {4, 1,-1},
      {2,-1, 1},
      {4,-1, 1},
      {2, 1, 1},
      {4, 1, 1},
      m=0, g=0, c=0xfff, s=0,id=3
   }
   --]]
   block{
      {-1,-0.1,-1},
      { 1,-0.1,-1},
      {-1, 0.1,-1},
      { 1, 0.1,-1},
      {-1,-0.1, 1},
      { 1,-0.1, 1},
      {-1, 0.1, 1},
      { 1, 0.1, 1},
      m=0, g=0, c=0xfff, s=0, id=2
   }
blocksEnd()
