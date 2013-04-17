--------------------------------------------------------------------------------
-- Building frame
--------------------------------------------------------------------------------

-- Main volume.
--component{ id="main", size={10,8, 5} }
component{ id="c1", size={16,8,10} }
component{ id="c2", size={8,4,10}, position={0,1,10} }
component{ 
   id="c3", size={10,10,10}, position={16,0,0},
   boundary={
      {0,0,0},{0,0,0.3},{0.5,0,0.3},{0.5,0,0.6},{0,0,0.6},{0,0,1},{1,0,1},{1,0,0},
      direction={0,1,0}
   }
}

for c in query( "c1" ) do
   for f in fquery( c, "SIDE" ) do
      local fc = component{ c, id="wall", boundary=f }
      local occ = occlusion( fc, "c2", "c3" )
      print( "occlusion factor: " .. occ )
   end
end



