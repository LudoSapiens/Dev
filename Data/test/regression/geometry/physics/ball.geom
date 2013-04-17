local params = ... or {}
if params.detailsError then detailsError( params.detailsError ) end
local r = params.radius or 0.5
local f = 0.5681818181 * 2 -- Special factor to get closest to a sphere.
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
blocksBegin()
   block{ r*f, c=0x000, id=colorToMatID[c] }
   collisionSphere{ radius=r }
blocksEnd()
