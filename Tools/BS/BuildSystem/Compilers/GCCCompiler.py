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
import os
import string
import sys
import tempfile

import BuildSystem.Defaults.Env
import BuildSystem.Defaults.Tools
import BuildSystem.Defaults.Variables
import BuildSystem.Parts
import BuildSystem.Utilities
from BuildSystem.Variants.Variant import Variant
from BuildSystem.TaskManagers.Task import Task

##
# CompileOptions:
#
# - TaskPrefix: a prefix to associate the Tasks
# - Defines: list of defines
# - Options: generic options sent to the compiler
# - Source
#   - Files
#   - BinaryObjects
#   - Directories
#     - Dependencies
#     - Includes
# - Intermediate
#   - Files
#   - Directories
#     - Objects
# - Destination
#   - Files
#   - Directory
#     - Libraries
#     - Executables
#     - Binaries
#
# Fallbacks:
# - If a file is not found, the directory is used
# - If the Intermediate is None, then the Destination is used
# - For Destination, if Libraries and/or Executables are None, then the more
#   generic Binaries will be used.
# - When a Library is listed in the Source, its Destination.Directory is used.
#
# Include Directories: Directories to look for header files.
# Source Directories: Directories to look for the source files.
#
# Objects Directories: Where to store intermediate object files.
#
# Binary Directory: Directory where to put compiled binaries (Libs,Apps).
# Install Directory: Directory where to move the binaries.
##

class GCCCompiler:
   """The Compiler class for the GCC tool suite."""
   def __init__(self, name="GCC Compiler"):
      self.name = name
      self.mappings = dict()
      self.sub_targets = set()
      #Set some platform-specific flags to use
      if BuildSystem.Defaults.Env.Platform == "windows":
         self.app_format = "${NAME}.exe"
         self.define_format = "-D ${NAME}"
         self.define_format_2 = '-D ${NAME}="${VALUE}"'
         self.framework_format = ""
         self.include_path_format = "-I ${NAME}"
         self.library_object_format = '-l${NAME}'
         self.library_path_format = '-L"${NAME}"'
         self.obj_format = "${NAME}.o"  #GCC still uses .o under Windows
         self.shared_lib_flag = "-shared"
         self.shared_lib_format = "${NAME}.dll"
         self.shared_lib_link_format = "-Wl,--out-implib,${NAME}"
         self.static_lib_flag = ""
         self.static_lib_format = "lib${NAME}.a"
      elif BuildSystem.Defaults.Env.Platform == "unix" and \
           BuildSystem.Defaults.Env.PlatformFlavor == "macosx":
         self.app_format = "${NAME}"
         self.define_format = "-D ${NAME}"
         self.define_format_2 = '-D ${NAME}="${VALUE}"'
         self.framework_format = "-framework ${NAME}"
         self.include_path_format = '-I"${NAME}"'
         self.library_object_format = '-l${NAME}'
         self.library_path_format = '-L"${NAME}"'
         self.obj_format = "${NAME}.o"
         self.shared_lib_flag = "-Wl,-single_module -dynamiclib"
         self.shared_lib_format = "lib${NAME}.dylib"
         self.shared_lib_link_format = None
         self.static_lib_flag = ""
         self.static_lib_format = "lib${NAME}.a"
      elif BuildSystem.Defaults.Env.Platform == "unix" and \
           BuildSystem.Defaults.Env.PlatformFlavor == "linux":
         self.app_format = "${NAME}"
         self.define_format = "-D ${NAME}"
         self.define_format_2 = '-D ${NAME}="${VALUE}"'
         self.framework_format = ""
         self.include_path_format = "-I ${NAME}"
         self.library_object_format = '-l${NAME}'
         self.library_path_format = '-L"${NAME}"'
         self.obj_format = "${NAME}.o"
         self.shared_lib_flag = "-shared"
         self.shared_lib_format = "lib${NAME}.so"
         self.shared_lib_link_format = None
         self.static_lib_flag = ""
         self.static_lib_format = "lib${NAME}.a"
      else:
         print "Unsupported platform: " + BuildSystem.Defaults.Env.Platform
         sys.exit(1)
      #CC_mappings (ext -> compiler tool)
      # Highest priority means more important.
      self.lang_ext_priority = {
         "C++"   : 3,
         "ObjC++": 2,
         "C"     : 1,
         "ObjC"  : 0,
      }
      self.lang_ext_mappings = {
         "c"  : "C",
         "cpp": "C++",
         "cxx": "C++",
         "c++": "C++",
         "cc" : "C++",
         "m"  : "ObjC",
         "mm" : "ObjC++",
      }
      self.CC_mappings = dict()
      self.compilerVariant = 'gcc'
      if os.environ.has_key('CC'):
         envvar_CC = os.environ['CC']
         if BuildSystem.Utilities.Options.opts.verbose >= 4: print "Using environment CC=" + envvar_CC
         self.CC_mappings["C"]    = envvar_CC
         self.CC_mappings["ObjC"] = envvar_CC
         if 'clang' in envvar_CC:
            self.compilerVariant = 'clang'
      else:
         self.CC_mappings["C"]    = "gcc"
         self.CC_mappings["ObjC"] = "gcc"
      if os.environ.has_key('CXX'):
         envvar_CXX = os.environ['CXX']
         if BuildSystem.Utilities.Options.opts.verbose >= 4: print "Using environment CXX=" + envvar_CXX
         self.CC_mappings["C++"]    = envvar_CXX
         self.CC_mappings["ObjC++"] = envvar_CXX
         if 'clang' in envvar_CXX:
            self.compilerVariant = 'clang'
      else:
         self.CC_mappings["C++"]    = "g++"
         self.CC_mappings["ObjC++"] = "g++"
      self.CFLAGS = None
      if os.environ.has_key('CFLAGS'):
         self.CFLAGS = os.environ['CFLAGS']
      self.CXXFLAGS = None
      if os.environ.has_key('CXXFLAGS'):
         self.CXXFLAGS = os.environ['CXXFLAGS']
      #self.LDFLAGS = None
      #if os.environ.has_key('LDFLAGS'):
      #   self.LDFLAGS = os.environ['LDFLAGS']
      self.CC_mappings_arg = ""
   def get_binary_path(self, output, dst_dir, mapping):
      real_output = BuildSystem.Defaults.Tools.FileManager.path.join(dst_dir, output)
      real_output = BuildSystem.Utilities.substituteVariables(real_output, mapping)
      return real_output
   def get_application_file_path(self, app):
      output = BuildSystem.Utilities.formatName(self.app_format, app.getOutput())
      dst_dir = app.getSettings().getDestinationDirectoryForExecutables()
      mapping = {'TARGET_TYPE': 'app', 'TARGET_NAME': app.getName()}
      mapping.update(self._get_variant_mapping(app.getVariant()))
      #rel_prefix = BuildSystem.Utilities.path_diff(os.path.realpath(os.path.curdir), app.env.realpath)
      #dst_dir = os.path.join(rel_prefix, dst_dir)
      return self.get_binary_path(output, dst_dir, mapping)
   def get_library_dir_path(self, lib):
      output = ""
      dst_dir = lib.getSettings().getDestinationDirectoryForLibraries()
      mapping = {'TARGET_TYPE': 'lib', 'TARGET_NAME': lib.getName()}
      mapping.update(self._get_variant_mapping(lib.getVariant()))
      return self.get_binary_path(output, dst_dir, mapping)
   def get_library_file_path(self, lib):
      lib_dir = lib.getSettings().getDestinationDirectoryForLibraries()
      if lib.isShared():
         lib_flags = self.shared_lib_flag
         lib_format = self.shared_lib_format
      else:
         lib_flags = self.static_lib_flag
         lib_format = self.static_lib_format
      lib_file = BuildSystem.Utilities.formatName(lib_format, lib.getOutput())
      mapping = {'TARGET_TYPE': 'lib', 'TARGET_NAME': lib.getName()}
      mapping.update(self._get_variant_mapping(lib.getVariant()))
      return self.get_binary_path(lib_file, lib_dir, mapping)
   def make_app(self, inputs, outputs, settings=None, variant=None, taskPrefix=""):
      if BuildSystem.Utilities.Options.opts.verbose >= 2:
         print "Making application: " + str(inputs) + " -> " + str(outputs)
         print "  settings=" + str(settings)
         print "  variant=" + str(variant)
      mapping = {'TARGET_TYPE': 'app', 'TARGET_NAME': outputs}
      mapping.update(self._get_variant_mapping(variant))
      if not settings:
         settings = BuildSystem.Defaults.Variables.settings
      real_output = BuildSystem.Defaults.Tools.FileManager.path.join(settings.getDestinationDirectoryForExecutables(),
                                                                     BuildSystem.Utilities.formatName(self.app_format, outputs))
      real_output = BuildSystem.Utilities.substituteVariables(real_output, mapping)
      dsts_dir = BuildSystem.Utilities.substituteVariables(settings.getDestinationDirectoryForExecutables(), mapping)
      all_intermediates = list()
      all_sources = list()
      all_libs = list()
      if not isinstance(inputs, list):
         inputs = list(inputs)
      for elem in inputs:
         if isinstance(elem, basestring):
            all_sources.append(elem)
         elif isinstance(elem, BuildSystem.Parts.Library):
            all_libs.append(elem)
         else:
            print "Don't know what to make with element: " + str(elem)
            print "when trying to make application: " + str(inputs)
            sys.exit(1)
      results = self.call_subtargets(all_libs)
      some_error = BuildSystem.Utilities.someError(results)
      some_change =  BuildSystem.Utilities.someChange(results)
      if some_error:
         return results
      settings.source_directories_includes += self.make_include_path_list(all_libs)
      for source_file in all_sources:
         intermediate = self.make_object(source_file, settings=settings, variant=variant, taskPrefix=taskPrefix, mapping=mapping)
         if not intermediate:
            if BuildSystem.Utilities.Options.opts.verbose >= 1: print "Error making object: " + source_file
            sys.exit(1)
         else:
            all_intermediates.append(intermediate)
      deps = dict()
      deps[real_output] = copy.copy(all_intermediates)
      deps[real_output].append(BuildSystem.Utilities.Env[-1].filename) #BSFile
      needs_updating = BuildSystem.Defaults.Tools.DependencyChecker.check_file_dependencies(real_output, deps)
      needs_updating = needs_updating or some_change
      if needs_updating:
         BuildSystem.Defaults.Tools.FileManager.mkdir( BuildSystem.Defaults.Tools.FileManager.path.dirname(real_output) )
         command_tmpl = [ "${CC}" ]
         command_tmpl.append("-o " + real_output)
         command_tmpl.append("${INPUTS}")
         lib_objs = self.make_library_object_list(all_libs)
         for lib in lib_objs:
            if isinstance(lib, list):
               cur_lib_flags = BuildSystem.Utilities.formatName(self.library_path_format, lib[1]) \
                               + " " + \
                               BuildSystem.Utilities.formatName(self.library_object_format, lib[0])
            else:
               cur_lib_flags = lib
            command_tmpl.append(cur_lib_flags)
         # Defines were already handled in make_object calls
         ext = BuildSystem.Utilities.getExt(all_sources)
         self._set_CC(ext)
         self._consider_variant(variant, command_tmpl, linking=True, lang=self._getLang(ext), outputType="app")
         if settings and settings.ldflags:
            command_tmpl.extend(settings.ldflags)
         command_tmpl.extend(self._get_settings_frameworks(settings))
         err = self._run_command(command_tmpl, all_intermediates, taskPrefix=taskPrefix)
         if BuildSystem.Utilities.Options.opts.verbose >= 5: print "Err6=" + str(err)
         return err
   def make_lib(self, inputs, outputs, shared=False, settings=None, variant=None, taskPrefix=""):
      if BuildSystem.Utilities.Options.opts.verbose >= 2:
         print "Making library: " + str(inputs) + " -> " + str(outputs)
         if shared: " (shared)"
         else:      " (static)"
         print "  settings=" + str(settings)
         print "  variant=" + str(variant)
      mapping = {'TARGET_TYPE': 'lib', 'TARGET_NAME': outputs}
      mapping.update(self._get_variant_mapping(variant))
      if shared:
         lib_flags = self.shared_lib_flag
         lib_format = self.shared_lib_format
      else:
         lib_flags = self.static_lib_flag
         lib_format = self.static_lib_format
      if not settings:
         settings = BuildSystem.Defaults.Variables.settings
      mapping.update(self.mappings)
      dst_dir = BuildSystem.Utilities.substituteVariables(settings.getDestinationDirectoryForLibraries(), mapping)
      real_output = BuildSystem.Defaults.Tools.FileManager.path.join(dst_dir, BuildSystem.Utilities.formatName(lib_format, outputs))
      all_intermediates = list()
      all_sources = list()
      all_libs = list()
      if not isinstance(inputs, list):
         inputs = list(inputs)
      for elem in inputs:
         if isinstance(elem, basestring):
            all_sources.append(elem)
         elif isinstance(elem, BuildSystem.Parts.Library):
            all_libs.append(elem)
         else:
            print "Don't know what to make with element: " + str(elem)
            print "when trying to make library: " + str(inputs)
            sys.exit(1)
      results = self.call_subtargets(all_libs)
      some_error =  BuildSystem.Utilities.someError(results)
      some_change =  BuildSystem.Utilities.someChange(results)
      if some_error:
         return results
      for source_file in all_sources:
         intermediate = self.make_object(source_file, settings=settings, variant=variant, taskPrefix=taskPrefix, mapping=mapping)
         if not intermediate:
            if BuildSystem.Utilities.Options.opts.verbose >= 1: print "Error making object: " + source_file
            sys.exit(1)
         else:
            #print "Intermediate=" + str(intermediate)
            all_intermediates.append(intermediate)
      deps = dict()
      deps[real_output] = copy.copy(all_intermediates)
      deps[real_output].append(BuildSystem.Utilities.Env[-1].filename) #BSFile
      needs_updating = BuildSystem.Defaults.Tools.DependencyChecker.check_file_dependencies(real_output, deps)
      needs_updating = needs_updating or some_change
      if needs_updating:
         BuildSystem.Defaults.Tools.FileManager.mkdir( BuildSystem.Defaults.Tools.FileManager.path.dirname(real_output) )
         command_tmpl = [ "${CC}" ]
         command_tmpl.append(lib_flags)
         if shared:
            command_tmpl.append("-o " + real_output)
            #Windows DLL require a .lib to be created along with the .dll
            if self.shared_lib_link_format:
               #Reuse static_lib format for the .lib equivalent (we use libtest.a with test.dll)
               link_options_filename = BuildSystem.Defaults.Tools.FileManager.path.join(dst_dir, BuildSystem.Utilities.formatName(self.static_lib_format, outputs))
               link_options = BuildSystem.Utilities.formatName(self.shared_lib_link_format, link_options_filename)
               command_tmpl.append(link_options)
         else:
            command_tmpl.append(real_output)
         command_tmpl.append("${INPUTS}")
         lib_objs = self.make_library_object_list(all_libs)
         for lib in lib_objs:
            if isinstance(lib, list):
               cur_lib_flags = BuildSystem.Utilities.formatName(self.library_path_format, lib[1]) \
                               + " " + \
                               BuildSystem.Utilities.formatName(self.library_object_format, lib[0])
            else:
               cur_lib_flags = lib
            command_tmpl.append(cur_lib_flags)
         #No defines when linking object files together (done in make_object)
         ext = BuildSystem.Utilities.getExt(all_sources)
         self._set_CC(ext, static_library=not shared)
         if settings and settings.ldflags:
            command_tmpl.extend(settings.ldflags)
         command_tmpl.extend(self._get_settings_frameworks(settings))
         self._consider_variant(variant, command_tmpl, linking=True, lang=self._getLang(ext), outputType="lib")
         err = self._run_command(command_tmpl, all_intermediates, taskPrefix=taskPrefix)
         return err
   def make_object(self, inputs, output=None, check_timestamps=True, settings=None, variant=None, taskPrefix="", mapping=dict()):
      if BuildSystem.Utilities.Options.opts.verbose >= 2:
         print "Making object file: " + str(inputs) + " -> " + str(output)
         print "  settings=" + str(settings)
         print "  variant=" + str(variant)
      mapping['TARGET_TYPE'] = 'obj'
      mapping.update(self.mappings)
      #Separate extension from filename in order to determine what compiler to use
      (root, ext) = BuildSystem.Defaults.Tools.FileManager.path.splitext(inputs)
      if not settings:
         settings = BuildSystem.Defaults.Variables.settings
      defines = settings.getDefines()
      inc_paths = BuildSystem.Utilities.substituteVariables(settings.getSourceIncludePaths(), mapping)
      src_paths = BuildSystem.Utilities.substituteVariables(settings.getSourcePaths(), mapping)
      dst_dir = BuildSystem.Utilities.substituteVariables(settings.getIntermediateDirectoryForObjects(), mapping)
      inputs = self._search_for_real_inputs(inputs, src_paths)
      if not output:
         output = BuildSystem.Utilities.formatName(self.obj_format, root)
      full_output = BuildSystem.Defaults.Tools.FileManager.path.join(dst_dir, output)
      #print "___"
      #print "Input=" + str(inputs)
      #print "FullOutput=" + str(full_output)
      #print "dst_dir=" + str(dst_dir)
      #print "output=" + str(output)
      #BuildSystem.Defaults.Tools.DependencyCache.print_deps()
      (outName,deps) = BuildSystem.Defaults.Tools.DependencyCache.get_out_deps(full_output)
      #print ">>> " + str(outName)
      #print "<<< " + str(deps)
      if deps:
         if BuildSystem.Defaults.Tools.DependencyChecker.check_file_dependencies(outName, deps):
            if BuildSystem.Utilities.Options.opts.verbose >= 2:
               print "Re-generating full list from source."
            deps = self.make_dep_list(inputs=inputs,
                                      outputDir=dst_dir,
                                      defines=defines,
                                      includePaths=inc_paths,
                                      sourcePaths=src_paths,
                                      fullOutput=full_output)
            BuildSystem.Defaults.Tools.DependencyCache.set_deps(full_output, deps.keys()[0], deps.values()[0])
         else:
            if BuildSystem.Utilities.Options.opts.verbose >= 2:
               print "Cache did the job:" + str(deps)
      else:
         if BuildSystem.Utilities.Options.opts.verbose >= 2:
            print "Cache didn't know about: '%s'.  Regenerating." % str(inputs)
         deps = self.make_dep_list(inputs=inputs,
                                   outputDir=dst_dir,
                                   defines=defines,
                                   includePaths=inc_paths,
                                   sourcePaths=src_paths,
                                   fullOutput=full_output)
         BuildSystem.Defaults.Tools.DependencyCache.set_deps(full_output, deps.keys()[0], deps.values()[0])
      real_output = full_output #BuildSystem.Defaults.Tools.FileManager.path.join(dst_dir, output)
      needs_updating = False
      for dst in deps:
         needs_updating = BuildSystem.Defaults.Tools.DependencyChecker.check_file_dependencies(dst, deps)
         if needs_updating:
            BuildSystem.Defaults.Tools.FileManager.mkdir( BuildSystem.Defaults.Tools.FileManager.path.dirname(dst) )
            command_tmpl = [ "${CC}" ]
            command_tmpl.append("-c")
            command_tmpl.append("-o " + dst)
            command_tmpl.append("${INPUTS}")
            command_tmpl += self._get_defines_list(defines)
            command_tmpl += self._get_include_path_list(inc_paths)
            self._set_CC(ext[1:])
            lang = self._getLang(ext[1:])
            if settings:
               if lang == "C" and settings.cflags:
                  command_tmpl.extend(settings.cflags)
               elif lang == "C++" and settings.cxxflags:
                  command_tmpl.extend(settings.cxxflags)
            self._consider_variant(variant, command_tmpl, compiling=True, lang=lang, outputType="obj")
            err = self._run_command(command_tmpl, inputs, taskPrefix=taskPrefix)
            if err:
               return None #Indicates error
            break
      return real_output
   def make(self, o):
      if not isinstance(o, BuildSystem.Parts.CompiledBinary):
         print "Error: Compiler.make() only accepts subclasses of BuildSystem.Parts.CompiledBinary"
         sys.exit(1)
      if BuildSystem.Utilities.Options.opts.verbose >= 2:
         print "Calling make on: " + str(o)
         print " settings=" + str(o.settings)
         print " variant=" + str(o.getVariant())
         print " env=" + str(o.env)
      BuildSystem.Utilities.Env.append(o.env)
      BuildSystem.Utilities.Env[-1].setCurrent()
      if isinstance(o, BuildSystem.Parts.Library):
         if BuildSystem.Utilities.Options.opts.verbose >= 3: print "LIB: " + o.env.path
         v = o.getVariant()
         shared_lib = v and "shared" in v
         out = o.getOutput()
         if o.settings:  settings = o.settings
         else:           settings = BuildSystem.Defaults.Variables.settings
         if shared_lib:
            if o.shared_define:
               # Need to copy, since we don't want to modify the originals
               # (lazy copy, more optimal than a full copy)
               settings = copy.copy(settings)
               settings.defines = copy.copy(settings.defines)
               settings.defines[o.shared_define] = None
            else:
               print "BuildSystem.Parts.Library '%s' is marked as shared but has no shared_define value" % o.name
               sys.exit(1)
         ret = self.make_lib(inputs=o.inputs, outputs=out, shared=shared_lib, settings=settings, variant=o.getVariant(), taskPrefix="<Lib:"+o.name+">")
      elif isinstance(o, BuildSystem.Parts.Application):
         if BuildSystem.Utilities.Options.opts.verbose >= 3: print "APP: " + o.env.path
         out = o.getOutput()
         ret = self.make_app(inputs=o.inputs, outputs=out, settings=o.settings, variant=o.getVariant(), taskPrefix="<App:"+o.name+">")
      else:
         print "Unkown type in Compiler:make() routine: " + str(o)
         sys.exit(1)
      BuildSystem.Utilities.Env.pop()
      BuildSystem.Utilities.Env[-1].setCurrent()
      return ret
   def clean(self, o, recursive=False):
      if BuildSystem.Utilities.Options.opts.verbose >= 2:
         print "Calling clean on: " + str(o)
         print " settings=" + str(o.settings)
         print " variant=" + str(o.getVariant())
         print " env=" + str(o.env)
      if not isinstance(o, BuildSystem.Parts.CompiledBinary):
         print "Error: Compiler.clean() only accepts subclasses of BuildSystem.Parts.CompiledBinary"
         sys.exit(1)
      if isinstance(o, BuildSystem.Parts.Library) or \
         isinstance(o, BuildSystem.Parts.Application):
         #List all of the object files
         files_to_delete = self.list_intermediate_files(o)
         for f in files_to_delete:
            BuildSystem.Defaults.Tools.FileManager.delete(f, delete_empty_dirs=True)
         #Call subtargets recursively
         if recursive:
            map(self.clean, o.getRecursiveTargets())
      else:
         print "Unkown type in Compiler:clean() routine: " + str(o)
         sys.exit(1)
   def get_mapping(self, o):
      mapping = dict()
      if isinstance(o, BuildSystem.Parts.CompiledBinary):
         mapping.update(self._get_variant_mapping(o.getVariant()))
         mapping["TARGET_NAME"] = o.getName()
         if isinstance(o, BuildSystem.Parts.Library):
            mapping["TARGET_TYPE"] = "lib"
         elif isinstance(o, BuildSystem.Parts.Application):
            mapping["TARGET_TYPE"] = "app"
      return mapping
   def list_intermediate_files(self, o, recursive=False):
      files = list()
      mapping = self.get_mapping(o)
      mapping["TARGET_TYPE"] = "obj"
      for elem in o.inputs:
         if isinstance(elem, BuildSystem.Parts.CompiledBinary):
            if recursive:
               files += self.list_intermediate_files(elem)
         else:
            (root, ext) = BuildSystem.Defaults.Tools.FileManager.path.splitext(elem)
            #Assume the extension is supported
            objFile = BuildSystem.Utilities.formatName(self.obj_format, root)
            objFile_Full = BuildSystem.Defaults.Tools.FileManager.path.join(
                               o.env.path,
                               o.getSettings().getIntermediateDirectoryForObjects(),
                               objFile)
            objFile_Full = BuildSystem.Utilities.substituteVariables(objFile_Full, mapping)
            files.append(objFile_Full)
      return files
   def _search_for_real_inputs(self, inputs, sourcePaths):
      if not sourcePaths:
         return inputs
      else:
         if isinstance(inputs, list):
            print "Error: _search_for_real_inputs does not support input list"
            sys.exit(1)
         if not isinstance(sourcePaths, list):
            print "Error: _search_for_real_inputs requires list for sourcePaths; got: " + str(sourcePaths)
            sys.exit(1)
         if BuildSystem.Defaults.Tools.FileManager.path.isfile(inputs):
            return inputs
         else:
            #Search in all of the source paths
            for d in sourcePaths:
               new_location = BuildSystem.Defaults.Tools.FileManager.path.join(d, inputs)
               if BuildSystem.Defaults.Tools.FileManager.path.isfile(new_location):
                  return new_location
         print "Could not find '" + inputs + "' in any of the following: " + str(sourcePaths)
         sys.exit(4)
         return None
   def _run_command(self, command_template, inputs, outputs=None, stdout=None, silent=False, taskPrefix=""):
      #Flatten list of strings to space-separated strings
      if isinstance(inputs, list):
         inputs = ' '.join(inputs)
      if isinstance(outputs, list):
         inputs = ' '.join(outputs)
      #Replace "${INPUTS} and ${OUTPUTS} with proper strings
      mappings = dict(INPUTS=inputs, OUTPUTS=outputs)
      #Add mapping for CC and other flags
      mappings.update(self.mappings)
      if BuildSystem.Utilities.Options.opts.verbose >= 4:
         print "Using template: " + str(command_template)
         print "with  mappings: " + str(mappings)
      command = BuildSystem.Utilities.substituteVariables(' '.join(command_template), mappings)
      return BuildSystem.Defaults.Tools.TaskManager.submit(Task(command), stdout, silent, taskPrefix)
   def _get_settings_frameworks(self, settings):
      ret = list()
      if settings and settings.frameworks:
         for framework in settings.frameworks:
            ret.append(BuildSystem.Utilities.formatName(self.framework_format, framework))
      return ret
   def _getLang(self, ext):
      if isinstance(ext, set):
         p = -1
         l = None
         for x in ext:
            lx = self.lang_ext_mappings[x]
            px = self.lang_ext_priority[lx]
            if px > p:
               p = px
               l = lx
         return l
      else:
         return self.lang_ext_mappings[ext]
   def _set_CC(self, ext, static_library=False):
      if not ext:
         ext = "c" #Use 'gcc' for empty libraries
      if isinstance(ext, set):
         it = iter(ext) #use first element if multiple extensions
         ext = it.next()
      if static_library:
         self.mappings["CC"] = "ar rcs" + self.CC_mappings_arg
      else:
         ext = ext.lower().strip()
         if ext[0] == '.':
            ext = ext[1:-1]
         lang = self._getLang(ext)
         self.mappings["CC"] = self.CC_mappings[lang]
         if self.CC_mappings_arg:
            self.mappings["CC"] += self.CC_mappings_arg
   def getVariantOutputStringList(self, variant):
      """Returns a list of strings to put in the output directory.
      Note: Not all variants change the output directory."""
      output_strings = list()
      if "debug" in variant:
         output_strings.append("debug")
      if "release" in variant:
         output_strings.append("release")
      if "multi-thread" in variant:
         output_strings.append("multi")
      else:
         output_strings.append("single")
      return output_strings
   def _get_variables_mapping(self):
      mapping = dict()
      mapping['PLATFORM'] = BuildSystem.Defaults.Env.Platform
      mapping['PLATFORM_FLAVOR'] = BuildSystem.Defaults.Env.PlatformFlavor
      return mapping
   def _get_variant_mapping(self, variant):
      mapping = dict()
      mapping.update(self._get_variables_mapping())
      mapping["VARIANTS"] = "default"
      if variant and isinstance(variant, Variant):
         var_str_list = self.getVariantOutputStringList(variant)
         if len(var_str_list):
            mapping["VARIANTS"] = '_'.join(var_str_list)
      return mapping
   def _consider_variant(self, variant, template, compiling=False, linking=False, lang=None, outputType=None):
      if BuildSystem.Utilities.Options.opts.verbose >= 3:
         print "Considering variant: " + str(variant)
         print "with template: " + str(template)
      if variant and isinstance(variant, Variant) and isinstance(template, list):
         if compiling:
            if "warnings" in variant:
               template.insert(1, "-Wall")
            if "release" in variant:
               template.insert(1, "-O2")
            if "debug" in variant:
               template.insert(1, "-ggdb -O0 -D_DEBUG")
            if "pic" in variant:
               template.insert(1, "-fPIC")
            if "asm" in variant:
               if BuildSystem.Defaults.Env.PlatformFlavor == "macosx":
                  template.insert(1, "-fasm-blocks")
            if "sse" in variant:
               if self.compilerVariant == 'gcc':
                  template.insert(1, "-ftree-vectorize")
                  template.insert(1, "-mfpmath=sse")
               template.insert(1, "-mssse3")
               template.insert(1, "-msse3")
               template.insert(1, "-msse2")
               template.insert(1, "-msse")
            if BuildSystem.Defaults.Env.PlatformArchitecture:
               if BuildSystem.Defaults.Env.PlatformArchitecture == "native":
                  # Special case, -march=native doesn't work, but -mtune=native does.
                  template.insert(1, "-mtune=native")
               elif BuildSystem.Defaults.Env.PlatformArchitecture != "none":
                  # Just set the architecture to what was specified.
                  template.insert(1, "-march=" + BuildSystem.Defaults.Env.PlatformArchitecture)
            if lang == "C++" or lang == "ObjC++":
               if "rtti" not in variant:
                  template.insert(1, "-fno-rtti")
               if "exceptions" not in variant:
                  template.insert(1, "-fno-exceptions")
               if "c++0x" in variant: # Explicit dialect.
                  if self.compilerVariant == 'clang':
                     template.insert(1, "-std=c++0x")
               if "g++0x" in variant: # Explicit dialect.
                  if self.compilerVariant == 'clang':
                     template.insert(1, "-std=g++0x")
               if "C++11" in variant:
                  if self.compilerVariant == 'clang':
                     template.insert(1, "-std=c++0x")
                  elif self.compilerVariant == 'gcc':
                     template.insert(1, "-std=c++0x")
               if self.CXXFLAGS:
                  template.insert(1, self.CXXFLAGS)
            if self.CFLAGS:
               template.insert(1, self.CFLAGS) # Always add the C flags.
            if "multi-thread" in variant:
               if BuildSystem.Defaults.Env.Platform == "unix":
                  # Non-windows platforms need some flags
                  if BuildSystem.Defaults.Env.PlatformFlavor == "macosx":
                     pass #MacOSX doesn't need the pthread flag
                  else:
                     template.insert(1, "-pthread") # Adds defines to preprocessor
            if "osx_pascal_strings" in variant:
               if BuildSystem.Defaults.Env.PlatformFlavor == "macosx":
                  template.insert(1, "-fpascal-strings")
         if linking:
            if "multi-thread" in variant:
               if (outputType == "app") or (outputType == "lib" and "shared" in variant):
                  if BuildSystem.Defaults.Env.Platform == "unix":
                     # Non-windows platforms need this flag
                     if BuildSystem.Defaults.Env.PlatformFlavor == "macosx":
                        pass #MacOSX doesn't need the pthread flag
                     else:
                        template.insert(1, "-pthread") # Missing symbols otherwise
      else:
         self.mappings["VARIANTS"] = "default"
   def _get_defines_list(self, defines):
      out = list()
      if defines:
         if isinstance(defines, dict):
            for define in defines:
               if defines[define] != None:
                  out.append(BuildSystem.Utilities.formatNameValue(self.define_format_2, define, defines[define]))
               else:
                  out.append(BuildSystem.Utilities.formatName(self.define_format, define))
         else:
            for define in BuildSystem.Utilities.list_iter(defines):
               out.append(BuildSystem.Utilities.formatName(self.define_format, define))
         if BuildSystem.Utilities.Options.opts.verbose >= 3:
            print "Defines list:"
            for i in out:
               print i
      return out
   def _get_include_path_list(self, includePaths):
      out = list()
      if includePaths:
         for incpath in BuildSystem.Utilities.list_iter(includePaths):
            out.append(BuildSystem.Utilities.formatName(self.include_path_format, incpath))
         if BuildSystem.Utilities.Options.opts.verbose >= 3:
            print "Include paths:"
            for i in out:
               print i
      return out
   def make_include_path_list(self, libs):
      return []
      tmp = list()
      for lib in libs:
         if BuildSystem.Utilities.Options.opts.verbose >= 3:
            print "BuildSystem.Parts.Library " + lib.name + " adds incpath '" + lib.env.path + "'"
         tmp.append( lib.env.path )
      return tmp
   def make_library_path_list(self, libs):
      tmp = list()
      for lib in libs:
         if BuildSystem.Utilities.Options.opts.verbose >= 3:
            print "BuildSystem.Parts.Library " + lib.name + " adds libpath '" + lib.env.path + "'"
         tmp.append( lib.env.path )
      return tmp
   def make_library_object_list(self, libs):
      tmp = list()
      for lib in libs:
         libname = lib.name
         mappings = {'TARGET_TYPE': 'lib', 'TARGET_NAME': lib.getName()}
         mappings.update(self._get_variant_mapping(lib.getVariant()))
         lib_path_prefix = BuildSystem.Utilities.get_path_to_reach(lib.env)
         if lib.settings:
            libpath = BuildSystem.Defaults.Tools.FileManager.path.join(lib_path_prefix, lib.settings.getDestinationDirectoryForLibraries())
         else:
            libpath = BuildSystem.Defaults.Tools.FileManager.path.join(lib_path_prefix, BuildSystem.Defaults.Variables.settings.getDestinationDirectoryForLibraries())
         #Need to replace variables here since the mapping can change from lib to lib
         libpath = BuildSystem.Utilities.substituteVariables(libpath, mappings)
         if BuildSystem.Utilities.Options.opts.verbose >= 3:
            print "BuildSystem.Parts.Library " + lib.name + " adds library object '" + libname + "' from " + libpath
         tmp.append( [libname, libpath] )
      #print "LIBS: " + str(tmp)
      return tmp
   def make_dep_list(self, inputs, outputDir=None, defines=[], includePaths=[], sourcePaths=[], fullOutput=None):
      if BuildSystem.Utilities.Options.opts.verbose >= 2:
         print "Making dependency list: " + str(inputs)
         print "  OutputDir=" + str(outputDir)
         print "  defines=" + str(defines)
         print "  includePaths=" + str(includePaths)
         print "  sourcePaths=" + str(sourcePaths)
      #Special case
      command_tmpl = [ "${CC}" ]
      command_tmpl.append("${INPUTS}")
      command_tmpl += self._get_defines_list(defines)
      command_tmpl += self._get_include_path_list(includePaths)
      temp = tempfile.TemporaryFile(mode="w+")
      self.CC_mappings_arg = " -M"
      self._set_CC(BuildSystem.Utilities.getExt(inputs))
      err = self._run_command(command_tmpl, inputs, stdout=temp, silent=True)
      if err:
         if BuildSystem.Utilities.Options.opts.no_run:
            # Rerun it to print it
            print ""
            print "Problem generating dependency list."
            print "To reproduce, run:"
            print "------------------"
            self._run_command(command_tmpl, inputs, stdout=temp, silent=False)
            print "------------------"
            print "under '" + BuildSystem.Utilities.Env[-1].path + "'."
         if BuildSystem.Utilities.Options.opts.verbose >= 1:
            print "Dependency generation failed: " + str(inputs)
         sys.exit(2)
      self.CC_mappings_arg = ""
      if BuildSystem.Utilities.Options.opts.verbose >= 4:
         print "--DEPS--"
         print inputs
         #temp.seek(0)
         #print temp.readlines()
      temp.seek(0)
      dep_list = dict()
      for line in temp:
         line = line.strip()
         parts = line.split(":")
         dst = fullOutput or BuildSystem.Defaults.Tools.FileManager.path.join(outputDir, parts[0])
         fullOutput = None #only the first one
         line = parts[1]
         dep_list[dst] = list()
         dep_list[dst].append(BuildSystem.Utilities.Env[-1].filename) #BSFile
         #Add continuing lines
         while line.endswith("\\"):
            line = line.strip(string.whitespace + "\\")
            # Unescape spaces
            line = line.replace("\ ", "[space]")
            files = line.split()
            for file in files:
               # Bring back spaces, and add to the dependency list
               dep_list[dst].append( file.replace("[space]", " ") )
            line = temp.next().strip()
         #Add last line
         line = line.strip()
         # Unescape spaces
         line = line.replace("\ ", "[space]")
         files = line.split()
         for file in files:
            # Bring back spaces, and add to the dependency list
            dep_list[dst].append( file.replace("[space]", " ") )
      for x in dep_list:
         if BuildSystem.Utilities.Options.opts.verbose >= 4: print str(x) + " depends on:"
         sorted = dep_list[x]
         sorted.sort()
         if BuildSystem.Utilities.Options.opts.verbose >= 4: 
            for d in sorted:
               print "  " + d
      if BuildSystem.Utilities.Options.opts.verbose >= 4: print "--END--"
      return dep_list
   def call_subtargets(self, subs):
      if BuildSystem.Utilities.Options.opts.fast:
         for s in subs:
            print "Skipping subtarget: " + s.name
         print "Warning: build may be incomplete"
         return list()
      results = list()
      for s in subs:
         if not s in self.sub_targets:
            if BuildSystem.Utilities.Options.opts.verbose >= 2:
               print "Calling subtarget: " + s.name
            #Same trick as in Load()
            BuildSystem.Utilities.Env.append(s.env)
            BuildSystem.Utilities.Env[-1].setCurrent()
            subresult = s()
            results.append(subresult)
            BuildSystem.Utilities.Env.pop()
            BuildSystem.Utilities.Env[-1].setCurrent()
            self.sub_targets.add(s)
            if BuildSystem.Utilities.Options.opts.verbose >= 2:
               print "Done with subtarget: " + s.name
      return results
