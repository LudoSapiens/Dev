--==============================================================================
-- Simple Refrigerator.
--==============================================================================
local params = ... or {}

local Mat = execute( "architecture/common_materials" )

-- Dimensions.
local w  = params.w  or 0.85
local h  = params.h  or 1.70
local d  = params.d  or 0.85
local dd = params.dd or 0.04
local de = params.de or 0.01
local r  = params.r  or 0.35
local rs = params.rs or 0.02
-- Type: top, bottom, left, right.
local t  = params.t  or "top"
-- Door type: square, round.
local dt = params.dt or "square"

-- Materials.
local mat  = params.mat or { Mat.METAL1, Mat.METAL3 }
local mat1 = mat[1]
local mat2 = mat[2]

local z1 = 0
local z2 = d - dd
local z3 = d

local c = component{ id="refrigerator", size={w,h,d}, connectorPosition={0,0,0} }
compositeBegin(c,200)
   blocksBegin()
      block{
         {0,0,z1},
         {w,h,z2},
         c=0xFFF, id=mat2
      }
   blocksEnd()

   local front = component{ c, id="refrigerator_front", size={w,h}, position={0,0,z2} }

   local typeSplit = {
      bottom = { "Y", { id="freezer", rel=r }, { id="space", rs }, { id="fresh", rel=1-r } },
      top    = { "Y", { id="fresh", rel=1-r }, { id="space", rs }, { id="freezer", rel=r } },
      left   = { "X", { id="freezer", rel=r }, { id="space", rs }, { id="fresh", rel=1-r } },
      right  = { "X", { id="fresh", rel=1-r }, { id="space", rs }, { id="freezer", rel=r } },
   }
   -- Vertical split.
   for f in query( "refrigerator_front" ) do
      split( f, unpack(typeSplit[t]) )
   end

   for f in query( "freezer", "fresh" ) do
      -- Normally, we'd create separate components, but we only need geometry.
      extrude( f, dd+de, { id="refrigerator_door"} )
   end

   local dpt = {
      square={ c=0xFFF, s=0x000000 },
      round={ c=0xF00, s=0x555555 },
   }
   local dp = dpt[dt]
   blocksBegin()
      for c in query( "refrigerator_door" ) do
         local p = c.position
         local s = c.size
         local t = { p[1]+s[1], p[2]+s[2], p[3]+s[3] }
         -- Actual door.
         p[3] = p[3] + de
         block{
            p,
            t,
            c=dp.c, s=dp.s, id=mat2
         }
         -- Rubber seal.
         p[1] = p[1] + de
         p[2] = p[2] + de
         p[3] = p[3] - de
         t[1] = t[1] - de
         t[2] = t[2] - de
         t[3] = t[3] - dd*0.5
         block{
            p,
            t,
            c=0xFFF, s=0x000000, id=mat1
         }
      end
   blocksEnd()
compositeEnd()
return c
