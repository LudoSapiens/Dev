# =============================================================================
#  Copyright (c) 2006, Ludo Sapiens Inc.
#  All rights reserved.
# 
#  These coded instructions, statements, and computer programs contain
#  unpublished, proprietary information and are protected by Federal copyright
#  law. They may not be disclosed to third parties or copied or duplicated in
#  any form, in whole or in part, without prior written consent.
# =============================================================================

import BuildSystem.Defaults.Tools
import BuildSystem.Utilities

import os

class TimestampDependencyChecker:
   """A dependency checker considering timestamps."""
   def __init__(self):
      self.timeCache = dict()
      self.outstanding = dict()
   def addOutstanding(self, theFile, theDir=None):
      theDir = theDir or BuildSystem.Utilities.getCurrentRealDir()
      if theDir not in self.outstanding:
         self.outstanding[theDir] = set()
      self.outstanding[theDir].add( theFile )
   def removeOutstanding(self, theFile, theDir=None):
      theDir = theDir or BuildSystem.Utilities.getCurrentRealDir()
      if theDir not in self.outstanding:
         print "Error: trying to remove file not in outstanding set"
      self.outstanding[theDir].remove( theFile )
   def updateOutstanding(self, theFile, theDir=None):
      theDir = theDir or BuildSystem.Utilities.getCurrentRealDir()
      if theDir in self.outstanding:
         dirSet = self.outstanding[theDir]
         if theFile in dirSet:
            self.updateTime(theFile, theDir)
            dirSet.remove( theFile )
   def updateTime(self, theFile, theDir=None):
      theDir = theDir or BuildSystem.Utilities.getCurrentRealDir()
      if theDir not in self.timeCache:
         self.timeCache[theDir] = dict()
      theDirDict = self.timeCache[theDir]
      theDirDict[theFile] = BuildSystem.Defaults.Tools.FileManager.path.getmtime(theFile)
      return theDirDict[theFile]
   def getTime(self, theFile, theDir=None):
      #return BuildSystem.Defaults.Tools.FileManager.path.getmtime(theFile)
      theDir = theDir or BuildSystem.Utilities.getCurrentRealDir()
      if theDir not in self.timeCache:
         self.timeCache[theDir] = dict()
      theDirDict = self.timeCache[theDir]
      if theFile not in theDirDict:
         theDirDict[theFile] = BuildSystem.Defaults.Tools.FileManager.path.getmtime(theFile)
      else:
         self.updateOutstanding(theFile, theDir)
      return theDirDict[theFile]
   def check_dependencies(self, files, deps):
      need_update = False
      if isinstance(files, list):
         for x in files:
            if self.check_file_dependencies(x, deps):
               return true
         return false
      else:
         return self.check_file_dependencies(files, deps)
   def check_file_dependencies(self, theFile, theDeps):
      if BuildSystem.Utilities.Options.opts.verbose >= 2:
         print "Checking if file '%s' needs updating" % theFile
      if not BuildSystem.Defaults.Tools.FileManager.path.exists(theFile):
         if BuildSystem.Utilities.Options.opts.verbose >= 2:
            print theFile + " needs to be created"
         return True
      #Check last modified time, and make sure no dep was updated since
      check_time = self.getTime(theFile)
      deps = theDeps[theFile]
      for x in deps:
         if not BuildSystem.Defaults.Tools.FileManager.path.exists(x):
            if BuildSystem.Utilities.Options.opts.verbose >= 2:
               print theFile + " needs updating because " + x + " got deleted"
            return True
         cur_time = self.getTime(x)
         if BuildSystem.Utilities.Options.opts.verbose >= 3:
            print " " + x + " (" + str(cur_time) + " vs. " + str(check_time) + ")"
         if cur_time > check_time:
            self.addOutstanding( theFile )
            if BuildSystem.Utilities.Options.opts.verbose >= 2:
               print theFile + " needs updating because of " + x
            return True
      if BuildSystem.Utilities.Options.opts.verbose >= 2:
         print "No"
      return False

