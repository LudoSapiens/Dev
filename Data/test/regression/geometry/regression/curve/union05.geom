unionBegin()
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
         c=0
      }
   blocksEnd()
   transformBegin{ t={1,0,0.5} }
      blocksBegin()
         block{
            {-1,-2,-1},
            { 1,-2,-1},
            {-1, 2,-1},
            { 1, 2,-1},
            {-1,-2, 1},
            { 1,-2, 1},
            {-1, 2, 1},
            { 1, 2, 1},
            c=0
         }
      blocksEnd()
   transformEnd()
unionEnd()
