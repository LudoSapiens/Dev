local p = ... or {}
local r = p.radius

blocksBegin()
   block{
      {-r,-r,-r},
      { r,-r,-r},
      {-r, r,-r},
      { r, r,-r},
      {-r,-r, r},
      { r,-r, r},
      {-r, r, r},
      { r, r, r},
      m=0, g=0, c=0x0, s=0, id=1
   }
blocksEnd()
