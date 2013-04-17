compositeBegin()

-- Volumes principales
component{ id="aile1", size={18,2,18}, position={-9,0,-9}, couleur=2 }
component{ id="aile2", size={8,2,25},  position={-17,0,-12.5}, couleur=2 }
component{ id="aile3", size={8,2,25},  position={9,0,-12.5}, couleur=2 }
component{ id="aile4", size={12,2,10}, position={-29,0,-5}, couleur=2 }
component{ id="aile5", size={12,2,10}, position={17,0,-5}, couleur=2 }

local s={}
for c in query( "aile1", "aile2", "aile3", "aile4", "aile5" ) do
   s[#s+1] = c
end
merge( s, { id="base" } )

for c in query( "aile2", "aile3" ) do
   split( c, "Z", { id="piece", rel=1 }, { id="vide", 3 }, { id="piece", rel=1 } )
end
for c in query( "aile4" ) do
   split( c, "X", { id="piece", rel=1 }, { id="sub1", 5 } )
end
for c in query( "aile5" ) do
   split( c, "X", { id="sub2", 5 }, { id="piece", rel=1 } )
end
for c in query( "sub1" ) do
   split( c, "Z", { id="sub3", rel=1 }, { id="vide", 3 }, { id="sub4", rel=1 } )
end
for c in query( "sub2" ) do
   split( c, "Z", { id="sub5", rel=1 }, { id="vide", 3 }, { id="sub4", rel=1 } )
end
for c in query( "sub3" ) do
   split( c, "X", { id="piece", rel=2 }, { id="piece", rel=1 } )
end
for c in query( "sub4" ) do
   split( c, "X", { id="piece", rel=1 }, { id="piece", rel=1 } )
end
for c in query( "sub5" ) do
   split( c, "X", { id="piece", rel=1 }, { id="piece", rel=2 } )
end

for c in query( "aile1" ) do
   split( c, "X", { id="sub6", 3 }, { id="sub7", rel=1 }, { id="sub6", 3 } )
end
for c in query( "sub6" ) do
   split( c, "Z", { id="piece", 5 }, { id="vide", rel=1 }, { id="piece", 5 } )
end
for c in query( "sub7" ) do
   split( c, "Z", { id="sub8", rel=1 }, { id="vide", 4 }, { id="sub8", rel=1 } )
end
for c in query( "sub8" ) do
   split( c, "X", { id="piece", rel=1 }, { id="piece", 3 } )
end

local s = {}
for c in query( "piece" ) do s[#s+1] = c end
for c in query( "base" ) do
   subtract( c, s, { id="couloir", couleur=3 } )
end

-- Interieur
for c in query( "piece", "couloir" ) do
   local s = {}
   for f in fquery( c, "SIDE", "BOTTOM" ) do
      s[#s+1] = component{ c, id="mur", boundary=f }
   end
   extrude( s, -0.1, { id="iwall", c.couleur } )
end

-- Facades
for c in query( "base" ) do
   local s = {}
   for f in fquery( c, "SIDE" ) do
      s[#s+1] = component{ c, id="facade", boundary=f }
   end
   extrude( s, 0.3, { id="ewall", couleur=1 } )
end

-- Geometrie
blocksBegin()
   for c in query( "iwall", "ewall" ) do
      blocks{ c, id=c.couleur }
   end
blocksEnd()

compositeEnd()
