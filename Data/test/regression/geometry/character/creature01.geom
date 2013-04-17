--==============================================================================
-- Simple creature
--==============================================================================

local params = ... or {}
local skel = params.skeleton

--==============================================================================



local body = {}
--------------------------------------------------------------------------------
-- Torso
function body.torso()
   local c = component{ id="torso" }
   region{ c, id="head",     rel={0}, position={0,0.4,-0.02} }
   region{ c, id="rightArm", rel={0}, position={-0.2,0.3,0},      orientation={{0,0,1},-0.25} }
   region{ c, id="leftArm",  rel={0}, position={ 0.2,0.3,0},      orientation={{0,0,1}, 0.25} }
   region{ c, id="rightLeg", rel={0}, position={-0.12,-0.2,0.05}, orientation={{0,0,1},-0.04} }
   region{ c, id="leftLeg",  rel={0}, position={ 0.12,-0.2,0.05}, orientation={{0,0,1}, 0.04} }
   compositeBlocksBegin(c)
      attraction(0,0)
      block{ {-0.2,-0.2,-0.15}, {0.2,0.4,0.25}, s=0xAA2288 }
   compositeBlocksEnd()
   c._bones = { root=vec3(0,-0.03,0.05), thorax=vec3(0,0.25,0) }
   return c
end
--------------------------------------------------------------------------------
-- Head
function body.head()
   local c = component{ id="head", connectorPosition={0,-0.25,-0.05} }
   region{ c, id="eye", rel={0}, position={-0.07,0,0.19} }
   region{ c, id="eye", rel={0}, position={0.07,0,0.19} }
   compositeBlocksBegin(c)
      --block{ {-0.15,-0.2,-0.1}, {0.15,0.2,0.2}, s=0x444455 }
      --scale posy 1.2
      block{ 
         {-0.15,-0.2,-0.1}, { 0.15,-0.2,-0.1}, {-0.18, 0.2,-0.13}, { 0.18, 0.2,-0.13}, 
         {-0.15,-0.2, 0.2}, { 0.15,-0.2, 0.2}, {-0.15, 0.2, 0.23}, { 0.15, 0.2, 0.23}, 
         s=0x444455
      }
      block{
         {-0.15,-0.25,-0.1}, { 0.15,-0.25,-0.1}, {-0.10, -0.2,-0.1}, { 0.10, -0.2,-0.1}, 
         {-0.15,-0.25, 0.1}, { 0.15,-0.25, 0.1}, {-0.10, -0.2, 0.1}, { 0.10, -0.2, 0.1},
      }
   compositeBlocksEnd()
   c._bones = { head=vec3(0,0.15,-0.08) }
   return c
end
--------------------------------------------------------------------------------
-- Arms
function body.leftArm()
   local c = component{ id="leftArm", connectorPosition={0,0.2,0} }
   compositeBlocksBegin(c)
      --block{ {-0.1,-0.2,-0.1}, {0.1,0.2,0.1}, s=0x880088 }
      --scale posy 0.5
      --[[
      block{ 
         {-0.1,-0.2,-0.1}, {0.1,-0.2,-0.1}, {-0.1*0.5,0.2,-0.1*0.5}, {0.1*0.5,0.2,-0.1*0.5},
         {-0.1,-0.2, 0.1}, {0.1,-0.2, 0.1}, {-0.1*0.5,0.2, 0.1*0.5}, {0.1*0.5,0.2, 0.1*0.5},
         s=0x880088
      }
      --]]
      block{ 
         {-0.08,-0.1,-0.1}, {0.08,-0.1,-0.1}, {-0.1*0.5,0.2,-0.1*0.5}, {0.1*0.5,0.2,-0.1*0.5},
         {-0.08,-0.1, 0.1}, {0.08,-0.1, 0.1}, {-0.1*0.5,0.2, 0.1*0.5}, {0.1*0.5,0.2, 0.1*0.5},
         s=0x880088
      }
      block{ 
         {-0.05,-0.25,-0.05}, {0.05,-0.25,-0.05}, {-0.1,-0.1,-0.1}, {0.1,-0.1,-0.1},
         {-0.05,-0.25, 0.05}, {0.05,-0.25, 0.05}, {-0.1,-0.1, 0.05}, {0.1,-0.1, 0.05},
         s=0x440044
      }
      block{ 
         {0.05,-0.2,-0.05}, {0.13,-0.19,-0.02}, {0.05,-0.1,-0.05}, {0.13,-0.14,-0.02},
         {0.05,-0.2, 0.02}, {0.13,-0.19, 0.02}, {0.05,-0.1, 0.02}, {0.13,-0.14, 0.02},
         s=0x111100
      }
   compositeBlocksEnd()
   c._bones = { lclavicle=vec3(0,0.17,0), lhumerus=vec3(0,0.0,0), lradius=vec3(0,-0.06,0) }
   return c
end
--------------------------------------------------------------------------------
function body.rightArm()
   local c = component{ id="rightArm", connectorPosition={0,0.2,0} }
   compositeBlocksBegin(c)
      --block{ {-0.1,-0.2,-0.1}, {0.1,0.2,0.1}, s=0x880088 }
      --scale posy 0.5
      --[[
      block{ 
         {-0.1,-0.2,-0.1}, {0.1,-0.2,-0.1}, {-0.1*0.5,0.2,-0.1*0.5}, {0.1*0.5,0.2,-0.1*0.5},
         {-0.1,-0.2, 0.1}, {0.1,-0.2, 0.1}, {-0.1*0.5,0.2, 0.1*0.5}, {0.1*0.5,0.2, 0.1*0.5},
         s=0x880088
      }
      --]]
      block{ 
         {-0.08,-0.1,-0.1}, {0.08,-0.1,-0.1}, {-0.1*0.5,0.2,-0.1*0.5}, {0.1*0.5,0.2,-0.1*0.5},
         {-0.08,-0.1, 0.1}, {0.08,-0.1, 0.1}, {-0.1*0.5,0.2, 0.1*0.5}, {0.1*0.5,0.2, 0.1*0.5},
         s=0x880088
      }
      block{ 
         {-0.05,-0.25,-0.05}, {0.05,-0.25,-0.05}, {-0.1,-0.1,-0.1}, {0.1,-0.1,-0.1},
         {-0.05,-0.25, 0.05}, {0.05,-0.25, 0.05}, {-0.1,-0.1, 0.05}, {0.1,-0.1, 0.05},
         s=0x440044
      }
      block{ 
         {-0.13,-0.19,-0.02}, {-0.05,-0.2,-0.05}, {-0.13,-0.14,-0.02}, {-0.05,-0.1,-0.05},
         {-0.13,-0.19, 0.02}, {-0.05,-0.2, 0.02}, {-0.13,-0.14, 0.02}, {-0.05,-0.1, 0.02},
         s=0x111100
      }
   compositeBlocksEnd()
   c._bones = { rclavicle=vec3(0,0.17,0), rhumerus=vec3(0,0.0,0), rradius=vec3(0,-0.06,0) }
   return c
end
--------------------------------------------------------------------------------
-- Legs
function body.leftLeg()
   local c = component{ id="leftLeg", connectorPosition={0,0.1,0} }
   compositeBlocksBegin(c)
      --block{ {-0.1,-0.1,-0.1}, {0.1,0.1,0.1}, s=0x880088 }
      --scale negy 1.5
      block{ 
         {-0.1*1.5,-0.1,-0.1*1.5}, {0.1*1.5,-0.1,-0.1*1.5}, {-0.1,0.1,-0.1}, {0.1,0.1,-0.1},
         {-0.1*1.5,-0.1, 0.13}, {0.1*1.5,-0.1, 0.13}, {-0.1,0.1, 0.1}, {0.1,0.1, 0.1},
         s=0x880088
      }
      block{ 
         {-0.10,-0.1,0.13}, {0.10,-0.1,0.13}, {-0.1,0.01,0.10}, {0.1,0.01,0.10},
         {-0.03,-0.1,0.15}, {0.03,-0.1,0.15}, {-0.03,-0.02,0.16}, {0.03,-0.02,0.16},
      }
   compositeBlocksEnd()
   c._bones = { lhipjoint=vec3(0,0.2,0), lfemur=vec3(0,0.05,0), ltibia=vec3(0,-0.07,0) }
   return c
end
--------------------------------------------------------------------------------
function body.rightLeg()
   local c = component{ id="rightLeg", connectorPosition={0,0.1,0} }
   compositeBlocksBegin(c)
      --block{ {-0.1,-0.1,-0.1}, {0.1,0.1,0.1}, s=0x880088 }
      --scale negy 1.5
      block{ 
         {-0.1*1.5,-0.1,-0.1*1.5}, {0.1*1.5,-0.1,-0.1*1.5}, {-0.1,0.1,-0.1}, {0.1,0.1,-0.1},
         {-0.1*1.5,-0.1, 0.13}, {0.1*1.5,-0.1, 0.13}, {-0.1,0.1, 0.1}, {0.1,0.1, 0.1},
         s=0x880088
      }
      block{ 
         {-0.10,-0.1,0.13}, {0.10,-0.1,0.13}, {-0.1,0.01,0.10}, {0.1,0.01,0.10},
         {-0.03,-0.1,0.15}, {0.03,-0.1,0.15}, {-0.03,-0.02,0.16}, {0.03,-0.02,0.16},
      }
   compositeBlocksEnd()
   c._bones = { rhipjoint=vec3(0,0.2,0), rfemur=vec3(0,0.05,0), rtibia=vec3(0,-0.07,0) }
   return c
end

--------------------------------------------------------------------------------
function body.eye()
   local c = component{ id="eye", connectorPosition={0,0,0} }
   compositeBegin(c)
      blocksBegin()
         block{ 
            {-0.04,-0.04,-0.02}, {0.04,-0.04,-0.02}, {-0.04,0.04,-0.02}, {0.04,0.04,-0.02},
            {-0.04,-0.04, 0.02}, {0.04,-0.04, 0.02}, {-0.04,0.04, 0.02}, {0.04,0.04, 0.02},
         }
      blocksEnd()
   compositeEnd()
   return c
end

--==============================================================================
-- Creation
--==============================================================================

compositeBegin()
   body.torso()
   for r in rquery() do
      connect( body[r.id](), r )
   end
   local bones = {}
   for c in query() do
      if c._bones then
         local m = c.transform
         for b,p in pairs( c._bones ) do
            bones[b] = (m*vec4(p,1)).xyz
         end
      end
   end
   --[[
   for r in rquery( "eye" ) do
      connect( body[r.id](), r )
   end
   -]]

compositeEnd()

-- Create skeleton with retargeting and weights computation.
if skel then
   skeleton( skel, bones )
end

-- Note: the positions are relative to the animation, and NOT to the mesh above (known inconsistency).
----[[
collisionSpheres{
   { 0, 1.00, 0,  0.2 }, -- At thalamus, for the head.
   { 0, 0.30, 0,  0.3 }, -- At navel, for the belly.
   { 0, 0.16, 0,  0.2 }, -- At knees, for the feet.
}
--]]
--[[
collisionSpheres{
  { 0, 0.90, 0,  0.2 }, -- At thalamus, for the head.
  { 0, 0.25, 0,  0.3 }, -- At crotch, for the belly and legs.
}
--]]
