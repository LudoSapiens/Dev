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

import BuildSystem.Defaults.Tools

class Environment:
   """A class containing the build directory, as well as other fields."""
   def __init__(self, path=None, realpath=None, defaultOutputDir=None, filename=None):
      if path:
         self.path = path
      else:
         self.path = os.path.curdir
      #Need to use os.path else '\' defines a drive under Windows in os.chdir
      if realpath:
         self.realpath = realpath
      else:
         self.realpath = os.path.realpath(self.path)
      #print "EnvPath=" + str(self.path) + ", realpath=" + self.realpath
      if defaultOutputDir:
         self.defaultOutputDir=defaultOutputDir
      else:
         self.defaultOutputDir=BuildSystem.Defaults.Tools.FileManager.path.join("out", sys.platform)
      self.filename = filename
      #print "Environment: " + self.path + ", " + self.defaultOutputDir
   def setCurrent(self):
      if BuildSystem.Utilities.Options.opts.verbose >= 2:
         print "Changing env directory to: " + str(self.realpath)
      os.chdir(self.realpath)
      if BuildSystem.Utilities.Options.opts.verbose >= 2:
         print "Current dir is: " + os.path.abspath(os.curdir)
   def __str__(self):
      return "<Environment: path=%s, realpath=%s>" \
             % (str(self.path), str(self.realpath))
   def __repr__(self):
      return "<Environment: path=%s, realpath=%s>" \
             % (str(self.path), str(self.realpath))
