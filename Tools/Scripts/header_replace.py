#!/usr/bin/env python

# =============================================================================
#  Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
#  See accompanying file LICENSE.txt for details.
# =============================================================================

import os
import re
import shutil
import sys

header_tmpl = \
"""/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
"""

re_file_ext = re.compile(".*\.(h|c|cpp|H|C|CPP)$")
re_dir_excl = re.compile(".*(\.svn)(/.*)*$")

def replaceHeader(filename):
   if not re_file_ext.match(filename):
      print "Excluding file: " + filename
      return
   print "Replacing header in: " + filename
   #return
   in_f = open(filename, "r")
   outfilename = filename + "_newHeader"
   out_f = open(outfilename, "wb")
   lines = in_f.readlines()
   in_f.close()
   lnws = (lines and lines[0].strip()) or ""
   #Skip starting header
   eocl = 0 #End of comment line number
   if lnws.startswith("/*"):
      #Search end-of-comment
      lnws = lines[eocl].strip()
      while not lnws.endswith("*/"):
         eocl = eocl + 1
         lnws = lines[eocl].strip()
         #print "<< " + lnws
      eocl = eocl + 1
      # Skip empty lines between old copyright and actual content.
      while lines[eocl].strip() == "":
         eocl = eocl + 1
      #print header_tmpl
   #Print new header (potentially adding it if it was absent)
   out_f.writelines(header_tmpl)
   #Write rest of the original file
   #for line in lines[eocl:]:
   #   print line,
   out_f.writelines(lines[eocl:])
   out_f.close()
   shutil.move(outfilename, filename)

def dispatch(paths):
   for e in paths:
      if os.path.isdir(e):
         replaceHeaderInDir(e)
      elif os.path.isfile(e):
         replaceHeader(e)
      else:
         print "Unkown type for: " + str(e)

def replaceHeaderInDir(top):
   #print "replaceHeaderInDir=" + top
   if not re_dir_excl.match(top):
      #print "Not excluding directory: " + top
      dirlist = os.listdir(top)
      #print "Dir contains: " + str(dirlist)
      elist = []
      for e in dirlist:
         elist.append(os.path.join(top, e))
      #print ">>" + str(elist)
      dispatch(elist)
   else:
     print "Excluding directory: " + top

########
# MAIN #
########

#print header_tmpl

dispatch(sys.argv[1:])
