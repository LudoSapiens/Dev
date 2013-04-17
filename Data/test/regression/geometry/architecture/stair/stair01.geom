--==============================================================================
-- Simple stair
--==============================================================================

local params  = ... or {}
local h       = params.h or 2
local w       = params.w or 1
local d       = params.d or 2
local stepNum = params.stepNum or 10
local th      = params.th or 0.03
local ns      = params.ns or 0.03
local rn      = params.rn
local riser   = params.riser or "linear"
local stw     = params.stw or 0.05
local sth     = params.sth or 0
local trimmed = params.trimmed
local railing = params.railing

if trimmed == nil then trimmed = true end

local Mat = execute( "architecture/common_materials" )
local mat = params.mat or {}
local mTread    = mat[1] or Mat.FLOOR1
local mRiser    = mat[2] or Mat.WALL1
local mStringer = mat[3] or mRiser
local mRailing  = mat[4] or Mat.DETAIL1

-- Derived parameters.
local sh = h/stepNum
local sd = d/stepNum
local rc = 0xfff
if rn then rc=0xf60 end
sth = sth+sh

--==============================================================================
-- Main component
--==============================================================================

local c = component{ id="stair", size={w,h,d} }
compositeBegin(c,50)

--==============================================================================
-- Geometry
--==============================================================================

   -- Treads.
   blocksBegin()
      translate(0,sh-th,d-sd)
      attraction(1,2)
      for i=0,stepNum-1 do
         block{ {stw,0,0}, {w-stw,th,sd}, c=0xfff, id=mTread, g=1 }
         -- Nose.
         if ns > 0 then
            block{ {stw,0,sd}, {w-stw,th,sd+ns}, c=0xfff, id=mTread, g=2, c=rc }
         end
         translate(0,sh,-sd)
      end
   blocksEnd()

   -- Riser.
   if (riser == "linear") or (riser == "square") or (riser == "full") then
      blocksBegin()
         translate(0,0,d-sd)
         for i=0,stepNum-1 do
            if (i == 0) and (trimmed) then
               block{ {stw,0,0}, {w-stw,sh-th,sd}, c=0xfff, id=mRiser }
            else
               if riser == "full" then
                  block{ {stw,-sh*i,0}, {w-stw,sh-th,sd}, c=0xfff, id=mRiser }
               elseif riser == "square" then
                  block{ {stw,-th,0}, {w-stw,sh-th,sd}, c=0xfff, id=mRiser }
               else
                  block{ 
                     {stw,0,0}, {w-stw,0,0}, {stw,sh-th,0}, {w-stw,sh-th,0},
                     {stw,-sh,sd}, {w-stw,-sh,sd}, {stw,sh-th,sd}, {w-stw,sh-th,sd},
                     c=0xfff, id=mRiser,
                  }
               end
            end
            translate(0,sh,-sd)
         end
      blocksEnd()
   end

   -- Stringer.
   if stw > 0 then
      blocksBegin()
         attraction(0,0)
         for i=0,1 do
            block{
               {0,h,0},{stw,h,0},{0,h+sth,0},{stw,h+sth,0},
               {0,0,d},{stw,0,d},{0,sth,d},{stw,sth,d},
               c=0xfff, g=0, id=mStringer,
            }
            if trimmed then
               block{
                  {0,h-sh,0},{stw,h-sh,0},{0,h,0},{stw,h,0},
                  {0,0,d-sd},{stw,0,d-sd},{0,0,d},{stw,0,d},
                  c=0xfff, g=0, id=mStringer,
               }
            else
               block{
                  {0,h-sh,0},{stw,h-sh,0},{0,h,0},{stw,h,0},
                  {0,-sh,d},{stw,-sh,d},{0,0,d},{stw,0,d},
                  c=0xfff, g=0, id=mStringer,
               }
            end
            translate(w-stw,0,0)
         end
      blocksEnd()
   end

   -- Railing.
   if railing then
      blocksBegin()
         block{ {w,h,0}, {w+0.1,h+0.8,d}, c=0xfff, id=mRailing }
         --block{ {w,h+0.7,0}, {w+0.1,h+0.8,d}, c=0xfff, id=mRailing }
      blocksEnd()
   end
compositeEnd()
return c
