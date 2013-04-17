compositeBegin()

--[[

-- Volumes principales
component{ id="base", size=vec3(7,12,5) }

for c in query( "base" ) do
   slice( c, "Y", { id="etage", 3, level=counter() } )
end

-- Interieur
for c in query( "etage" ) do
   local s = {}
   for f in fquery( c, "SIDE", "BOTTOM" ) do
      s[#s+1] = component{ c, id="mur", boundary=f }
   end
   extrude( s, -0.1, { id="iwall", couleur=2 } )
end

-- Facades
for c in query( "etage" ) do
   for f in fquery( c, "SIDE" ) do
      component{ c, id="facade", boundary=f }
   end
   for c2 in query( c, "facade" ) do
      slice( c2, "X", { id="f1", 2 }, 2 )
   end

   local s = {}
   for c2 in query( c, "f1" ) do s[#s+1] = c2 end
   extrude( s, 0.1, { id="ewall", couleur=1 } )
end

-- Elements architecturaux.
for c in query( "f1" ) do region{ c, id="fenetre" } end

for r in rquery( "fenetre" ) do
   connect( execute( "architecture/window/window04", {h=1,w=0.5,d=0.2} ), r, {0.5,0.5,0}, {-0.3,0,-0.1} )
   connect( execute( "architecture/window/window04", {h=1,w=0.5,d=0.2} ), r, {0.5,0.5,0}, {0.3,0,-0.1} )
end
--]]

--[[
-- Volumes principales
component{ id="base", size=vec3(7,12,5) }

for c in query( "base" ) do
   slice( c, "Y", { id="etage", 3, niveau=counter() } )
end
-- Interieur
for c in query( "etage" ) do
   local s = {}
   for f in fquery( c, "SIDE", "BOTTOM" ) do
      s[#s+1] = component{ c, id="mur", boundary=f }
   end
   extrude( s, -0.1, { id="iwall", couleur=2 } )
end

-- Facades
for c in query( "etage" ) do
   for f in fquery( c, "SIDE" ) do
      component{ c, id="facade", boundary=f }
   end
   for c2 in query( c, "facade" ) do
      if c2.niveau < 3 then
         alternate( c2, "X", { id="f4", 0.5 }, { id="f3", 1.5 }, 3 )
      else
         split( c2, "Y", { id="f2", 0.1, rel=1 }, { id="f5", 0.3 }, { id="f2", 0.1, rel=1 }, { id="f6", 0.3 } )
      end
   end
   for c2 in query( c, "f3" ) do
      split( c2, "Y", { id="f7", 0.2 }, { id="f9", 0.4 }, { id="f7", 0.1 }, { id="f1", 0.1, rel=1 } )
   end
   for c2 in query( c, "f4" ) do
      split( c2, "Y", { id="f8", 0.2 }, { id="f2", 0.4 }, { id="f8", 0.1 }, { id="f2", 0.1, rel=1 } )
   end

   local s = {}
   for c2 in query( c, "f1", "f9" ) do s[#s+1] = c2 end
   extrude( s, 0.1, { id="ewall", couleur=1 } )
   local s = {}
   for c2 in query( c, "f2" ) do s[#s+1] = c2 end
   extrude( s, 0.2, { id="ewall", couleur=1 } )
   local s = {}
   for c2 in query( c, "f5" ) do s[#s+1] = c2 end
   extrude( s, 0.3, { id="ewall", couleur=1 } )
   local s = {}
   for c2 in query( c, "f6" ) do s[#s+1] = c2 end
   extrude( s, 0.4, { id="ewall", couleur=4 } )
   local s = {}
   for c2 in query( c, "f7" ) do s[#s+1] = c2 end
   extrude( s, 0.15, { id="ewall", couleur=2 } )
   local s = {}
   for c2 in query( c, "f8" ) do s[#s+1] = c2 end
   extrude( s, 0.25, { id="ewall", couleur=2 } )

end

-- Elements architecturaux.
for c in query( "f1" ) do region{ c, id="fenetre" } end

for r in rquery( "fenetre" ) do
   connect( execute( "architecture/window/window04", {h=1.5,w=0.5,d=0.2} ), r, {0.5,0.5,0}, {-0.3,0,-0.1} )
   connect( execute( "architecture/window/window04", {h=1.5,w=0.5,d=0.2} ), r, {0.5,0.5,0}, {0.3,0,-0.1} )
end
--]]


---[[
-- Volumes principales
component{ id="base", size=vec3(7,12,5), boundary={{0,0,0},{0,0,1},{1,0,1},{1,0,0}, id={"b","t","s","f","s","b"}} }

for c in query( "base" ) do
   slice( c, "Y", { id="etage", 3, niveau=counter() } )
end

-- Interieur
for c in query( "etage" ) do
   local s = {}
   for f in fquery( c, "SIDE", "BOTTOM" ) do
      s[#s+1] = component{ c, id="mur", boundary=f }
   end
   extrude( s, -0.1, { id="iwall", couleur=2 } )
end

-- Facades
for c in query( "etage" ) do
   for f in fquery( c, "SIDE" ) do
      component{ c, id="facade", boundary=f }
   end
   for c2 in query( c, "facade" ) do
      if c2.niveau == 0 then
         alternate( c2, "X", { id="b2", 0.5 }, { id="b1", 1.5, col=counter() }, 3 )
      elseif c2.niveau < 3 then
         alternate( c2, "X", { id="f4", 0.5 }, { id="f3", 1.5, col=counter() }, 3 )
      else
         split( c2, "Y", { id="f2", 0.1, rel=1 }, { id="f5", 0.3 }, { id="f2", 0.1, rel=1 }, { id="f6", 0.3 } )
      end
   end
   for c2 in query( c, "f3" ) do
      split( c2, "Y", { id="f7", 0.2 }, { id="f9", 0.4 }, { id="f7", 0.1 }, { id="f1", 0.1, rel=1 } )
   end
   for c2 in query( c, "f4" ) do
      split( c2, "Y", { id="f8", 0.2 }, { id="f2", 0.4 }, { id="f8", 0.1 }, { id="f2", 0.1, rel=1 } )
   end
   for c2 in query( c, "b1" ) do
      split( c2, "Y", { id="f7", 1.0 }, { id="f9", 0.4 }, { id="f7", 0.1 }, { id="f1", 0.1, rel=1 } )
   end
   for c2 in query( c, "b2" ) do
      split( c2, "Y", { id="f8", 1.0 }, { id="f2", 0.4 }, { id="f8", 0.1 }, { id="f2", 0.1, rel=1 } )
   end

   local s = {}
   for c2 in query( c, "f1", "f9" ) do s[#s+1] = c2 end
   extrude( s, 0.1, { id="ewall", couleur=1 } )
   local s = {}
   for c2 in query( c, "f2" ) do s[#s+1] = c2 end
   extrude( s, 0.2, { id="ewall", couleur=1 } )
   local s = {}
   for c2 in query( c, "f5" ) do s[#s+1] = c2 end
   extrude( s, 0.3, { id="ewall", couleur=1 } )
   local s = {}
   for c2 in query( c, "f6" ) do s[#s+1] = c2 end
   extrude( s, 0.4, { id="ewall", couleur=4 } )
   local s = {}
   for c2 in query( c, "f7" ) do s[#s+1] = c2 end
   extrude( s, 0.15, { id="ewall", couleur=2 } )
   local s = {}
   for c2 in query( c, "f8" ) do s[#s+1] = c2 end
   extrude( s, 0.25, { id="ewall", couleur=2 } )

end

-- Elements architecturaux.
for c in query( "f1" ) do 
   if c.col == 1 and hasFaceID( c, "f" ) then 
      if c.niveau > 0 then region{ c, id="fenetre2" } end
   else
      region{ c, id="fenetre" } 
   end
end

for c in query( "b1" ) do 
   if c.col == 1 and hasFaceID( c, "f" )  then 
      region{ c, id="porte" } 
   end
end

for r in rquery( "fenetre" ) do
   connect( execute( "architecture/window/window04", {h=1.5,w=0.5,d=0.2} ), r, {0.5,0.5,0}, {-0.3,0,-0.1} )
   connect( execute( "architecture/window/window04", {h=1.5,w=0.5,d=0.2} ), r, {0.5,0.5,0}, {0.3,0,-0.1} )
end
for r in rquery( "fenetre2" ) do
   connect( execute( "architecture/window/window01", {h=1.5,w=1.4,d3=0.2,d2=0.1,mat={1,2,2}} ), r, {0.5,0.5,0}, {0,0,-0.1} )
end

for r in rquery( "porte" ) do
   connect( execute( "architecture/door/set01", {door={w=1.0,h=2, o=-0.2}, frame={d=0.3,jamb=0.2,lintel=0.3,mat=2} } ), r, {0.5,0,0}, {0,0.1,-0.1} )
end

--]]

--[[
-- Volumes principales
component{ id="aile", size=vec3(7,12,5), position={-10,0,2}, boundary={{0,0,0},{0,0,1},{1,0,1},{1,0,0}, id={"b","t","s","f","s","b"}} }
component{ id="aile", size=vec3(7,12,5), position={10,0,2}, boundary={{0,0,0},{0,0,1},{1,0,1},{1,0,0}, id={"b","t","s","f","s","b"}} }
component{ id="aile", size=vec3(25,9,15), position={-9,0,-10}, boundary={{0,0,0},{0,0,1},{1,0,1},{1,0,0}, id={"b","t","s","s","s","b"}} }

local s={}
for c in query( "aile" ) do s[#s+1] = c end
merge( s, { id="base" } )


for c in query( "base" ) do
   slice( c, "Y", { id="etage", 3, niveau=counter() } )
end

-- Interieur
for c in query( "etage" ) do
   local s = {}
   for f in fquery( c, "SIDE", "BOTTOM" ) do
      s[#s+1] = component{ c, id="mur", boundary=f }
   end
   extrude( s, -0.1, { id="iwall", couleur=2 } )
end

-- Facades
for c in query( "etage" ) do
   for f in fquery( c, "SIDE" ) do
      component{ c, id="facade", boundary=f }
   end
   for c2 in query( c, "facade" ) do
      if c2.niveau == 0 then
         alternate( c2, "X", { id="b2", 0.5 }, { id="b1", 1.5, col=counter() }, 3 )
      elseif c2.niveau < 3 then
         alternate( c2, "X", { id="f4", 0.5 }, { id="f3", 1.5, col=counter() }, 3 )
      else
         split( c2, "Y", { id="f2", 0.1, rel=1 }, { id="f5", 0.3 }, { id="f2", 0.1, rel=1 }, { id="f6", 0.3 } )
      end
   end
   for c2 in query( c, "f3" ) do
      split( c2, "Y", { id="f7", 0.2 }, { id="f9", 0.4 }, { id="f7", 0.1 }, { id="f1", 0.1, rel=1 } )
   end
   for c2 in query( c, "f4" ) do
      split( c2, "Y", { id="f8", 0.2 }, { id="f2", 0.4 }, { id="f8", 0.1 }, { id="f2", 0.1, rel=1 } )
   end
   for c2 in query( c, "b1" ) do
      split( c2, "Y", { id="f7", 1.0 }, { id="f9", 0.4 }, { id="f7", 0.1 }, { id="f1", 0.1, rel=1 } )
   end
   for c2 in query( c, "b2" ) do
      split( c2, "Y", { id="f8", 1.0 }, { id="f2", 0.4 }, { id="f8", 0.1 }, { id="f2", 0.1, rel=1 } )
   end


   local s = {}
   for c2 in query( c, "f1", "f9" ) do s[#s+1] = c2 end
   extrude( s, 0.1, { id="ewall", couleur=1 } )
   local s = {}
   for c2 in query( c, "f2" ) do s[#s+1] = c2 end
   extrude( s, 0.2, { id="ewall", couleur=1 } )
   local s = {}
   for c2 in query( c, "f5" ) do s[#s+1] = c2 end
   extrude( s, 0.3, { id="ewall", couleur=1 } )
   local s = {}
   for c2 in query( c, "f6" ) do s[#s+1] = c2 end
   extrude( s, 0.4, { id="ewall", couleur=4 } )
   local s = {}
   for c2 in query( c, "f7" ) do s[#s+1] = c2 end
   extrude( s, 0.15, { id="ewall", couleur=2 } )
   local s = {}
   for c2 in query( c, "f8" ) do s[#s+1] = c2 end
   extrude( s, 0.25, { id="ewall", couleur=2 } )

end

-- Elements architecturaux.
for c in query( "f1" ) do 
   if c.col == 1 and hasFaceID( c, "f" ) then 
      if c.niveau > 0 then region{ c, id="fenetre2" } end
   else
      region{ c, id="fenetre" } 
   end
end

for c in query( "b1" ) do 
   if c.col == 1 and hasFaceID( c, "f" )  then 
      region{ c, id="porte" } 
   end
end


for r in rquery( "fenetre" ) do
   connect( execute( "architecture/window/window04", {h=1.5,w=0.5,d=0.2} ), r, {0.5,0.5,0}, {-0.3,0,-0.1} )
   connect( execute( "architecture/window/window04", {h=1.5,w=0.5,d=0.2} ), r, {0.5,0.5,0}, {0.3,0,-0.1} )
end
for r in rquery( "fenetre2" ) do
   connect( execute( "architecture/window/window01", {h=1.5,w=1.4,d3=0.2,d2=0.1,mat={1,2,2}} ), r, {0.5,0.5,0}, {0,0,-0.1} )
end

for r in rquery( "porte" ) do
   connect( execute( "architecture/door/set01", {door={w=1.0,h=2, o=-0.2}, frame={d=0.3,jamb=0.2,lintel=0.3,mat=2} } ), r, {0.5,0,0}, {0,0.1,-0.1} )
end
--]]

-- Geometrie
blocksBegin()
   for c in query( "iwall", "ewall" ) do
      blocks{ c, id=c.couleur }
   end
blocksEnd()

compositeEnd()
