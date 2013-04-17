differenceBegin()
   blocksBegin()
      block{
         {-2,-2,-2},
         { 2,-2,-2},
         {-2, 2,-2},
         { 2, 2,-2},
         {-2,-2, 2},
         { 2,-2, 2},
         {-2, 2, 2},
         { 2, 2, 2},
         c=0xfff
      }
   blocksEnd()
   transformBegin{ r={0.125,{0,1,0}} }
      blocksBegin()
         block{
            {-1,-1,-3},
            { 1,-1,-3},
            {-1, 1,-3},
            { 1, 1,-3},
            {-1,-1, 3},
            { 1,-1, 3},
            {-1, 1, 3},
            { 1, 1, 3},
            c=0xfff
         }
      blocksEnd()
   transformEnd()
   transformBegin{ r={0.125,{1,0,0}} }
      blocksBegin()
         block{
            {-1,-1,-3},
            { 1,-1,-3},
            {-1, 1,-3},
            { 1, 1,-3},
            {-1,-1, 3},
            { 1,-1, 3},
            {-1, 1, 3},
            { 1, 1, 3},
            c=0xfff
         }
      blocksEnd()
   transformEnd()
differenceEnd()
