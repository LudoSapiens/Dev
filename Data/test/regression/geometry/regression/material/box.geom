local params = ... or {}
--if params.detailsError then detailsError( params.detailsError ) end
local s = params.size
local m = params.mat

blocksBegin()
   block{ s, c=0xFFF, id=m }
   collisionBox{ size=s }
blocksEnd()
