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

import BuildSystem.Defaults
from BuildSystem.TaskManagers.Task import Task
import BuildSystem.Utilities
from BuildSystem.Utilities.Environment import Environment


class AutoconfHandler:
   """A class handling autoconf projects.
   Typically does the following:
      ./configure <some flags>
      make
      make install
   """
   def __init__(self, name, source_dir, build_dir=None,
                steps=["configure", "build", "install"],
                conf_flags=None, build_flags=None, install_flags=None,
                conf_cmd="configure", make_cmd="make" ):
      self.name = name
      self.source_dir = source_dir
      self.build_dir = build_dir
      if not self.build_dir:
         self.build_dir = self.source_dir
      self.steps = steps
      self.conf_flags = conf_flags
      if not self.conf_flags: self.conf_flags = list()
      self.build_flags = build_flags
      if not self.build_flags: self.build_flags = list()
      self.install_flags = install_flags
      if not self.install_flags: self.install_flags = list()
      self.conf_cmd = conf_cmd
      self.make_cmd = make_cmd
   def __call__(self):
      return self.run()
   def run(self):
      for step in self.steps:
         if step == "configure":
            err = self.configure()
         elif step == "build":
            err = self.build()
         elif step == "test":
            err = self.test()
         elif step == "install":
            err = self.install()
         else:
            raise ValueError, "Unknown step: " + str(step)
         if err:
            raise RuntimeError, "Error while executing: " + str(step)
            return err
      return None
   def run_command(self, command):
      taskPrefix = "<Autoconf:"+str(self.name)+">"
      #Go into the build directory
      build_dir_real = BuildSystem.Defaults.Tools.FileManager.path.realpath(self.build_dir)
      build_dir_relative = BuildSystem.Utilities.path_diff(BuildSystem._start_dir, build_dir_real)
      e = Environment(build_dir_relative, build_dir_real)
      BuildSystem.Utilities.Env.append(e)
      BuildSystem.Utilities.Env[-1].setCurrent()
      err = BuildSystem.Defaults.Tools.TaskManager.submit(Task(command), stdout=None, silent=False, taskPrefix=taskPrefix)
      BuildSystem.Utilities.Env.pop()
      BuildSystem.Utilities.Env[-1].setCurrent()
      return err
   def configure(self):
      BuildSystem.Defaults.Tools.FileManager.mkdir(self.build_dir)
      configure_path_relative = BuildSystem.Utilities.path_diff(self.build_dir, self.source_dir)
      command = list()
      if BuildSystem.Defaults.Env.PlatformFlavor == "mingw":
         #MinGW has problems launching the configure script
         #(and this workaround has a drawback: CTRL-C will not kill the job)
         command.append("sh")
      command.append(BuildSystem.Defaults.Tools.FileManager.path.join(configure_path_relative, self.conf_cmd))
      command += self.conf_flags
      return self.run_command(command)
   def build(self):
      command = list()
      command.append(self.make_cmd)
      command += self.build_flags
      return self.run_command(command)
   def test(self):
      command = list()
      command.append(self.make_cmd)
      command.append("test")
      return self.run_command(command)
   def install(self):
      command = list()
      command.append(self.make_cmd)
      command += self.install_flags
      command.append("install")
      return self.run_command(command)
   def clean(self):
      print("Cleaning: " + self.build_dir)
      BuildSystem.Defaults.Tools.FileManager.delete(self.build_dir, delete_non_empty_dirs=True)
