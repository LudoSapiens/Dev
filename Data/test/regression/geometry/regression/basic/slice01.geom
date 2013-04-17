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
   local sizes = { 1, 1.25, 1.5, 2 }

   for i,v in ipairs( sizes ) do
      local comp = component{ size={5,1,1}, id="box", position={0,0,2*(i-1)} }
      slice( comp, "X", { id="xslice", v } )
   end

   blocksBegin()
      for c in query( "xslice" ) do
         componentBlocks( c, { "-X", "+X", "-Y", "-Z", "+Z" } )
      end
   blocksEnd()


   for i,v in ipairs( sizes ) do
      local comp = component{ size={1,5,1}, id="box", position={2*(i-1),0,-2} }
      slice( comp, "Y", { id="yslice", v } )
      local comp = component{ size={1,5,1}, id="box", position={-2*i,0,-2} }
      slice( comp, "Y", { id="yslice", 2*v } )
   end

   blocksBegin()
      for c in query( "yslice" ) do
         componentBlocks( c, { "-X", "+X", "-Y", "+Y", "-Z" } )
      end
   blocksEnd()


   for i,v in ipairs( sizes ) do
      local comp = component{ size={1,1,5}, id="box", position={-2*i,0,0} }
      slice( comp, "Z", { id="zslice", v } )
   end

   blocksBegin()
      for c in query( "zslice" ) do
         componentBlocks( c, { "-X", "+X", "-Y", "-Z", "+Z" } )
      end
   blocksEnd()
compositeEnd()
