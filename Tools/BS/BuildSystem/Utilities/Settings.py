# =============================================================================
#  Copyright (c) 2006, Ludo Sapiens Inc.
#  All rights reserved.
# 
#  These coded instructions, statements, and computer programs contain
#  unpublished, proprietary information and are protected by Federal copyright
#  law. They may not be disclosed to third parties or copied or duplicated in
#  any form, in whole or in part, without prior written consent.
# =============================================================================

import BuildSystem.Defaults.Variables
import copy

class Settings:
   """This class collects various variables which can tweak the behavior of
   various tools interpreting them.
   These values are compiler-agnostic, and therefore, cross-platform.
   These values are also non-specific, and can be used across multiple files."""
   def __init__(self,
                defines=dict(),
                source_paths=list(), dependency_paths=list(), include_paths=list(),
                obj_dir=None,
                out_dir=None
               ):
      self.defines = defines
      self.cflags = list()
      self.cxxflags = list()
      self.frameworks = list()
      self.ldflags = list()
      self.source_directories_source = source_paths
      self.source_directories_dependencies = dependency_paths
      self.source_directories_includes = include_paths
      self.intermediate_directory_objects = obj_dir
      self.intermediate_directory_binaries = None
      self.intermediate_directory_executables = None
      self.intermediate_directory_libraries = None
      if out_dir:
         self.destination_directory_binaries = out_dir
         self.destination_directory_executables = out_dir
         self.destination_directory_libraries = out_dir
      else:
         self.destination_directory_binaries = None
         self.destination_directory_executables = None
         self.destination_directory_libraries = None
   #Utility routines
   def getDefines(self):
      return self.defines
   #Source routines
   def getSourcePaths(self):
      return self.source_directories_source
   def getSourceDependencyPaths(self):
      return self.source_directories_dependencies
   def getSourceIncludePaths(self):
      return self.source_directories_includes
   #Intermediate routines
   ##
   # These routines use the following priorities:
   # - Locally-specified directory
   # - Globally-specified default
   # - Destination equivalent
   # In addition, executables and libraries will fallback on binaries values.
   def getIntermediateDirectoryForObjects(self):
      if self.intermediate_directory_objects:
         return self.intermediate_directory_objects
      elif BuildSystem.Defaults.Variables.settings.intermediate_directory_objects:
         return BuildSystem.Defaults.Variables.settings.intermediate_directory_objects
      else:
         return self.getDestinationDirectoryForBinaries()
   def getIntermediateDirectoryForBinaries(self):
      if self.intermediate_directory_binaries:
         return self.intermediate_directory_binaries
      elif BuildSystem.Defaults.Variables.settings.intermediate_directory_binaries:
         return BuildSystem.Defaults.Variables.settings.intermediate_directory_binaries
      else:
         return self.getDestinationDirectoryForBinaries()
   def getIntermediateDirectoryForExecutables(self):
      if self.intermediate_directory_executables:
         return self.intermediate_directory_executables
      elif BuildSystem.Defaults.Variables.intermediate_directory_executables:
         return BuildSystem.Defaults.Variables.settings.intermediate_directory_executables
      elif self.intermediate_directory_binaries:
         return self.intermediate_directory_binaries
      elif BuildSystem.Defaults.Variables.settings.intermediate_directory_binaries:
         return BuildSystem.Defaults.Variables.settings.intermediate_directory_binaries
      else:
         return self.getIntermediateDirectoryForExecutables()
   def getIntermediateDirectoryForLibraries(self):
      if self.intermediate_directory_libraries:
         return self.intermediate_directory_libraries
      elif BuildSystem.Defaults.Variables.settings.intermediate_directory_libraries:
         return BuildSystem.Defaults.Variables.settings.intermediate_directory_libraries
      elif self.intermediate_directory_binaries:
         return self.intermediate_directory_binaries
      elif BuildSystem.Defaults.Variables.settings.intermediate_directory_binaries:
         return BuildSystem.Defaults.Variables.settings.intermediate_directory_binaries
      else:
         return self.getIntermediateDirectoryForLibraries()
   #Destination routines
   def getDestinationDirectoryForBinaries(self):
      if self.destination_directory_binaries:
         return self.destination_directory_binaries
      else:
         return BuildSystem.Defaults.Variables.settings.destination_directory_binaries
   def getDestinationDirectoryForExecutables(self):
      if self.destination_directory_executables:
         return self.destination_directory_executables
      elif BuildSystem.Defaults.Variables.settings.destination_directory_executables:
         return BuildSystem.Defaults.Variables.settings.destination_directory_executables
      else:
         return self.getDestinationDirectoryForBinaries()
   def getDestinationDirectoryForLibraries(self):
      if self.destination_directory_libraries:
         return self.destination_directory_libraries
      elif BuildSystem.Defaults.Variables.settings.destination_directory_libraries:
         return BuildSystem.Defaults.Variables.settings.destination_directory_libraries
      else:
         return self.getDestinationDirectoryForBinaries()
   def __str__(self):
      strList = list()
      strList.append("")
      strList.append("define=" + str(self.defines))
      if self.source_directories_source         : strList.append("source_directories_source="          + str(self.source_directories_source))
      if self.source_directories_dependencies   : strList.append("source_directories_dependencies="    + str(self.source_directories_dependencies))
      if self.source_directories_includes       : strList.append("source_directories_includes="        + str(self.source_directories_includes))
      if self.intermediate_directory_objects    : strList.append("intermediate_directory_objects="     + str(self.intermediate_directory_objects))
      if self.intermediate_directory_binaries   : strList.append("intermediate_directory_binaries="    + str(self.intermediate_directory_binaries))
      if self.intermediate_directory_executables: strList.append("intermediate_directory_executables=" + str(self.intermediate_directory_executables))
      if self.intermediate_directory_libraries  : strList.append("intermediate_directory_libraries="   + str(self.intermediate_directory_libraries))
      if self.destination_directory_binaries    : strList.append("destination_directory_binaries="     + str(self.destination_directory_binaries))
      if self.destination_directory_executables : strList.append("destination_directory_executables="  + str(self.destination_directory_executables))
      if self.destination_directory_libraries   : strList.append("destination_directory_libraries="    + str(self.destination_directory_libraries))
      return "Settings[" + '\n    '.join(strList) + "\n]"
   def copy(self):
      return copy.deepcopy(self)
