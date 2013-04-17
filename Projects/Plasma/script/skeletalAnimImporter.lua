--------------------------------------------------------------------------------
--convert a simple set of animation file to skeletal animation.
--------------------------------------------------------------------------------

local animFile = arg[1]
local out = arg[2]


local poses = {}

-- Reading animation sequence.
print( "loading animation: " .. animFile )

local file = io.open( animFile, "r" )

local nbPose = file:read( "*n" )
print( "number of pose " .. nbPose )

while true do
   local id, f, x, y, z, q0, q1, q2, q3 = file:read( "*n", "*n", "*n", "*n", "*n", "*n", "*n", "*n", "*n" )
   if not id then 
      break 
   end

   if not poses[f+1] then
      poses[f+1] = {}
   end
   local pose = poses[f+1]

   pose[id+1] = { q0, q1, q2, q3 }
end


file:close()

--------------------------------------------------------------------------------
-- output a table content.
local function outputTable( file, tab )
   file:write( "{" )
   local intid = 1
   local first = true
   for k,v in pairs( tab ) do
      if first then
         first = false
      else
         file:write( "," )
      end
      -- write key
      local keyWrote = false
      if type( k ) ~= "number" or k ~= intid then
         keyWrote = true
         file:write( k .. " = " )
      else
         intid = intid + 1
      end
      -- write value
      if type( v ) == "table" then
         if not keyWrote then file:write( "\n" ) end
         outputTable( file, v )
      elseif type( v ) == "string" then
         file:write( "\"" .. v .. "\"" )
      else
         file:write( v )
      end
   end

   file:write( "}" )
end

-- Output new file.
file = io.open( out, "w" )

anim = { poses = poses }

file:write( "return plasma.skeletalAnimation" )
outputTable( file, anim )

file:close()
