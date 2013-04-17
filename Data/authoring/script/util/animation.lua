--=============================================================================
-- Copyright (c) 2007, Ludo Sapiens Inc.
-- All rights reserved.
--
-- These coded instructions, statements, and computer programs contain
-- unpublished, proprietary information and are protected by Federal copyright
-- law. They may not be disclosed to third parties or copied or duplicated in
-- any form, in whole or in part, without prior written consent.
--=============================================================================

include("util/cgmath")
include("util/string")
include("util/table")

Animation = Animation or {}

--=============================================================================
-- Retrieves the next line of the specified iterator, skipping comment lines
-- starting with '#'.
--=============================================================================
local function getNextLine(iter)
   local line = iter()
   while line and string.find(line, "^[%s]*#") do
      --print("Skipping comment")
      line = iter()
   end
   return line
end

-------------------------------------------------------------------------------
local function rotateX( angle )
   return Quat.axisAngle( 1, 0, 0, angle )
end

-------------------------------------------------------------------------------
local function rotateY( angle )
   return Quat.axisAngle( 0, 1, 0, angle )
end

-------------------------------------------------------------------------------
local function rotateZ( angle )
   return Quat.axisAngle( 0, 0, 1, angle )
end

local dofFuncMap = {}
dofFuncMap["rx"] = rotateX
dofFuncMap["ry"] = rotateY
dofFuncMap["rz"] = rotateZ

--=============================================================================
-- Outputs a skeleton table into the specified file.
--=============================================================================
function Animation.saveSkeletonTable(skeleton, outfilename)
   local outfile = io.open(outfilename, "w")
   
   if not outfile then
      print("Error - Could not open output file '" .. outfilename .. "'.")
      return
   end
   
   outfile:write("return Plasma.skeleton")
   table.serialize( outfile, skeleton )
   outfile:close()
end

--=============================================================================
-- Reads an input ASF file and returns a skeleton table representing it as well
-- as some extra information required to parse AMC files.
--=============================================================================
function Animation.loadSkeletonTable_ASF( infilename )

   local function toDOF( dof )
      val = 0
      if dof then
         if dof[1] == "rx" then
            val = 1
         end
         if dof[1] == "ry" or dof[2] == "ry" then
            val = val + 2
         end
         if dof[1] == "rz" or dof[2] == "rz" or dof[3] == "rz" then
            val = val + 4
         end
      end
      return val
   end

   local infile = io.open(infilename, "r")
   if not infile then
      print("Error - Could not open input file '" .. infilename .. "'.")
      return
   end

   local info         = {}
   info.dofFuncs      = {}
   info.axis          = {}
   info.mass          = 1
   info.length        = 1
   info.angle         = 1
   
   local asf_bonedata = {}
   local asf_section

   local curBoneID = 0

   local iter = infile:lines()
   local line = getNextLine(iter)

   -- A table containing a sort of state machine to parse the file.
   -- Every routine relies on 2 global variables: line and iter.
   -- Both must be set and valid before reaching any routine.
   -- That means that early out happens outside of those.
   local dispatch = {}

   -- Main (top-level) state
   dispatch[0] = function()
      if string.sub(line, 1, 1) == ":" then
         --print(line)
         asf_section = string.match(line, "%w+", 2)
         --print("SECTION='" .. tostring(asf_section) .. "'")
         line = getNextLine(iter)
         local dispatch_func = dispatch[asf_section]
         if dispatch_func then
            dispatch_func()
         end
      else
         line = getNextLine(iter)
      end
   end
   -- Units
   dispatch["units"] = function()
      while string.sub(line, 1, 1) ~= ':' do
         local parts = string.split( "%s+", string.strip(line) )
         --print(#parts, table.toString(parts))
         if parts[1] == "mass" then
            info.mass = tonumber(parts[2])
         elseif parts[1] == "length" then
            info.length = tonumber(parts[2])
         elseif parts[1] == "angle" then
            if parts[2] == "deg" then
               info.angle = CGM.degreesToTurnsRatio
            elseif parts[2] == "rad" then
               info.angle = CGM.radiansToTurnsRatio
            else
               info.angle = 1
            end
         else
            print("Unknown units command: " .. tostring(line))
         end
         line = getNextLine(iter)
      end
      --print("Mass=", info.mass)
      --print("Length=", info.length)
      --print("Angle=", info.angle*270)
   end
   -- Root
   dispatch["root"] = function()
      local root = {}
      root.id = -1
      root.name = "root"
      while string.sub(line, 1, 1) ~= ':' do
         local parts = string.split( "%s+", string.strip(line) )
         --print(#parts, table.toString(parts))
         if parts[1] == "order" then
         elseif parts[1] == "axis" then
         elseif parts[1] == "position" then
            root.position = { tonumber(parts[2]), tonumber(parts[3]), tonumber(parts[4]) }
         elseif parts[1] == "orientation" then
            root.orientation = { tonumber(parts[2]), tonumber(parts[3]), tonumber(parts[4]) }
         else
            print("Unknown root command: " .. tostring(line))
         end
         line = getNextLine(iter)
      end
      asf_bonedata["root"] = root
   end
   -- Bonedata
   dispatch["bonedata"] = function()
      while string.sub(line, 1, 1) ~= ':' do
         if string.strip(line) ~= "begin" then
            print("Missing 'begin' statement")
            return
         end
         local bone = {}
         bone.id = #asf_bonedata
         line = string.strip(getNextLine(iter))
         while line ~= "end" do
            if string.sub(line, 1, 1) == ':' then
               print("Found new section before encountering an 'end' statement... Aborting.")
               return
            end

            local parts = string.split( "%s+", line )
            --print(#parts, table.toString(parts))

            if parts[1] == "id" then
               -- This is an optional field and is irrelevant for us
            elseif parts[1] == "name" then
               bone.name = parts[2]
            elseif parts[1] == "direction" then
               bone.direction = { tonumber(parts[2]), tonumber(parts[3]), tonumber(parts[4]) }
            elseif parts[1] == "length" then
               bone.length = tonumber(parts[2]) * info.length
            elseif parts[1] == "axis" then
               if not parts[5] or parts[5] == "XYZ" then
                  bone.axis = { tonumber(parts[2]), tonumber(parts[3]), tonumber(parts[4]) }
                  Vec3.mulEq( bone.axis, info.angle )
               else
                  error("Only XYZ swizzling is supported in axis group.")
               end
            elseif parts[1] == "dof" then
               bone.dof = {}
               local dofFuncs = {}
               for i=2,#parts do
                  table.insert( bone.dof, parts[i] )
                  table.insert( dofFuncs, dofFuncMap[parts[i]] )
               end
               info.dofFuncs[bone.name] = dofFuncs
            elseif parts[1] == "limits" then
               bone.doflimits = {}
               local dofFuncs = info.dofFuncs[bone.name]
               -- To do (for now, just skip)
               --print("Skipping " .. #dofFuncs-1 .. " lines for limits")
               for i=2,#dofFuncs do
                  line = getNextLine(iter)
               end
            else
               print("Unknown root command: " .. tostring(line))
            end

            line = string.strip(getNextLine(iter))
         end
         -- Add bone
         if asf_bonedata[id] then
            error("File specified bonedata #" .. tostring(id) " more than once.")
         end
         bone.endpoint = Vec3.rescale(bone.direction, bone.length)
         asf_bonedata[bone.name] = bone
         table.insert(asf_bonedata, bone)
         -- Skip the 'end'
         line = string.strip(getNextLine(iter))
      end
   end
   -- Hierarchy
   dispatch["hierarchy"] = function()
      if string.strip(line) ~= "begin" then
         print("Missing 'begin' statement")
         return
      end
      line = string.strip(getNextLine(iter))
      while line ~= "end" do
         if string.sub(line, 1, 1) == ':' then
            print("Found new section before encountering an 'end' statement... Aborting.")
            return
         end

         local parts = string.split( "%s+", line )
         local parent = parts[1]
         for i=2,#parts do
            local child = parts[i]
            if asf_bonedata[child] then
               asf_bonedata[child].parent = parent
            else
               print("Child bone '" .. child .. "' does not exist")
            end
         end

         line = string.strip(getNextLine(iter))
      end
   end

   --------------------
   -- Main parsing loop
   while line do
      dispatch[0]()
   end
   infile:close()


   ---------------------------------------------------------------
   -- Conversion of the asf_bonedata into the cleaned up skelTable
   local skelTable = {}

   local root = asf_bonedata.root
   root.e     = {0, 0, 0}
   root.qwi   = {0, 0, 0, 1}
   
   local rootTable = {}
   rootTable.name  = root.name
   rootTable.p     = root.position
   rootTable.q     = {0, 0, 0, 1} --root.orientation
   skelTable.root  = rootTable
   
   info.q = {}
   info.names = {}

   for _,bone in ipairs(asf_bonedata) do
      bone.qw  = Quat.eulerZYX( unpack( bone.axis ) )
      bone.qwi = Quat.inverse( bone.qw )
      
      local parentBoneData = asf_bonedata[bone.parent]
      local boneTable = {}
      boneTable.name   = bone.name
      boneTable.parent = parentBoneData.id
      boneTable.dof    = toDOF(bone.dof)
      boneTable.p      = parentBoneData.e
      boneTable.q      = Quat.mulQuat( parentBoneData.qwi, bone.qw )
      boneTable.e      = Quat.mulVec( bone.qwi, bone.endpoint )
      bone.e           = boneTable.e
      table.insert( skelTable, boneTable )
      
      info.q[bone.name] = boneTable.q
      table.insert( info.names, bone.name )
   end

   return skelTable,info
end

--=============================================================================
-- Converts an input ASF into our proprietary .skel format.
--=============================================================================
function Animation.convertASF( infilename, outfilename )
   local skel,info = Animation.loadSkeletonTable_ASF(infilename)
   Animation.saveSkeletonTable(
      skel,
      outfilename
   )
   return info
end


--=============================================================================
-- Outputs an animation table into the specified file.
--=============================================================================
function Animation.saveAnimationTable(animation, outfilename)
   local outfile = io.open(outfilename, "w")
   if not outfile then
      print("Error - Could not open output file '" .. outfilename .. "'.")
      return
   end

   outfile:write("return Plasma.skeletalAnimation")
   table.serialize( outfile, animation )
   outfile:close()
end

--=============================================================================
-- Reads an input AMC file and returns an animation table representing it.
--=============================================================================
function Animation.loadAnimationTable_AMC( infilename, info, rate )
   local infile = io.open(infilename, "r")
   if not infile then
      print("Error - Could not open input file '" .. infilename .. "'.")
      return
   end

   local function boneDataToAnimData(boneName, boneData)
      local animData = {}
      if boneName == "root" then
         animData.p = { boneData[1], boneData[2], boneData[3] }
         animData.q = {0, 0, 0, 1}
         print("P:", animData.p[1], animData.p[2], animData.p[3])
      else
         -- Increasing index indicate order of operation, therefore:
         --   M = q[3] * q[2] * q[1]
         -- which is done in reverse order:
         --   M = q[1]
         --   M = q[2] * M = q[2] * q[1]
         --   M = q[3] * M = q[3] * q[2] * q[1]
         local dofFuncs = info.dofFuncs[boneName]
         animData.q = dofFuncs[1](boneData[1])
         for i=2,#boneData do
            --Quat.mulQuatEq( animData.q, dofFuncs[i](boneData[i]) )
            animData.q = Quat.mulQuat( dofFuncs[i](boneData[i]), animData.q )
         end
      end
      return animData
   end

   local amc_angle  = info.angle or degreesToTurnsRatio
   local amc_length = info.length or 1
   local amc_frame

   local iter = infile:lines()
   local line = getNextLine(iter)

   local poses = {}

   while line do
      if string.sub(line, 1, 1) == ":" then
         --print(line)
         local endPos     = string.find(line, "[^%w%-]", 2) or (#line+1)
         local command    = string.sub(line, 2, endPos-1)
         local command_uc = string.upper(command)
         if command_uc == "FULLY-SPECIFIED" then
         elseif command_uc == "DEGREES" then
            info.angle = CGM.degToTurns
         else
            print("Unknown command: '" .. command .. "'.")
         end
         line = getNextLine(iter)
      else
         -- Once frames start, they don't stop until the end of the file.
         while line do
            amc_frame = tonumber(line)
            line = getNextLine(iter)
            local frameData = {}
            local data = {}
            while line and string.find(line, "^[%a]") do
               local f, s, var = string.gmatch(line, "[^%s]+")
               local boneName = f(s, var)
               local boneData = {}
               if boneName == "root" then
                  for n in f,s,var do
                     table.insert( boneData, tonumber(n) )
                  end
                  frameData.p = { boneData[1]*amc_length, boneData[2]*amc_length, boneData[3]*amc_length }
                  frameData.q = Quat.eulerZYX( boneData[4]*amc_angle, boneData[5]*amc_angle, boneData[6]*amc_angle )
               else
                  for n in f,s,var do
                     table.insert( boneData, tonumber(n)/360 )
                  end
                  local animData = boneDataToAnimData(boneName, boneData)
                  --table.insert(frameData, animData.q)
                  data[boneName] = animData.q
               end
               line = getNextLine(iter)
            end
            
            for k, v in ipairs( info.names ) do
               frameData[k] = info.q[v]
               if data[v] then
                  frameData[k] = Quat.mulQuat( info.q[v], data[v] )
               end
            end
            
            table.insert(poses, frameData)
         end
      end
   end

   infile:close()

   local animationTable = {}
   animationTable.poses = poses
   animationTable.rate  = rate or 120

   --[[
   for i=1,#animationTable.poses do
      print("[" .. i .. "]: " .. table.toString(animationTable.poses[i]))
   end
   ]]

   return animationTable
end

--=============================================================================
-- Converts an input AMC into our proprietary .anim format.
--=============================================================================
function Animation.convertAMC( infilename, outfilename, info, rate )
   Animation.saveAnimationTable(
      Animation.loadAnimationTable_AMC(infilename, info, rate),
      outfilename
   )
end
