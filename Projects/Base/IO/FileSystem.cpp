/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#include <Base/IO/FileSystem.h>

#include <Base/Dbg/Defs.h>
#include <Base/IO/TextStream.h>

#if !defined(BASE_IO_USE_DIRENT)
#  if defined(_POSIX_VERSION)
#    define BASE_IO_USE_DIRENT 1
#  else
#    define BASE_IO_USE_DIRENT 0
#  endif
#endif

#if !defined(BASE_IO_USE_STAT)
#  if defined(_POSIX_VERSION)
#    define BASE_IO_USE_STAT 1
#  else
#    define BASE_IO_USE_STAT 0
#  endif
#endif

#if !defined(BASE_IO_USE_WIN32)
#  if defined(_WIN32)
#    define BASE_IO_USE_WIN32 1
#  else
#    define BASE_IO_USE_WIN32 0
#  endif
#endif


#if BASE_IO_USE_DIRENT
#include <dirent.h>
#endif

#if BASE_IO_USE_STAT
#include <sys/stat.h>
#include <unistd.h>
#if !defined(_POSIX_VERSION) && !defined(st_atime)
#  define st_atime st_atimespec.tv_sec
#  define st_mtime st_mtimespec.tv_sec
#  define st_ctime st_ctimespec.tv_sec
#endif
#endif

#if BASE_IO_USE_WIN32
// Some refs:
//   http://msdn.microsoft.com/en-us/library/14h5k7ff.aspx
//   http://msdn.microsoft.com/en-us/library/aa273367.aspx
#include <io.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <windows.h>
#endif


/*==============================================================================
  UNNAMED NAMESPACE
==============================================================================*/
UNNAMESPACE_BEGIN

#if BASE_IO_USE_STAT
//------------------------------------------------------------------------------
//!
FS::Perm  toPerm( mode_t mode )
{
   int tmp = 0;
   //if( mode & S_ISUID )  tmp |= FS::PERM_UID;
   //if( mode & S_ISGID )  tmp |= FS::PERM_GID;
   //if( mode & S_ISVTX )  tmp |= FS::PERM_;
   if( mode & S_IRUSR )  tmp |= (FS::PERM_READ  << (FS::PERM_TYPE_OWNER*4));
   if( mode & S_IWUSR )  tmp |= (FS::PERM_WRITE << (FS::PERM_TYPE_OWNER*4));
   if( mode & S_IXUSR )  tmp |= (FS::PERM_EXEC  << (FS::PERM_TYPE_OWNER*4));
   return (FS::Perm)tmp;
}

//------------------------------------------------------------------------------
//!
FS::Type  toType( mode_t mode )
{
   switch( mode & S_IFMT )
   {
      case S_IFIFO :  return FS::TYPE_FIFO;
      case S_IFCHR :  return FS::TYPE_CHR;
      case S_IFDIR :  return FS::TYPE_DIRECTORY;
      case S_IFBLK :  return FS::TYPE_BLK;
      case S_IFREG :  return FS::TYPE_FILE;
      case S_IFLNK :  return FS::TYPE_SYMLINK;
      case S_IFSOCK:  return FS::TYPE_SOCKET;
      //case S_IFWHT :  return FS::TYPE_UNKNOWN;
      default      :  return FS::TYPE_UNKNOWN;
   }
}
#endif //BASE_IO_USE_STAT

#if BASE_IO_USE_WIN32
//------------------------------------------------------------------------------
//!
FS::Perm  toPerm( ushort mode )
{
   int tmp = 0;
   if( mode & _S_IREAD  )  tmp |= (FS::PERM_READ  << (FS::PERM_TYPE_OWNER*4));
   if( mode & _S_IWRITE )  tmp |= (FS::PERM_WRITE << (FS::PERM_TYPE_OWNER*4));
   if( mode & _S_IEXEC  )  tmp |= (FS::PERM_EXEC  << (FS::PERM_TYPE_OWNER*4));
   return (FS::Perm)tmp;
}

//------------------------------------------------------------------------------
//!
FS::Type  toType( ushort mode )
{
   switch( mode & _S_IFMT )
   {
      case _S_IFCHR :  return FS::TYPE_CHR;
      case _S_IFDIR :  return FS::TYPE_DIRECTORY;
      case _S_IFREG :  return FS::TYPE_FILE;
      default       :  return FS::TYPE_UNKNOWN;
   }
}
#endif //BASE_IO_USE_WIN32

UNNAMESPACE_END


NAMESPACE_BEGIN

namespace FS
{

/*==============================================================================
  CLASS Entry
==============================================================================*/

//------------------------------------------------------------------------------
//!
Entry::Entry( const Path& path ):
   _path( path ),
   _info( NULL )
{
}

//------------------------------------------------------------------------------
//!
Entry::~Entry()
{
   delete _info;
}

//------------------------------------------------------------------------------
//!
void
Entry::refresh() const
{
   if( _info == NULL )
   {
      _info = new EntryInfo( _path );
   }
   else
   {
      _info->refresh( _path );
   }
}

//------------------------------------------------------------------------------
//!
String
Entry::subpath() const
{
   const String& str = _path.string();
   String::SizeType pos = str.rfind( '/' );
   return str.sub( pos + 1 );
}

//------------------------------------------------------------------------------
//!
bool
Entry::needsRedirect() const
{
   int type = info()._type;
#if defined(_WIN32)
   return (type == TYPE_SYMLINK) || (type == TYPE_SHORTCUT);
#elif defined(__APPLE__)
   return (type == TYPE_SYMLINK) || (type == TYPE_ALIAS);
#elif defined(_POSIX_VERSION)
   return (type == TYPE_SYMLINK);
#else
#  error Please define a redirectable file-system type for your platform.
   return false;
#endif
}

//------------------------------------------------------------------------------
//!
Path
Entry::target() const
{
   CHECK( false );
   return Path();
}

//------------------------------------------------------------------------------
//!
String
Entry::toStr() const
{
   if( _info )
   {
      return String().format("FS::Entry{path='%s',type=%x,perm=%04x}", _path.cstr(), _info->_type, _info->_perm);
   }
   else
   {
      return String().format("FS::Entry{path='%s',???}", _path.cstr());
   }
}


/*==============================================================================
  CLASS EntryInfo
==============================================================================*/

//------------------------------------------------------------------------------
//!
Entry::EntryInfo::EntryInfo( const Path& path )
{
   refresh( path );
}

//------------------------------------------------------------------------------
//!
void
Entry::EntryInfo::refresh( const Path& path )
{
#if BASE_IO_USE_STAT
   // Posix.
   struct stat st;
   if( stat(path.cstr(), &st) == 0 )
   {
      // Type.
      _type = toType( st.st_mode );

      // Time.
      _ctime.set( st.st_ctime );
      _mtime.set( st.st_mtime );

      // Perm.
      _perm = toPerm( st.st_mode );

      // Size.
      _size = st.st_size;
   }
   else
   {
      _type = TYPE_UNKNOWN;
      //_ctime = 0;
      //_mtime = 0;
      _perm = 0;
      _size = (size_t)-1;
   }
#elif BASE_IO_USE_WIN32
   // Win32.
   struct _stat st;
   if( _stat(path.cstr(), &st) == 0 )
   {
      // Type.
      _type = toType( st.st_mode );

      // Time.
      _ctime.set( st.st_ctime );
      _mtime.set( st.st_mtime );

      // Perm.
      _perm = toPerm( st.st_mode );

      // Size.
      _size = st.st_size;
   }
   else
   {
      _type = TYPE_UNKNOWN;
      //_ctime = 0;
      //_mtime = 0;
      _perm = 0;
      _size = (size_t)-1;
   }
#endif
}


#if BASE_IO_USE_DIRENT

/*==============================================================================
  CLASS DirIterator::Impl
==============================================================================*/
class DirIterator::Impl
{
public:

   /*----- methods -----*/

   Impl( const Path& path )
   {
      init( path );
   }

   ~Impl()
   {
      term();
   }

   void  reset( const Path& path )
   {
      term();
      init( path );
   }

   bool  valid() const
   {
      return _dirent != NULL;
   }

   void  next()
   {
      if( _dir )
      {
         _dirent = readdir( _dir );
      }
   }

   String data() const
   {
      if( _dir )
      {
         return String( _dirent->d_name );
      }

      return String();
   }


protected:

   /*----- data members -----*/

   DIR*     _dir;
   dirent*  _dirent;

   /*----- methods -----*/

   void  init( const Path& path )
   {
      _dir = opendir( path.cstr() );
      if( _dir )
      {
         // WARNING: Not the re-entrant version (readdir_r), but there are issues there anyways.
         // Ref: http://womble.decadentplace.org.uk/readdir_r-advisory.html
         _dirent = readdir( _dir );
      }
      else
      {
         _dirent = NULL;
      }
   }

   void term()
   {
      if( _dir )
      {
         closedir( _dir );
      }
   }

}; //class DirIterator::Impl

#elif BASE_IO_USE_WIN32

/*==============================================================================
  CLASS DirIterator::Impl
==============================================================================*/
//!< Ref: http://msdn.microsoft.com/en-us/library/t0wd4t32.aspx
class DirIterator::Impl
{
public:

   /*----- methods -----*/

   Impl( const Path& path ):
      _state( -1 )
   {
      init( path );
   }

   ~Impl()
   {
      term();
   }

   void  reset( const Path& path )
   {
      term();
      init( path );
   }

   bool  valid() const
   {
      return (_state != -1);
   }

   void  next()
   {
      if( _handle != -1 )
      {
         _state = _findnext( _handle, &_findData );
      }
   }

   String data() const
   {
      if( _handle != -1 )
      {
         return String( _findData.name );
      }

      return String();
   }


protected:

   /*----- data members -----*/

   intptr_t     _handle;
   _finddata_t  _findData;
   int          _state;

   /*----- methods -----*/

   void  init( const Path& path )
   {
      // Need to put a wildcard to find files inside the directory.
      size_t s = path.string().size() + 3;
      char* tmp = new char[s]; // append "\*" and \0.
      strcpy( tmp, path.cstr() );
      // We don't seem to need to convert slashes.
      //Path::fs2bs( tmp );
      //tmp[s-3] = '\\';
      tmp[s-3] = '/';
      tmp[s-2] = '*';
      tmp[s-1] = '\0';
      _handle = _findfirst(tmp, &_findData);
      _state  = (_handle != -1) ? 0 : -1;
      delete [] tmp;
   }

   void term()
   {
      if( _handle != -1 )
      {
         _findclose( _handle );
      }
   }

}; //class DirIterator::Impl

#endif //BASE_IO_USE_WIN32


/*==============================================================================
  CLASS DirIterator
==============================================================================*/

//------------------------------------------------------------------------------
//!
DirIterator::DirIterator( const Path& path )
{
   _impl = new Impl( path );
}

//------------------------------------------------------------------------------
//!
DirIterator::~DirIterator()
{
   delete _impl;
}

//------------------------------------------------------------------------------
//! Drops the previous state to start anew with the specified path.
DirIterator&
DirIterator::reset( const Path& path )
{
   _impl->reset( path );
   return *this;
}

//------------------------------------------------------------------------------
//! Returns whether or not the iterator points to valid data.
//! A false value normally indicates the end of the iteration.
bool
DirIterator::operator()() const
{
   return _impl->valid();
}

//------------------------------------------------------------------------------
//! Sets the iterator to the next value.
DirIterator&
DirIterator::operator++()
{
   _impl->next();
   return *this;
}

//------------------------------------------------------------------------------
//! Converts the current element into a string (gets only the subpath).
String
DirIterator::operator*() const
{
   return _impl->data();
}


//------------------------------------------------------------------------------
//! Search the specified directory for various patterns (files or subdirectories).
//! The list of patterns must be of type "const char*" and end with the NULL pointer
//! to indicate the last pattern.
//! The specified entry points to found element, if any (or the last candidate checked otherwise).
bool  searchUp( Entry& entry, const Path& dir, const char* patterns[] )
{
   Path curDir = dir;
   Path curPath;
   int loop = 0;
   while( !curDir.empty() )
   {
      if( ++loop > 100 )  break;  // Safeguard.
      for( const char** p = patterns;
           *p != NULL && *p[0] != '\0';
           ++p )
      {
         curPath = curDir;
         curPath /= *p;
         entry.refresh( curPath );
         if( entry.exists() )  return true;
      }
      curDir.goUp();
   }
   return false;
}

//------------------------------------------------------------------------------
//! Search the specified directory for various patterns (files or subdirectories) of
//! a specific type.
//! The list of patterns must be of type "const char*" and end with the NULL pointer
//! to indicate the last pattern.
//! The specified entry points to found element, if any (or the last candidate checked otherwise).
bool  searchUp( Entry& entry, const Type type, const Path& dir, const char* patterns[] )
{
   Path curDir = dir;
   Path curPath;
   int loop = 0;
   while( !curDir.empty() )
   {
      if( ++loop > 100 )  break;  // Safeguard.
      for( const char** p = patterns;
           *p != NULL && *p[0] != '\0';
           ++p )
      {
         curPath = curDir;
         curPath /= *p;
         entry.refresh( curPath );
         if( entry.exists() && entry.type() == type )  return true;
      }
      curDir.goUp();
   }
   return false;
}

//------------------------------------------------------------------------------
//! Creates a single directory at the specified path.
//! Returns false on failure.
bool  createDirectory( const Path& path )
{
#if BASE_IO_USE_STAT
   return mkdir( path.cstr(), 0777 ) == 0;
#elif BASE_IO_USE_WIN32
   return CreateDirectoryA( path.cstr(), NULL ) == TRUE;
#else
#error MKDIR missing implementation.
   return false;
#endif
}

//------------------------------------------------------------------------------
//! Creates a directory and potentially all of the parents at the specified path.
//! Returns false on failure.
bool  createDirectories( const Path& path )
{
   Path cur = path;
   FS::Entry entry = cur;
   if( entry.exists() )  return true;

   Vector< Path >  subdirs;
   Path subdir = cur.split();
   while( !subdir.empty() )
   {
      subdirs.pushBack( subdir );
      entry = cur;
      if( entry.exists() )  break;
      subdir = cur.split();
   }

   while( !subdirs.empty() )
   {
      cur /= subdirs.back();
      if( !createDirectory(cur) )  return false;
      subdirs.popBack();
   }

   return true;
}

//------------------------------------------------------------------------------
//! Deletes a path entry.
//! Returns false on failure.
bool  remove( const Path& path )
{
#if BASE_IO_USE_STAT
   return unlink( path.cstr() ) == 0;
#elif BASE_IO_USE_WIN32
   //return RemoveDirectoryA( path.cstr() ) == TRUE;
   return DeleteFileA( path.cstr() ) == TRUE;
#else
#error MKDIR missing implementation.
   return false;
#endif
}

} //namespace FS

NAMESPACE_END
