--==============================================================================
-- Simple Chair.
--==============================================================================
-- A small chair with 4 legs.
-- Dimensions come from:
--   http://www.woodbin.com/ref/furniture/chairs.htm
local params = ... or {}

-- Legs dimensions.
local lw = params.lw or 0.05
local lh = params.lh or 0.45
local ld = params.ld or lw
-- Seat dimensions.
local sw = params.sw or 0.45
local sh = params.sh or 0.08
local sd = params.sd or 0.40
-- Back dimensions.
local bw = params.bw or sw
local bh = params.bh or 0.50
local bd = params.bd or 0.07
local bf = params.bf or 0.4

local w = sw
local h = lh + sh + bh
local d = sd

local lw_2 = lw * 0.5
local ld_2 = ld * 0.5
local bh1  = bh * bf
local lh1  = lh + sh + bh1

local c = component{ id="chair", size={w,h,d}, connectorPosition={0,0,0} }
compositeBegin(c,200)
   -- Legs.
   blocksBegin()
      block{
         { 0, 0, 0},
         {lw,lh1,ld},
         c=0xf0f, id=1,
      }
      translate( sw - lw, 0, 0 )
      block{
         { 0, 0, 0},
         {lw,lh1,ld},
         c=0xf0f, id=1,
      }
      translate( 0, 0, sd - ld )
      block{
         { 0, 0, 0},
         {lw,lh,ld},
         c=0xf0f, id=1,
      }
      translate( -(sw - lw), 0, 0 )
      block{
         { 0, 0, 0},
         {lw,lh,ld},
         c=0xf0f, id=1,
      }
   blocksEnd()

   translate( 0, lh, 0 )

   -- Seat.
   blocksBegin()
      block{
         { 0, 0, 0},
         {sw,sh,sd},
         c=0xffb, id=1,
      }
   blocksEnd()

   translate( 0, sh, 0 )

   -- Back.
   blocksBegin()
      translate( 0, 0, (ld-bd)*0.5 )
      block{
         { 0,bh1, 0},
         {bw,bh ,bd},
         c=0xff9, id=1,
      }
   blocksEnd()
compositeEnd()
return c
