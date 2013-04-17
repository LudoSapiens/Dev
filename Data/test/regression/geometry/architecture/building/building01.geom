local Mat = execute( "architecture/common_materials" )

--------------------------------------------------------------------------------
-- Building frame
--------------------------------------------------------------------------------

--==============================================================================
-- Main volume
--==============================================================================

compositeBegin()
   local s = {24,11,16}
   -- Main volume.
   component{ 
      id="main", size=s, 
      boundary={
         {0,0,0},{0,0,0.6},{0.3,0,0.8},{0.3,0,1},{0.7,0,1},{0.7,0,0.8},{1,0,0.8},{1,0,0},direction={0,1,0},
         id={"b","t","f","f","f","c","f","f","f","f"}
      }
   }
   local p = {1,s[2]-0.8,1}
   local s = {s[1]-2,s[2],s[3]*0.8}
   component{ 
      id="main", size=s, position=p,
      boundary={
         {0,0,0},{0,0,0.6},{0.3,0,0.8},{0.7,0,0.8},{1,0,0.8},{1,0,0},direction={0,1,0},
         id={"b","t","f","f","c","f","f","f"}
      }
   }

   -- Level.
   local levelCounter = counter()
   for c in query( "main" ) do
      split( c, "Y", { id="hull", rel=1 }, { id="roof", 0.8 } )
   end
   for c in query( "hull" ) do
      slice( c, "Y", { id="level", 2.5, level=levelCounter } )
   end
   for c in query( "roof" ) do
      c.level = levelCounter.current
   end

   local lastLevel = levelCounter.current-1

--==============================================================================
-- Interior
--==============================================================================

   -- Rooms.
   for c in query( "level" ) do
      split( c, "X", { id="room", rel=1 }, { id="right", rel=1 } )
   end
   for c in query( "right" ) do
      split( c, "Z", { id="room", rel=1 }, { id="room", rel=1 } )
   end

   -- Walls
   for c in query( "room" ) do
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

   -- Regions for window placement.
   for c in query( "room" ) do
      for f in fquery( c, "Z" ) do
         local fid = faceID( c, f )
         if fid and ((fid ~= "c") or (c.level ~= 0 and c.level ~= 4)) then
            region{ c, f, id="window" }
         end
      end
   end

--==============================================================================
-- Exterior
--==============================================================================

   for c in query( "level", "roof" ) do
      for f in fquery( c, "SIDE" ) do
         component{ c, id="lfacade", boundary=f }
      end
   end
   for c in query( "lfacade" ) do
      split( c, "Y", { id="wallFacade", rel=1 }, { id="ledge1", 0.1 }, { id="ledge2", 0.1 } )
   end
   for c in query( "wallFacade" ) do
      if c.level == 0 then
         split( c, "X", { id="wallFacade1", 0.3 }, { id="wallFacade2", rel=1 }, { id="wallFacade1", 0.3 } )
      elseif c.level < levelCounter.current then
         split( c, "X", { id="wallFacade1", 0.3 }, { id="wallFacade2", rel=0.4 }, { id="wallFacade1", rel=1 }, { id="wallFacade2", rel=0.4 }, { id="wallFacade1", 0.3 } )
      else
         split( c, "X", { id="wallFacade1", 0.3 }, { id="wallFacade2", rel=1 },  { id="wallFacade1", 0.3 } )
      end
   end

   -- Regions for door placement.
   for c in query( "lfacade" ) do
      local fid = faceID( c )
      if fid == "c" and (c.level == 0 or c.level == 4) then
         region{ c, id="door" }
      end
   end

   local s = {}
   for c in query( "wallFacade2" ) do
      s[#s+1] = c
   end
   extrude( s, 0.15, { id="ewall", mat=Mat.EXT_WALL1 } )

   local s = {}
   for c in query( "wallFacade1" ) do
      s[#s+1] = c
   end
   extrude( s, 0.1, { id="ewall", mat=Mat.EXT_WALL2 } )


   local s = {}
   for c in query( "ledge1" ) do
      s[#s+1] = c
   end
   extrude( s, 0.2, { id="eledge", mat=Mat.EXT_WALL3 } )

   local s = {}
   for c in query( "ledge2" ) do
      s[#s+1] = c
   end
   extrude( s, 0.3, { id="eledge", mat=Mat.EXT_WALL4 } )

--==============================================================================
-- Geometry
--==============================================================================

   -- Geometry.
   blocksBegin()
      for c in query( "iwall" ) do blocks{ c, id=c.mat } end
      for c in query( "ewall" ) do blocks{ c, id=c.mat } end
      for c in query( "eledge" ) do blocks{ c, id=c.mat } end
   blocksEnd()

--==============================================================================
-- Connections
--==============================================================================

   local m = { Mat.WINDOW1, Mat.WINDOW2, Mat.WINDOW3 }
   for r in rquery( "window" ) do
      if r.component.level == lastLevel then
         connect( execute( "architecture/window/window01", { mat=m, w=1.6, h=1.6, d2=0.3,d3=0.26} ), r, {0.5,0.5,0}, {0,-0.2,-0.1} )
      else
         connect( execute( "architecture/window/window01", { mat=m, w=1, h=1.6, d2=0.3,d3=0.26} ), r, {0.5,0.5,0}, {0,0,-0.1} )
      end
   end

   local m = { Mat.DOOR1, Mat.DOORFRAME1, Mat.DOORKNOB1 }
   for r in rquery( "door" ) do
      connect( execute( "architecture/door/set01", {mat=m,frame={d=0.3,jamb=0.1}} ), r, {0.5,0,0}, { -0.5,0.1,-0.15}, {{0,1,0}, 0.0} )
      connect( execute( "architecture/door/set01", {mat=m,frame={d=0.3,jamb=0.1}} ), r, {0.5,0,0}, {  0.5,0.1,0.15},  {{0,1,0}, 0.5} )
   end

compositeEnd()
