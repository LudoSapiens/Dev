local Mat = execute( "architecture/common_materials" )

--==============================================================================
-- Main volume
--==============================================================================

compositeBegin()
   component{ 
      --id="main", size={20,27,20},
      id="main", size={20,17,20},
      boundary={
         {0,0,0},{0,0,1},{1,0,1},{1,0,0},direction={0,1,0},
         id={"b","t","s","f","s","r"}
      }
   }

   local levelCounter = counter()
   for c in query( "main" ) do
      split( c, "Y", { id="hull", rel=1 }, { id="roof", 1 } )
   end
   for c in query( "hull" ) do
      slice( c, "Y", { id="level", 3.5, level=levelCounter }, -1 )
   end
   for c in query( "roof" ) do
      c.level = levelCounter.current
   end

   local lastLevel = levelCounter.current-1

--==============================================================================
-- Interior
--==============================================================================

   -- Staircase.
   local sc = component{ id="staircase", size={4,19,4}, position={10,0,0} }

   -- Split staircase by level.
   local cs = {}
   for c in query( "level" ) do
      for f in fquery( c, "T" ) do
         cs[#cs+1] = planeConstraint{ c, f }
      end
   end
   for c in query( "staircase" ) do
      slice( c, "Y", { id="staircaseLevel", level=counter() }, cs )
   end

   local m = { Mat.FLOOR1, Mat.WALL1 }
   for c in query( "staircase" ) do
      execute( "architecture/stair/staircase01", {mat=m, c, ldd=1,ew=0.2,sth=1} )
   end

   --doors
   for c in query( "staircaseLevel" ) do
      for f in fquery( c, "Z" ) do
         region{ c, f, id="door" }
      end
   end

   -- Wall, floor and ceilling.
   for c in query( "level" ) do
      subtract( c, sc, { id="room" } )
   end

   for c in query( "room", "staircase" ) do
   --for c in query( "room" ) do
      local s = {}
      for f in fquery( c, "SIDE" )  do
         s[#s+1] = component{ c, id="wall", boundary=f, mat=Mat.WALL1 }
      end
      for f in fquery( c, "B" )  do
         s[#s+1] = component{ c, id="floor", boundary=f, mat=Mat.FLOOR1 }
      end
      for f in fquery( c, "T" )  do
         s[#s+1] = component{ c, id="ceiling", boundary=f, mat=Mat.CEILING1 }
      end
      extrude( s, -0.1, { id="iwall" } )
   end

--==============================================================================
-- Exterior
--==============================================================================

   -- Roof
   for c in query( "roof" ) do
      for f in fquery( c, "SIDE" ) do
         component{ c, id="rfacade", boundary=f }
      end
   end

   for c in query( "rfacade" ) do
      split( c, "Y", { id="rA", rel=1 }, { id="ledge1", 0.2 }, { id="ledge2", 0.3 } )
   end

   local s={}
   for c in query( "rA" ) do s[#s+1] = c end
   extrude( s, 0.1, { id="ewall", mat=Mat.EXT_DETAIL1 } )

   -- Main facade
   for c in query( "level" ) do
      for f in fquery( c, "f", "s" ) do
         component{ c, id="facade", boundary=f }
      end
   end

   for c in query( "facade" ) do
      split( c, "X", { id="fA", 1 }, { id="fB", rel=1} )
   end
   for c in query( "fB" ) do
      slice( c, "X", { id="fC", 6 }, 2 )
   end
   for c in query( "fC" ) do
      split( c, "X", { id="fD", rel=1 }, { id="fA", 1 } )
   end
   for c in query( "fD" ) do
      split( c, "Y", { id="fE", rel=1 }, { id="fF", 0.8 } )
   end
   for c in query( "fF" ) do
      split( c, "Y", { id="ledge1", 0.1 }, { id="fG", rel=1 }, { id="ledge1", 0.1 }, { id="ledge2", 0.2 } )
   end
   for c in query( "fA" ) do
      split( c, "Y", { id="fAA", 0.3 }, { id="fAB", rel=1 } )
   end

   local s={}
   for c in query( "fAB" ) do s[#s+1] = c end
   extrude( s, 0.4, { id="ewall", mat=Mat.EXT_WALL1 } )

   local s={}
   for c in query( "fE" ) do s[#s+1] = c end
   extrude( s, 0.1, { id="ewall", mat=Mat.EXT_WALL1 } )

   local s={}
   for c in query( "fG" ) do s[#s+1] = c end
   extrude( s, 0.2, { id="ewall", mat=Mat.EXT_WALL2 } )

   local s={}
   for c in query( "ledge1" ) do s[#s+1] = c end
   extrude( s, 0.3, { id="ewall", mat=Mat.EXT_DETAIL1 } )

   local s={}
   for c in query( "ledge2" ) do s[#s+1] = c end
   extrude( s, 0.5, { id="ewall", mat=Mat.EXT_DETAIL1 } )

   local s={}
   for c in query( "fAA" ) do s[#s+1] = c end
   extrude( s, 0.5, { id={"ewall"} } )

   -- Window region
   for c in query( "fE" ) do
      region{ c, id="window" }
   end

--==============================================================================
-- Geometry
--==============================================================================

   blocksBegin()
      for c in query( "iwall" ) do blocks{ c, id=c.mat } end
      for c in query( "ewall" ) do blocks{ c, id=c.mat } end
   blocksEnd()

--==============================================================================
-- Connections
--==============================================================================

   local m  = { Mat.WINDOW1, Mat.WINDOW2, Mat.WINDOW2 }
   local mF = { Mat.EXT_WALL4, Mat.EXT_WALL4, Mat.WINDOW2 }
   for r in rquery( "window" ) do
      if (r.component.level > 0) and (r.component.level < lastLevel) then
         connect( execute( "architecture/window/window01", { mat=m, w=2.2, h=2.5, d2=0.2, d3=0.2 } ), r, {0.0,0.5,0}, {1.1,0,-0.1} )
         connect( execute( "architecture/window/window01", { mat=m, w=2.2, h=2.5, d2=0.2, d3=0.2 } ), r, {1.0,0.5,0}, {-1.1,0,-0.1} )
      elseif r.component.level == lastLevel then
         connect( execute( "architecture/window/window02", { mat=m, w=1.4, h=2, d=0.2, d2=0.1 } ), r, {0.0,0.5,0}, {0.7,0,-0.1} )
         connect( execute( "architecture/window/window02", { mat=m, w=1.4, h=2, d=0.2, d2=0.1 } ), r, {0.5,0.5,0}, {0,0,-0.1} )
         connect( execute( "architecture/window/window02", { mat=m, w=1.4, h=2, d=0.2, d2=0.1 } ), r, {1.0,0.5,0}, {-0.7,0,-0.1} )
      else
         connect( execute( "architecture/window/window02", { mat=mF, w=4, h=3, d=0.2, d2=0.3, t=0.4, f=0.1 } ), r, {0.5,0.0,0}, {0,1.5,-0.1} )
      end
   end

   local m = nil
   for r in rquery( "door" ) do
      connect( execute( "architecture/door/frame02", {mat=m,w=2,h=2,d=0.24} ), r, {0.5,0,0}, {0,0.1,-0.12} )
   end

compositeEnd()
