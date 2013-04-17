local s
blocksBegin()
   scopeBegin()
   s = { 1, 1, 1 }
   translate( 0, -s[2], 0 )
   block{ s, c=0xFFF, id=1 }
   collisionBox{ size=s }
   scopeEnd()

   translate( 0, -2, -1 )

   s = { 1, 5, 0.01 }
   translate( 0, s[2], 0 )
   block{ s, c=0xFFF, id=2 }
   collisionBox{ size=s }
   translate( 0, s[2], 0 )

   s = { 1, 5, 0.01 }
   translate( 0, s[2], 0 )
   block{ s, c=0xFFF, id=3 }
   collisionBox{ size=s }
blocksEnd()
