detailsError(1)

--------------------------------------------------------------------------------
-- Utility routines
--------------------------------------------------------------------------------

local _compBlocksID = 1
local function componentBlocks( c, faces )
   local d = -0.1
   local s = {}
   for f in fquery( c, unpack(faces) ) do
      s[#s+1] = component{ c, boundary=f }
   end
   local name = "__componentBlocks" .. _compBlocksID
   _compBlocksID = _compBlocksID + 1
   extrude( s, d, { id=name } )
   for cb in query( name ) do
      blocks{ cb }
   end
end


--------------------------------------------------------------------------------
-- Structure
--------------------------------------------------------------------------------

compositeBegin()
   local comp = component{ size={1,5,1}, id="box", position={2,0,-2} }
   split( comp, "Y", { id="bottom", rel=1 }, { id="top", rel=4 } )

   local comp = component{ size={1,5,1}, id="box", position={-4,0,-2} }
   split( comp, "Y", { id="bottom", rel=4 }, { id="top", rel=1 } )

   blocksBegin()
      for c in query( "bottom", "top" ) do
         componentBlocks( c, { "-X", "+X", "-Y", "+Y", "-Z" } )
      end
   blocksEnd()
compositeEnd()
