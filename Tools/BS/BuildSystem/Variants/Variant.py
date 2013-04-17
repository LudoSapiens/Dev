# =============================================================================
#  Copyright (c) 2006, Ludo Sapiens Inc.
#  All rights reserved.
# 
#  These coded instructions, statements, and computer programs contain
#  unpublished, proprietary information and are protected by Federal copyright
#  law. They may not be disclosed to third parties or copied or duplicated in
#  any form, in whole or in part, without prior written consent.
# =============================================================================

import copy
import sys

class Variant:
   """A list of string and values used to tweak build execution."""
   def __init__(self, values=None):
      self.values = set()
      self.add(values)
   def __contains__(self, item):
      return item in self.values
   def __str__(self):
      return "Variant:" + str(self.values)
   def add(self, item):
      if item:
         if isinstance(item, basestring):
            self.values.add(item)
         elif isinstance(item, list):
            for x in item:
               self.add(x)
         elif isinstance(item, Variant):
            self.values.update(item.values)
         else:
            print "Error: unknown type for " + str(item)
            sys.exit(1)
   def remove(self, item):
      return self.values.remove(item)
   def discard(self, item):
      return self.values.discard(item)
   def copy(self):
      return copy.deepcopy(self)
