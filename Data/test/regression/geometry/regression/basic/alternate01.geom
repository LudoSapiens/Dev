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
   local sizes = {
      {   3  ,   1   }, -- A which doesn't even fit.
      {   2  ,   2   }, -- A+A because B doesn't fit.
      {   1  ,   2   }, -- A+B+A.
      {  5/4 ,  5/8  }, -- A+B+A+B+A      A twice as big as B.
      {  5/7 , 10/7  }, -- A+B+A+B+A      B twice as big as A.
      { 10/11,  5/11 }, -- A+B+A+B+A+B+A  A twice as big as B.
      {  5/10, 10/10 }, -- A+B+A+B+A+B+A  B twice as big as A.
   }

   for i,v in ipairs( sizes ) do
      local comp = component{ size={5,1,1}, id="box", position={0,0,2*(i-1)} }
      alternate( comp, "X", { id="xslice", type="A", v[1] }, { id="xslice", type="B", v[2] } )
   end

   blocksBegin()
      for c in query( "xslice" ) do
         componentBlocks( c, { "-X", "+X", "-Y", "-Z", "+Z" } )
      end
   blocksEnd()


   for i,v in ipairs( sizes ) do
      local comp = component{ size={1,5,1}, id="box", position={2*(i-1),0,-2} }
      alternate( comp, "Y", { id="yslice", type="A", v[1] }, { id="yslice", type="B", v[2] } )
      --local comp = component{ size={1,5,1}, id="box", position={-2*i,0,-2} }
      --alternate( comp, "Y", { id="yslice", type="A", 2*v[1] }, { id="yslice", type="B", 2*v[2] } )
   end

   blocksBegin()
      for c in query( "yslice" ) do
         componentBlocks( c, { "-X", "+X", "-Y", "+Y", "-Z" } )
      end
   blocksEnd()


   for i,v in ipairs( sizes ) do
      local comp = component{ size={1,1,5}, id="box", position={-2*i,0,0} }
      alternate( comp, "Z", { id="zslice", type="A", v[1] }, { id="zslice", type="B", v[2] } )
   end

   blocksBegin()
      for c in query( "zslice" ) do
         componentBlocks( c, { "-X", "+X", "-Y", "-Z", "+Z" } )
      end
   blocksEnd()

compositeEnd()
