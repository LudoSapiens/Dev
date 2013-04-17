# =============================================================================
#  Copyright (c) 2006, Ludo Sapiens Inc.
#  All rights reserved.
# 
#  These coded instructions, statements, and computer programs contain
#  unpublished, proprietary information and are protected by Federal copyright
#  law. They may not be disclosed to third parties or copied or duplicated in
#  any form, in whole or in part, without prior written consent.
# =============================================================================

import os
import string
import sys
import tarfile
import zipfile

import BuildSystem.Defaults.Env
import BuildSystem.Defaults.Tools
import BuildSystem.Utilities.Options
from BuildSystem.Utilities.Environment import Environment

# ----------------------------------------------------------------------------
# A somewhat global stack of all of the previous Environment instances
# ----------------------------------------------------------------------------
Env = list()

# ----------------------------------------------------------------------------
# Utility routines
# ----------------------------------------------------------------------------
def list_iter(node):
   if isinstance(node, list):
      return iter(node)
   else:
      return []


def breadth_first(tree, children=iter):
   """Traverse the nodes of a tree in breadth-first order.
   The first argument should be the tree root; children
   should be a function taking as argument a tree node and
   returning an iterator of the node's children.
   """
   yield tree
   last = tree
   for node in breadth_first(tree, children):
      for child in children(node):
         yield child
         last = child
      if last == node:
         return

def getCurrentDir():
   return BuildSystem.Utilities.Env[-1].path

def getCurrentRealDir():
   return BuildSystem.Utilities.Env[-1].realpath

def getExt(strings):
   """Returns the extension of a string. If the input is a list of string,
   returns the extension of the first string"""
   ret = set()
   if strings:
      if isinstance(strings, list):
         for elem in strings:
            ret.add(getExt(elem))
      else:
         (root, ext) = BuildSystem.Defaults.Tools.FileManager.path.splitext(strings)
         ret.add(ext[1:])
   if len(ret) == 1:
      for x in ret:
         elem = x
      return elem
   else:
      return ret

def formatName(format, strings):
   """Replaces the occurences of 'NAME' with all of the incoming strings using
   the specified format template."""
   t = string.Template(format)
   output = list()
   if isinstance(strings, list):
      for x in list_iter(strings):
         d = dict(NAME=x)
         output.append(t.substitute(d))
   else:
      d = dict(NAME=strings)
      output.append(t.substitute(d))
   return ' '.join(output)


def formatNameValue(format, nameStr, valueStr):
   """Replaces the occurences of 'NAME' and 'VALUE' with the oncoming string
   using the specified format template."""
   t = string.Template(format)
   d = dict(NAME=nameStr, VALUE=valueStr)
   return t.substitute(d)


def someNotInSet(someList, someSet):
   """Returns true if any element in someList is not in someSet"""
   for elem in someList:
      if elem not in someSet:
         return True
   return False


def someChange(someList):
   """Returns true if any element in someList is not None"""
   return someNotInSet(someList, [None])


def someError(someList):
   """Returns true if any element in someList is not None nor 0"""
   return someNotInSet(someList, [None, 0])


def substituteVariables(aString, aMapping):
   if isinstance(aString, list):
      ret = list()
      for elem in aString:
         ret.append(substituteVariables(elem, aMapping))
      return ret
   else:
      tmpl = string.Template(aString)
      bString = tmpl.substitute(aMapping)
      while aString != bString:
         aString = bString
         tmpl = string.Template(aString)
         bString = tmpl.substitute(aMapping)
      return bString

def get_path_to_reach(dst_env, src_env=None):
   if not src_env:
      src_env = BuildSystem.Utilities.Env[-1]
   src_to_dst_path = BuildSystem.Utilities.path_diff(src_env.realpath, dst_env.realpath)
   #Substitute full path if it's shorter than the relative path
   if len(dst_env.realpath) < len(src_to_dst_path):
      src_to_dst_path = dst_env.realpath
   #print "src: %s, dst: %s, diff: %s" % (src_env.realpath, dst_env.realpath, src_to_dst_path)
   return src_to_dst_path

def run(app, args=None):
   """Quickly tests the app by appending the shared library path(s)."""
   err = app() #Compile first
   if not err:
      #Flush (for MinGW)
      sys.stdout.flush()
      #Set environment
      BuildSystem.Utilities.Env.append(app.env)
      BuildSystem.Utilities.Env[-1].setCurrent()
      shared_lib_paths = list()
      for elem in app.inputs:
         if isinstance(elem, BuildSystem.Parts.Library) and elem.isShared():
            relative_path = BuildSystem.Defaults.Tools.FileManager.path.join(
                              BuildSystem.Utilities.get_path_to_reach(src_env=app.env, dst_env=elem.env),
                              BuildSystem.Defaults.Tools.Compiler.get_library_dir_path(elem))
            shared_lib_paths.append(relative_path)
      if BuildSystem.Defaults.Env.Platform == "windows":
         var = "PATH"
      elif BuildSystem.Defaults.Env.PlatformFlavor == "linux":
         var = "LD_LIBRARY_PATH"
      elif BuildSystem.Defaults.Env.PlatformFlavor == "macosx":
         var = "DYLD_LIBRARY_PATH"
      else:
         print "Error: Unsupported platform in run()"
         sys.exit(1)
      #Add the previous variable value, if it exists
      if var in os.environ:
         shared_lib_paths.append(os.environ[var])
      command = list()
      command.append('env')
      #Pathsep is ';' under Windows, ':' otherwise
      command.append(var + '=' + os.path.pathsep.join(shared_lib_paths))
      if "BS_RUN_PREFIX" in os.environ:
         bs_run_prefix = os.environ["BS_RUN_PREFIX"]
         if BuildSystem.Utilities.Options.opts.verbose >= 1:
            print "Using BS_RUN_PREFIX: '" + bs_run_prefix + "'"
         bs_run_split = bs_run_prefix.split(" ")
         command.extend(bs_run_split)
      app_path = BuildSystem.Defaults.Tools.Compiler.get_application_file_path(app)
      command.append(app_path)
      if args:
         if BuildSystem.Utilities.Options.opts.verbose >= 1:
            print "Running with arguments: " + str(args)
         if isinstance(args, list):
            command.extend(args)
         elif isinstance(args, basestring):
            command.append(args)
         else:
            "Unsupported argument type: " + str(args)
            sys.exit(2)
      if "BS_RUN_SUFFIX" in os.environ:
         bs_run_suffix = os.environ["BS_RUN_SUFFIX"]
         if BuildSystem.Utilities.Options.opts.verbose >= 1:
            print "Using BS_RUN_SUFFIX: '" + bs_run_suffix + "'"
         bs_run_split = bs_run_suffix.split(" ")
         command.extend(bs_run_split)
      #print "Submitting: " + str(command)
      theTask = BuildSystem.Task(command)
      if BuildSystem.Utilities.Options.opts.no_run:
         print "Running app: " + str(theTask)
      else:
         #Execute
         err = BuildSystem.Defaults.Tools.TaskManager.submit(theTask, silent=True)
      #Restore environment
      BuildSystem.Utilities.Env.pop()
      BuildSystem.Utilities.Env[-1].setCurrent()
      return err

def deploy(app, dstDir=None):
   """Copies all of the dependent libraries and application file into the destination directory."""
   err = app() #Compile first
   if err:
      print "Error compiling application, deploy aborted."
      return err
   #Flush (for MinGW)
   sys.stdout.flush()
   dstDir = dstDir or (BuildSystem.Defaults.Tools.FileManager.path.join("deployed", app.getName()))
   print "Deploying '" + app.getName() + "' into: " + dstDir
   cur_env = BuildSystem.Utilities.Env[-1]
   #Create destination directory
   err = BuildSystem.Defaults.Tools.FileManager.mkdir(dstDir)
   if err:
      print "Error creating directory: " + dstDir
      return err
   app_path = BuildSystem.Defaults.Tools.FileManager.path.join(
                  BuildSystem.Utilities.get_path_to_reach(src_env=cur_env, dst_env=app.env),
                  BuildSystem.Defaults.Tools.Compiler.get_application_file_path(app))
   err = BuildSystem.Defaults.Tools.FileManager.copy(app_path, dstDir)
   if err:
      print "Error copying application file: " + app_path
      return err
   else:
      print "Copied " + app_path
   for elem in app.inputs:
      if isinstance(elem, BuildSystem.Parts.Library) and elem.isShared():
         lib_path = BuildSystem.Defaults.Tools.FileManager.path.join(
                        BuildSystem.Utilities.get_path_to_reach(src_env=cur_env, dst_env=elem.env),
                        BuildSystem.Defaults.Tools.Compiler.get_library_file_path(elem))
         err = BuildSystem.Defaults.Tools.FileManager.copy(lib_path, dstDir)
         if err:
            print "Error copying shared library file: " + lib_path
            return err
         else:
            print "Copied " + lib_path
   return None

def path_diff(fromPath, toPath):
   """Generates a difference path which can be used to go from a path
   to another.
   Ex:
      a = "Base/DirA/same"
      b = "Base/DirB/same"
      print path_diff(a, b)
      >> ../../DirB/same
   """
   fromFull = os.path.realpath(fromPath)
   toFull = os.path.realpath(toPath)
   fromParts = fromFull.split(os.path.sep)
   toParts = toFull.split(os.path.sep)
   #Eat leading parts which are similar
   while len(fromParts) > 0 and len(toParts) > 0 and fromParts[0] == toParts[0]:
      del fromParts[0]
      del toParts[0]
   ret = list()
   #Move out of the source directory
   for x in fromParts:
      ret.append("..")
   #Dig inside the destination
   ret.extend(toParts)
   #Join the list of parts (flattened as args)
   if ret:
      return os.path.join(*ret)
   else:
      return "."


def uncompress(src_path, dst_path=""):
   """Uncompresses an archive.
   """
   if zipfile.is_zipfile(src_path):
      if BuildSystem.Utilities.Options.opts.verbose >= 1:
         print "Uncompressing zip file: " + str(src_path)
         if dst_path:
            print "into: " + dst_path
      zf = zipfile.ZipFile(src_path, "r")
      join = BuildSystem.Defaults.Tools.FileManager.path.join
      normpath = BuildSystem.Defaults.Tools.FileManager.path.normpath
      split = BuildSystem.Defaults.Tools.FileManager.path.split
      for n in zf.namelist():
         if BuildSystem.Utilities.Options.opts.verbose >= 2:
            print ">>>" + str(n)
         if not n.endswith('/'):
            dirName, fileName = split(n)
            newDirName = normpath(join(dst_path, dirName))
            BuildSystem.Defaults.Tools.FileManager.mkdir(newDirName)
            file(join(newDirName, fileName), 'wb').write(zf.read(n))
      zf.close()
   elif tarfile.is_tarfile(src_path):
      if BuildSystem.Utilities.Options.opts.verbose >= 1:
         print "Uncompressing tar file: " + str(src_path)
         if dst_path:
            print "into: " + dst_path
      tf = tarfile.open(src_path, "r")
      members = tf.getmembers()
      for m in members:
         if BuildSystem.Utilities.Options.opts.verbose >= 2:
            print ">>>" + str(m.name)
         tf.extract(m, dst_path)
      tf.close()
   if BuildSystem.Utilities.Options.opts.verbose >= 1:
      print "Done."

def path_to_unix(path):
   new_path = path.replace("\\", "/").strip()
   #if new_path[1:3] == ":/":
   #   #Convert "c:/path" into "/c/path"
   #   new_path = "/" + new_path[0] + new_path[2:]
   return new_path
