--==============================================================================
-- Simple Round Table.
--==============================================================================
local params = ... or {}

-- Table dimensions.
local r = params.r or 1.00
local h = params.h or 0.75

-- Surface height.
local sh = 0.03

-- Leg dimensions.
local lh = 0.75
local ld = 0.05
local l_f = {
   { 0.00*r, 0.50*lh },
   { 0.02*r, 0.45*lh },
   { 0.05*r, 0.25*lh },
   { 0.10*r, 0.15*lh },
   { 0.30*r, 0.12*lh },
   { 0.35*r, 0.01*lh },
}

-- Temps.
local h    = lh + sh
local r2   = r * 2
local ld_2 = ld * 0.5
local lh1  = l_f[1][2]

local c = component{ id="table", size={r2,h,r2}, connectorPosition={0,0,0} }
compositeBegin(c,200)
   -- Surface.
   blocksBegin()
      block{
         { 0,lh, 0},
         {r2, h,r2},
         c=0xf0f, id=1,
      }
   blocksEnd()

   -- Legs.
   local function makeLeg()
      local a = l_f[1]
      local b = l_f[2]
      block{
         {a[1],   0, -ld_2},
         {b[1],   0, -ld_2},
         {a[1], a[2],-ld_2},
         {b[1], b[2],-ld_2},
         {a[1],   0,  ld_2},
         {b[1],   0,  ld_2},
         {a[1], a[2], ld_2},
         {b[1], b[2], ld_2},
         c=0x93f, id=1, g=1,
      }
      for i=3,#l_f-1 do
         a = b
         b = l_f[i]
         block{
            {a[1],   0, -ld_2},
            {b[1],   0, -ld_2},
            {a[1], a[2],-ld_2},
            {b[1], b[2],-ld_2},
            {a[1],   0,  ld_2},
            {b[1],   0,  ld_2},
            {a[1], a[2], ld_2},
            {b[1], b[2], ld_2},
            c=0x00f, id=1, g=1,
         }
      end
      a = b
      b = l_f[#l_f]
      block{
         {a[1],   0, -ld_2},
         {b[1],   0, -ld_2},
         {a[1], a[2],-ld_2},
         {b[1], b[2],-ld_2},
         {a[1],   0,  ld_2},
         {b[1],   0,  ld_2},
         {a[1], a[2], ld_2},
         {b[1], b[2], ld_2},
         c=0x2cf, id=1, g=1,
      }
   end
   blocksBegin()
      -- Center post.
      translate( r, 0, r )
      block{
         {-ld_2, 0 ,-ld_2},
         { ld_2,lh1, ld_2},
         c=0xfff, id=1, g=1,
      }
      block{
         {-ld_2,lh1,-ld_2},
         { ld_2,lh , ld_2},
         c=0xfff, id=1, g=2,
      }
      -- Four leg supports.
      translate(  ld_2, 0, 0 )
      makeLeg()
      translate( -ld_2, 0, 0 )
      rotate( 0.25, 0, 1, 0 )
      translate(  ld_2, 0, 0 )
      makeLeg()
      translate( -ld_2, 0, 0 )
      rotate( 0.25, 0, 1, 0 )
      translate(  ld_2, 0, 0 )
      makeLeg()
      translate( -ld_2, 0, 0 )
      rotate( 0.25, 0, 1, 0 )
      translate(  ld_2, 0, 0 )
      makeLeg()
      attraction( 1, 1 )
   blocksEnd()
compositeEnd()
return c
