--==============================================================================
-- Simple 3 1/2 apartment
--==============================================================================

local params = ... or {}

local comp = params[1]
if comp == nil then return end
queryBegin( comp )

local Mat = execute( "architecture/common_materials" )

--==============================================================================
-- Room subdivision
--==============================================================================

split( comp, "Z", { id="backApart", rel=3.0 }, { id="middleApart", rel=2.5 }, { id="frontApart", rel=5.5 } )
   
for c in query( "backApart" ) do
   split( c, "X", { id="b1Apart", 1.8 }, { id="kitchen_sub","room", rel=1 } )
end
for c in query( "b1Apart" ) do
   split( c, "Z", { id="kitchen_sub", rel=1 }, { id={"closet","room"}, 1 } )
end
local s={}
for c in query( "kitchen_sub" ) do s[#s+1] = c end
merge( s, {id={"kitchen","room"}, fmat=Mat.FLOOR3} )

for c in query( "middleApart" ) do
   split( c, "X", { id={"entryway","room"}, rel=3.10 }, { id="m1Apart", rel=2.30 } )
end
for c in query( "m1Apart" ) do
   split( c, "Z", { id="m2Apart", rel=1 }, { id="m3Apart", 0.9 } )
end
for c in query( "m2Apart" ) do
   split( c, "Y", { id={"bathroom","room"}, fmat=Mat.FLOOR2, wmat=Mat.WALL2, rel=1 }, { id="empty", 0.3 } )
end
for c in query( "m3Apart" ) do
   split( c, "X", { id={"closet","room","storage"}, rel=0.4, orientation=1 }, { id={"closet","room"}, rel=0.6 } )
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

--==============================================================================
-- Doors
--==============================================================================

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
   for f in fquery( c, "Z" )  do region{ c, f, id="arch" } end
   for f in fquery( c, "-Z" ) do region{ c, f, id="door", rel={{0,0},{0.3,1.2}} } end
   for f in fquery( c, "-X" ) do region{ c, f, id="door" } end
end

--==============================================================================
-- Furniture and Details
--==============================================================================

-- Bath
for c in query( "bathroom" ) do
   split( c, "X", { id="toilet_space", rel=1 }, { id="sink_space", 0.8 }, { id="bath_space", orientation=1, 0.8 } )
end
for c in query( "bath_space" ) do region{ c, id="bath" } end
for r in rquery( "bath" ) do
   local s=r.size
   connect( execute( "furniture/bath/bath01", { w=s[1]-0.1,d=s[3]-0.1} ), r, {0,0,0}, {0.05,0.05,0.05} )
end

for c in query( "sink_space" ) do region{ c, id="sink" } end
for r in rquery( "sink" ) do
   connect( execute( "furniture/sink/sink01", {w=0.5,d=0.3} ), r, {0,0,0}, {0,0.05,0.05} )
end

for c in query( "toilet_space" ) do region{ c, id="toilet" } end
for r in rquery( "toilet" ) do
   connect( execute( "furniture/toilet/toilet01", {w=0.5,d=0.8} ), r, {0,0,0}, {0.2,0.05,0.05} )
end

-- Closets.
for c in query( "storage" ) do slice( c, "Y", { id="partition", 0.4 }, 2 ) end
for c in query( "partition" ) do
   local s={}
   for f in fquery( c, "B" ) do
      s[#s+1] = component{ c, id="shelf_base", boundary=f }
   end
   extrude( s, -0.02, { id="iwall" } )
end

-- Room.
for c in query( "bedroom" ) do
   split( c, "X", { id="empty_space", rel=0.5 }, { id="bed_space", 1.5, orientation=5 }, { id="empty_space", rel=1 } )
end
for c in query( "bed_space" ) do region{ c, id="bed" } end
for r in rquery( "bed" ) do
   connect( execute( "furniture/bed/bed01", {type="single", mh=0.18} ), r, {0,0,0}, {0,0.05,1.3} )
end

-- Living room
for c in query( "livingroom" ) do region{ c, id="couch" } end
for r in rquery( "couch" ) do
   connect( execute( "furniture/couch/couch01", {} ), r, {0,0,0.5}, {1.2,0.05,1.2}, {0.25,{0,1,0}} )
end

-- Kitchen
for c in query( "kitchen" ) do region{ c, id="kitchen_floor" } end
for r in rquery( "kitchen_floor" ) do
   connect( execute( "appliance/range/range01", {} ), r, {1,0,1}, {-0.3,0.05,-0.1}, {0.5,{0,1,0}} )
   connect( execute( "appliance/refrigerator/refrigerator01", {} ), r, {0,0,0}, {0.2,0.05,2.3}, {0.25,{0,1,0}} )
   connect( execute( "furniture/table/table01", {} ), r, {0,0,0}, {3.5,0.05,0.1} )
end


queryEnd()
