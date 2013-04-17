compositeBegin()

-- Volumes principales
component{ id="base", 
   boundary={ {0,0,3},{0,0,9},{1,0,9},{2,0,10},{3,0,10},{4,0,9},{14,0,9},{14,0,7},{16,0,7},{16,0,0},{9,0,0},{9,0,1},{2,0,1},{2,0,3}, direction={0,2,0}}
}
---[[
for c in query( "base" ) do
   component{ c, id="espace1", size={7,2,7}, position={9,0,0} }
   component{ c, id={"chambre", "piece_a"}, size={7,2,2}, position={9,0,7},     prio=1 }
   component{ c, id={"salon",   "piece_a"}, size={4,2,4.5}, position={6,0,1},   prio=2 }
   component{ c, id={"boudoir", "piece_a"}, size={4,2,3.5}, position={6,0,5.5}, prio=3 }
   component{ c, id={"cuisine", "piece_a"}, size={6,2,4.5}, position={0,0,1},   prio=4 }
   component{ c, id={"entree",  "piece_a"}, size={6,2,3.5}, position={0,0,5.5}, prio=5 }
   component{ c, id={"salle",   "piece_a"}, size={4,2,7}, position={0,0,3},     prio=6 }
end
for c in query( "espace1" ) do
   split( c, "X", { id="espace2", 3 }, { id={"garage", "piece_a"}, rel=1, prio=0.5 } )
end
for c in query( "espace2" ) do
   split( c, "Z", 
      { id={"bain",    "piece_a"}, rel=1.5, prio=0.1 },
      { id={"lavage",  "piece_a"}, rel=1,   prio=0.2 },
      { id={"placard", "piece_a"}, rel=1,   prio=0.3 },
      { id={"foyer",   "piece_a"}, rel=2,   prio=0.4 }
   )
end

--for c in query( "piece_a" ) do
--   subtract( c, "prio", { id="piece" } )
--end

-- Interieur
for c in query( "piece_a" ) do
--for c in query( "piece" ) do
   local s = {}
   for f in fquery( c, "SIDE", "BOTTOM" ) do
      s[#s+1] = component{ c, id="mur", boundary=f }
   end
   extrude( s, -0.1, { id="iwall", couleur=2 } )
end
--]]
--[[
for c in query( "base" ) do
   component{ c, id="espace1", size={7,2,7}, position={9,0,0} }
   component{ c, id={"chambre", "piece_a"}, size={7,2,2}, position={9,0,7},     prio=1 }
   component{ c, id={"salon",   "piece_c"}, size={4,2,4.5}, position={6,0,1} }
   component{ c, id={"boudoir", "piece_a"}, size={4,2,3.5}, position={6,0,5.5}, prio=3 }
   component{ c, id={"entree",  "piece_a"}, size={6,2,3.5}, position={0,0,5.5}, prio=5 }
   component{ c, id={"cuisine", "piece_b"}, size={6,2,4.5}, position={0,0,1} }
   component{ c, id={"salle",   "piece_b"}, size={4,2,7}, position={0,0,3} }
end
for c in query( "espace1" ) do
   split( c, "X", { id="espace2", 3 }, { id={"garage", "piece_a"}, rel=1, prio=0.5 } )
end
for c in query( "espace2" ) do
   split( c, "Z", 
      { id={"bain",    "piece_a"}, rel=1.5, prio=0.1 },
      { id={"lavage",  "piece_a"}, rel=1,   prio=0.2 },
      { id={"placard", "piece_a"}, rel=1,   prio=0.3 },
      { id={"foyer",   "piece_c"}, rel=2 }
   )
end

local s={}
for c in query( "piece_b" ) do s[#s+1] = c end
merge( s, { id="piece_a", prio=6 } )

local s={}
for c in query( "piece_c" ) do s[#s+1] = c end
merge( s, { id="piece_a", prio=2 } )

for c in query( "piece_a" ) do
   subtract( c, "prio", { id="piece" } )
end

-- Interieur
for c in query( "piece" ) do
   local s = {}
   for f in fquery( c, "SIDE", "BOTTOM" ) do
      s[#s+1] = component{ c, id="mur", boundary=f }
   end
   extrude( s, -0.1, { id="iwall", couleur=2 } )
end
--]]

-- Facades
for c in query( "base" ) do
   local s = {}
   for f in fquery( c, "SIDE" ) do
      s[#s+1] = component{ c, id="facade", boundary=f }
   end
   extrude( s, 0.1, { id="ewall", couleur=1 } )
end

-- Geometrie
--blocksBegin()
   for c in query( "iwall", "ewall" ) do
      blocksBegin()
      blocks{ c, id=c.couleur }
      blocksEnd()
   end
--blocksEnd()

compositeEnd()
