compositeBegin()
   local s = { 5, 1, 2.5 }
   local roofs = { {0.8,0,1}, {0,0.8,1}, {0.5,0.5,1}, {0,0,1} }
   local c = component{ id="main", size=s, position={-6,0,0}, roof=1 }
   local c = component{ id="main", size=s, position={0,0,0},  roof=2 }
   local c = component{ id="main", size=s, position={6,0,0},  roof=3 }
   local c = component{ id="main", size=s, position={12,0,0}, roof=4 }

   local b = {{0.25,0,0},{0,0,0.5},{0.25,0,1},{0.75,0,1},{1,0,0.5},{0.75,0,0}}

   local c = component{ id="main", size=s, position={-6,0,4}, roof=1, boundary=b }
   local c = component{ id="main", size=s, position={0,0,4},  roof=2, boundary=b }
   local c = component{ id="main", size=s, position={6,0,4},  roof=3, boundary=b }
   local c = component{ id="main", size=s, position={12,0,4}, roof=4, boundary=b }

   for c in query( "main" ) do
      local s = {}
      for f in fquery( c, "SIDE", "B" ) do
         s[#s+1] = component{ c, id="wall", boundary=f }
      end
      extrude( s, -0.1, { id="ewall" } )

      local s = {}
      for f in fquery( c, "T" ) do
         s[#s+1] = component{ c, id="ceiling", boundary=f }
      end
      roof( s, roofs[c.roof], { id="roof" } )
   end

   for c in query( "roof" ) do
      local s = {}
      for f in fquery( c, nil ) do
         s[#s+1] = component{ c, id="roofside", boundary=f }
      end
      extrude( s, -0.01, { id="roofsides" } )
   end

   blocksBegin()
      for c in query( "ewall" ) do blocks{ c } end
   blocksEnd()

   blocksBegin()
      translate( 0, 1, 0 )
      for c in query( "roofsides" ) do blocks{ c } end
   blocksEnd()
compositeEnd()
