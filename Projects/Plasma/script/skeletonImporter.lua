--------------------------------------------------------------------------------
--convert a simple set of file to skeleton.
--------------------------------------------------------------------------------

local dir = arg[1]
local skelFile = dir .. "/skel_hier.txt"
local refFile  = dir .. "/anim_ref.txt"
local out = arg[2]

print( "directory to load: " .. arg[1] )

local skeleton = {}

-- Reading skeleton: bone hierarchy.
print( "loading hierarchy: " .. skelFile )

local file = io.open( skelFile, "r" )

local nbBone = file:read( "*n" )
print( "reading " .. nbBone .. " bones" )

for i = 1, nbBone do
   local id, parentID, name = file:read( "*n", "*n", "*l" )
   skeleton[id+1] = { parent = parentID }
end

file:close()

-- Reading reference position.
print( "loading reference: " .. refFile )

file = io.open( refFile, "r" )

--skip first line.
_ = file:read( "*n" )

while true do
   local id, f, x, y, z, q0, q1, q2, q3 = file:read( "*n", "*n", "*n", "*n", "*n", "*n", "*n", "*n", "*n" )
   if not id then 
      break 
   end
   local bone = skeleton[id+1]
   if bone then
      bone.p = {x,y,z}
      bone.q = {q0,q1,q2,q3}
   end
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

file:write( "return plasma.skeleton" )
outputTable( file, skeleton )

file:close()
