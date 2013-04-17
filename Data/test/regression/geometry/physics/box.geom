local params = ... or {}
--if params.detailsError then detailsError( params.detailsError ) end
local s = params.size
local c = params.color or 0
local colorToMatID = {
   black   = 1,
   blue    = 2,
   cyan    = 3,
   gray    = 4,
   green   = 5,
   magenta = 6,
   red     = 7,
   white   = 8,
   yellow  = 9,
}

local mat = colorToMatID[c] or c
blocksBegin()
   block{ s, c=0xFFF, id=mat }
   collisionBox{ size=s }
blocksEnd()
