--==============================================================================
-- Simple tree
--==============================================================================

local branches = {}

--==============================================================================
function branches.trunk()
   local c = component{ id="trunk" }
   compositeBlocksBegin(c)
      attraction(0,0)
      for i = 1, 10 do
         local r = 0.5 + ((10-i)/9)*0.5
         block{  {-r,0,-r}, {r,1,r} }
         translate(0,1,0)
      end
   compositeBlocksEnd()
   return c
end

--==============================================================================
function branches.level1()
   local c = component{ id="level1", connectorPosition={0,0,0} }
   compositeBlocksBegin(c)
      attraction(0,1)
      attraction(1,1)
      for i = 1, 10 do
         local r = 0.25 + ((10-i)/9)*0.25
         block{  {-r,0,-r}, {r,0.75,r} }
         translate(0,0.75,0)
      end
   compositeBlocksEnd()
   return c
end

--==============================================================================
function branches.level2()
   local c = component{ id="level1", connectorPosition={0,0,0} }
   compositeBlocksBegin(c)
      attraction(1,2)
      attraction(2,2)
   compositeBlocksEnd()
   return c
end

--==============================================================================
function branches.level3()
   local c = component{ id="level1", connectorPosition={0,0,0} }
   compositeBlocksBegin(c)
      attraction(2,3)
      attraction(3,3)
   compositeBlocksEnd()
   return c
end

--==============================================================================
-- Creation
--==============================================================================

local names = { "level1", "level2", "level3" }

compositeBegin()
   branches.trunk()
   for i = 1, #names do
      for r in rquery( names[i] ) do
         connect( branches[r.id](), r )
      end
   end
compositeEnd()
