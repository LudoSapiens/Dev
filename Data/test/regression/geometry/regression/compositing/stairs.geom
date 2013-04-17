compositeBegin()

--==============================================================================
-- Building frame
--==============================================================================

   -- Main volume.
   component{ id="main", size={16,6,10} }
   component{ id="stairway", size={2,9,3}, position={-13,0,0}, num=0 }
   component{ id="stairway", size={2,9,3}, position={-10,0,0}, num=1 }
   component{ id="stairway", size={2,9,3}, position={-7,0,0}, num=2 }
   component{ id="stairway", size={2,9,3}, position={-4,0,0}, num=3 }
   component{ id="stairway", size={3,9,3}, position={-13,0,10}, num=4 }
   component{ id="stairway", size={3,9,3}, position={-9,0,10}, num=5 }
   component{ id="stairway", size={3,9,3}, position={-5,0,10}, num=6 }

   -- Level.
   for c in query( "main" ) do
      slice( c, "Y", { id="level", 3, level=counter() } )
   end

   -- Rooms.
   for c in query( "level" ) do
      if c.level == 0 then
         split( c, "Z", { id="roomA", rel=1 }, { id="roomB", rel=1 } )
      else
         split( c, "Z", { id="roomC", rel=1 }, { id="roomD", rel=1 } )
      end
   end

   -- Walls, ceilling, floor.
   for c in query( "roomA" ) do
      local s = {}
      for f in fquery( c ) do
         s[#s+1] = component{ c, id="wall", boundary=f }
      end
      extrude( s, -0.1, { id="ewall" } )
   end

   for c in query( "roomB" ) do
      local s = {}
      for f in fquery( c, "-X", "+X", "B" ) do
         s[#s+1] = component{ c, id="wall", boundary=f }
      end
      extrude( s, -0.1, { id="ewall" } )
   end

   for c in query( "roomC" ) do
      local s = {}
      for f in fquery( c, "-X", "X", "-Z", "B", "T" ) do
         s[#s+1] = component{ c, id="wall", boundary=f }
      end
      extrude( s, -0.1, { id="ewall" } )
   end

   for c in query( "roomD" ) do
      local s = {}
      for f in fquery( c, "-X", "+X" ) do
         s[#s+1] = component{ c, id="wall", boundary=f }
      end
      extrude( s, -0.1, { id="ewall" } )
   end

--==============================================================================
-- Interior
--==============================================================================

   for c in query( "roomB" ) do
      slice( c, "X", { id="section", 3, num=counter() } )
   end

   for c in query( "section" ) do
      split( c, "X", { id="stair", 2 }, { id="waste", rel=1 } )
   end

   for c in query( "stair" ) do
      region{ c, id="stair" }
   end

   for c in query( "stairway" ) do
      local s={}
      for f in fquery( c, "B" ) do
         s[#s+1] = component{ c, id="wall", boundary=f }
      end
      extrude( s, -0.1, { id="wall" } )
   end

   for c in query( "stairway" ) do
      local num = c.num
      if num == 0 then
         execute( "architecture/stair/staircase01", { c, lh=3, stw=0 } )
      elseif num == 1 then
         execute( "architecture/stair/staircase01", { c, lh=3 } )
      elseif num == 2 then
         execute( "architecture/stair/staircase01", { c, lh=3, sth=0.4, rn=true } )
      elseif num == 3 then
         execute( "architecture/stair/staircase02", { c, lh=3, sth=0.4, rn=true } )
      elseif num == 4 then
         execute( "architecture/stair/staircase03", { c, lh=3, sth=0.4, rn=true } )
      elseif num == 5 then
         execute( "architecture/stair/staircase04", { c, lh=3, sth=0.4, rn=true } )
      elseif num == 6 then
         execute( "architecture/stair/staircase05", { c, lh=3, sth=0.4, rn=true } )
      end
   end

--==============================================================================
-- Geometry
--==============================================================================

   blocksBegin()
      for c in query( "ewall" ) do blocks{ c } end
   blocksEnd()

--==============================================================================
-- Connection
--==============================================================================

   -- stairs.
   for r in rquery( "stair" ) do
      local size = r.size
      local num  = r.component.num
      if num == 0 then
         connect( execute( "architecture/stair/stair01", { w=size[1], h=size[2], d=size[3]*0.8, stw=0 } ), r, {0,0,0},{0.1,0.1,0} )
      elseif num == 1 then
         connect( execute( "architecture/stair/stair01", { w=size[1], h=size[2], d=size[3]*0.8, rn=true } ), r, {0,0,0},{0.1,0.1,0} )
      elseif num == 2 then
         connect( execute( "architecture/stair/stair01", { w=size[1], h=size[2], d=size[3]*0.8, stepNum=15, sth=0.1 } ), r, {0,0,0},{0.1,0.1,0} )
      elseif num == 3 then
         connect( execute( "architecture/stair/stair01", { w=size[1], h=size[2], d=size[3]*0.8, raiser="none" } ), r, {0,0,0},{0.1,0.1,0} )
      else
         connect( execute( "architecture/stair/stair01", { w=size[1], h=size[2], d=size[3]*0.8, raiser="square" } ), r, {0,0,0},{0.1,0.1,0} )
      end
   end

compositeEnd()
