
--==============================================================================
-- Function
--==============================================================================

local function apartment01( comp )
   queryBegin(comp)
   split( comp, "Z", { id="backApart", rel=3.0 }, { id="middleApart", rel=2.5 }, { id="frontApart", rel=5.5 } )
   for c in query( "backApart" ) do
      split( c, "X", { id="b1Apart", 1.8 }, { id={"kitchen","room"}, rel=1 } )
   end
   for c in query( "b1Apart" ) do
      split( c, "Z", { id="empty", rel=1 }, { id={"closet","room"}, 1 } )
   end
   for c in query( "middleApart" ) do
      split( c, "X", { id={"entryway","room"}, rel=3.10 }, { id="m1Apart", rel=2.30 } )
   end
   for c in query( "m1Apart" ) do
      split( c, "Z", { id="m2Apart", rel=1 }, { id="m3Apart", 0.9 } )
   end
   for c in query( "m2Apart" ) do
      split( c, "Y", { id={"bathroom","room"}, rel=1 }, { id="empty", 0.3 } )
   end
   for c in query( "m3Apart" ) do
      split( c, "X", { id={"closet","room"}, rel=0.4, orientation=1 }, { id={"closet","room"}, rel=0.6 } )
   end
   for c in query( "frontApart" ) do
      component{ c, id="f1Apart", boundary={{0,0,0},{0,0,1},{0.45,0,1},{0.45,0,0.28},{0.27,0,0.15},{0.27,0,0}} }
      component{ c, id={"livingroom","room"}, 
         boundary={
            {1,0,1},{1,0,0},{0.27,0,0},{0.27,0,0.15},{0.45,0,0.28},{0.45,0,1},
            id={"b","t","s","s","s","d","s","s"}
         }
      }
   end
   for c in query( "f1Apart" ) do
      split( c, "Z", { id={"closet", "room"}, 0.7 }, { id={"bedroom","room"}, rel=1 } )
   end
   queryEnd()
end

--==============================================================================
-- Building frame
--==============================================================================

compositeBegin()

   --component{ id="main", size={5.4,2.8,11} }
   component{ id="main", size={5.4,2.8,11}, boundary={{0.16,0,0},{0.16,0,0.5},{0,0,0.5},{0,0,1},{1,0,1},{1,0,0}} }

--==============================================================================
-- Interior
--==============================================================================

   for c in query( "main" ) do
      apartment01( c )
   end

   -- Walls
   for c in query( "room" ) do
      local s = {}
      for f in fquery( c, "SIDE" ) do
         s[#s+1] = component{ c, id="wall", boundary=f }
      end
      for f in fquery( c, "B", "T" ) do
         s[#s+1] = component{ c, id="floor", boundary=f }
      end
      extrude( s, -0.05, { id="ewall" } )
   end

   -- Interior wall decoration.
   for c in query( "ewall" ) do
      if hasParentID( c, "wall" ) then
         for f in fquery( c, "E" ) do
            component{ c, id="iwall", boundary=f }
         end
      end
   end

   for c in query( "iwall" ) do
      split( c, "Y", { id="molding", 0.12 }, { id="emptyw", rel=1 }, { id="molding", 0.12 }, { id="molding2", 0.01 } )
   end

   local s={}
   for c in query( "molding" ) do s[#s+1] = c end
   extrude( s, 0.02, { id="emolding" } )

   local s={}
   for c in query( "molding2" ) do s[#s+1] = c end
   extrude( s, 0.1, { id="emolding" } )

   -- Doors
   for c in query( "closet" ) do
      for f in fquery( c, "Z" ) do region{ c, f, id="door" } end
   end

   for c in query( "bathroom" ) do
      for f in fquery( c, "-X" ) do region{ c, f, id="door" } end
   end

   for c in query( "livingroom" ) do
      for f in fquery( c, "d" ) do region{ c, f, id="door" } end
   end

   for c in query( "entryway" ) do
      for f in fquery( c, "Z" ) do region{ c, f, id="arch" } end
      for f in fquery( c, "-Z" ) do
         region{ c, f, id="door", rel={{0,0},{0.5,1}} }
      end
   end

--==============================================================================
-- Geometry
--==============================================================================

   blocksBegin()
      for c in query( "ewall", "emolding" ) do blocks{ c } end
   blocksEnd()

--==============================================================================
-- Connections
--==============================================================================

   -- Doors
   for r in rquery( "door" ) do
      local s = r.size
      local w = 0.76
      connect( execute( "architecture/door/frame02", {w=w,h=2,d=0.14} ), r, {0.5,0,0}, {0,0.05,-0.07} )
   end

   for r in rquery( "arch" ) do
      connect( execute( "architecture/door/arch01", {w=1.14,h=2.2,d=0.14,r=0.7,t=0} ), r, {1,0,0}, {-0.67,0.05,-0.07} )
   end

compositeEnd()
