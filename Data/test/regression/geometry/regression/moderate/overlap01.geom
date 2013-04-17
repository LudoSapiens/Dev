detailsError(1)

--------------------------------------------------------------------------------
-- Utility routines
--------------------------------------------------------------------------------

-----------------------------------------------------------------------------
-- Creates a block of WxH at the bottom left corner.
local function newBlock( w, h )
   local w_2 = (w or 1)*0.5
   local h_2 = (h or 1)*0.5
   scopeBegin()
      translate( w_2, h_2, 0 )
      block{ {w_2, h_2, 0.5}, c=0xfff }
   scopeEnd()
end

--------------------------------------------------------------------------------
-- Blocks
--------------------------------------------------------------------------------

if true then
   blocksBegin()
      scopeBegin()
         -- 1x1x1 in +X+Z quadrant.
         translate( 0.5, 0.5, 0.5 )
         block{ { 0.5, 0.5, 0.5 }, c=0xfff }
      scopeEnd()
      scopeBegin()
         -- 2x2x2 in +X-Z quadrant.
         translate( 1.0, 1.0, -1.0 )
         block{ { 1.0, 1.0, 1.0 }, c=0xfff }
      scopeEnd()
      scopeBegin()
         -- 4x4x4 in -X offsetted in Z.
         translate( -2.0, 2.0, 1.0 )
         block{ { 2.0, 2.0, 2.0 }, c=0xfff }
      scopeEnd()
   blocksEnd()
end

if false then
   --[[
      2    +---+
           |   |
      1+---+-------+
       |   |   |   |
      0+---+---+---+
       0   1   2   3
           <===> inscribed overlap
   --]]
   blocksBegin()
      newBlock( 3, 1 )
      translate( 1, 0, 0 )
      newBlock( 1, 2 )
   blocksEnd()
end

if false then
   --[[
      3            +-------+
                   |       |
      2    +---+   |       |
           |   |   |       |
      1+---+-------+---+   |
       |   |   |   |   |   |
      0+---+---+---+---+---+
       0   1   2   3   4   5
           <===> inscribed overlap
                   <===> spanning overlap
   --]]
   blocksBegin()
      newBlock( 4, 1 )
      translate( 1, 0, 0 )
      newBlock( 1, 2 )
      translate( 2, 0, 0 )
      newBlock( 2, 3 )
   blocksEnd()
end
