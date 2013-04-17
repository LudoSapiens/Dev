--=============================================================================
-- Copyright (c) 2007, Ludo Sapiens Inc.
-- All rights reserved.
--
-- These coded instructions, statements, and computer programs contain
-- unpublished, proprietary information and are protected by Federal copyright
-- law. They may not be disclosed to third parties or copied or duplicated in
-- any form, in whole or in part, without prior written consent.
--=============================================================================

string = string or {}

-------------------------------------------------------------------------------
function isDigit(ch)
   return ch >= "0" and ch <= "9"
end

-------------------------------------------------------------------------------
function isLower(ch)
   return ch >= "a" and ch <= "z"
end

-------------------------------------------------------------------------------
function isUpper(ch)
   return ch >= "A" and ch <= "Z"
end

-------------------------------------------------------------------------------
function isLetter(ch)
   return isLower(ch) or isUpper(ch)
end

-------------------------------------------------------------------------------
function isWhitespace(ch)
   return ch==' ' or ch=='\t' or ch=='\n' or ch=='\r'
end

--=============================================================================
-- Returns the position in the string corresponding to the first non-whitespace
-- character, starting at the specified position.
--=============================================================================
function string.skipWS( str, pos )
   if not str then
      return pos
   end

   pos = pos or 1
   while isWhitespace( string.sub(str, pos, pos) ) do
      pos = pos + 1
   end

   return pos
end


--=============================================================================
-- This routine is an auxiliary routine to toNumber (see below).
-- It returns a triplet (sign, value, pos) where:
--   sign: corresponds to the sign (either 1 or -1)
--   value: corresponds to the actual magnitude of the number
--   pos: corresponds to the character position past the parsed characters
-- The parameters to the function are:
--   str: The incoming string to convert from.
--   start: The initial character position (or 1 if unspecified).
--   disallowSign: A flag indicating if a leading sign ('+' or '-') is allowed
--                 or not (optional).
--=============================================================================
function string.toIntegerAux( str, start, disallowSign )
   local value = 0
   local sign  = 1
   start       = start or 1
   local c     = string.sub(str, start, start)
   if isDigit(c) then
      value = ( c - '0' )
   elseif not disallowSign then
      if c == '-' then
         sign = -1
      elseif c == '+' then
         --Nothing to do, but accept it
      else
         return nil, nil, nil
      end
   else
      return nil, nil, nil
   end
   local pos = start + 1
   while true do
      c = string.sub(str, pos, pos)
      if isDigit(c) then
         value = value * 10 + (c - '0')
      else
         --print("returning:", sign, value, pos)
         --print("---")
         return sign, value, pos
      end
      pos = pos + 1
   end
end

--=============================================================================
-- This routine converts an incoming string into a number and returns it along
-- with the position of the next unparsed character.
-- If the string cannot be converted, the number returned will be nil, and the
-- position will be a valid position (either start or 1 if start wasn't valid).
--=============================================================================
function string.toNumber( str, start )
   start = start or 1
   start = string.skipWS( str, start )
   local sign, int, pos = string.toIntegerAux(str, start, false)
   sign        = sign or 1
   pos         = pos or start
   local frac  = 0
   local scale = 1
   local c = string.sub(str, pos, pos)

   if c == '.' then
      pos = pos + 1
      int = int or 0
      local fs, fi, fp = string.toIntegerAux(str, pos, true)
      --print(".:", str, pos, fs, fi, fp)
      if fs then
         frac = fi * math.pow(10, -(fp-pos))
         pos  = fp
      end
      c = string.sub(str, pos, pos)
   end

   if int and c == 'e' then
      pos = pos + 1
      local es, ei, ep = string.toIntegerAux(str, pos, false)
      --print("e:", str, pos, es, ei, ep)
      if es then
         scale = math.pow(10, ei)
         pos   = ep
      end
      -- No need to assign a new c
   end

   if int then
      return sign * scale * (int + frac), pos
   else
      return nil, start
   end
end

--=============================================================================
-- Extracts the extension of the specified string.
--=============================================================================
function string.getExt(s)
   local dot = string.find(s, "%.[^.]*$")
   --print("DOT: " .. tostring(dot))
   return dot and string.sub(s, dot+1)
end

--=============================================================================
-- Strips leading and trailing spaces on the specified string.
--=============================================================================
function string.strip(s)
   return string.gsub(s, "%s*(.-)%s*$", "%1", 1)
end

--=============================================================================
-- Splits a string into parts using the specified delimiter pattern.
-- Taken from: http://lua-users.org/wiki/SplitJoin.
--=============================================================================
-- Split text into a list consisting of the strings in text,
-- separated by strings matching delimiter (which may be a pattern). 
-- example: strsplit(",%s*", "Anna, Bob, Charlie,Dolores")
function string.split(delimiter, text)
  local list = {}
  local pos  = 1
  if string.find("", delimiter, 1) then -- this would result in endless loops
    error("delimiter matches empty string!")
  end
  while 1 do
    local first, last = string.find(text, delimiter, pos)
    if first then -- found?
      table.insert(list, string.sub(text, pos, first-1))
      pos = last+1
    else
      table.insert(list, string.sub(text, pos))
      break
    end
  end
  return list
end

