compositeBegin()
--component{ id="aile1", size={18,2,18}, position={-9,0,-9}, couleur=2 }
component{ id="aile1", size={18,2,18}, position={-9,0,-9}, 
   boundary={ {0,0,0},{0,0,0.9},{0.1,0,0.9},{0.1,0,1},{1,0,1},{1,0,0.2},{0.5,0,0.2},{0.5,0,0}, direction={0,1,0}},
   couleur=2
}
--component{ id="aile2", size={10,2,25}, position={-19,0,-12}, couleur=2 }
--component{ id="aile2", size={10,2,25}, position={-19,0,-6}, couleur=2 }
component{ id="aile2", size={10,2,25}, position={-19,0,-6}, couleur=2,
   boundary={ {0,0,0},{0,0,0.2},{0.1,0,0.2},{0.1,0,0.25},{0,0,0.25},
   {0,0,0.8},{0.1,0,0.8},{0.1,0,0.85},{0,0,0.85},
   {0,0,1},{1,0,1},{1,0,0}, direction={0,1,0}}
}

local s={}
for c in query( "aile1", "aile2" ) do
   s[#s+1] = c
end
merge( s, { id="base", couleur=2 } )


for c in query( "aile1" ) do
   split( c, "Z", { id="appartement_z", rel=1 }, { id="div_b", 2 }, { id="appartement_z", rel=1 } )
end
for c in query( "aile2" ) do
   split( c, "X", { id="appartement_x", rel=1 }, { id="div_c", 2 }, { id="div_a", rel=1 } )
end

local cs={}
for c in query( "div_b" ) do
   for f in fquery( c, "SIDE" ) do cs[#cs+1] = faceConstraint{ c, f } end
end

for c in query( "div_a" ) do
   split( c, "Z", { id="appartement_x", 0 }, { id="div_c", rel=1 }, { id="appartement_x", 0 }, cs )
   --split( c, "Z", { id="appartement_x", rel=1 }, { id="div_c", 2 }, { id="appartement_x", rel=1 } )
end

for c in query( "appartement_x" ) do
   slice( c, "Z", { id="piece", 4 }, 2 )
end
for c in query( "appartement_z" ) do
   slice( c, "X", { id="piece", 4 }, 2 )
end

local s={}
for c in query( "div_c", "div_b" ) do
   s[#s+1] = c
end
merge( s, { id="couloir", couleur=3 } )



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
