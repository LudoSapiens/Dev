--=============================================================================
-- Copyright (c) 2007, Ludo Sapiens Inc.
-- All rights reserved.
--
-- These coded instructions, statements, and computer programs contain
-- unpublished, proprietary information and are protected by Federal copyright
-- law. They may not be disclosed to third parties or copied or duplicated in
-- any form, in whole or in part, without prior written consent.
--=============================================================================

table = table or {}

--=============================================================================
-- A routine to convert simple tables into a string (usually for printing).
--=============================================================================
function table.val_to_str ( v )
   if "string" == type( v ) then
      v = string.gsub( v, "\n", "\\n" )
      if string.match( string.gsub(v,"[^'\"]",""), '^"+$' ) then
         return "'" .. v .. "'"
      end
      return '"' .. string.gsub(v,'"', '\\"' ) .. '"'
   else
      return "table" == type( v ) and table.toString( v ) or tostring( v )
   end
end

function table.key_to_str ( k )
   if "string" == type( k ) and string.match( k, "^[_%a][_%a%d]*$" ) then
      return k
   else
      return "[" .. table.val_to_str( k ) .. "]"
   end
end

function table.toString( tbl )
   if type( tbl ) == "table" then
      local result, done = {}, {}
      for k, v in ipairs( tbl ) do
         table.insert( result, table.val_to_str( v ) )
         done[ k ] = true
      end
      for k, v in pairs( tbl ) do
         if not done[ k ] then
            table.insert( result,
            table.key_to_str( k ) .. "=" .. table.val_to_str( v ) )
         end
      end
      return "{" .. table.concat( result, "," ) .. "}"
   else
      return tostring( tbl )
   end
end


--==============================================================================
-- Outputs a table to an opened file. 
--==============================================================================

--------------------------------------------------------------------------------
function table.isComplex( tab )
   if #tab > 10 then
      return true
   end
   for k,v in pairs(tab) do
      if type(v) == "table" then
         return true
      end
   end
   return false
end

--------------------------------------------------------------------------------
function table.serialize( out, tab, mode, space )
   if mode == "compact" then
      table.serializeCompact( out, tab )
   elseif mode == "expanded" then
      table.serializeExpanded( out, tab, mode, space )
   else
      if table.isComplex(tab) then
         table.serializeExpanded( out, tab, mode, space )
      else
         table.serializeCompact( out, tab )
      end
   end
end

--------------------------------------------------------------------------------
function table.serializeCompact( out, tab )
   out:write( "{" )
   local intID = 1
   local first = true
   for k, v in pairs( tab ) do
      -- Write separator
      if first then
         first = false
      else
         out:write( "," )
      end
      -- Write key
      if type(k) ~= "number" then
         out:write( k .. "=" )
      elseif k ~= intID then
         out:write( "[" .. k .. "]=" )
      else
         intID = intID + 1
      end
      -- Write value
      if type(v) == "table" then
         table.serializeCompact( out, v )
      elseif type(v) == "string" then
         out:write( "\"" .. v .. "\"" )
      else
         out:write( string.format( "%.8g", v ) )
      end
   end
   out:write( "}" )
end

--------------------------------------------------------------------------------
function table.serializeExpanded( out, tab, mode, space )
   space = space or ""
   local newSpace = space .. "  "

   out:write( "{\n" )
   local intID = 1
   local first = true
   for k, v in pairs( tab ) do
      -- Write separator
      if first then
         first = false
      else
         out:write( ",\n" )
      end
      
      -- Write spacing
      out:write( newSpace )
      
      -- Write key
      if type(k) ~= "number" then
         out:write( k .. "=" )
      elseif k ~= intID then
         out:write( "[" .. k .. "]=" )
      else
         intID = intID + 1
      end
      -- Write value
      if type(v) == "table" then
         table.serialize( out, v, mode, newSpace )
      elseif type(v) == "string" then
         out:write( "\"" .. v .. "\"" )
      else
         out:write(tostring(v))
      end
   end
   out:write( "\n" .. space .. "}" )
end

-----------------------------------------------------------------------------
-- Code taken from http://lua-users.org/wiki/CopyTable
-- The metatable will be shared; change 'getmetatable(object)' to 
-- '_copy( getmetatable(object) )' if you want a different behavior.
function table.deepcopy( t )
    local lookup_table = {}
    local function _copy(object)
        if type(object) ~= "table" then
            return object
        elseif lookup_table[object] then
            return lookup_table[object]
        end
        local new_table = {}
        lookup_table[object] = new_table
        for index, value in pairs(object) do
            new_table[_copy(index)] = _copy(value)
        end
        return setmetatable(new_table, getmetatable(object))
    end
    return _copy( t )
end
