--[[
-- Example for vertex connections.
blocksBegin()
   rotate( -0.02,0,1,0)
   translate( -1.4,0,0)
   block{
      {-1,-1,-1},
      {1, 1, 1},
      c=0xfff, s=0
   }
   translate(1.4,0,0)
   rotate( 0.04, 0,1,0 )
   translate( 1.4,-0.2,0 )
   block{
      {-0.8,-0.8,-0.8},
      {0.8, 0.8, 0.8},
      c=0xfff, s=0
   }
blocksEnd()
--]]

--[[
-- Example for vertex merge.
blocksBegin()
   translate( -1.5, -1.5, 0 )
   block{ {-1,-1,-1}, {1, 1, 1}, c=0xfff, s=0 }
   translate( 3, 0, 0 )
   block{ {-1,-1,-1}, {1, 1, 1}, c=0xfff, s=0 }
   translate( 0, 3, 0 )
   block{ {-1,-1,-1}, {1, 1, 1}, c=0xfff, s=0 }
   translate( -3, 0, 0 )
   block{ {-1,-1,-1}, {1, 1, 1}, c=0xfff, s=0 }
blocksEnd()
--]]

-- Example for face merge.
--[[
blocksBegin()
   block{ {-1,-2,-1},{1,2,1}, c=0xfff, s=0x440044 }
   translate( 3, 1.5, 0 )
   rotate( -0.2, 0,0,1 )
   block{ {-0.8,-0.8,-0.8}, {0.8, 0.8, 0.8}, c=0xfff, s=0 }
   rotate( 0.2, 0,0,1 )
   translate( 0, -4, 0)
   rotate( 0.2, 0,0,1 )
   block{ {-0.8,-0.8,-0.8}, {0.8, 0.8, 0.8}, c=0xfff, s=0 }
blocksEnd()
--]]

--[[
-- Example for invalid merge.
blocksBegin()
   block{ {-3,-1,-1},{-1,1,1}, c=0xfff }
   block{ {-1,-1,-1},{ 1,1,1}, c=0xfff }
   block{ { 1,-1,-1},{ 3,1,1}, c=0xfff }
   block{ {-3, 1,-1},{-1,3,1}, c=0xfff }
   translate(0,0.8,0)
   block{ {0, 1,-1},{4,3,1}, c=0xfff }
blocksEnd()
--]]

-- Example of t-vertices correction.
blocksBegin()
   attraction(0,1)
   attraction(0,2)
   attraction(0,0)
   block{ {-1,-1,-1}, {1,1,1}, c=0x000, s=0x001000 }
   block{ {-1,1,-1},{0,3,1}, c=0x0 }
   block{ { 0,1,-1},{1,3,1}, c=0x0 }
   --[[
   block{
      {-1,1,-1},{0,1,-1},{-1.5,3,-1},{-0.5,3,-1},
      {-1,1,1},{0,1,1},{-1.5,3,1},{-0.5,3,1},
      c=0x0, g=2
   }
   block{
      {0,1,-1},{1,1,-1},{0.5,3,-1},{1.5,3,-1},
      {0,1,1},{1,1,1},{0.5,3,1},{1.5,3,1},
      c=0x0, g=1
   }
   --]]
blocksEnd()
