--------------------------------------------------------------------------------
--convert a simple set of file to geometry.
--------------------------------------------------------------------------------

local dir = arg[1]
local triFile = dir .. "/tri.txt"
local texFile = dir .. "/uv.txt"
local verFile = dir .. "/vert.txt"
local wFile   = dir .. "/weight.txt"
local out = arg[2]

print( "directory to load: " .. arg[1] )


-- Reading vertices.
print( "loading vertices: " .. verFile )

local vertex = {}

local file = io.open( verFile, "r" )

local nbVert = file:read( "*n" )
print( "reading " .. nbVert .. " vertices" )

for i = 1, nbVert do
   local x, y, z = file:read( "*n", "*n", "*n" )
   table.insert( vertex, {x,y,z} )
end

file:close()

-- Reading skinning.
print( "loading skin: " .. wFile )

local skin = {}

file = io.open( wFile, "r" )

while true do
   local v, b, w = file:read( "*n", "*n", "*n" )
   if not v then 
      break 
   end
   v = v + 1
   local s = skin[v]
   -- create a new skin input?
   if not s then
      skin[v] = { b = { b }, w = { w } }
   else
      -- add to current skin input
      table.insert( s.b, b )
      table.insert( s.w, w )
   end
end

file:close()

-- Reading texture coordinates.
print( "reading texcoord: " .. texFile )

local uv = {}

file = io.open( texFile, "r" )

local nbUV = file:read( "*n" )
print( "reading " .. nbUV .. " texcoords" )

for i = 1, nbUV do
   local u, v = file:read( "*n", "*n" )
   table.insert( uv, {u,v} )
end

file:close()

-- Reading triangles.
print( "reading triangles: " .. triFile )

local wedge = {}
local face  = {}
local index = {}

file = io.open( triFile, "r" )

local nbTri = file:read( "*n" )
print( "reading " .. nbTri .. " triangles" )

for i = 1, nbTri do
   local v1, v2, v3, t1, t2, t3 = 
      file:read( "*n", "*n", "*n", "*n", "*n", "*n" )

   --compute hashing index.
   local h1 = v1 + t1*16777216
   local h2 = v2 + t2*16777216
   local h3 = v3 + t3*16777216
   
   local w1 = index[h1]
   if not w1 then
      w1 = table.getn( wedge )
      index[h1] = w1
      table.insert( wedge, { v = v1, t = uv[t1+1] } )
   end

   local w2 = index[h2]
   if not w2 then
      w2 = table.getn( wedge )
      index[h2] = w2
      table.insert( wedge, { v = v2, t = uv[t2+1] } )
   end

   local w3 = index[h3]
   if not w3 then
      w3 = table.getn( wedge )
      index[h3] = w3
      table.insert( wedge, { v = v3, t = uv[t3+1] } )
   end

   table.insert( face, {w1,w2,w3} )
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
geom = {
   vertex = vertex,
   skin   = skin,
   wedge  = wedge,
   surface = {
      { face = face, m = 0 }
   }
}

file = io.open( out, "w" )

file:write( "return plasma.geometry" )
outputTable( file, geom )

file:close()
