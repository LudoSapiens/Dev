# =============================================================================
#  Copyright (c) 2007, Ludo Sapiens Inc.
#  All rights reserved.
#
#  These coded instructions, statements, and computer programs contain
#  unpublished, proprietary information and are protected by Federal copyright
#  law. They may not be disclosed to third parties or copied or duplicated in
#  any form, in whole or in part, without prior written consent.
# =============================================================================

import BuildSystem.Defaults.Tools
import BuildSystem.Utilities

import gzip
import os
import pickle

class CachedDependencyChecker:
   """
   A dependency checker which uses a cache.
   The cache consists in a dictionary for every "Environment" directory.
   Every entry in that dictionary consists in a (outFile, depList) pair
   which has to be split outside to play well with the TimestampDependencyChecker.
   The key to that second level of dictionary is the inFile.
   """
   def __init__(self):
      self.cacheFilename = ".bs_dep_cache"
      self.cache = dict()
      self.pickle_prot = -1
      self.use_gzip = False
   def save_all_deps(self):
      #print "Storing caches into '%s' files." % self.cacheFilename
      #self.print_deps()
      for (dirName, dirDict) in self.cache.iteritems():
         self.save_deps(dirName, dirDict)
   def save_deps(self, dirName, dirDict):
      #print "OS=" + str(os)
      #print "OS.PATH=" + str(os.path)
      #print "OS.PATH.JOIN=" + str(os.path.join)
      filename = os.path.join(dirName, self.cacheFilename)
      #print "Dumping into: " + str(filename)
      if self.use_gzip:
         cacheFile = gzip.open(filename, "wb", 1)
      else:
         cacheFile = open(filename, "wb")
      if cacheFile:
         pickle.dump(dirDict, cacheFile, self.pickle_prot)
      else:
         print "Error opening '%s' for writing." % filename
   def print_deps(self):
      print "Printing all of the currently-cached dependencies:"
      for (dirName, dirDict) in self.cache.iteritems():
         print "Under: " + dirName
         for (fileName, (outFile, deps)) in dirDict.iteritems():
            for dep in deps:
               print "  %s ---> %s" % (fileName, dep)
   def set_deps(self, inFile, outFile, deps, theDir=None):
      outFile = outFile or inFile
      theDir = theDir or BuildSystem.Utilities.getCurrentRealDir()
      if theDir not in self.cache:
         self.cache[theDir] = dict()
      self.cache[theDir][inFile] = (outFile, deps)
   def get_out_deps(self, theFile, theDir=None):
      theDir = theDir or BuildSystem.Utilities.getCurrentRealDir()
      out = None
      deps = dict()
      if theDir in self.cache:
         dirDict = self.cache[theDir]
      else:
         dirDict = None
      if not dirDict:
         cacheFile = None
         if os.path.exists(self.cacheFilename):
            if self.use_gzip:
               cacheFile = gzip.open(self.cacheFilename, 'rb')
            else:
               cacheFile = open(self.cacheFilename, 'rb')
         if cacheFile:
            #print "Loading cache from '%s'." % BuildSystem.Defaults.Tools.FileManager.path.join(theDir, theFile)
            try:
               dirDict = pickle.load(cacheFile)
            except EOFError:
               # Probably corrupted, just ignore it.
               print "Ignoring corrupt cache file: %s" % BuildSystem.Defaults.Tools.FileManager.path.join(os.getcwd(), self.cacheFilename)
               dirDict = dict()
         else:
            #print "Initializing the dict for dir: '%s'" % theDir
            dirDict = dict()
         #print "Adding dir '%s': %s" % (theDir, str(dirDict))
         self.cache[theDir] = dirDict
      if theFile in dirDict:
         out = dirDict[theFile][0]
         deps[out] = dirDict[theFile][1]
      return (out,deps)
