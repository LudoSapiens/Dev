geometricError(0.04)
--detailsError(0.02)
detailsError(1)
--detailsError(0.02)

local levelH = 3.5
local mainH  = 7
--==============================================================================
-- Frame
--==============================================================================

compositeBegin()
   -- Main volume.
   component{
      id={"A","wing"}, size={40,mainH,50}, position={0,0,0}, 
      boundary={{0,0,0},{0,0,1},{0.4,0,1},{0.4,0,0.95},{0.6,0,0.95},{0.6,0,1},{1,0,1},{1,0,0},
         id={"b","t","s","f","s","c","s","f","s","s"}
      }
   }

   component{ c, id={"B","wing"}, position={40,0,10}, size={20,mainH+levelH,30},
      --boundary={{0,0,0},{0,0,1},{1,0,1},{1,0,0} } --, id={"b","t","f","f","s","s"}}
   }
   --component{ c, id={"C","wing"}, position={60,0,5}, orientation={-0.1,{0,1,0}}, size={30,mainH,60},
   component{ c, id={"C","wing"}, position={60,0,10}, orientation={-0.05,{0,1,0}}, size={30,mainH,60},
   --component{ c, id={"C","wing"}, position={70,0,0}, orientation={-0.05,{0,1,0}}, size={30,mainH,60},
   --component{ c, id={"C","wing"}, position={60,0,0}, orientation={0,{0,1,0}}, size={30,mainH,60},
      boundary={{0,0,0},{0,0,1},{0.8,0,1},{0.8,0,0.8},{1,0,0.8},{1,0,0},
         id={"b","t", "s","f","s","s"}
      }
   }

--==============================================================================
-- Exterior
--==============================================================================

   local s={}
   for c in query( "wing" ) do s[#s+1] = c end
   merge( s, { id="hull" } )

   local s={}
   for c in query( "hull" ) do
      --for f in fquery( c, "T" ) do s[#s+1] = component{ c, id="roof", boundary=f } end
      for f in fquery( c, "S" ) do component{ c, id="facade", boundary=f } end
   end
   extrude( s, 0.2, { id="ewall" } )

   for c in query( "facade" ) do
      slice( c, "Y", { id="facadeLevel", levelH, level=counter() }, 1 )
   end

   for c in query( "facadeLevel" ) do split( c, "X", { id="fA",2 }, { id="fC", rel=1 } ) end
   for c in query( "fC" ) do slice( c, "X", { id="fD", 3.5 }, 2 ) end
   for c in query( "fD" ) do split( c, "X", { id="fB", rel=1 }, { id="fA", 2 } ) end
   for c in query( "fA" ) do split( c, "Y", { id="fA2", rel=1 }, { id="fA1", 0.1 }, { id="fA3", 0.5 }, { id="fA1", 0.1 } ) end

   local s={}
   for c in query( "fA2", "fA3" ) do s[#s+1] = c end
   extrude( s, 0.2, { id="ewall" } )

   local s={}
   for c in query( "fA1" ) do s[#s+1] = c end
   extrude( s, 0.3, { id="ewall" } )

   local s={}
   for c in query( "fB" ) do s[#s+1] = c end
   extrude( s, 0.4, { id="ewall" } )

   -- Windows.
   for c in query( "fA2" ) do 
      local f = faceID(c)
      if f == "f" or f == "c"  then
         region{ c, id="window" }
      end
   end

   -- Doors.
   for c in query( "facadeLevel" ) do
      if c.level == 0 and faceID(c) == "c" then
         region{ c, id="extdoor" }
      end
   end

   -- Connections.
   for r in rquery( "extdoor" ) do
      connect( execute( "architecture/door/frame02", {w=2.5,h=2.2,d=0.5, jamb=0.2} ), r, {0.5,0,0}, {0,0.05,-0.07} )
   end
   for r in rquery( "window" ) do
      local s=r.component.size
      connect( execute( "architecture/window/window03", { w=s[1], h=1.6, d2=0.3,d3=0.26 } ), r, {0.5,0.5,0},{0,0,-0.05} )
   end

--==============================================================================
-- Interior
--==============================================================================

   local wcs={}
   for c in query( "window" ) do
      wcs[#wcs+1] = volumeConstraint{ c, offset={0.2,0,0}, repulse=true } 
   end

--------------------------------------------------------------------------------
-- Main partitionning.
   for c in query( "A" ) do
      slice( c, "Y", { id={"level","levelA"}, levelH, level=counter() }, 1 )
   end

   for c in query( "levelA" ) do
      split( c, "Z", { id="spaceA", rel=2}, { id="mainHall", 5 }, { id="spaceB", rel=1 } )
   end
   for c in query( "spaceA" ) do
      split( c, "X", { id={"spaceA1","room_space"}, rel=1}, { id="secHall", 3 }, { id="spaceA2", 30 } )
   end
   for c in query( "spaceA2" ) do
      split( c, "Z", { id="gym_space", 23 }, { id="spaceA22", rel=1 } )
   end
   for c in query( "spaceA22" ) do
      if c.level == 0 then
         split( c, "X", { id={"looker_room","room_space"}, rel=1 }, 
                        { id={"looker_room","room_space"}, rel=1 }, 
                        { id={"looker_room","room_space"}, rel=1 } )
      else
         component{ c, id="room_space" }
      end
   end
   for c in query( "spaceA1" ) do slice( c, "Z", { id="class_room", 9, entrance=0 } ) end

   local s={}
   for c in query( "gym_space" ) do s[#s+1]=c end
   merge( s, { id={"gym","room_space"} } )

   for c in query( "spaceB" ) do
      if c.level == 0 then
         split( c, "X", { id={"spaceB","room_space"}, rel=1 }, { id="mainEntrance", 5 }, { id={"spaceB","room_space"}, rel=1 }, wcs )
      else
         slice( c, "X", { id="class_room", 8, entrance=counter() }, 2, wcs )
      end
   end

   for c in query( "spaceC" ) do slice( c, "X", { id="class_room", 8 }, 2 ) end


   -- Stairwell.
   component{ id={"stairwell","room_space"}, size={8,7,4}, position={16,0,26},
      importance=10, type=0
   }
--------------------------------------------------------------------------------
-- Wing B partitionning.
   for c in query( "B" ) do
      slice( c, "Y", { id={"level","levelB"}, levelH, level=counter() }, 1 )
   end

   local cs={}
   for c in query( "mainHall" ) do
      for f in fquery( c, "X" ) do cs[#cs+1] = faceConstraint{ c, f } end
   end
   for c in query( "levelB" ) do
      split( c, "Z", { id="audi_space", 0, importance=6 }, { id="mainHall", rel=1 }, { id="room_space", 0, importance=6 }, cs )
   end
   for c in query( "audi_space" ) do
      component{ c, id="room_space" }
      component{ c, id="room_space", boundary={{0.2,0,0},{0.2,0,0.8},{0.4,0,1},{0.6,0,1},{0.8,0,0.8},{0.8,0,0}}, importance=7 }
   end

   -- stairwell.
   for c in query( "B" ) do
      component{ c,
         id={"stairwell","room_space"}, size={5,c.size[2],5}, position={15,0,25}, orientation=5, importance=10, type=1
      }
   end

--------------------------------------------------------------------------------
-- Wing C partitionning.
   for c in query( "C" ) do
      slice( c, "Y", { id={"level","levelC"}, levelH, level=counter() }, 1 )
   end

   local cs={}
   for c in query( "mainHall" ) do
      if hasParentID( c, "B" ) then
         for f in fquery( c, "X" ) do cs[#cs+1] = faceConstraint{ c, f } end
      end
   end

   for c in query( "levelC" ) do
      split( c, "Z", { id="spaceCb", 0 }, { id="mainHall", rel=1 }, { id="spaceCf", 0 }, cs )
   end

   for c in query( "spaceCb" ) do
      split( c, "X", { id="spaceCb1", rel=1 }, { id="secHall", 3 } )
   end

   for c in query( "spaceCb1" ) do
      split( c, "Z", { id="spaceCb2", rel=1 }, { id="secHall", 2 }, { id="lab_space1", rel=1 } )
   end
   for c in query( "spaceCb2" ) do
      split( c, "X", { id="room_space", rel=1 }, { id="secHall", 2 }, { id="lab_space2", rel=1 } )
   end

   for c in query( "spaceCf" ) do
      split( c, "Z", { id="spaceCf1", rel=1 }, { id="secHall", 3 } )
   end
   for c in query( "spaceCf1" ) do
      split( c, "X", { id="lab_space1", orientation=4, rel=1 }, { id="secHall", 3 }, { id="lab_space3", orientation=1, rel=1 } )
   end

   for c in query( "lab_space2", "lab_space3" ) do
      component{ c, id="room_space" }
   end

   for c in query( "lab_space1" ) do 
      slice( c, "X", { id="room_space", 7 }, 2 )
      component{ c, id="lab_space1_1", boundary={{0.2,0,0.5},{0.2,0,1},{0.8,0,1},{0.8,0,0.5}}, importance=2 }
   end
   for c in query( "lab_space1_1" ) do
      slice( c, "X", { id={"room_space","lab1"}, 5 }, 0 )
   end

   for c in query( "lab_space3" ) do 
      slice( c, "X", { id="room_space", 6 }, 2 )
      component{ c, id="lab_space1_3", boundary={{0,0,0.7},{0,0,1},{0.8,0,1},{0.8,0,0.7}}, importance=2 }
   end
   for c in query( "lab_space1_3" ) do
      slice( c, "X", { id={"room_space","lab1"}, 4 }, 0 )
   end

   for c in query( "lab1" ) do
      for f in fquery( c, "Z" ) do region{ c, f, id="labwindow" } end
   end

   -- stairwell.
   for c in query( "C" ) do
      local s=c.size
      component{ c,
         id={"stairwell","room_space"}, size={5,s[2],5}, position={0,0,s[3]-8}, importance=10, type=1
      }
      component{ c,
         id={"stairwell","room_space"}, size={5,s[2],5}, position={s[1]-8,0,0}, orientation=4, importance=10, type=1
      }
   end

--------------------------------------------------------------------------------
-- class room.
   for c in query( "class_room" ) do
      split( c, "Z", { id={"cloak_room","room_space"}, 2 }, { id={"class","room_space"}, rel=1 } )
      local s=c.size
      component{ c, id="room_space", position={0,2.7,0}, size={s[1],s[2]-2.7,4}, importance=2 }
      if mod( c.entrance, 2 ) == 0 then
         component{ c, id="entrance", position={s[1]-1.5,0,0}, size={1.5,s[2]-0.5,3} }
      else
         component{ c, id="entrance", position={0,0,0}, size={1.5,s[2]-0.5,3} }
      end
   end
   for c in query( "entrance" ) do
      for f in fquery( c, "Z" ) do region{ c, f, id="door" } end
   end
   for c in query( "cloak_room" ) do
      if mod( c.entrance, 2 ) == 0 then
         for f in fquery( c, "Z" ) do region{ c, f, id="door", rel={{0,0},{0,1}}, abs={{2,0},{0,0}} } end
      else
         for f in fquery( c, "Z" ) do region{ c, f, id="door", rel={{1,0},{1,1}}, abs={{-2,0},{0,0}} } end
      end
   end

--------------------------------------------------------------------------------
-- Main hall column.
   for c in query( "mainHall" )    do slice( c, "X", { id="hall_space", 7, num=counter() }, 2 ) end
   for c in query( "hall_space" )  do split( c, "X", { id="empty", rel=1 }, { id="hall_space2", 0.5 }, { id="empty", rel=1 } ) end
   for c in query( "hall_space2" ) do split( c, "Z", { id="hall_column", 0.2 }, { id="hall_space3", rel=1 }, { id="hall_column", 0.2 } ) end
   for c in query( "hall_space3" ) do split( c, "Y", { id="empty", rel=1 }, { id="hall_column", 0.5 } ) end

   for c in query( "hall_column" ) do
      if c.num ~= 2 then c.importance = 7 end
   end

--------------------------------------------------------------------------------
-- Merge hall together.
   local s = {}
   for c in query( c, "mainHall", "secHall", "mainEntrance", "entrance" ) do
      local ls   = s[c.level] or {}
      s[c.level] = ls
      ls[#ls+1]  = c
   end
   for k, v in pairs( s ) do
      merge( v, { id="hall_space", importance=5 } )
   end

-- Stairwell interior.
   local cs={}
   for c in query( "level" ) do
      for f in fquery( c, "T" ) do cs[#cs+1] = planeConstraint{ c, f } end
   end
   for c in query( "stairwell" ) do
      slice( c, "Y", { id="staircaseLevel", level=counter() }, cs )
   end
   for c in query( "stairwell" ) do 
      if c.type == 0 then
         execute( "architecture/stair/staircase04", {c, ldd=1, sth=1} )
      else
         execute( "architecture/stair/staircase01", {c, ldd=1.5, ew=0.2, sth=1} )
      end
   end

   -- Importance.
   for c in query( "room_space" ) do subtract( c, "importance", { id="room" } ) end
   for c in query( "hall_space" ) do subtract( c, "importance", { id="hall" } ) end

   -- Walls.
   for c in query( "room", "hall" ) do
      local s={}
      for f in fquery( c, "S" ) do
         s[#s+1] = component{ c, id="wall", boundary=f }
      end
      for f in fquery( c, "B" ) do s[#s+1] = component{ c, id="ceiling2d", boundary=f } end
      --for f in fquery( c, "T" ) do s[#s+1] = component{ c, id="floor2d", boundary=f } end
      extrude( s, -0.05, { id="iwall" } )
   end

   for c in query( "staircaseLevel" ) do
      for f in fquery( c, "Z" ) do region{ c, f, id="frame" } end
   end

   for c in query( "wall" ) do
      if hasParentID( c, "looker_room" ) then
         if occlusion( c, "mainHall" ) > 0.01 or occlusion( c, "gym_space" ) > 0.01 then
            region{ c, id="door" }
         end
      end
   end
   ---[[
   for c in query( "wall" ) do
      if hasParentID( c, "room" ) then
         split( c, "Y", { id="empty", 0.5 }, { id="molding", 0.12 }, { id="emptyw", rel=1 }, { id="molding", 0.12 }, { id="empty", 0.05 } )
      end
      if hasParentID( c, "hall" ) then
         split( c, "Y", { id="empty", 0.5 }, { id="molding", 0.12 }, { id="emptyw", rel=1 }, { id="molding", 0.12 }, { id="empty", 0.05 } )
      end
   end

   for c in query( "room", "hall" ) do
      local s={}
      for c2 in query( c, "molding" ) do s[#s+1] = c2 end
      extrude( s, -0.07, { id="emolding" } )
   end
   --]]
--==============================================================================
-- Geometry
--==============================================================================

   blocksBegin()
      for c in query( "emolding" ) do blocks{ c, id=0 } end
      for c in query( "iwall" ) do blocks{ c, id=0 } end
      for c in query( "ewall" ) do blocks{ c, id=c.mat } end
   blocksEnd()

--==============================================================================
-- Connections
--==============================================================================

   for r in rquery( "labwindow" ) do
      local s=r.component.size
      connect( execute( "architecture/window/window04", { w=s[1]-0.2, h=1.6,d=0.12 } ), r, {0.5,0.5,0},{0,0,-0.06} )
   end

   for r in rquery( "frame" ) do
      local s = r.component.size
      connect( execute( "architecture/door/frame02", {w=s[1]-0.2,h=s[2]-0.4,d=0.14, jamb=0.2} ), r, {0.5,0,0}, {0,0.05,-0.07} )
   end

   for r in rquery( "door" ) do
      connect( execute( "architecture/door/frame02", {w=1.1,h=2.2,d=0.14, jamb=0.1} ), r, {0.5,0,0}, {0,0.05,-0.07} )
   end

compositeEnd()
