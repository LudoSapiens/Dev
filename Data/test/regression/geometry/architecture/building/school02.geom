geometricError(0.04)
--detailsError(0.02)
detailsError(1)
--detailsError(0.02)

--==============================================================================
-- Main frame
--==============================================================================

compositeBegin()
   -- Main volume.
   component{ 
      id={"k","wing"}, size={40,30,25}, 
      boundary={ {0,0,0},{0,0,1},{1,0,1},{1,0,0}, id={"b","t","s","m","s","b"} },
      position={0,0,15}
   }
   component{ 
      id={"k","wing"}, size={30,27,42}, 
      boundary={ {0,0,0},{0,0,1},{1,0,1},{1,0,0}, id={"b","t","s","m","s","b"} },
      position={5,0,0}
   }
   component{ 
      id={"l","wing"}, size={15,30,15}, 
      boundary={ {0,0,0},{0,0,1},{1,0,1},{1,0,0}, id={"b","t","s","m","s","b"} },
      position={12.5,30,20}
   }
   component{ 
      id={"l","wing"}, size={10,10,10}, 
      boundary={ {0,0,0},{0,0,1},{1,0,1},{1,0,0}, id={"b","t","s","m","s","b"} },
      position={15,60,22.5}
   }
   component{ 
      id={"m","wing"}, size={40,27,15}, 
      boundary={ {0,0,0},{0,0,1},{1,0,1},{1,0,0}, id={"b","t","s","m","s","b"} },
      position={40,0,22}
   }
   component{ 
      id={"h","wing"}, size={40,27,15}, 
      boundary={ {0,0,0},{0,0,1},{1,0,1},{1,0,0}, id={"b","t","s","m","s","b"} },
      position={-40,0,22}
   }
   component{ 
      id={"p","wing"}, size={15,30,30}, 
      boundary={ {0,0,0},{0,0,1},{1,0,1},{1,0,0}, id={"b","t","s","m","s","b"} },
      position={70,0,25}
   }
   component{ 
      id={"e","wing"}, size={15,30,30}, 
      boundary={ {0,0,0},{0,0,1},{1,0,1},{1,0,0}, id={"b","t","s","m","s","b"} },
      position={-45,0,25}
   }
   component{ 
      id={"s","wing"}, size={60,27,15}, 
      boundary={ {0,0,0},{0,0,1},{1,0,1},{1,0,0}, id={"b","t","s","m","s","b"} },
      position={40,0,50}
   }
   component{ 
      id={"d","wing"}, size={60,27,15}, 
      boundary={ {0,0,0},{0,0,1},{1,0,1},{1,0,0}, id={"b","t","s","m","s","b"} },
      position={-60,0,50}
   }
   component{ 
      id={"t","wing"}, size={15,24,30}, 
      boundary={ {0,0,0},{0,0,1},{1,0,1},{1,0,0}, id={"b","t","s","m","s","b"} },
      position={45,0,65}
   }
   component{ 
      id={"u","wing"}, size={15,24,30}, 
      boundary={ {0,0,0},{0,0,1},{1,0,1},{1,0,0}, id={"b","t","s","m","s","b"} },
      position={85,0,65}
   }
   component{ 
      id={"b","wing"}, size={15,24,30}, 
      boundary={ {0,0,0},{0,0,1},{1,0,1},{1,0,0}, id={"b","t","s","m","s","b"} },
      position={-60,0,65}
   }
   component{ 
      id={"a","wing"}, size={15,24,30}, 
      boundary={ {0,0,0},{0,0,1},{1,0,1},{1,0,0}, id={"b","t","s","m","s","b"} },
      position={-20,0,65}
   }

   -- Merge all wings together.
   local s={}
   for c in query( "wing", "lucarne", "wingRoof" ) do s[#s+1] = c end
   merge( s, { id="hull" } )

--==============================================================================
-- Interior
--==============================================================================

   for c in query( "hull" ) do
      slice( c, "Y", { id={"level","room"}, 3, level=counter()}, 2 )
   end

   for c in query( "room" ) do
      local s={}
      for f in fquery( c ) do
         s[#s+1] = component{ c, id="wall2d", boundary=f }
      end
      extrude( s, -0.05, { id="iwall" } )
   end

--==============================================================================
-- Exterior
--==============================================================================

   local s={}
   for c in query( "hull" ) do
      for f in fquery( c, "S","T" ) do
         s[#s+1] = component{ c, id="facade", boundary=f }
      end
   end
   extrude( s, 0.2, { id="ewall" } )

   local s={}
   for c in query( "roof" ) do
      output(c)
      for f in fquery( c ) do
         s[#s+1] = component{ c, id="roofFacade", boundary=f }
      end
   end
   extrude( s, -0.2, { id="ewall" } )

   --[[
   -- Windows placement.
   for c in query( "wall2d" ) do
      if hasFaceID( c, "m" ) and c.level==2 then
         slice( c, "X", { id="fA", 4 } )
      end
   end

   for c in query( "fA" ) do
      region{ c, id="window" }
   end
   --]]

--==============================================================================
-- Geometry
--==============================================================================

   blocksBegin()
      for c in query( "iwall" ) do blocks{ c, id=0 } end
      for c in query( "ewall" ) do blocks{ c, id=c.mat } end
   blocksEnd()

--==============================================================================
-- Connections
--==============================================================================

   for r in rquery( "window" ) do
      connect( execute( "architecture/window/window01", { w=1, h=1.6, d2=0.3,d3=0.26 } ), r, {0.5,0.5,0},{0,0,-0.05} )
   end

compositeEnd()
