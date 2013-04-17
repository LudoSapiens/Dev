# =============================================================================
#  Copyright (c) 2006, Ludo Sapiens Inc.
#  All rights reserved.
# 
#  These coded instructions, statements, and computer programs contain
#  unpublished, proprietary information and are protected by Federal copyright
#  law. They may not be disclosed to third parties or copied or duplicated in
#  any form, in whole or in part, without prior written consent.
# =============================================================================

import BuildSystem.Utilities
from BuildSystem.Parts.CompiledBinary import CompiledBinary

class Application(CompiledBinary):
   """An application container object defining all of the necessary fields to make an executable"""
   def __init__(self, name=None, inputs=None, output=None, settings=None, variant=None, env=None):
      CompiledBinary.__init__(self, name=name, inputs=inputs, output=output, settings=settings, variant=variant, env=env)
   def run(self, args=None):
      return BuildSystem.Utilities.run(self, args=args)
   def deploy(self, dstDir=None):
      return BuildSystem.Utilities.deploy(self, dstDir)
