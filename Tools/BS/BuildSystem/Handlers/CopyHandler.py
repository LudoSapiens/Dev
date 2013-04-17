# =============================================================================
#  Copyright (c) 2006, Ludo Sapiens Inc.
#  All rights reserved.
# 
#  These coded instructions, statements, and computer programs contain
#  unpublished, proprietary information and are protected by Federal copyright
#  law. They may not be disclosed to third parties or copied or duplicated in
#  any form, in whole or in part, without prior written consent.
# =============================================================================

import BuildSystem.Defaults

class CopyHandler:
   """A class handling copying of files.
   Uses an internal mapping to keep track of src and dst.
   """
   def __init__(self, name):
      self.name = name
      self.filemap = list()
   def addMapping(self, src, dst):
      self.filemap.append( (src, dst) )
   def clean_destination(self):
      for (src, dst) in self.filemap:
         print "Cleaning: " + str(dst)
         BuildSystem.Defaults.Tools.FileManager.delete(dst, delete_non_empty_dirs=True)
   def copy_destination(self):
      for (src, dst) in self.filemap:
         dst_dir = BuildSystem.Defaults.Tools.FileManager.path.dirname(dst)
         if not BuildSystem.Defaults.Tools.FileManager.path.exists(dst_dir):
            BuildSystem.Defaults.Tools.FileManager.mkdir(dst_dir)
         BuildSystem.Defaults.Tools.FileManager.copy(src=src, dst=dst)
