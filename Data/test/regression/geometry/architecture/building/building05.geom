local Mat = execute( "architecture/common_materials" )

--==============================================================================
-- Main volume
--==============================================================================

compositeBegin()
   component{ id="level", size={80,6,48}, position={0,0,12} }
   component{ id="level", size={80,6,48}, position={0,6,6} }
   component{ id="level", size={80,6,48}, position={0,12,0} }

   for c in query( "level" ) do
      component{ c, id="hall", size={80,3,8}, position={0,0,0} }
      component{ c, id="hall", size={80,3,8}, position={0,3,0} }
      component{ c, id="habitat", size={20,6,14}, position={30,0,28}, orientation={0.25,{0,1,0}} }
      component{ c, id="habitat", size={20,6,14}, position={60,0,28}, orientation={0.25,{0,1,0}} }
      component{ c, id="habitat", size={20,6,14}, position={0,0,28}, orientation={0.25,{0,1,0}} }
      component{ c, id="habitat2", size={24,6,18}, position={40,0,8} }
      component{ c, id="habitat2", size={24,6,18}, position={10,0,8} }
   end

   for c in query( "habitat" ) do
      component{ c, id="wing", size={20,3,14}, position={0,0,0} }
      component{ c, id="wing", size={20,3,14}, position={20,3,14}, orientation={0.5,{0,1,0}} }
   end

   for c in query( "habitat2" ) do
      component{ c, id="wing", size={20,3,14}, position={0,0,4} }
      component{ c, id="wing", size={20,3,14}, position={24,3,14}, orientation={0.5,{0,1,0}} }
   end

   for c in query( "wing" ) do
      component{ c, id="room", boundary={{0,0,2/14},{0,0,8/14},{14/20,0,8/14},{14/20,0,1},{1,0,1},{1,0,0},{14/20,0,0},{14/20,0,2/14} } }
   end

--==============================================================================
-- Interior
--==============================================================================

   -- Walls
   for c in query( "room", "hall" ) do
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
      extrude( s, -0.05, { id="iwall" } )
   end

--==============================================================================
-- Exterior
--==============================================================================

   -- Windows
   for c in query( "room" ) do
      for f in fquery( c, "S" ) do
         component{ c, id="facade", boundary=f, mat=Mat.EXT_WALL1 }
      end
   end

   for c in query( "facade" ) do
      if occlusion( c, "hall" ) > 0.1 then
         region{ c, id="door" }
      else
         region{ c, id="window" }
      end
   end

--==============================================================================
-- Geometry
--==============================================================================

   blocksBegin()
      for c in query( "iwall" ) do blocks{ c, id=c.mat } end
      for c in query( "ewall" ) do blocks{ c, id=c.mat } end
      for c in query( "eledge" ) do blocks{ c, id=c.mat } end
   blocksEnd()

--==============================================================================
-- Connections
--==============================================================================

   for r in rquery( "window" ) do
      local w = 1.3
      local s = r.component.size
      if s[1] < 14 then
         connect( execute( "architecture/window/window01", { w=w, h=1.6, d2=0.1, d3=0.16} ), r, {0.5,0.5,0}, {0,0,-0.1} )
      else
         connect( execute( "architecture/window/window01", { w=w, h=1.6, d2=0.1, d3=0.16} ), r, {0.3,0.5,0}, {0,0,-0.1} )
         connect( execute( "architecture/window/window01", { w=w, h=1.6, d2=0.1, d3=0.16} ), r, {0.7,0.5,0}, {0,0,-0.1} )
      end
   end

   -- Doors
   for r in rquery( "door" ) do
      local s = r.size
      local w = 0.9
      connect( execute( "architecture/door/frame02", {w=w,h=2,d=0.14} ), r, {0.5,0,0}, {0,0.05,-0.07} )
   end

compositeEnd()
