
displacement(
   function( IN )
      local p = IN.pos

      --return p + IN.n * tex( "terrain/yosemite_hm", (p.xy+vec2(0,1))*0.5 ).x*0.4
      --if IN.f ~= 3 then return p end
      if IN.f ~= 3 and p.y < 0.05 then return p end
      return p + IN.n * tex( "terrain/yosemite_hm", (p.xz+vec2(1,1))*0.5 ).x*0.5
      --return p + IN.n * (1-abs(p.z))*0.4
   end
)
--[[
mapping(
   function( IN )
      return vec2(0.5)
   end
)--]]
blocksBegin()
--[[
   block{
      {-1,-0.1,-1},
      { 1,-0.1,-1},
      {-1, 0.1,-1},
      { 1, 0.1,-1},
      {-1,-0.1, 1},
      { 1,-0.1, 1},
      {-1, 0.1, 1},
      { 1, 0.1, 1},
      m=0, g=0, c=0x0, s=0, id=2
   }
   --]]
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

blocksEnd()
