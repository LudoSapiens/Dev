--==============================================================================
-- Simple Table.
--==============================================================================
local params = ... or {}

local Mat = execute( "architecture/common_materials" )

-- Table dimensions.
local w = params.w or 1.50
local h = params.h or 0.75
local d = params.d or 0.90

-- Surface height.
local sh = params.sh or 0.07

-- Materials.
local mat  = params.mat or { Mat.FURNITURE1, Mat.FURNITURE1 }
local mat1 = mat[1]
local mat2 = mat[2]

-- Leg dimensions.
local lw1 = 0.05  -- Width of the leg on the floor.
local lw2 = 0.10  -- Width of the leg right under the surface.
local lh  = h - sh
local ld1 = lw1   -- Depth of the leg on the floor.
local ld2 = lw2   -- Depth of the leg on the floor.
local lx  = 0.05  -- Offset from the corner of the table.
local ly  = lx

-- Temps.
local lw1_2 = lw1 * 0.5
local lw2_2 = lw2 * 0.5
local ld1_2 = ld1 * 0.5
local ld2_2 = ld2 * 0.5
local loffx = lx + lw2_2
local loffy = ly + ld2_2

local c = component{ id="table", size={w,h,d}, connectorPosition={0,0,0} }
compositeBegin(c,200)
   -- Surface.
   blocksBegin()
      block{
         {0,h-sh,0},
         {w,h   ,d},
         c=0xfff, id=mat2
      }
   blocksEnd()

   -- Legs.
   local function makeLeg( pos )
      scopeBegin()
         translate( pos[1], pos[2], pos[3] )
         block{
            {-lw1_2,  0,-ld1_2 },
            { lw1_2,  0,-ld1_2 },
            {-lw2_2, lh,-ld2_2 },
            { lw2_2, lh,-ld2_2 },
            {-lw1_2,  0, ld1_2 },
            { lw1_2,  0, ld1_2 },
            {-lw2_2, lh, ld2_2 },
            { lw2_2, lh, ld2_2 },
            c=0xfff, id=mat1
         }
      scopeEnd()
   end
   blocksBegin()
      makeLeg( {  loffx, 0,   loffy} )
      makeLeg( {w-loffx, 0,   loffy} )
      makeLeg( {w-loffx, 0, d-loffy} )
      makeLeg( {  loffx, 0, d-loffy} )
   blocksEnd()
compositeEnd()
return c
