local Mat = execute( "architecture/common_materials" )

local cut = false

if cut then
   differenceBegin()
end

--==============================================================================
-- Frame
--==============================================================================

compositeBegin()

   local rh = 0.5
   -- Main volume.
   component{ 
      id={"mainWing","wing"}, size={8,11+rh,12}, 
      boundary={ {0,0,0},{0,0,1},{1,0,1},{1,0,0}, id={"b","t","l","m","r","b"} }
   }
   component{
      id={"frontWing","wing"}, size={3,11+rh,1},
      boundary={ {0,0,0},{0.3,0,1},{0.7,0,1},{1,0,0}, id={"b","t","sb","fb","sb","bb"} },
      position={5,0,12}
   }

   -- Interior stairs.
   for c in query( "mainWing" ) do
      local s=c.size
      component{ 
         c, id={"stairwell","room"}, size={1.2,s[2]-rh,5}, position={0,0,3},
         boundary={{0,0,0},{0,0,1},{1,0,1},{1,0,0}, id={"b","t","s", "inv","inv","inv"}}
      }
   end

   -- Exterior stairs.
   component{ id="exteriorStaircase", size={2.5,1.8,2}, position={0.1,1,12} }
   for c in query( "exteriorStaircase" ) do
      region{ c, id="staircase" }
   end

   -- Merge all wings together.
   local s={}
   for c in query( "wing" ) do s[#s+1] = c end
   merge( s, { id="hull" } )

   for c in query( "hull" ) do
      split( c, "Y", { id="baseHull", rel=1 }, { id="roof", rh } )
   end

--==============================================================================
-- Interior
--==============================================================================

   for c in query( "baseHull" ) do
      slice( c, "Y", { id="level", 2.5, level=counter() }, 2 )
   end

   for c in query( "level" ) do
      split( c, "X", { id="hall_space", 2.3 }, { id="apartment", rel=1 } )
   end

   local s={}
   for c in query( "stairwell" ) do s[#s+1] = c end
   for c in query( "hall_space" ) do
      subtract( c, s, { id={"hall","room"}, fmat=Mat.FLOOR2 } )
   end

   -- Staircase.
   local cs={}
   for c in query( "level" ) do
      for f in fquery( c, "T" ) do cs[#cs+1] = planeConstraint{ c, f } end
   end
   for c in query( "stairwell" ) do
      slice( c, "Y", { id="staircaseLevel", level=counter() }, cs )
   end
   for c in query( "stairwell" ) do
      execute( "architecture/stair/staircase07", {c, sth=0.5,mat={Mat.FLOOR2}} )
   end

   -- Apartment.
   for c in query( "apartment" ) do
      execute( "architecture/apartment/apartment01", {c} )
   end

   --Walls
   for c in query( "room" ) do
      local s = {}
      for f in fquery( c, "SIDE" ) do
         if faceID( c, f ) ~= "inv" then
            s[#s+1] = component{ c, id="wall", boundary=f, mat=c.wmat or Mat.WALL1 }
         end
      end
      for f in fquery( c, "B" ) do
         s[#s+1] = component{ c, id="floor", boundary=f, mat=c.fmat or Mat.FLOOR1 }
      end
      if not cut then
         for f in fquery( c, "T" ) do
            s[#s+1] = component{ c, id="ceiling", boundary=f, mat=Mat.CEILING1 }
         end
      end
      extrude( s, -0.05, { id="iwall" } )
   end

   -- Interior wall decoration.
   for c in query( "iwall" ) do
      if hasParentID( c, "wall" ) then
         for f in fquery( c, "E" ) do component{ c, id="iwall2d", boundary=f } end
      end
   end

   for c in query( "iwall2d" ) do
      split( c, "Y", { id="molding", 0.12 }, { id="emptyw", rel=1 }, { id="molding", 0.12 }, { id="molding2", 0.01 } )
   end

   local s={}
   for c in query( "molding" ) do s[#s+1] = c end
   extrude( s, 0.02, { id="emolding", mat=Mat.DETAIL1 } )

   local s={}
   for c in query( "molding2" ) do s[#s+1] = c end
   extrude( s, 0.1, { id="emolding", mat=Mat.DETAIL2 } )

--==============================================================================
-- Exterior
--==============================================================================

   for c in query( "level" ) do
      local s={}
      for f in fquery( c, "S" ) do
         s[#s+1] = component{ c, id="facade", boundary=f }
      end
      if c.level == 0 then
         extrude( s, 0.15, { id="ewall", mat=Mat.EXT_WALL1 } )
      else
         extrude( s, 0.2, { id="ewall", mat=Mat.EXT_WALL2 } )
      end
   end

   -- Roof
   for c in query( "roof" ) do
      local s={}
      for f in fquery( c, "S" ) do s[#s+1]=component{ c, id="roofFacade", boundary=f } end
      extrude( s, 0.25, { id="ewall" } )
      local s={}
      for f in fquery( c, "B" ) do s[#s+1]=component{ c, id="roofCeiling", boundary=f } end
      extrude( s, -0.1, { id="ewall" } )
   end

   -- Windows
   for c in query( "facade" ) do
      if hasFaceID( c, "sb" ) then region{ c, id="window" } end
      if hasFaceID( c, "fb" ) then region{ c, id="window" } end
      if hasFaceID( c, "m" ) then
         split( c, "X", { id="fA", 2.5 }, { id="fB", rel=1 } )
      end
      if hasFaceID( c, "r" ) then
         region{ c, id="window", rel={{1,0},{1,1}}, abs={{-3,0},{0,0}} }
      end
   end

   for c in query( "facade" ) do
      if c.level > 0 then
         split( c, "Y", { id="empty", rel=1 }, { id="ledge1", 0.1 }, { id="ledge2", 0.2 } )
      end
   end

   local s={}
   for c in query( "ledge1" ) do s[#s+1] = c end
   extrude( s, 0.25, { id="eledge", mat=Mat.EXT_DETAIL1 } )

   local s={}
   for c in query( "ledge2" ) do s[#s+1] = c end
   extrude( s, 0.3, { id="eledge" } )

   for c in query( "fA" ) do
      if c.level > 1 then
         region{ c, id="window" }
      elseif c.level == 1 then
         region{ c, id="extdoor" }
      end
   end

   for c in query( "fB" ) do
      region{ c, id="window" }
   end

   -- Terrain.
   component{ id="terrain_space", position={-5,0.8,-3}, size={18,2,9} }
   component{ id="terrain_space", position={-5,0.8, 6}, size={18,2,10} }

   local s={}
   for c in query( "hull" ) do s[#s+1]=c end
   for c in query( "terrain_space" ) do subtract( c, s, { id="terrain" } ) end
   local s={}
   for c in query( "terrain" ) do
      for f in fquery( c, "B" ) do
         s[#s+1] = component{ c, id="fterrain", boundary=f }
      end
   end
   extrude( s, -0.2, { id="eterrain", mat=Mat.EXT_FLOOR1 } )

--==============================================================================
-- Geometry
--==============================================================================

   blocksBegin()
      for c in query( "iwall", "emolding" ) do blocks{ c, id=c.mat } end
      for c in query( "ewall" ) do blocks{ c, id=c.mat } end
      for c in query( "eledge" ) do blocks{ c, id=c.mat } end
      for c in query( "eterrain" ) do blocks{ c, id=c.mat } end
   blocksEnd()

--==============================================================================
-- Connections
--==============================================================================

   for r in rquery( "staircase" ) do
      local size = r.size
      connect( 
         execute( 
            "architecture/stair/stair01", 
            {w=size[1],h=size[2],d=size[3],stw=0.2,sth=0.5,ns=0.05,rn=true,stepNum=8,riser="full",mat={Mat.EXT_FLOOR2,Mat.EXT_FLOOR2,Mat.EXT_FLOOR2,Mat.EXT_FLOOR2}}
         ), 
         r, {0,0,0},{0,0,0.2}
      )
   end

   -- Doors
   for r in rquery( "door" ) do
      local s = r.size
      local w = 0.76
      connect( execute( "architecture/door/frame02", {w=w,h=2,d=0.14} ), r, {0.5,0,0}, {0,0.05,-0.07} )
   end

   for r in rquery( "arch" ) do
      connect( execute( "architecture/door/arch01", {w=1.14,h=2.2,d=0.14,r=0.7,t=0} ), r, {1,0,0}, {-0.67,0.05,-0.07} )
   end

   for r in rquery( "extdoor" ) do
      connect( execute( "architecture/door/frame02", {w=1.5,h=2.2,d=0.35, jamb=0.2} ), r, {0.5,0,0}, {0,0.05,-0.07} )
   end

   -- Windows
   for r in rquery( "window" ) do
      local c = r.component
      if c.level == 0 then
         connect( execute( "architecture/window/window01", { w=1, h=1, d2=0.2,d3=0.26} ), r, {0.5,1,0}, {0,-1.0,-0.06} )
      else
         connect( execute( "architecture/window/window01", {w=1,h=1.6,d2=0.2,d3=0.26,bn=4,bd=0.15} ), r, {0.5,0.5,0}, {0,0,-0.06} )
      end
   end

compositeEnd()

if cut then
   blocksBegin()
      block{ {-10,5.5-0.06,-10},{20,20,20}, c=0xfff }
   blocksEnd()
   differenceEnd()
end
