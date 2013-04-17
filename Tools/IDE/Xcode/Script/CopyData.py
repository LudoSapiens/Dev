#!/usr/bin/env python
##
# Parses a manifest file describing files to copy in the application bundle.
# The format of the manifest file is as follows:
# - Lines starting with an optional '+' are added to a whitelist.
# - Lines starting with a '-' are added to a blacklist.
# - Lines starting with '#' or '//' are comments, and are ignored.
# - Other manifest files can be included by specifying them inside parentheses.
# When all of the lines are parsed, the whitelist is traversed to add every file it contains,
# but the blacklist is checked to exclude some.
# While no wildcard are supported, elements can be either a file or a directory.
#
# Here is a sample manifest file:
#   // A comment
#
##

import os
import shutil
import subprocess
import sys

# The value the process will return.
ret = 0

# Print all of the environment variables (for debugging).
#for (var,val) in os.environ.items():
#   print(var + ": " + val)

path = os.path

# Define the source directory (i.e. the Data directory).
srcDir = path.normpath( path.join( os.environ["DEV_ROOT"], "Data" ) )

# Define the destination directory (i.e. the app bundle's Resources path).
dstDir = path.normpath( path.join( os.environ["BUILT_PRODUCTS_DIR"], os.environ["UNLOCALIZED_RESOURCES_FOLDER_PATH"], "Data" ) )

#print( "Src: " + srcDir )
#print( "Dst: " + dstDir )

def PrintError( msg ):
   """Prints an error that Xcode will properly report."""
   print( "error: " + msg )
   global ret
   ret = 1

def PrintWarning( msg ):
   """Prints a warning that Xcode will properly report."""
   print( "warning: " + msg )

def StripComment( line ):
   """Removes the comment from a line."""
   p = line.find("#")
   if p >= 0:
      line = line[0:p]
   p = line.find("//")
   if p >= 0:
      line = line[0:p]
   return line.strip()

def ParseManifest( manifest ):
   """Parses the specified manifest file and return a white and a black list."""
   wl = set()
   bl = set()
   if manifest:
      with open( manifest ) as f:
         for line in f:
            line = StripComment( line )
            if len(line) == 0:
               # Comment/empty lines.
               pass
            elif line.startswith( "-" ):
               bl.add( line[1:] )
            elif line.startswith( "+" ):
               wl.add( line[1:] )
            elif line.startswith( "(" ):
               swl,sbl = ParseManifest( line[1:-1] )
               # Merge lists together.
               wl |= swl
               bl |= sbl
            else:
               wl.add( line )
   return wl, bl

def LazyCopy( src, dst ):
   """Copy src into dst if dst needs an update."""
   try:
      if (not path.exists( dst )) or (path.getmtime( dst ) < path.getmtime( src )):
         #print( "Would copy: ", src, dst )
         d = path.dirname( dst )
         if not path.exists( d ):
            os.makedirs( d )
         shutil.copy( src, dst )
         return True
   except IOError as e:
      PrintWarning( str(e) )
   except OSError as e:
      PrintError( str(e) )
   return False

def Delete( p ):
   """Removes the specified file/directory"""
   PrintWarning( "Deleting: '%s'" % (p) )
   try:
      if path.isdir( p ):
         # Directory must be empty.
         os.rmdir( p )
      else:
         # File of link.
         os.remove( p )
      return True
   except OSError as e:
      PrintError( str(e) )
   return False

def Listed( theList, theFile ):
   """Checks if 'theFile' is listed in 'theList' (either file is present, or a parent directory is)."""
   #print('Checking for "%s" in %s' % (theFile, theList))
   #print(theFile, theFile in theList)
   if theFile in theList:
      return True
   theDir = path.dirname( theFile )
   while theDir != theFile:
      #print(theDir, theDir in theList)
      if theDir in theList:
         return True
      theFile = theDir
      theDir  = path.dirname( theDir )
   return False

def Extraneous( wl, bl, theFile ):
   return len(theFile) > 0 and not Listed( wl, theFile ) or Listed( bl, theFile )

def DeepCopy( srcDir, dstDir, wl, bl ):
   """Copies all of the elements present in the whitelist and not present in the blacklist."""
   didCopy = False
   for i in wl:
      top = path.join( srcDir, i )
      if path.exists( top ):
         if path.isdir(top):
            for root, dirs, files in os.walk( top, topdown=False ):
               for name in files:
                  f = path.join( root, name )
                  rel = f[len(srcDir)+1:]  # The resource path, relative to srcDir.
                  if not Listed( bl, rel ):
                     didCopy |= LazyCopy( f, path.join( dstDir, rel ) )
               #for name in dirs:
               #   d = path.join( root, name )
               #   print( 'Dir: "%s"' % (d) )
         elif path.isfile(top):
            if not Listed( bl, i ):
               didCopy |= LazyCopy( top, path.join( dstDir, i ) )
         else:
            PrintWarning( "Attempt to copy irregular file: '%s'" % (i) )
      else:
         PrintWarning( "Non-existent file/directory: '%s'" % (i) )
   return didCopy

def DeepClean( srcDir, dstDir, wl, bl ):
   """Removes all of the extraneous files present in dstDir that all not allowed in srcDir."""
   didClean = False
   for root, dirs, files in os.walk( dstDir, topdown=True ):
      for name in files:
         f   = path.join( root, name )
         rel = f[len(dstDir)+1:] # The resource path, relative to dstDir.
         if Extraneous( wl, bl, rel ):
            didClean |= Delete( f )
            rel = path.dirname( rel )
            while Extraneous( wl, bl, rel ):
               f = path.dirname( f )
               Delete( f )
               rel = path.dirname( rel )
   return didClean

def GetManifest():
   """Determines the manifest file to use.
   If no argument was sent, it uses the EXECUTABLE_NAME in the Data/manifest directory.
   If an argument is sent, it tries to find a file of the specified name from the current directory,
   or from the Data/manifest directory otherwise."""
   manifestFile = None
   if len(sys.argv) > 1:
      # Just use whatever was specified on the command line.
      manifestFile = sys.argv[1]
      if not path.isfile( manifestFile ):
         # Try .../Data/<arg>
         manifestFile = path.join( srcDir, manifestFile )
         if not path.isfile( manifestFile ):
            # Try .../Data/manifest/<arg>
            manifestFile = path.join( srcDir, "manifest", sys.argv[1] )
            if not path.isfile( manifestFile ):
               PrintError("Could not find manifest file: '" + sys.argv[1] + "'.")
               return None
   else:
      # Use the EXECUTABLE_NAME (the TARGET_NAME has spaces and parentheses).
      manifestFile = path.join( srcDir, "manifest", os.environ["EXECUTABLE_NAME"] )
   return manifestFile

def Touch( filename, times = None ):
   """Touches a file/directory in order to force an update (e.g. Code Signing)."""
   PrintWarning( "Touching: " + filename )
   os.utime( filename, times )

def CodeSign():
   """Force Code Signing to be redone."""
   # Sample Xcode trace:
   #    /usr/bin/codesign
   #      -f
   #      -s "iPhone Developer: Jocelyn Houle (Y9UDRRGJ22)"
   #      --resource-rules=/Users/jph/src/Dev/Tools/IDE/Xcode/build/Release-iphoneos/Fusion.app/ResourceRules.plist
   #      --entitlements "/Users/jph/src/Dev/Tools/IDE/Xcode/build/Fusion.build/Release-iphoneos/FusionApp CocoaTouch (GLES).build/Fusion.xcent"
   #      /Users/jph/src/Dev/Tools/IDE/Xcode/build/Release-iphoneos/Fusion.app
   Touch( path.join( os.environ["BUILT_PRODUCTS_DIR"], os.environ["EXECUTABLE_PATH"] ) )
   return
   PrintWarning( "Code Signing manually" )
   cmd = [ 'codesign' ] # Xcode uses "/usr/bin/codesign", but why set the PATH in this case?!?
   #cmd.append( '-vv' )
   cmd.append( '-f' )
   iden = os.environ["CODE_SIGN_IDENTITY"]
   cmd.extend( [ '-s', iden ] )
   rsrc = path.join( os.environ["CODESIGNING_FOLDER_PATH"], "ResourceRules.plist" )
   cmd.extend( [ '--resource-rules', rsrc ] )
   entl = path.join( os.environ["TARGET_TEMP_DIR"], os.environ["PRODUCT_NAME"] + ".xcent" )  # PRODUCT_NAME or EXECUTABLE_NAME?
   cmd.extend( [ '--entitlements', entl ] )
   cmd.append( os.environ["CODESIGNING_FOLDER_PATH"] )
   #print("---")
   #print( " ".join(cmd) )
   #print("---")
   # For some reason, Xcode sets this.
   os.environ["CODESIGN_ALLOCATE"] = path.join( os.environ["PLATFORM_DEVELOPER_BIN_DIR"], "codesign_allocate" )
   retcode = subprocess.call( cmd )
   if retcode < 0:
      PrintError( "Error in code signing:\n   " + " ".join(cmd) )

wl, bl = ParseManifest( GetManifest() )

#print( "Whitelist" )
#for i in wl:
#   print( " >> " + i )
#print( "Blacklist" )
#for i in bl:
#   print( " >> " + i )

didSomething = False
didSomething |= DeepCopy( srcDir, dstDir, wl, bl )
didSomething |= DeepClean( srcDir, dstDir, wl, bl )
if didSomething:
   #Touch( dstDir )
   #Touch( path.join( os.environ["BUILT_PRODUCTS_DIR"], os.environ["WRAPPER_NAME"] ) )
   #Touch( path.join( os.environ["BUILT_PRODUCTS_DIR"], os.environ["EXECUTABLE_PATH"] ) )
   if ("CODE_SIGNING_REQUIRED" in os.environ) and (os.environ["CODE_SIGNING_REQUIRED"] == "YES"):
      CodeSign()
   pass

sys.exit( ret )
