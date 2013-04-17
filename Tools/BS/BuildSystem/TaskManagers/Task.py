# =============================================================================
#  Copyright (c) 2006, Ludo Sapiens Inc.
#  All rights reserved.
# 
#  These coded instructions, statements, and computer programs contain
#  unpublished, proprietary information and are protected by Federal copyright
#  law. They may not be disclosed to third parties or copied or duplicated in
#  any form, in whole or in part, without prior written consent.
# =============================================================================

import subprocess
import types

class Task:
   """A task: any operation performed by and issued to the TaskManager."""
   def __init__(self, args, executable=None):
      self.args = args
      self.executable = executable
   def getCommand(self):
      if(self.executable):
         command = self.executable
      else:
         command = ''
      if not isinstance(self.args, types.StringTypes):
         self.args = subprocess.list2cmdline(self.args)
      command += self.args
      return command
   def __str__(self):
      return "<Task: %s>" % str(self.getCommand())
