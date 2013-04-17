# =============================================================================
#  Copyright (c) 2006, Ludo Sapiens Inc.
#  All rights reserved.
# 
#  These coded instructions, statements, and computer programs contain
#  unpublished, proprietary information and are protected by Federal copyright
#  law. They may not be disclosed to third parties or copied or duplicated in
#  any form, in whole or in part, without prior written consent.
# =============================================================================

from BuildSystem.Parts.CompiledBinary import CompiledBinary
from BuildSystem.Variants.Variant import Variant

class Library(CompiledBinary):
   """A library container object defining all of the necessary fields to make a library"""
   def __init__(self, name=None, inputs=None, output=None, outputDir=None, settings=None, variant=None, shared=None, shared_define=None, env=None):
      CompiledBinary.__init__(self, name=name, inputs=inputs, output=output, settings=settings, variant=variant, env=env)
      if shared:
         self.variant.add("shared")
         self.shared_define = shared_define
   def isShared(self):
      return self.variant and "shared" in self.variant

