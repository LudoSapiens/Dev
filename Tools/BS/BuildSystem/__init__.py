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
import sys

##
# Notes:
##
# The reason we use a form like:
#   from BuildSystem.Compiler.GCCCompiler import GCCCompiler
# is because this is the only way we can uses classes (which allow for
# for inheritance, and therefore, more fine-tuning of behavior) while still
# keeping cleanliness.
# We debated whether or not to name the compiler GCC instead of GCCCompiler,
# but settled on the more verbose style in order to prevent name clashes
# (e.g. Mac, or Standard, which would be shared across different tools).
#
# Another option would have been to have a single Compilers.py file containing
# all of the various compilers.  But this approach makes extending a less clean.
##

import BuildSystem.Defaults.Variables
import BuildSystem.Utilities

from BuildSystem.Handlers.AutoconfHandler import AutoconfHandler
from BuildSystem.Handlers.CopyHandler import CopyHandler
from BuildSystem.Parts.Application import Application
from BuildSystem.Parts.Library import Library
from BuildSystem.TaskManagers.Task import Task
from BuildSystem.Utilities.Environment import Environment
from BuildSystem.Utilities.Settings import Settings
from BuildSystem.Variants.Variant import Variant

_loaded_files = dict()

def FindProjectFiles(dir_root, filename):
   """Returns the list of project files starting at the specified directory."""
   ret = list()
   (head, tail) = os.path.split(dir_root)
   # Putting recursive call here so that more generic project files
   # get sourced earlier
   if head != dir_root:
      ret += FindProjectFiles(head, filename)
   f = os.path.join(dir_root, filename)
   if os.path.exists(f):
      ret.append(f)
   return ret

_load_file_level = 0
def LoadFile(filename):
   """Sources an auxiliary BSFile, properly handling the current environment
   (Env)."""
   real_path = os.path.realpath(filename)
   if real_path in _loaded_files:
      #Register this path
      _loaded_files[real_path].add(filename)
      if BuildSystem.Utilities.Options.opts.verbose >= 1:
         print "Skipping previously loaded file: " + filename
   else:
      #Register this path
      _loaded_files[real_path] = set()
      _loaded_files[real_path].add(filename)
      #Actually load the file
      if BuildSystem.Utilities.Options.opts.verbose >= 1:
         print "+++"
         print "Loading(%d): %s" % (BuildSystem._load_file_level, filename)
      (theDir, theFile) = os.path.split(real_path)
      #Convert directory to a relative value from the start dir
      relDir = BuildSystem.Utilities.path_diff(BuildSystem._start_dir, theDir)
      e = Environment(relDir, theDir, filename=theFile)
      BuildSystem.Utilities.Env.append(e)
      BuildSystem.Utilities.Env[-1].setCurrent()
      BuildSystem._load_file_level = BuildSystem._load_file_level + 1
      execfile(theFile, globals()) # Sending the globals table in order to share get them updated across scripts.
      BuildSystem._load_file_level = BuildSystem._load_file_level - 1
      BuildSystem.Utilities.Env.pop()
      BuildSystem.Utilities.Env[-1].setCurrent()
      if BuildSystem.Utilities.Options.opts.verbose >= 1:
         print "Done loading(%d): %s" % (BuildSystem._load_file_level, filename)
         print "---"

_start_dir = None
def _run():
   try:
      #-------------------
      # Starting execution
      #-------------------
      if BuildSystem.Utilities.Options.opts.verbose >= 1:
         print "Starting BS build"
         print "-----------------"
   
      if BuildSystem.Utilities.Options.opts.verbose >= 2:
         print "Options: " + str(BuildSystem.Utilities.Options.opts)
         print "Args: " + str(BuildSystem.Utilities.Options.args)
   
      # Determine BSFile to source
      if BuildSystem.Utilities.Options.opts.bsfile == None:
         if BuildSystem.Utilities.Options.opts.verbose >= 2: print "Using default BSFile name"
         BuildSystem.Utilities.Options.opts.bsfile = "BSFile"
      else:
         if BuildSystem.Utilities.Options.opts.verbose >= 2: print "Using file " + BuildSystem.Utilities.Options.opts.bsfile + " instead"
   
      # Determine BSProject to source
      if BuildSystem.Utilities.Options.opts.bsproject == None:
         if BuildSystem.Utilities.Options.opts.verbose >= 2: print "Using default BSProject name"
         BuildSystem.Utilities.Options.opts.bsproject = "BSProject"
      else:
         if BuildSystem.Utilities.Options.opts.verbose >= 2: print "Using file " + BuildSystem.Utilities.Options.opts.bsproject + " instead"
   
      # Setting initial environment
      BuildSystem.Utilities.Env.append(BuildSystem.Utilities.Environment())
      BuildSystem._start_dir = os.path.realpath(os.path.curdir)
      if BuildSystem.Utilities.Options.opts.verbose >= 2:
         print "_start_dir: " + str(BuildSystem._start_dir)
   
      # Try to source a project file
      proj_files = BuildSystem.FindProjectFiles(BuildSystem._start_dir, BuildSystem.Utilities.Options.opts.bsproject)
      for f in proj_files:
         BuildSystem.LoadFile(f)
   
      #Set up defaut variants (after project defaults)
      if BuildSystem.Utilities.Options.opts.variants:
         BuildSystem.Defaults.Variables.variants = Variant(BuildSystem.Utilities.Options.opts.variants)
      if BuildSystem.Utilities.Options.opts.verbose >= 1:
         print "Default Variants: " + str(BuildSystem.Defaults.Variables.variants)
   
      # Try to find the BSFile and source it
      BuildSystem.LoadFile(BuildSystem.Utilities.Options.opts.bsfile)
   
      # Then, execute all of the command-line arguments
      #print args
      if BuildSystem.Utilities.Options.args == []:
         if BuildSystem.Utilities.Options.cmd_args:
            BuildSystem.default(BuildSystem.Utilities.Options.cmd_args)
         else:
            BuildSystem.default()
      else:
         for arg in iter(BuildSystem.Utilities.Options.args):
            exec_line = "err = BuildSystem." + arg
            exec_line.strip()
            if exec_line[-1] != ')':
               if BuildSystem.Utilities.Options.cmd_args:
                  exec_line = exec_line + "(" + str(BuildSystem.Utilities.Options.cmd_args) + ")"
               else:
                  exec_line = exec_line + "()"
            #print "EXEC:LINE: " + str(exec_line)
            exec exec_line
   
      # The dump the dep files
      BuildSystem.Defaults.Tools.DependencyCache.save_all_deps()
   
      # Print optional end message
      if BuildSystem.Utilities.Options.opts.verbose >= 1:
         print "-----------------"
         print "Done"
      return err
   except KeyboardInterrupt:
      print "User cancelled the execution"
      return 255

def default():
   "Default target: defined with error message just to make sure it exists"
   print "Warning - No default target defined (nothing was done)  {BuildSystem/__init__.py}"
