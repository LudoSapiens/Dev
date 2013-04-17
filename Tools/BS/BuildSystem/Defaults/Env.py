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
import platform

##
# This modules list environment values
# It *MUST NOT* contain any dependency on tools, since the intent
# is for tools to import this module to tweak behavior based on 
# these values (which would result in circular imports)
##

Platform             = None
PlatformFlavor       = None
PlatformVersion      = None
PlatformArchitecture = None
if "BS_PLATFORM" in os.environ:
   Platform = os.environ["BS_PLATFORM"]
if "BS_PLATFORM_FLAVOR" in os.environ:
   PlatformFlavor = os.environ["BS_PLATFORM_FLAVOR"]
if "BS_PLATFORM_VERSION" in os.environ:
   PlatformVersion = os.environ["BS_PLATFORM_VERSION"]
if "BS_PLATFORM_ARCH" in os.environ:
   PlatformArchitecture = os.environ["BS_PLATFORM_ARCH"]

# Setting up platform
if sys.platform == "win32":
   Platform = Platform or "windows"
   PlatformFlavor = PlatformFlavor or "mingw"
   PlatformVersion = PlatformVersion or platform.release()
   PlatformArchitecture = PlatformArchitecture or "native"
elif sys.platform == "cygwin":
   Platform = Platform or "windows"
   PlatformFlavor = PlatformFlavor or "cygwin"
   PlatformVersion = PlatformVersion or platform.release()
   PlatformArchitecture = PlatformArchitecture or "native"
elif sys.platform == "darwin":
   Platform = Platform or "unix"
   PlatformFlavor = PlatformFlavor or "macosx"
   PlatformVersion = PlatformVersion or platform.release()
   PlatformArchitecture = PlatformArchitecture or "native"
elif sys.platform.startswith("linux"):
   Platform = Platform or "unix"
   PlatformFlavor = PlatformFlavor or "linux"
   PlatformVersion = PlatformVersion or platform.release()
   PlatformArchitecture = PlatformArchitecture or "native"
else:
   print "Unsupported platform: " + sys.platform
   if Platform and PlatformFlavor and PlatformVersion:
      print "... but since you specified everything...!"
   else:
      sys.exit(1)

#print(Platform, PlatformFlavor, PlatformVersion)
