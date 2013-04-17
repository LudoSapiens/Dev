detailsError(1)

--------------------------------------------------------------------------------
-- Utility routines
--------------------------------------------------------------------------------

local _compBlocksID = 1
local function componentBlocks( c, faces )
   local d = -0.01
   local s = {}
   for f in fquery( c, unpack(faces) ) do
      s[#s+1] = component{ c, boundary=f }
   end
   local name = "__componentBlocks" .. _compBlocksID
   _compBlocksID = _compBlocksID + 1
   extrude( s, d, { id=name } )
   for cb in query( name ) do
      blocks{ cb, id=cb.mat }
   end
end


--------------------------------------------------------------------------------
-- Structure
--------------------------------------------------------------------------------

compositeBegin()
   local x = 3

   local b  = { {0,0,0}, {0,0,2}, {1,0,3}, {2,0,2}, {2,0,0} }
   --local c0 = component{ boundary=b, id="base"                          }
   if c0 then x = x + 2 end
   local c1 = component{ boundary=b, id="separate", position={ x, 0, 0} }
   local c2 = component{ boundary=b, id="merged"  , position={-x, 0, 0} }

   -- Extrude c0 just a little.
   if c0 then
      local t = {}
      for f in fquery( c0, "SIDE" ) do
         local i = #t+1
         t[i] = component{ c0, id="face", boundary=f, mat=i }
      end
      extrude( t, 0.03, { id="side" } )
   end

   -- Extrude c1 separately.
   local i = 1
   for f in fquery( c1, "SIDE" ) do
      local s = component{ c1, id="face", boundary=f }
      extrude( s, 0.5, { id="side", mat=i } )
      i = i + 1
   end

   -- Extrude c2 together.
   local t = {}
   for f in fquery( c2, "SIDE" ) do
      local i = #t+1
      t[i] = component{ c2, id="face", boundary=f, mat=i }
   end
   extrude( t, 0.5, { id="side" } )

   blocksBegin()
      for c in query( "base", "merged", "separate" ) do
         componentBlocks( c, { "BOTTOM" } )
      end
      for c in query( "side" ) do
         componentBlocks( c, { "TOP", "SIDE" } )
      end
   blocksEnd()
compositeEnd()
