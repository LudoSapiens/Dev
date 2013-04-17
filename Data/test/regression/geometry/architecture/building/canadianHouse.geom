local Mat = execute( "architecture/common_materials" )

--geometricError(0.04)
--detailsError(1)

--------------------------------------------------------------------------------
-- Building frame
--------------------------------------------------------------------------------

--==============================================================================
-- Main volume
--==============================================================================

compositeBegin()
   component{ id="main", size={14,3,10}, rf=5 }
   component{ id="main", size={10,3,8}, position={14,0,0}, rf=4 }

   for c in query( "main" ) do
      split( c, "Z", { id="waste", 0.5 }, { id="level", rel=1 }, { id="waste", 1 } )
   end

   -- Lucarne
   component{ id="lucarne", size={1.6,2,3}, position={2,3.5,6} }
   component{ id="lucarne", size={1.6,2,3}, position={10.4,3.5,6} }
   component{ id="lucarne", size={1.6,2,3}, position={18.2,3.5,4} }

--==============================================================================
-- Roof
--==============================================================================

   for c in query( "main" ) do
      for f in fquery( c, "T" ) do
         component{ c, id="ceiling", boundary=f }
      end
   end

   for c in query( "ceiling" ) do
      roof( c, {1,0,c.rf}, { id="roof" } )
   end

   for c in query( "lucarne" ) do
      for f in fquery( c, "T" ) do
         component{ c, id="lceiling", boundary=f }
      end
   end

   for c in query( "lceiling" ) do
      roof( c, {0,1,1}, { id="lroof" } )
   end
   --[[
   local s = {}
   for c in query( "lroof" ) do
      for f in fquery( c ) do
         s[#s+1] = component{ c, id="roofside", boundary=f }
      end
      extrude( s, -0.1, { id="eroof" } )
   end
   for c in query( "roof" ) do
      local s = {}
      for f in fquery( c ) do
         s[#s+1] = component{ c, id="roofside", boundary=f }
      end
      extrude( s, -0.1, { id="eroof" } )
   end
   --]]

   for c in query( "roof" ) do
      split( c, "Y", { id="iroof", rel=1 }, { id="wroof", 1 } )
   end

   local s={}
   for c in query( "lucarne", "iroof", "lroof" ) do
      s[#s+1] = c
   end
   merge( s, { id="iroof_final" } )

   for c in query( "iroof_final", "wroof" ) do
      local s = {}
      for f in fquery( c ) do
         s[#s+1] = component{ c, id="wall", boundary=f }
      end
      extrude( s, -0.1, { id="iwall" } )
   end

--==============================================================================
-- Interior
--==============================================================================

   for c in query( "level" ) do
      local s = {}
      for f in fquery( c, "SIDE" ) do
         s[#s+1] = component{ c, id="wall", boundary=f, mat=Mat.WALL1 }
      end
      for f in fquery( c, "B" ) do
         s[#s+1] = component{ c, id="wall", boundary=f, mat=Mat.FLOOR1 }
      end
      for f in fquery( c, "T" ) do
         s[#s+1] = component{ c, id="wall", boundary=f, mat=Mat.CEILING1 }
      end
      extrude( s, -0.1, { id="iwall" } )
   end

--==============================================================================
-- Exterior
--==============================================================================

   for c in query( "lucarne" ) do
      for f in fquery( c, "Z" ) do
         region{ c, f, id="window" }
      end
   end

--==============================================================================
-- Geometry
--==============================================================================

   blocksBegin()
      for c in query( "iwall" ) do blocks{ c, id=c.mat } end
      for c in query( "ewall" ) do blocks{ c, id=c.mat } end
      for c in query( "eroof" ) do blocks{ c, id=c.mat } end
   blocksEnd()

--==============================================================================
-- Connections
--==============================================================================

   for r in rquery( "window" ) do
      connect( execute( "architecture/window/window01", { w=1, h=1.2, d2=0.1,d3=0.1} ), r, {0.5,0.5,0}, {0,0.4,-0.1} )
   end

compositeEnd()
