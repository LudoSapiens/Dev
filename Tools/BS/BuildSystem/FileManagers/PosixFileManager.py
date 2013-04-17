# =============================================================================
#  Copyright (c) 2006, Ludo Sapiens Inc.
#  All rights reserved.
# 
#  These coded instructions, statements, and computer programs contain
#  unpublished, proprietary information and are protected by Federal copyright
#  law. They may not be disclosed to third parties or copied or duplicated in
#  any form, in whole or in part, without prior written consent.
# =============================================================================

import posixpath
import shutil
import os

import BuildSystem.Utilities

class PosixFileManager:
   """A class to create/move/delete/manipulate files and directories."""
   def __init__(self):
      self.path = posixpath  #use '/' everywhere
      self.path.to_unix = BuildSystem.Utilities.path_to_unix
   def mkdir(self, path):
      if path:
         if not self.path.isdir(path):
            err = os.makedirs(path)
            if err:
               print "Error making directory: " + str(path)
               sys.exit(3)
            return err
      return None
   def copy(self, src, dst, symlinks=False):
      if not isinstance(dst, basestring):
         raise TypeError, "PosixFileManager.copy: wrong destination type"
      if self.path.isdir(src):
         shutil.copytree(src, dst)
      else:
         shutil.copy2(src, dst)
   def delete(self, path, delete_empty_dirs=False, delete_non_empty_dirs=False):
      if path and self.path.lexists(path):
         if self.path.isdir(path):
            if not os.listdir(path):
               #Delete empty directory
               if BuildSystem.Utilities.Options.opts.verbose >= 1:
                  print "Removing directory: " + str(path)
               err = os.rmdir(path)
               if err:
                  #Could not delete directory for some reason
                  return "Directory not deleted"
               else:
                  #Check recursively
                  if delete_empty_dirs:
                     if BuildSystem.Utilities.Options.opts.verbose >= 1:
                        print "Removing parent directory: " + str(os.path.dirname(path))
                     os.removedirs(os.path.dirname(path))
                  return None
            else:
               if delete_non_empty_dirs:
                  err = shutil.rmtree(path)
                  if err:
                     raise RuntimeError, "Directory could not be removed"
                  else:
                     return None
               else:
                  raise RuntimeError, "Directory not empty"
               return "Directory not empty"
         else:
            if BuildSystem.Utilities.Options.opts.verbose >= 1:
               print "Removing file: " + str(path)
            err = os.remove(path)
            if err:
               #Could not delete file for some reason
               return "File not deleted"
            else:
               #Check recursively
               parent_dir = os.path.dirname(path)
               if delete_empty_dirs and not os.listdir(parent_dir):
                  if BuildSystem.Utilities.Options.opts.verbose >= 1:
                     print "Removing parent directory: " + str(parent_dir)
                  os.removedirs(parent_dir)
               return None
   def rename(self, src_path, dst_path):
      if BuildSystem.Utilities.Options.opts.verbose >= 1:
         print "Renaming '%s' to '%s'" % (src_path, dst_path)
      return os.rename(src_path, dst_path)
