--detailsError(1)

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
   local sizes = { 1, 2, 4, 8 }

   for i,v in ipairs( sizes ) do
      local comp = component{ size={5,1,1}, id="box", position={0,0,2*(i-1)} }
      split( comp, "X", { id="left", rel=1 }, { id="right", rel=v } )
   end

   blocksBegin()
      for c in query( "left", "right" ) do
         componentBlocks( c, { "-X", "+X", "-Y", "-Z", "+Z" } )
      end
   blocksEnd()


   for i,v in ipairs( sizes ) do
      local comp = component{ size={1,5,1}, id="box", position={2*(i-1),0,-2} }
      split( comp, "Y", { id="bottom", rel=1 }, { id="top", rel=v } )
      local comp = component{ size={1,5,1}, id="box", position={-2*i,0,-2} }
      split( comp, "Y", { id="bottom", rel=v }, { id="top", rel=1 } )
   end

   blocksBegin()
      for c in query( "bottom", "top" ) do
         componentBlocks( c, { "-X", "+X", "-Y", "+Y", "-Z" } )
      end
   blocksEnd()


   for i,v in ipairs( sizes ) do
      local comp = component{ size={1,1,5}, id="box", position={-2*i,0,0} }
      split( comp, "Z", { id="back", rel=1 }, { id="front", rel=v } )
   end

   blocksBegin()
      for c in query( "back", "front" ) do
         componentBlocks( c, { "-X", "+X", "-Y", "-Z", "+Z" } )
      end
   blocksEnd()
compositeEnd()
