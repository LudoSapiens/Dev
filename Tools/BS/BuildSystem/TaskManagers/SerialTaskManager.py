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
import sys

from BuildSystem.TaskManagers.Task import Task
import BuildSystem.Utilities

class SerialTaskManager:
   """A task manager: handles tasks issued to the system.
   This task manager handles tasks one after the other (serially).
   Returns None if nothing was executed (no run).
   Returns 0 if the command was sucessful.
   Returns the command's error code if it executed but hit an error.
   Returns an OSError if the command could not be executed."""
   def submit(self, tasks, stdout=None, silent=False, taskPrefix=""):
      sys.stdout.flush() #For MinGW
      try:
         for task in BuildSystem.Utilities.breadth_first(tasks, BuildSystem.Utilities.list_iter):
            if isinstance(task, Task):
               command = task.getCommand()
               if BuildSystem.Utilities.Options.opts.no_run and not silent:
                  if not silent: print "*" + taskPrefix + " " + command
                  return None
               else:
                  if not silent: print taskPrefix + " " + command
                  retcode = subprocess.call(command, stdout=stdout, shell=True)
                  if not silent and BuildSystem.Utilities.Options.opts.verbose >= 1:
                     if retcode < 0:
                        print >>sys.stderr, "Child was terminated by signal", -retcode
                     else:
                        print >>sys.stderr, "Child returned", retcode
                  return retcode
      except OSError, e:
         print >>sys.stderr, "Execution failed:", e
         return e

def __call__():
   return SerialTaskManager()

