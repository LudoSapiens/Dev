geometricError(0.04)
--detailsError(0.02)
detailsError(1)
--detailsError(0.02)

--==============================================================================
-- Frame
--==============================================================================

compositeBegin()

   component{ 
      id={"mainWing","wing"}, size={15,12,40}, 
      boundary={ {0,0,0},{0,0,1},{1,0,1},{1,0,0}, id={"b","t","s","m","s","b"} },
      position={2,0,0}
   }
   component{ 
      id={"frontWing","wing"}, size={3,10,4}, 
      boundary={ {0,0,0},{0,0,1},{1,0,1},{1,0,0}, id={"b","t","s","m","s","b"} },
      position={8,0,38}
   }
   component{ 
      id={"lowerWing","wing"}, size={19,9,36}, 
      boundary={ {0,0,0},{0,0,1},{1,0,1},{1,0,0}, id={"b","t","s","f","s","b"} },
      position={0,0,4}
   }
   component{ 
      id={"upperWing","wing"}, size={17,12,6}, 
      boundary={ {0,0,0},{0,0,1},{1,0,1},{1,0,0}, id={"b","t","s","f","s","b"} },
      position={1,0,10}
   }
   component{ 
      id={"upperWing","wing"}, size={17,12,6}, 
      boundary={ {0,0,0},{0,0,1},{1,0,1},{1,0,0}, id={"b","t","s","f","s","b"} },
      position={1,0,20}
   }
   component{ 
      id={"upperWing","wing"}, size={17,12,6}, 
      boundary={ {0,0,0},{0,0,1},{1,0,1},{1,0,0}, id={"b","t","s","f","s","b"} },
      position={1,0,30}
   }
   component{
      id={"tower","wing"}, size={3,12,3},
      boundary={ {1/3,0,0},{0,0,1/3},{0,0,2/3},{1/3,0,1},{2/3,0,1},{1,0,2/3},{1,0,1/3},{2/3,0,0} },
      position={3,0,39}
   }
   component{
      id={"tower","wing"}, size={3,12,3},
      boundary={ {1/3,0,0},{0,0,1/3},{0,0,2/3},{1/3,0,1},{2/3,0,1},{1,0,2/3},{1,0,1/3},{2/3,0,0} },
      position={13,0,39}
   }
   --[[
   component{
      id="lucarne", size={1,1.5,3}, position={4,8,6}
   }
   component{
      id="lucarne", size={1,1.5,3}, position={6,8,6}
   }
   component{
      id="lucarne", size={1,1.5,3}, position={8,8,6}
   }
   component{
      id="lucarne", size={2,1.5,2}, position={0,8,4}, orientation=1
   }
   component{
      id="lucarne", size={1,1.5,1}, position={1,8,9}
   }
   --]]

   -- Roof
   for c in query( "upperWing" ) do
      for f in fquery( c, "T" ) do
         local r = component{ c, id="roofFloor", boundary=f }
         roof( r, {1,0,3}, { id={"wingRoof"} } )
      end
   end
   for c in query( "mainWing" ) do
      for f in fquery( c, "T" ) do
         local r = component{ c, id="roofFloor", boundary=f }
         roof( r, {0.4,0.8,3.5}, { id={"wingRoof"} } )
      end
   end
   for c in query( "tower" ) do
      for f in fquery( c, "T" ) do
         local r = component{ c, id="roofFloor", boundary=f }
         roof( r, {0,0,3.5}, { id={"wingRoof"} } )
      end
   end
   for c in query( "frontWing" ) do
      for f in fquery( c, "T" ) do
         local r = component{ c, id="roofFloor", boundary=f }
         roof( r, {0.2,0.2,3.5}, { id={"wingRoof"} } )
      end
   end
   --[[
   for c in query( "lucarne" ) do
      for f in fquery( c, "T" ) do
         local r = component{ c, id="roofFloor", boundary=f }
         roof( r, {0,1,0.5}, { id={"wingRoof"} } )
      end
   end
   --]]

   --[[
   local s={}
   for c in query( "wingRoof", "lucarne" ) do s[#s+1] = c end
   merge( s, { id="roof" } )
   --]]
   -- Merge all wings together.
   local s={}
   for c in query( "wing", "lucarne", "wingRoof" ) do s[#s+1] = c end
   merge( s, { id="hull" } )

--==============================================================================
-- Interior
--==============================================================================

   for c in query( "hull" ) do
      slice( c, "Y", { id={"level","room"}, 3 }, 2 )
   end

   for c in query( "room" ) do
      local s={}
      for f in fquery( c, "B", "T" ) do
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
      for f in fquery( c ) do
         s[#s+1] = component{ c, id="roofFacade", boundary=f }
      end
   end
   extrude( s, -0.2, { id="ewall" } )

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

compositeEnd()

