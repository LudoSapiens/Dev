#!/usr/bin/env python
##
# A script used to synchronize directories and files together.
# This is useful for bundling the Data directory with an application.
##

import argparse
import itertools
import os
import shlex
import shutil
import subprocess
import sys

# Some shorthands.
path = os.path

verbose = 0
active  = True

# Debugging:

# Redirect output to a file.
#sys.stdout = open( "/Users/jph/temp.log", "w" )
#sys.stderr = sys.stdout

# Print all of the environment variables.
#for (var,val) in os.environ.items():
#   print(var + ": " + val)

# Print all of the arguments.
#for arg in sys.argv:
#	print( ">> %s" % (arg) )

#sys.exit( 0 )

def PrintError( msg ):
	"""Prints an error that Xcode will properly report."""
	print( "error: " + msg )
	sys.exit( 1 )

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

def LazyCopyFile( src, dst, excluded, copied ):
	"""Copy src into dst if dst needs an update."""
	if src in excluded:
		return False
	copied.add( dst )
	parent = path.dirname( dst )
	while parent not in copied:
		copied.add( parent )
		parent = path.dirname( parent )
	try:
		if path.exists( src ):
			if (not path.exists( dst )) or (path.getmtime( dst ) < path.getmtime( src )):
				if verbose >= 1:
					print( "[+] '"+src+"' -> '"+dst+"'" )
				d = path.dirname( dst )
				if active:
					if not path.exists( d ):
						os.makedirs( d )
					shutil.copy( src, dst )
				else:
					if not path.exists( d ) and verbose >= 3:
						print( "Would create directory '%s'" % (d) )
					#print( "Would copy '%s' -> '%s'" % (src, dst) )
				return True
			else:
				if verbose >= 2:
					print( "[=] '"+src+"' == '"+dst+"'" )
		else:
			if verbose >= 1:
				PrintWarning( "'%s' is missing" % (src) )
	except IOError as e:
		PrintWarning( str(e) )
	except OSError as e:
		PrintError( str(e) )
	return False

def LazyCopyDir( srcDir, dstDir, excluded, copied ):
	if srcDir in excluded:
		return False
	copied.add( dstDir )
	parent = path.dirname( dstDir )
	while parent not in copied:
		copied.add( parent )
		parent = path.dirname( parent )
	didCopy = False
	for src in os.listdir( srcDir ):
		srcPath = path.join( srcDir, src )
		dstPath = path.join( dstDir, src )
		if path.isdir( srcPath ):
			didCopy |= LazyCopyDir( srcPath, dstPath, excluded, copied )
		else:
			didCopy |= LazyCopyFile( srcPath, dstPath, excluded, copied )
	return didCopy

def LazyCopy( src, dst, excluded, copied ):
	if path.isdir( src ):
		return LazyCopyDir( src, dst, excluded, copied )
	else:
		return LazyCopyFile( src, dst, excluded, copied )

def Delete( p ):
	"""Removes the specified file/directory"""
	if verbose >= 1:
		print( "[-] '%s'" % (p) )
	try:
		if path.isdir( p ):
			#os.rmdir( p ) # Directory must be empty.
			if active:
				shutil.rmtree( p ) # Directory can be full.
			elif verbose >= 3:
				print( "Would remove directory '%s'" % (p) )
		else:
			# File or link.
			if active:
				os.remove( p )
			elif verbose >= 3:
				print( "Would remove file '%s'" % (p) )
		return True
	except OSError as e:
		PrintError( str(e) )
	return False

def Clean( dstDir, copied ):
	if path.isdir( dstDir ):
		dstItems = os.listdir( dstDir )
		for dst in dstItems:
			dstPath = path.normpath( path.join( dstDir, dst ) )
			if dstPath not in copied:
				Delete( dstPath )
				# Remove dstPath from copied to accelerate things?
			elif path.isdir( dstPath ):
				Clean( dstPath, copied )

class SyncAtom:
	def __init__( self, src, dst ):
		self.src = src
		self.dst = dst or src
		self.exc = set()
		self.cpy = set()
	def exclude( self, e ):
		self.exc.add( e )
	def __str__( self ):
		return "src='%s' dst='%s' exc=%s" % (self.src, self.dst, self.exc)
	def debug( self ):
		print("-----")
		print("  "+self.src)
		print("->"+self.dst)
		for exc in self.exc:
			print("["+exc+"]")
		print("-----")
	def sync( self, copied ):
		LazyCopy( self.src, self.dst, self.exc, copied )
		#self.clean( dstDir )

def ParseManifest( manifest, srcDir, dstDir, copied ):
	"""Parses the specified manifest file."""
	if manifest:
		with open( manifest ) as f:
			atom = None
			for line in f:
				line = StripComment( line )
				if len(line) == 0:
					# Comment/empty lines.
					pass
				elif line.startswith( "-" ):
					excPath = path.normpath( path.join( srcDir, line[1:].strip() ) )
					atom.exclude( excPath )
				else:
					if atom:
						atom.sync( copied )
						atom = None
					src,sep,dst = line.partition('->')
					src = src.strip()
					dst = (dst and dst.strip()) or src
					srcPath = path.normpath( path.join( srcDir, src ) )
					dstPath = path.normpath( path.join( dstDir, dst ) )
					atom = SyncAtom( srcPath, dstPath )
			if atom:
				atom.sync( copied )

def ParseArgs( args ):
	global verbose, active
	parser = argparse.ArgumentParser(description='Synchronizes directory contents.')
	parser.add_argument('-v', help="Verbosity level (repeat -v for more)", dest='verbose', action='count')
	parser.add_argument('-m', nargs=1, help="Manifest file to use")
	parser.add_argument('-n', help="No run (display what would happen)", dest='norun', action="store_true")
	parser.add_argument('src', help="Source directory")
	parser.add_argument('dst', help="Destination directory")
	opts = parser.parse_args( args )
	verbose = opts.verbose
	active  = not opts.norun
	if active and verbose == 0:
		verbose = 1
	if verbose >= 3:
		print( opts )
	copied = set()
	srcDir = path.normpath( opts.src )
	dstDir = path.normpath( opts.dst )
	if srcDir == dstDir:
		PrintError( "Source and destination are identical" )
	if opts.m:
		ParseManifest( opts.m[0], srcDir, dstDir, copied )
	else:
		atom = SyncAtom( srcDir, dstDir )
		atom.sync( copied )
	if copied:
		if verbose >= 4:
			print("Copied:")
			print("-------")
			copiedSorted = list( copied )
			copiedSorted.sort()
			for i in copiedSorted:
				print(i)
			print("-------")
		Clean( dstDir, copied )
	else:
		if verbose >= 1:
			PrintWarning( "Did not copy anything (empty manifest?)" )

## Main program

ParseArgs( sys.argv[1:] )
