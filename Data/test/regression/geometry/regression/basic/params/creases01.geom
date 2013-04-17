local m = { [0]=0x0, [1]=0x1, [2]=0x2, [3]=0x4, [4]=0x8 }

blocksBegin()
   translate( -9, -9, 0 )
   for yi = 0, 4 do
      translate( 0, 3, 0 )
      scopeBegin()
         for xi = 0, 4 do
            translate( 3, 0, 0 )
            local c = (15-m[xi]) + (15-m[yi])*16 + (15-m[0])*16*16
            block{
               {-1,-1,-1},
               { 1,-1,-1},
               {-1, 1,-1},
               { 1, 1,-1},
               {-1,-1, 1},
               { 1,-1, 1},
               {-1, 1, 1},
               { 1, 1, 1},
               c = c
            }
         end
      scopeEnd()
   end
blocksEnd()
