local m = { [0]=0x0, [1]=0x1, [2]=0x2, [3]=0x4, [4]=0x8 }

for zi = 0, 0 do
   local z = 6-6 + 3*zi
   for yi = 0, 4 do
      local y = -6 + 3*yi
      for xi = 0, 4 do
         local x = -6 + 3*xi
         local c = m[xi] + m[yi]*16 + m[zi]*16*16
         transformBegin{ t={x,y,z} }
            blocksBegin()
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
            blocksEnd()
         transformEnd()
      end
   end
end
