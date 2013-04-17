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
from BuildSystem.Variants.Variant import Variant

import os

class CompiledBinary:
   """A compiled binary is the base class for Library or Application"""
   def __init__(self, name=None, inputs=None, output=None, settings=None, variant=None, env=None):
      #print ""
      #print " curdir " + str(os.path.realpath(os.curdir))
      #print ' env[-1]=' + str(BuildSystem.Utilities.Env[-1])
      #print "CompiledBinary:" + str(name) + ", " + str(settings) + ' env=' + str(env)
      self.setName(name)
      self.setEnvironment(env)
      self.setInputs(inputs)
      self.setOutput(output)
      self.setSettings(settings)
      self.setVariant(variant)
   def setName(self, name):
      self.name = name
   def getName(self):
      return self.name
   def setInputs(self, inputs):
      if not isinstance(inputs, list):
         self.inputs = list()
         self.inputs.append(inputs)
      else:
         self.inputs = inputs
   def setOutput(self, output):
      self.output = output
   def getOutput(self):
      if self.output:
         return self.output
      else:
         return self.name
   def setSettings(self, settings):
      self.settings = settings
   def getSettings(self):
      if self.settings:
         return self.settings
      else:
         return BuildSystem.Defaults.Variables.settings
   def setVariant(self, variant):
      self.variant=variant or Variant()
   def getVariant(self):
      #print "Default variant: " + str(BuildSystem.Defaults.Variables.variants)
      v = Variant()
      v.add(BuildSystem.Defaults.Variables.variants)
      if self.variant:
         v.add(self.variant)
      return v
   def setEnvironment(self, env):
      if env:
         self.env = env
      else:
         self.env = BuildSystem.Utilities.Env[-1]
   def __call__(self):
      return self.make()
   def make(self):
      return BuildSystem.Defaults.Tools.Compiler.make(self)
   def clean(self, recursive=False):
      return BuildSystem.Defaults.Tools.Compiler.clean(self, recursive=recursive)
   def getRecursiveTargets(self):
      return [x for x in self.inputs if isinstance(x, CompiledBinary)]

