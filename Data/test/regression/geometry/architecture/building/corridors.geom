--TODO
-- 1. eviter les chevauffement entre ailes...
-- 4. pieces internes
-- 5. portes
-- 6. escalier.

--==============================================================================
-- 
--==============================================================================

local h        = 3
local hf       = 1
local hr       = 1
local minLevel = 2
local maxLevel = 2
local s        = vec3(70,h,50)
local corsize  = { 3, 2, 2, 2, 2, 2, 2, 2, 2 }
local corlen   = { 40, 20, 20, 10, 10, 10, 10, 10, 10 }
local corlenv  = { 30, 10, 10, 10, 10, 10, 10, 10, 10 }
local roomsize = { 6, 8, 8, 8, 8, 8, 8, 8, 8 }
local maxrooms = 14
local roomlen  = 8
local cord     = 10
local sw       = 4

local outside = false

--local rng      = RNG(565756)
--local rng      = RNG(103)
--local rng2     = RNG(103)

--local rng      = RNG(913986)  -- nice one!
--local rng2     = RNG(2350)

--local rng      = RNG(8139886)  -- nice shape but corridors in windows
--local rng2     = RNG(2381485)

--local rng      = RNG(8251280)  --
--local rng2     = RNG(2551286)

local rng      = RNG(1851075)  -- mini patte du L
local rng2     = RNG(7091285)

--local rng      = RNG(292613)  -- 3 big rect
--local rng2     = RNG(416152)

--local rng      = RNG(7298575)  -- 2 rect
--local rng2     = RNG(5216355)
--n--local rng2     = RNG(6816355)
--n--local rng2     = RNG(3816255)
--n--local rng2     = RNG(4814255)

--local rng      = RNG(8228815)  -- 3 rect
--local rng2     = RNG(4516355)
--n--local rng2     = RNG(4516355)

--local rng      = RNG(192813)  -- 4 rect
--local rng2     = RNG(416352)


--local rng      = RNG(5198575)  -- small Z
--local rng2     = RNG(3216355)

--local rng      = RNG(7538575)  -- bad look T
--local rng2     = RNG(2156355)

--local rng      = RNG(1519875)  -- croix bizarre
--local rng2     = RNG(4119485)
--local rng2     = RNG(1119385)

--local rng      = RNG(251875)  -- L un peu plus large
--local rng2     = RNG(7491485)
--local rng2     = RNG(1401385)

--local rng      = RNG(551875)  -- L3 etroit
--local rng2     = RNG(4149485)

--local rng      = RNG(5573874)  -- ok
--local rng2     = RNG(8449485)

--local rng      = RNG(508987)  -- moyen
--local rng2     = RNG(486287)
--local rng2     = RNG(487786)
--local rng2     = RNG(118496)
--local rng2     = RNG(118994)
--local rng2     = RNG(118805)
--local rng2     = RNG(128105)
--local rng2     = RNG(248935)
--local rng2     = RNG(648835)

--local rng      = RNG(565756) -- ok
--local rng      = RNG(20473521)

--local rng      = RNG(20473521)  -- in U
--local rng2     = RNG(347677)



local rng3     = RNG(1)
-- bed
local bmats={12,13}
-- couch
local cmats={14,15}
-- table
local tmats={16}
-- chair
local chmats={17,18}


local mode = 1

compositeBegin()

    mapping(
      function( IN )
         local p = IN.pos
         local n = abs( IN.fn+vec3(0,0.01,0) )
         if n.x > n.y then
            if n.x > n.z then
               p = p.zy
            else
               p = p.xy
            end
         else
            if n.y > n.z then
               p = p.xz
            else
               p = p.xy
            end
         end
         return p
      end
   )

--==============================================================================
-- Corridor growing
--==============================================================================

   local corridorTag = "corridorML"
   local spaceTag    = "spaceML"

   local numLevel = rng(minLevel,maxLevel)
   local s = vec3( corlen[1]+rng()*corlenv[1], h*numLevel+hf+hr, corsize[1] )
   component{
      id={ "corridor0", corridorTag }, size=s, l=true, r=true, t=true, b=true, numLevel=numLevel
   }
   local ids = { "corridor0", "corridor1", "corridor2" }
   local prio = 1
   for i=1,#ids do
      local corl = corlen[i]
      local cors = corsize[i]
      local corv = corlenv[i]
      for c in query( ids[i] ) do
         local s   = c.size
         local div = {}
         local zt  = max( corlen[i]*rng(), 24 )
         local zb  = max( corlen[i]*rng(), 24 )
         if not c.t then zt = 0 end
         if not c.b then zb = 0 end
         if zb > 0 and rng(2) == 1 then zt = 0 end
         if zt > 0 and rng(2) == 1 then zb = 0 end

         if i < #ids then
            -- bottom
            local v = rng()*corv
            if c.b and rng(2) == 1 then
               div.b = cord + rng()*(s.x-cors-2*cord)
               if zb > 0 then
                  component{ c, id={ corridorTag },
                     size        = vec3( zb, h*c.numLevel+hf+hr, cors ), position = {div.b,0,0}, 
                     orientation = { 0.25, {0,1,0} } 
                  }
               end
               local numLevel = rng(minLevel,maxLevel)
               component{ c, id={ ids[i+1], corridorTag }, l=false, r=true, b=true, t=true, numLevel=numLevel,
                  size        = vec3( corl+v, h*numLevel+hf+hr, cors ), position = {div.b,0,-zb}, 
                  orientation = { 0.25, {0,1,0} } 
               }
            end
            -- top
            local v = rng()*corv
            if c.t and rng(2) == 1 then
               div.t = cord + rng()*(s.x-cors-2*cord)
               if zt > 0 then
                  component{ c, id={ corridorTag },
                     size        = vec3( zt, h*c.numLevel+hf+hr, cors ), position = { cors+div.t, 0, s.z },
                     orientation = { -0.25, {0,1,0} } 
                  }
               end
               local numLevel = rng(minLevel,maxLevel)
               component{ c, id={ ids[i+1], corridorTag }, l=false, r=true, b=true, t=true, numLevel=numLevel,
                  size        = vec3( corl+v, h*numLevel+hf+hr, cors ), position = { cors+div.t, 0, s.z+zt },
                  orientation = { -0.25, {0,1,0} } 
               }
            end
            -- left
            if c.l and rng(2) == 1 then
               local numLevel = rng(minLevel,maxLevel)
               component{ c, id={ ids[i+1], corridorTag }, l=true, r=true, b=false, t=true, numLevel=numLevel,
                  size        = vec3( corl+v, h*numLevel+hf+hr, cors ), position = {0,0,-rng()*((corl+v)-s.z)}, 
                  orientation = { -0.25, {0,1,0} } 
               }
               div.l = true
            end
            -- right
            if c.r and rng(2) == 1 then
               local numLevel = rng(minLevel,maxLevel)
               component{ c, id={ ids[i+1], corridorTag }, l=true, r=true, b=true, t=false, numLevel=numLevel,
                  size        = vec3( corl+v, h*numLevel+hf+hr, cors ), position = {s.x+cors,0,-rng()*((corl+v)-s.z)}, 
                  orientation = {-0.25, {0,1,0} } 
               }
               div.r = true
            end
         end
         -- Rooms
         local h = h*c.numLevel+hf+hr
         if zt > 0 then
            if div.t ~= nil then
               local c0 = component{ c, id=spaceTag, position={0,0,s.z}, size={div.t,h,zt} }
               local c1 = component{ c, id=spaceTag, position={div.t+cors,0,s.z}, size={s.x-div.t-cors,h,zt} }
               for c in nquery( c0, -0.1, spaceTag ) do c0.invalid = true end
               for c in nquery( c1, -0.1, spaceTag ) do c1.invalid = true end
            else
               local c0 = component{ c, id=spaceTag, position={0,0,s.z}, size={s.x,h,zt} }
               for c in nquery( c0, -0.1, spaceTag ) do c0.invalid = true end
            end
            component{ c, id="staircase", door="-Z", wallmat=2, position={2,0,s.z}, size={sw,h-hr,sw}, prio=100 }
         end
         if zb > 0 then
            if div.b ~= nil then
               local c0 = component{ c, id=spaceTag, position={0,0,-zb}, size={div.b,h,zb} }
               local c1 = component{ c, id=spaceTag, position={div.b+cors,0,-zb}, size={s.x-div.b-cors,h,zb} }
               for c in nquery( c0, -0.1, spaceTag ) do c0.invalid = true end
               for c in nquery( c1, -0.1, spaceTag ) do c1.invalid = true end
            else
               local c0 = component{ c, id=spaceTag, position={0,0,-zb}, size={s.x,h,zb} }
               for c in nquery( c0, -0.1, spaceTag ) do c0.invalid = true end
            end
            component{ c, id="staircase", door="Z", wallmat=2, position={2,0,-sw}, size={sw,h-hr,sw}, prio=100 }
         end
         prio = prio+1
      end
   end

--==============================================================================
-- Levels creation
--==============================================================================

   -- split main corridors and spaces.
   for c in query( "corridorML" ) do
      split( c, "Y", { id="corridor", h+hf, level=1 }, { id="corridorML1", rel=1 }, { id="roofsec", hr } )
   end
   for c in query( "corridorML1" ) do
      slice( c, "Y", { id="corridor", h, level=counter(2) } )
   end
   for c in query( "spaceML" ) do
      --if c.invalid == nil then
         split( c, "Y", { id="spacebc", h+hf, level=1 }, { id="spaceML1", rel=1 }, { id="roofsec", hr } )
      --end
   end
   for c in query( "spaceML1" ) do
      slice( c, "Y", { id="spacebc", h, level=counter(2) } )
   end

   -- Cut with priorities.
   for c in query( "spacebc" ) do
      subtract( c, "prio", { id="space0" } )
   end

   -- Create level volume.
   local levels = {}
   for c in query( "corridor", "spacebc" ) do
      local l = c.level
      if levels[l] == nil then levels[l] = {} end
      local s = levels[l]
      s[#s+1] = c
   end
   for k,v in ipairs( levels ) do
      merge( v, { id="level", level=k, prio=20 } )
   end

   -- Create roof volume.
   local s = {}
   for c in query( "roofsec" ) do s[#s+1] = c end
   merge( s, { id="roof" } )

   -- Split staircases.
   for c in query( "staircase" ) do
      split( c, "Y", { id="staircaseLevel", h+hf, level=1 }, { id="stlML", rel=1 } )
   end
   for c in query( "stlML" ) do
      slice( c, "Y", { id="staircaseLevel", h, level=counter(2) } )
   end
   for c in query( "staircaseLevel" ) do
      for f in fquery( c, c.door ) do region{ c, f, id="frame" } end
   end
   for c in query( "staircase" ) do
      execute( "architecture/stair/staircase01", { mat={2,2,2,2}, c, ldd=1,ew=0.2,sth=1} )
   end

--==============================================================================
-- Facade
--==============================================================================

   local function faceExtrude( id, d, nid )
      local s={}
      for c in query( id ) do s[#s+1] = c end
      if nid == nil then nid = "facade3d" end
      extrude( s, d, { id=nid } )
   end

   for c in query( "corridor" ) do
      for f in fquery( c, "SIDE" ) do
         local nc = component{ c, boundary=f, id="corwall" }
      end
   end

   for c in query( "level" ) do
      queryBegin( c )
      -- Face creation.
      for f in fquery( c, "SIDE" ) do
         component{ c, boundary=f, id="facade2d", mat=5 }
      end

      -- Splitting sections.
      for c in query( "facade2d" ) do
         if c.level == 1 then --or c.level == 3 then
            split( c, "Y", { id="fA", rel=1 }, { id="ledge", 0.3 } )
         else
            component{ c, id="fA" }
         end
      end
      for c in query( "fA" ) do
         slice( c, "X", { id="fB", 4 }, 0 ) 
      end
      for c in query( "fB" ) do
         local dm = 6
         if c.level == 1 then dm = 5 end
         split( c, "X", { id="fC", 0.5 }, { id="fD", rel=1, mat=dm }, { id="fC", 0.5 } )
      end
      for c2 in query( "fD" ) do
         --[[
         local ok = true
         queryEnd()
         for c3 in nquery( c2, 0.1, "corwall" ) do
            ok = false
         end
         queryBegin(c)
         if ok then region{ c2, id="window" } end
         --]]
         --[[
         queryEnd()
         local occ = occlusion( c2, "corwall" )
         if  occ > 0.99 or occ < 0.01 then region{ c2, id="window" } end
         queryBegin(c)
         --]]
         region{ c2, id="window" }
      end

      -- Extrusion.
      faceExtrude( "fC", 0.4 )
      faceExtrude( "fD", 0.1 )
      faceExtrude( "ledge", 0.5 )
      queryEnd()
   end

   for c in query( "roof" ) do
      queryBegin(c)
      for f in fquery( c, "SIDE" ) do
         component{ c, boundary=f, id="rfacade2d", mat=6 }
      end
      for c in query( "rfacade2d" ) do
         split( c, "Y", { id="rfA", rel=1 }, { id="rfB", rel=1 } )
      end
      faceExtrude( "rfA", 0.3, "facade3d" )
      faceExtrude( "rfB", 0.5, "facade3d" )
      --for c in query( "rfacade3d" ) do
      --   subtract( c, "prio", { id="facade3d" } )
      --end
      if outsize then
         local s={}
         for f in fquery( c, "B" ) do
            s[#s+1] = component{ c, boundary=f, id="roof2d" }
         end
         extrude( s, -0.1, { id="facade3d", mat=2 } )
      end
      queryEnd()
   end

--==============================================================================
-- Windows and exteriors doors
--==============================================================================

   for r in rquery( "window" ) do
      connect( execute( "architecture/window/window03", { w=1,h=2,d=0.1} ), r, {0.5,0.5,0}, {0,0,-0.05} )
   end

--==============================================================================
-- Volume subdivision.
--==============================================================================

   local function findConstraints( c )
      local cs = {}
      for c in nquery( c, "window" ) do
         cs[#cs+1] = volumeConstraint{ c, offset={0.2,0,0}, repulse=true }
      end
      return cs
   end

   if mode == 1 then

   local corridorTag = "corridor"
   local roomTag     = "rooms"
   local ids = { "space0", "space1", "space2", "space3", "space4", "space5", "space6", "space7" }

   -- Creating corridors.
   for i=1,#ids do
      local cors  = corsize[i]
      local rooms = roomsize[i]
      for c in query( ids[i] ) do
         -- split rules.
         local size = c.size.xz
         local hvs  = floor((size-rooms)/(cors+rooms))
         -- Can we split?
         -- HACK!!
         if i==#ids or ( hvs.x <= 0.5 and hvs.y <= 0.5 ) or c.level < c.numLevel then
            if ( max(size.x, size.y) > maxrooms) and c.level == c.numLevel then
               if size.x > size.y then
                  split( c, "X", { id=roomTag, rel=1 }, { id=roomTag, rel=1 } )
               else
                  split( c, "Z", { id=roomTag, rel=1 }, { id=roomTag, rel=1 } )
               end
            else
               component{ c, id=roomTag }
            end
         else
            -- Can we do an O?
            if hvs.x >= 2 and hvs.y >= 2 and rng2(4) == 1 then
               c.pattern = "o"
               split( c, "Z", { id=ids[i+1], rooms }, { id="tmpspace0", rel=1 }, { id=ids[i+1], rooms } )
               queryBegin(c)
               for c in query( "tmpspace0" ) do
                  split( c, "X", { id=ids[i+1], rooms }, { id=corridorTag, cors, nh=true }, { id="tmpspace1", rel=1 }, { id=corridorTag, cors, nh=true }, { id=ids[i+1], rooms } )
               end
               for c in query( "tmpspace1" ) do
                  split( c, "Z", { id=corridorTag, cors }, { id=ids[i+1], rel=1 }, { id=corridorTag, cors } )
               end
               queryEnd()
            -- Can we split h?
            elseif hvs.y <= 0.5 or ( hvs.x > 0.5 and rng2(2) == 1 ) then
               -- Find constraints.
               local cs = findConstraints(c)
               -- Which pattern?
               if c.pattern == "h" and hvs.y <= 0.5 and hvs.x < 3 then
                  slice( c, "X", { id=roomTag, roomlen }, 2, cs )
               else
                  local a = rng2()
                  c.pattern = "h"
                  split( c, "X", { id=ids[i+1], rel=a, rooms }, { id=corridorTag, cors }, { id=ids[i+1], rel=1-a, rooms }, cs )
               end
            else
               -- Find constraints.
               local cs = findConstraints(c)
               -- Which pattern?
               if c.pattern == "v" and hvs.x <= 0.5 and hvs.y < 3 then
                  slice( c, "X", { id=roomTag, roomlen }, 2, cs )
               else
                  local a = rng2()
                  c.pattern = "v"
                  split( c, "Z", { id=ids[i+1], rel=a, rooms }, { id=corridorTag, cors }, { id=ids[i+1], rel=1-a, rooms }, cs )
               end
            end
         end
      end
   end

   end

--==============================================================================
-- Furnitures
--==============================================================================

local function blob( size, mid, cp )
   if cp == nil then cp = vec3( 0.5,0,0.5 )*size else cp = cp*size end
   local c = component{ id="furniture", size=size, connectorPosition=cp }
   compositeBegin(c,200)
      blocksBegin()
         block{ {0,0,0}, size, c=0xfff, id=mid }
      blocksEnd()
   compositeEnd()
   return c
end

--==============================================================================
-- APPARTMENT/ROOMS
--==============================================================================

   local function maxComp( t )
      local v=0
      local m=1
      for i = 1, #t do
         if t[i] > v then
            v = t[i]
            m = i
         end
      end
      return m
   end

   -- Rooms patterns.
   --===========================================================================
   local function rooms01( c, cs )
      queryBegin(c )
         c.doortype = "door"
         split( c, "Z", { id="rA", rel=1 }, { id={"living","lroom"}, 3 }, cs )
         for c in query( "rA" ) do
            split( c, "X", { id="rB", 2 }, { id="hall", 1.5 }, { id="rC", rel=1 } )
         end
         for c in query( "rB" ) do
            split( c, "Z", { id="entry", 2 }, { id="rD", rel=1 }, cs )
         end
         for c in query( "rC" ) do
            slice( c, "Z", { id={"room","broom"}, door="-X",3 }, 2, cs )
         end
         for c in query( "rD" ) do
            if c.size.z >= 6 then
               split( c, "Z", { id="room", door="-Z", 1 }, { id="room", door="X", 2, floormat=7, wallmat=10 }, { id="room", floormat=9, door="X", doortype="frame", 5 }, { id="living", rel=1 }, cs )
            else
               split( c, "Z", { id="room", door="-Z", 1 }, { id="room", door="X", 2, floormat=7, wallmat=10 }, { id="living", rel=1 }, cs )
            end
         end
         for c in query( "entry" ) do
            for f in fquery( c, "-Z" ) do region{ c, f, id=c.doortype } end
         end
         for c in query( "room" ) do
            for f in fquery( c, c.door ) do region{ c, f, id=c.doortype } end
         end
         local s={}
         for c in query( "living", "hall", "entry" ) do
            s[#s+1] = c
         end
         merge( s, { id="room" } )
         -- Furniture.
         for c in query( "broom" ) do
            local r = region{ c, id="furniture" }
            -- bed
            local bm = rng3.pick( bmats )
            connect( blob( vec3(2,1,1), bm, vec3(1,0,0.5) ), r, {1,0,0.5},{-0.1,0.06,0} )
         end
         for c in query( "lroom" ) do
            local r = region{ c, id="furniture" }
            -- couch
            local cm = rng3.pick( cmats )
            connect( blob( vec3(2.5,0.6,1), cm, vec3(1,0,0) ), r, {1,0,0},{-1,0.06,0.1} )
            connect( blob( vec3(2.5,0.6,1), cm, vec3(1,0,1) ), r, {1,0,1},{-1,0.06,-0.1} )

            local o = 0
            if c.size.z < 10 then o = 1 end
            -- Table
            local tm = rng3.pick( tmats )
            connect( blob( vec3(1.2,0.5,2), tm, vec3(0.5,0,0.5) ), r, {0,0,1},{1.5,0.06,-2.5+o} )
            -- chairs
            local cm = rng3.pick( chmats )
            connect( blob( vec3(0.4,1,0.4), cm, vec3(0.5,0,0.5) ), r, {0,0,1},{0.65,0.06,-2.0+o} )
            connect( blob( vec3(0.4,1,0.4), cm, vec3(0.5,0,0.5) ), r, {0,0,1},{2.35,0.06,-2.0+o} )
            connect( blob( vec3(0.4,1,0.4), cm, vec3(0.5,0,0.5) ), r, {0,0,1},{0.65,0.06,-3.0+o} )
            connect( blob( vec3(0.4,1,0.4), cm, vec3(0.5,0,0.5) ), r, {0,0,1},{2.35,0.06,-3.0+o} )
         end
      queryEnd()
   end
   --===========================================================================
   local function rooms02( c, cs )
      queryBegin(c)
         local s = c.size
         if max( s.x, s.z ) < 4 then
            component{ c, id="room", door="-Z", docc="corridor" }
         elseif s.z > s.x then
            split( c, "Z", { id="room", door="-Z", docc="corridor", rel=1 }, { id="room", door="-Z", docc="room", rel=1 }, cs )
         else
            split( c, "X", { id="room", door="-Z", docc="corridor", rel=1 }, { id="room", door="-X", docc="room", rel=1 }, cs )
         end
         for c in query( "room" ) do
            for f in fquery( c, c.door ) do
               queryEnd()
               local nc = component{ c, boundary=f, id="testWall" }
               if occlusion( nc, 0.1, "staircase" ) < 0.01 then region{ c, f, id="door" } end
               queryBegin(c)
            end
         end
      queryEnd()
   end

   --===========================================================================
   local function rooms03( c, cs )
      queryBegin(c)
         c.doortype = "door"
         split( c, "Z", { id="rA", rel=1 }, { id="living", 3 }, cs )
         for c in query( "rA" ) do
            split( c, "X", { id="hall", 1.5 }, { id="rB", rel=1 } )
         end
         for c in query( "rB" ) do
            split( c, "Z", { id="entry", 2 }, { id="rD", rel=1 }, cs )
         end
         for c in query( "rD" ) do
            if c.size.z >= 6 then
               split( c, "Z", { id="room", door="-Z", 1 }, { id="room", door="-X", 2, floormat=7, wallmat=10 }, { id="room", floormat=9, door="-X", doortype="frame", 5 }, { id="living", rel=1 }, cs )
            else
               split( c, "Z", { id="room", door="-Z", 1 }, { id="room", door="-X", 2, floormat=7, wallmat=10 }, { id="living", rel=1 }, cs )
            end
         end
         local s={}
         for c in query( "living", "hall", "entry" ) do
            s[#s+1] = c
         end
         for c in query( "entry" ) do
            for f in fquery( c, "-Z" ) do region{ c, f, id=c.doortype } end
         end
         for c in query( "room" ) do
            for f in fquery( c, c.door ) do region{ c, f, id=c.doortype } end
         end
         merge( s, { id="room" } )
      queryEnd()
   end

   --===========================================================================
   local function rooms04( c, cs )
      local s = c.size
      if s.x < s.z then return rooms03( c, cs ) end
      queryBegin(c)
         split( c, "X", { id={"room","br1"}, 3, rel=2, door="X" }, { id="rA", door="-Z", 2, rel=1 }, { id={"room","br2"}, 3, rel=2, door="-X" }, cs )
         for c in query( "rA" ) do
            split( c, "Z", { id="room", rel=1 }, { id="room", 2, floormat=7 } )
         end
         for c in query( "room" ) do
            for f in fquery( c, c.door ) do region{ c, f, id="door" } end
         end
         -- Furniture.
         for c in query( "br1" ) do
            local r = region{ c, id="furniture" }
            -- bed
            local bm = rng3.pick( bmats )
            connect( blob( vec3(2,1,1), bm, vec3(0,0,0) ), r, {0,0,0},{0.2,0.06,0.2} )
            -- couch
            local cm = rng3.pick( cmats )
            connect( blob( vec3(2,1,1), cm, vec3(0.5,0,1) ), r, {0.5,0,1},{0.0,0.06,-0.5} )
         end
         for c in query( "br2" ) do
            if c.size.x > 4 then
               local r = region{ c, id="furniture" }
               -- Table
               local tm = rng3.pick( tmats )
               connect( blob( vec3(1.2,0.5,2), tm, vec3(0.5,0,0.5) ), r, {0.5,0,0},{0,0.06,2.5} )
               -- chairs
               local cm = rng3.pick( chmats )
               connect( blob( vec3(0.4,1,0.4), cm, vec3(0.5,0,0.5) ), r, {0.5,0,0},{-0.85,0.06,2.0} )
               connect( blob( vec3(0.4,1,0.4), cm, vec3(0.5,0,0.5) ), r, {0.5,0,0},{0.85,0.06,2.0} )
               connect( blob( vec3(0.4,1,0.4), cm, vec3(0.5,0,0.5) ), r, {0.5,0,0},{-0.85,0.06,3.0} )
               connect( blob( vec3(0.4,1,0.4), cm, vec3(0.5,0,0.5) ), r, {0.5,0,0},{0.85,0.06,3.0} )
            end
         end
      queryEnd()
   end
   --===========================================================================
   local function rooms05( c, cs )
      queryBegin(c)
         c.doortype = "door"
         split( c, "Z", { id="rA", rel=1 }, { id="rB", rel=1 }, cs )
         for c in query( "rA" ) do
            split( c, "X", { id="rC", rel=1 }, { id="roombc", door="-X", rel=1 } )
         end
         for c in query( "rB" ) do
            split( c, "X", { id={"roombc","br1"}, door="X", rel=1 }, { id={"roombc","br2"}, door="-X", rel=1 }, cs )
         end
         for c in query( "rC" ) do
            split( c, "Z", { id="roombc", 3, door="X", entry=true, floormat=7, wallmat=10 }, { id="roombc", rel=1, floormat=9, door="X", doortype="frame" }, cs )
         end

         local size = c.size
         component{ c, id="living", position={2,0,3}, size={size.x-4,size.y,size.z*0.5}, prio=10 }
         component{ c, id="entry", position={(size.x-1.5)/2,0,0}, size={1.5,size.y,3}, prio=10 }
         for c in query( "roombc" ) do
            subtract( c, "prio", { id="room" } )
         end
         local s={}
         for c in query( "living", "entry" ) do s[#s+1]=c end
         merge( s, { id="room" } )
         for c in query( "entry" ) do
            for f in fquery( c, "-Z" ) do region{ c, f, id="door" } end
         end
         for c in query( "room" ) do
            if c.door then
               for f in fquery( c, c.door ) do 
                  local nc = component{ c, id="rtmp", boundary=f }
                  if c.entry or (occlusion( nc, "living" ) > 0.5) then region{ c, f, id=c.doortype } end
               end
            end
         end
         -- Furniture.
         for c in query( "br1" ) do
            local r = region{ c, id="furniture" }
            -- bed
            local bm = rng3.pick( bmats )
            connect( blob( vec3(2,1,1), bm, vec3(1,0,1) ), r, {1,0,1},{-0.2,0.06,-0.2} )
         end
         for c in query( "br2" ) do
            local r = region{ c, id="furniture" }
            -- bed
            local bm = rng3.pick( bmats )
            connect( blob( vec3(2,1,1), bm, vec3(0,0,1) ), r, {0,0,1},{0.2,0.06,-0.2} )
         end
         for c in query( "living" ) do
            local r = region{ c, id="furniture" }
            -- couch
            local cm = rng3.pick( cmats )
            connect( blob( vec3(2.5,0.6,1), cm, vec3(0.5,0,1) ), r, {0.5,0,1},{0,0.06,-0.3} )
            -- Table
            local tm = rng3.pick( tmats )
            connect( blob( vec3(1.2,0.5,2), tm, vec3(0.5,0,0.5) ), r, {0.5,0,0},{0,0.06,2.5} )
            -- Chairs
            local cm = rng3.pick( chmats )
            connect( blob( vec3(0.4,1,0.4), cm, vec3(0.5,0,0.5) ), r, {0.5,0,0},{-0.85,0.06,2.0} )
            connect( blob( vec3(0.4,1,0.4), cm, vec3(0.5,0,0.5) ), r, {0.5,0,0},{0.85,0.06,2.0} )
            connect( blob( vec3(0.4,1,0.4), cm, vec3(0.5,0,0.5) ), r, {0.5,0,0},{-0.85,0.06,3.0} )
            connect( blob( vec3(0.4,1,0.4), cm, vec3(0.5,0,0.5) ), r, {0.5,0,0},{0.85,0.06,3.0} )
         end
      queryEnd()
   end
   --===========================================================================
   local function rooms06( c, cs )
      queryBegin(c)
         component{ c, id="room", door="-Z" }
         for c in query( "room" ) do
            for f in fquery( c, c.door ) do region{ c, f, id="door" } end
         end
      queryEnd()
   end

   -- Selecting/creating rooms.
   for c in query( "rooms" ) do
      -- Check for staircase.
      for n in nquery( c, "staircase" ) do c.stc = true end
      -- Check for corridors positions.
      local cor = {}
      for f in fquery( c, "-Z" ) do
         local nc = component{ c, boundary=f, id="roomWall" }
         cor[1] = occlusion( nc, "corridor" )
      end
      for f in fquery( c, "Z" ) do
         local nc = component{ c, boundary=f, id="roomWall" }
         cor[2] = occlusion( nc, "corridor" )
      end
      for f in fquery( c, "-X" ) do
         local nc = component{ c, boundary=f, id="roomWall" }
         cor[3] = occlusion( nc, "corridor" )
      end
      for f in fquery( c, "X" ) do
         local nc = component{ c, boundary=f, id="roomWall" }
         cor[4] = occlusion( nc, "corridor" )
      end
      -- Windows constraints.
      local cs = findConstraints(c)
      local lc = c
      local mc = maxComp( cor )
      if mc ~= 1 then
         if mc == 2 then      lc = component{ c, id="rotRoom", orientation=5 }
         elseif mc == 3 then  lc = component{ c, id="rotRoom", orientation=4 }
         else                 lc = component{ c, id="rotRoom", orientation=1 }
         end
      end
      
      if c.stc then
         rooms02(lc,cs)
      else
         local s = lc.size
         if min( s.x, s.z ) < 7 then
            rooms04(lc,cs)
         else
            if rng2(3) == 1 and s.z > 10 then
               rooms05(lc,cs)
            else
               rooms01(lc,cs)
            end
         end
      end
   end

--==============================================================================
-- Create walls.
--==============================================================================

   local str = { "room", "space0" }

   for c in query( "corridor" ) do
      c.floormat = 1
      c.wallmat  = 2
   end
   for c in query( "room" ) do
      if not c.floormat then c.floormat = 3 end
      if not c.wallmat  then c.wallmat  = 4 end
   end

   -- Walls
   for c in query( "corridor", "staircase", str[mode] ) do
   --for c in query( "room", "corridor" ) do
      local s = {}
      for f in fquery( c, "SIDE" ) do
         local nc = component{ c, id="wall2d", boundary=f, mat=c.wallmat }
         s[#s+1] = nc
         if hasParentID( nc, "corridor" ) and occlusion( nc, "corridor" ) > 0.9 then
            region{ c, f, id="hole" }
         end
      end
      for f in fquery( c, "B" ) do
         s[#s+1] = component{ c, id="floor2d", boundary=f, mat=c.floormat }
      end
      extrude( s, -0.05, { id="wall3d" } )
   end

   -- Geometry.
   blocksBegin()
      for c in query( "wall3d" ) do blocks{ c, id=c.mat } end
   blocksEnd()

   blocksBegin()
      for c in query( "facade3d" ) do blocks{ c, id=c.mat } end
   blocksEnd()

--==============================================================================
-- "Connect" corridors and doors
--==============================================================================

if not outside then

   for r in rquery( "hole" ) do
      local s = r.size
      connect( execute( "architecture/door/hole", {w=s.x-0.1,h=s.y-0.05,d=0.1,mat=1} ), r, {0.5,0,0}, {0,0.05,-0.05} )
   end

   for r in rquery( "door" ) do
      --connect( execute( "architecture/door/frame02", {d=0.15,mat=8} ), r, {0.5,0,0}, {0,0.05,-0.05} )
      connect( execute( "architecture/door/set01", { door={mat=11,o=-0.1}, frame={d=0.15,hmat=3,mat=8} } ), r, {0.5,0,0}, {0,0.05,-0.075} )
   end

   for r in rquery( "frame" ) do
      local s = r.size
      connect( execute( "architecture/door/frame02", {w=s.x-0.2, h=s.y-0.2, d=0.15,mat=8, hmat=r.component.floormat} ), r, {0.5,0,0},{0,0.05,-0.075} )
   end
end
compositeEnd()

