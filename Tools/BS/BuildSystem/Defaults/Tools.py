# =============================================================================
#  Copyright (c) 2006, Ludo Sapiens Inc.
#  All rights reserved.
# 
#  These coded instructions, statements, and computer programs contain
#  unpublished, proprietary information and are protected by Federal copyright
#  law. They may not be disclosed to third parties or copied or duplicated in
#  any form, in whole or in part, without prior written consent.
# =============================================================================

import BuildSystem.Defaults.Env

from BuildSystem.Compilers.GCCCompiler import GCCCompiler
from BuildSystem.DependencyCheckers.CachedDependencyChecker import CachedDependencyChecker
from BuildSystem.DependencyCheckers.TimestampDependencyChecker import TimestampDependencyChecker
from BuildSystem.FileManagers.PosixFileManager import PosixFileManager
from BuildSystem.TaskManagers.SerialTaskManager import SerialTaskManager
from BuildSystem.Utilities.Settings import Settings
from BuildSystem.Variants.Variant import Variant


####################
## INITIALIZATION ##
####################
# Setting up static instances
TaskManager = None
FileManager = None
DependencyCache = None
DependencyChecker = None
Compiler = None
if BuildSystem.Defaults.Env.Platform == "windows":
   if BuildSystem.Defaults.Env.PlatformFlavor == "mingw":
      TaskManager = SerialTaskManager()
      FileManager = PosixFileManager()
      DependencyCache = CachedDependencyChecker()
      DependencyChecker = TimestampDependencyChecker()
      Compiler = GCCCompiler() #MinGW/Cygwin environment
   elif BuildSystem.Defaults.Env.PlatformFlavor == "cygwin":
      TaskManager = SerialTaskManager()
      FileManager = PosixFileManager()
      DependencyCache = CachedDependencyChecker()
      DependencyChecker = TimestampDependencyChecker()
      Compiler = GCCCompiler() #MinGW/Cygwin environment
   elif BuildSystem.Defaults.Env.PlatformFlavor == "msvc":
      TaskManager = SerialTaskManager()
      FileManager = PosixFileManager()
      DependencyCache = CachedDependencyChecker()
      DependencyChecker = TimestampDependencyChecker()
      #Compiler = VSCompiler()
elif BuildSystem.Defaults.Env.Platform == "unix":
   if BuildSystem.Defaults.Env.PlatformFlavor == "macosx":
      TaskManager = SerialTaskManager()
      FileManager = PosixFileManager()
      DependencyCache = CachedDependencyChecker()
      DependencyChecker = TimestampDependencyChecker()
      Compiler = GCCCompiler()
   elif BuildSystem.Defaults.Env.PlatformFlavor == "linux":
      TaskManager = SerialTaskManager()
      FileManager = PosixFileManager()
      DependencyCache = CachedDependencyChecker()
      DependencyChecker = TimestampDependencyChecker()
      Compiler = GCCCompiler()
