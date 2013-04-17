--==============================================================================
-- Simple Chair.
--==============================================================================
-- A small chair with 4 legs.
local params = ... or {}

-- Legs dimensions.
local lw = params.lw or 0.05
local lh = params.lh or 0.45
local ld = params.ld or lw
-- Seat dimensions.
local sw = params.sw or 0.50
local sh = params.sh or 0.05
local sd = params.sd or 0.40
-- Back dimensions.
local bw = params.bw or sw
local bh = params.bh or 0.50
local bd = params.bd or 0.02
local bf = params.bf or 0.6
local bz = params.bz or 0.05

local w = sw
local h = lh + sh + bh
local d = sd

local lw_2 = lw * 0.5
local ld_2 = ld * 0.5
local bh1  = bh * bf
local bzf  = bz * bf

local c = component{ id="chair", size={w,h,d}, connectorPosition={0,0,0} }
compositeBegin(c,200)
   -- Legs.
   blocksBegin()
      block{
         { 0, 0, 0},
         {lw,lh,ld},
         c=0xfff, id=1,
      }
      translate( sw - lw, 0, 0 )
      block{
         { 0, 0, 0},
         {lw,lh,ld},
         c=0xfff, id=1,
      }
      translate( 0, 0, sd - ld )
      block{
         { 0, 0, 0},
         {lw,lh,ld},
         c=0xfff, id=1,
      }
      translate( -(sw - lw), 0, 0 )
      block{
         { 0, 0, 0},
         {lw,lh,ld},
         c=0xfff, id=1,
      }
   blocksEnd()

   translate( 0, lh, 0 )

   -- Seat.
   blocksBegin()
      block{
         { 0, 0, 0},
         {sw,sh,sd},
         c=0xfff, id=1,
      }
   blocksEnd()

   -- Back.
   blocksBegin()
      block{
         { 0,sh, 0   },
         {lw,sh, 0   },
         { 0,bh, 0-bz},
         {lw,bh, 0-bz},
         { 0,sh,ld   },
         {lw,sh,ld   },
         { 0,bh,ld-bz},
         {lw,bh,ld-bz},
         c=0xfff, id=1,
      }
      block{
         {sw-lw,sh, 0   },
         {sw   ,sh, 0   },
         {sw-lw,bh, 0-bz},
         {sw   ,bh, 0-bz},
         {sw-lw,sh,ld   },
         {sw   ,sh,ld   },
         {sw-lw,bh,ld-bz},
         {sw   ,bh,ld-bz},
         c=0xfff, id=1,
      }
      translate( 0, 0, (ld-bd)*0.5 )
      block{
         { 0,bh1, 0-bzf},
         {bw,bh1, 0-bzf},
         { 0,bh , 0-bz },
         {bw,bh , 0-bz },
         { 0,bh1,bd-bzf},
         {bw,bh1,bd-bzf},
         { 0,bh ,bd-bz },
         {bw,bh ,bd-bz },
         c=0xfff, id=1,
      }
   blocksEnd()
compositeEnd()
return c
