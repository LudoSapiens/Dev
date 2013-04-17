local Mat = execute( "architecture/common_materials" )

--==============================================================================
-- Main volume
--==============================================================================

compositeBegin()
   component{ 
      id={"leftWing","wing"}, size={3,8,10}, 
      boundary={ {0,0,0},{0,0,1},{1,0,1},{1,0,0}, id={"b","t","s","m","s","b"} }
   }
   component{ 
      id={"centerWing","wing"}, size={8,8,9}, 
      boundary={ {0,0,0},{0,0,1},{1,0,1},{1,0,0}, id={"b","t","s","f","s","b"} },
      position={2,0,0}
   }
   component{
      id={"baywindow","wing"}, size={1.5,5,3},
      boundary={ {1,0,0},{0.6,0,0},{0,0,0.2},{0,0,0.8},{0.6,0,1},{1,0,1}, id={"b","t","s","bw","bw","bw","s","s"} },
      position={-1.5,0,4}
   }
   component{ id="fireplace_space", position={9.7,0,5}, size={0.8,11,1},
      boundary={ {0,0,0},{0,0,1},{1,0,1},{1,0,0}, id={"b","t","fi","s","s","s"} }
   }
   component{ id={"fireplace_space","firebase"}, position={9.7,0,4.5}, size={0.3,3.8,2},
      boundary={ {0,0,0},{0,0,1},{1,0,1},{1,0,0}, id={"b","t","fi","s","s","s"} }
   }
   local s={}
   for c in query( "fireplace_space" ) do s[#s+1] = c end
   merge( s, { id={"fireplace","room"} } )

   component{ id="lucarne", size={1,1.5,3}, position={4,8,6} }
   component{ id="lucarne", size={1,1.5,3}, position={6,8,6} }
   component{ id="lucarne", size={1,1.5,3}, position={8,8,6} }
   component{ id="lucarne", size={2,1.5,2}, position={0,8,4}, orientation=1 }
   component{ id="lucarne", size={1,1.5,1}, position={1,8,9} }

   -- Exterior stairs.
   component{ id="exteriorStaircase", size={1.8,1.12,2}, position={5.6,1.6,8.9} }
   for c in query( "exteriorStaircase" ) do
      region{ c, id="staircase" }
   end

   -- Roof
   for c in query( "leftWing" ) do
      for f in fquery( c, "T" ) do
         local r = component{ c, id="roofFloor", boundary=f }
         roof( r, {0.2,0.8,3}, { id={"wingRoof"} } )
      end
   end

   for c in query( "centerWing" ) do
      for f in fquery( c, "T" ) do
         local r = component{ c, id="roofFloor", boundary=f }
         roof( r, {1,0.4,2.5}, { id={"wingRoof"} } )
      end
   end

   for c in query( "lucarne" ) do
      for f in fquery( c, "T" ) do
         local r = component{ c, id="roofFloor", boundary=f }
         roof( r, {0,1,0.5}, { id={"wingRoof"} } )
      end
   end

   local s={}
   for c in query( "wingRoof", "lucarne" ) do s[#s+1] = c end
   merge( s, { id="roof_space" } )

   -- Roof priorities.
   local s={}
   for c in query( "fireplace" ) do s[#s+1] = c end
   for c in query( "roof_space" ) do subtract( c, s, { id="roof", mat=Mat.EXT_ROOF1 } ) end

   -- Merge all wings together.
   local s={}
   for c in query( "wing" ) do s[#s+1] = c end
   merge( s, { id="hull" } )

   -- Level.
   for c in query( "hull" ) do
      slice( c, "Y", { id="level", 2.5, level=counter() }, 2 )
   end

-- TODO: example with and without constraint!!!

--==============================================================================
-- Exterior
--==============================================================================

   local s={}
   for c in query( "level" ) do
      for f in fquery( c, "S" ) do
         s[#s+1] = component{ c, id="facade", boundary=f }
      end
   end

   for c in query( "facade" ) do
      if c.level==2 then
         split( c, "Y", { id="f1", rel=1 }, { id="ledge1", 0.2 }, { id="ledge2", 0.1 } )
      end
   end

   for c in query( "facade", "f1" ) do
      if c.level < 2 or hasID( c, "f1" ) then
         local fid = faceID(c)
         if fid == "m" then
            split( c, "X", { id="fA", 0.4 }, { id="f2", rel=1 }, { id="fA", 0.4 } )
         elseif fid == "f" then
            split( 
               c, "X", 
               { id="fA", 0.4 }, { id="f2", rel=1 }, { id="f2", rel=1 }, { id="fA", 0.2 }, 
               { id="fC", 1.5 }, 
               { id="fA", 0.2 }, { id="f2", rel=1 }, { id="f2", rel=1 }, { id="fA", 0.4 }
            )
         else
            component{ c, id="fA" }
         end
      end
   end

   for c in query( "f2" ) do
      if c.level == 1 then
         split( c, "Y", { id="fB", rel=1 }, { id="ledge3", 0.2 } )
      else
         component{ c, id="fB" }
      end
   end

   local s={}
   for c in query( "ledge1" ) do s[#s+1] = c end
   extrude( s, 0.3, {id="eledge"} )

   local s={}
   for c in query( "ledge2" ) do s[#s+1] = c end
   extrude( s, 0.4, {id="eledge"} )

   local s={}
   for c in query( "ledge3" ) do s[#s+1] = c end
   extrude( s, 0.25, {id="eledge"} )

   local s={}
   for c in query( "fA" ) do s[#s+1] = c end
   extrude( s, 0.2, {id="ewall", mat=Mat.EXT_WALL1} )

   local s={}
   for c in query( "fB" ) do s[#s+1] = c end
   extrude( s, 0.15, {id="ewall", mat=Mat.EXT_WALL2} )

   local s={}
   for c in query( "fC" ) do s[#s+1] = c end
   extrude( s, 0.1, {id="ewall", mat=Mat.EXT_WALL2} )

   -- Windows.
   for c in query( "fB" ) do region{ c, id="window" } end
   for c in query( "facade" ) do
      if faceID(c) == "bw" and c.level == 1 then region{ c, id="window1" } end
   end

   for c in query( "lucarne" ) do
      for f in fquery( c, "Z" ) do region{ c, f, id="window2" } end
   end

   -- Doors.
   for c in query( "fC" ) do 
      if c.level > 0 then region{ c, id="extdoor" } end
   end

   -- Terrain.
   component{ id="terrain_space", position={-5,1.5,-5}, size={20,2,10} }
   component{ id="terrain_space", position={-5,1.5, 5}, size={20,2,10} }

   local s={}
   for c in query( "hull" ) do s[#s+1]=c end
   for c in query( "terrain_space" ) do
      subtract( c, s, { id="terrain" } )
   end
   local s={}
   for c in query( "terrain" ) do
      for f in fquery( c, "B" ) do
         s[#s+1] = component{ c, id="fterrain", boundary=f }
      end
   end
   extrude( s, -0.1, {id="eterrain"} )

--==============================================================================
-- Connections
--==============================================================================

   -- Windows
   for r in rquery( "window" ) do
      local c = r.component
      if c.level == 0 then
         connect( execute( "architecture/window/window03", {w=0.8, h=0.6, d2=0.20,d3=0.27} ), r, {0.5,1,0}, {0,-0.6,-0.07} )
      else
         connect( execute( "architecture/window/window01", {w=0.8, h=1.6, d2=0.20,d3=0.27, w2=0.05} ), r, {0.5,0.5,0}, {0,0,-0.07} )
      end
   end
   for r in rquery( "window1" ) do
      local s = r.size
      connect( execute( "architecture/window/window01", {w=s[1], h=1.6, d2=0.20,d3=0.27, w2=0.05} ), r, {0.5,0.5,0}, {0,0,-0.07} )
   end
   for r in rquery( "window2" ) do
      connect( execute( "architecture/window/window01", { w=0.8, h=1.2, d2=0.06,d3=0.10} ), r, {0.5,0.5,0}, {0,0,-0.07} )
   end

   -- Doors.
   for r in rquery( "extdoor" ) do
      if r.component.level == 1 then
         connect( execute( "architecture/door/frame02", {w=1.1,h=2.1,d=0.18} ), r, {0.5,0,0}, {0,0.05,-0.07} )
      else
         connect( execute( "architecture/door/frame02", {w=0.9,h=2.1,d=0.18} ), r, {0.5,0,0}, {0,0.05,-0.07} )
         connect( execute( "architecture/balcony/balcony01", {w=1.8} ), r, {0.5,0,0}, {0,0.05,0.1} )
      end
   end

   for r in rquery( "staircase" ) do
      local size = r.size
      connect( 
         execute( "architecture/stair/stair01", {w=size[1],h=size[2],d=size[3],stw=0,ns=0.05,rn=true,stepNum=6,riser="full"} ), 
         r, {0,0,0},{0,0,0.2}
      )
   end

--==============================================================================
-- Interior
--==============================================================================

   local cs={}
   for c in query( "window" ) do
      if c.level ~= nil then
         cs[#cs+1] = volumeConstraint{ c, offset={0.1,0,0}, repulse=true }
      end
   end

   for c in query( "level" ) do
      split( c, "X", { id="leftLevel", rel=1 }, { id="rightLevel", rel=1 }, cs )
   end

   for c in query( "leftLevel", "rightLevel" ) do
      if c.level == 0 then component{ c, id="room_space" } end
   end

   for c in query( "leftLevel" ) do
      if c.level == 1 then
         split( c, "Z", { id="room_space", type="kitchen", rel=1 }, { id="room_space", type="dinning", rel=2 } )
      elseif c.level > 1 then
         split( c, "Z", { id="room_space", rel=1 }, { id="room_space", rel=0.5 }, { id="room_space", rel=2 } )
      end
   end
   ---[[
   for c in query( "rightLevel" ) do
      if c.level == 1 then
         component{ c, id="room_space", type="living" }
      elseif c.level > 1 then
         split( c, "X", { id="room_space", 1.5 }, { id={"room_space"}, rel=1 } ) 
      end
   end
   --]]
   -- Staircase.
   local cs={}
   for c in query( "rightLevel" ) do
      for f in fquery( c, "-X" ) do cs[#cs+1] = planeConstraint{ c, f } end
   end

   for c in query( "hull" ) do
      split( c, "X", { id="leftHull", rel=1 }, { id="rightHull", rel=1 }, cs )
   end
   for c in query( "rightHull" ) do
      local s = c.size
      component{ 
         c, id={"stairwell","room"}, position={0,0,3}, size={1,s[2],s[3]-6.5},
         boundary={{0,0,0},{0,0,1},{1,0,1},{1,0,0}, id={"b","t","s", "inv","inv","inv"}}
      }
   end
   local cs={}
   for c in query( "level" ) do
      for f in fquery( c, "B" ) do cs[#cs+1] = planeConstraint{ c, f } end
   end
   for c in query( "stairwell" ) do
      slice( c, "Y", { id="staircaseLevel", level=counter() }, cs )
   end
   for c in query( "stairwell" ) do
      execute( "architecture/stair/staircase07", {c} )
   end

   -- Room priorities.
   local s={}
   for c in query( "stairwell", "fireplace" ) do s[#s+1] = c end
   for c in query( "room_space" ) do subtract( c, s, { id="room" } ) end

   -- Walls.
   for c in query( "room", "hall", "roof" ) do
      local s={}
      for f in fquery( c, "SIDE" ) do
         if faceID( c, f ) ~= "inv" then
            s[#s+1] = component{ c, id="wall", boundary=f }
         end
      end
      for f in fquery( c, "B" ) do
         s[#s+1] = component{ c, id="floor", boundary=f }
      end
      for f in fquery( c, "T" ) do
         s[#s+1] = component{ c, id="ceiling", boundary=f }
      end
      extrude( s, -0.05, { id="iwall" } )
   end

   --[[
   -- Molding.
   for c in query( "iwall" ) do
      if hasParentID( c, "wall" ) and faceID(c) ~= "fi" and c.level == 1 then
         for f in fquery( c, "E" ) do component{ c, id="iwall2d", boundary=f } end
      end
   end
   for c in query( "iwall2d" ) do
      split( c, "Y", { id="molding", 0.12 }, { id="emptyw", rel=1 }, { id="molding", 0.12 }, { id="molding2", 0.01 } )
   end

   local s={}
   for c in query( "molding" ) do s[#s+1] = c end
   extrude( s, 0.02, { id="emolding" } )

   local s={}
   for c in query( "molding2" ) do s[#s+1] = c end
   extrude( s, 0.1, { id="emolding" } )
   --]]

   --Doors
   for c in query( "room" ) do
      if c.type == "kitchen" then
         for f in fquery( c, "Z" ) do region{ c, f, id="door" } end
         for f in fquery( c, "X" ) do region{ c, f, id="door" } end
      end
      if c.type == "dinning" then
         for f in fquery( c, "X" ) do 
            local t = component{ c, id="empty", boundary=f }
            if t.size[1] > 1 then
               region{ c, f, id="door", rel={{0,0},{0,1}}, abs={{0,0},{2,0}} }
            end
         end
      end
   end

   for r in rquery( "door" ) do
      connect( execute( "architecture/door/frame02", {w=1.2,h=2.1,d=0.14} ), r, {0.5,0,0}, {0,0.05,-0.07} )
   end

   -- Couch
   for c in query("room") do
      if c.type == "living" then
         region{ c, id="couch" }
      end
   end

---[[
   for r in rquery( "couch" ) do
      connect( execute( "furniture/couch/couch01", {} ), r, {0.3,0,0.7}, {0,0.1,0}, {{0,1,0}, 0.25} )
      connect( execute( "furniture/couch/couch01", {} ), r, {0.9,0,0.9}, {0,0.1,0}, {{0,1,0}, 0.50} )
      connect( execute( "furniture/table/table01", { w=1, h=0.4, d=1, sh=0.03} ), r, {0.6,0,0.55}, {0,0.1,0}, {{0,1,0}, 0.0} )
   end
--]]
   -- Fireplace.
   for c in query( "wall" ) do
      if c.level == 1 and faceID(c) == "fi" then
         region{ c, id="firedoor" }
      end
   end

   for r in rquery( "firedoor" ) do
      connect( execute( "architecture/door/arch01", {w=01.2,h=0.8,d=0.14} ), r, {0.5,0,0}, {0,0.05,-0.07} )
   end

   for c in query( "firebase" ) do
      for f in fquery( c, "-X", "-Z", "Z" ) do
         component{ c, id="fireplace_wall", boundary=f }
      end
   end
   for c in query( "fireplace_wall" ) do
      split( c, "Y", { id="empty", rel=1 }, { id="fireledge", 0.1 } )
   end
   local s={}
   for c in query( "fireledge" ) do s[#s+1] = c end
   extrude( s, 0.2, { id="eledge" } )

--==============================================================================
-- Geometry
--==============================================================================

   blocksBegin()
      for c in query( "iwall" ) do blocks{ c, id=c.mat } end
      for c in query( "ewall" ) do blocks{ c, id=c.mat } end
      for c in query( "eledge" ) do blocks{ c, id=2 } end
      for c in query( "emolding" ) do blocks{ c, id=0 } end
      for c in query( "eterrain" ) do blocks{ c, id=3 } end
   blocksEnd()
compositeEnd()

