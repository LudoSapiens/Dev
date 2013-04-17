/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#ifndef BASE_FILE_SYSTEM_H
#define BASE_FILE_SYSTEM_H

#include <Base/StdDefs.h>

#include <Base/ADT/Vector.h>
#include <Base/IO/Path.h>
#include <Base/Util/Date.h>

NAMESPACE_BEGIN

namespace FS
{

   /*----- types -----*/
   enum Type
   {
      TYPE_UNKNOWN,
      // Common types.
      TYPE_DIRECTORY,
      TYPE_FILE,
      // DOS-specific types.
      TYPE_DEVICE,
      // OSX-specific types.
      TYPE_ALIAS,
      TYPE_BUNDLE,
      // UNIX-specific types.
      TYPE_BLK,
      TYPE_CHR,
      TYPE_FIFO,
      TYPE_SOCKET,
      TYPE_SYMLINK,
      // WIN-specific types.
      TYPE_SHORTCUT
   };

   enum Perm
   {
      PERM_EXEC  = 0x01,
      PERM_WRITE = 0x02,
      PERM_READ  = 0x04
   };

   enum PermType
   {
      PERM_TYPE_OTHER = 0,
      PERM_TYPE_GROUP = 1,
      PERM_TYPE_USER  = 2,
      PERM_TYPE_OWNER = 3
   };


   /*==============================================================================
     CLASS DirIterator
   ==============================================================================*/
   //!< This class is used to iterate over all of the elements of a directory:
   //!<   for( FS::DirIterator it(path); it(); ++it )
   //!<   {
   //!<      String subpath = *it;
   //!<   }
   class DirIterator
   {
   public:

      /*----- methods -----*/
      BASE_DLL_API DirIterator( const Path& path );
      BASE_DLL_API ~DirIterator();

      // Disable copy.
      DirIterator( const DirIterator& );
      DirIterator& operator=( const DirIterator& );

      //------------------------------------------------------------------------------
      //! Drops the previous state to start anew with the specified path.
      BASE_DLL_API DirIterator&  reset( const Path& path );

      //------------------------------------------------------------------------------
      //! Returns whether or not the iterator points to valid data.
      //! A false value normally indicates the end of the iteration.
      BASE_DLL_API bool  operator()() const;

      //------------------------------------------------------------------------------
      //! Sets the iterator to the next value.
      BASE_DLL_API DirIterator&  operator++();

      //------------------------------------------------------------------------------
      //! Converts the current element into a string (gets only the subpath).
      BASE_DLL_API String operator*() const;

      //------------------------------------------------------------------------------
      //! Converts the current element into a string (gets only the subpath).
      //operator String() const { return *(*this); }

   protected:

      class Impl;

      /*----- data members -----*/

      Impl*  _impl;  //!< Private, platform-dependent implementation.

   }; //class DirIterator


   /*==============================================================================
     CLASS Entry
   ==============================================================================*/
   class Entry
   {
   public:

      /*----- methods -----*/

      inline Entry(): _info(NULL) {}
      BASE_DLL_API Entry( const Path& path );
      BASE_DLL_API ~Entry();

      BASE_DLL_API void  refresh() const;
      inline       void  refresh( const Path& path ) { _path = path; refresh(); }

      inline const Path&  path() const { return _path; }
      //inline const Path&  operator() const { return _path; }
      inline void  path( const Path& path ) { _path = path; delete _info; _info = NULL; }

      BASE_DLL_API String  subpath() const;

      inline Type  type() const { return (Type)info()._type; }

      BASE_DLL_API bool  needsRedirect() const;
      BASE_DLL_API Path  target() const;

      inline bool  exists() { return (info()._type != TYPE_UNKNOWN); }
      //inline bool  hidden();

      inline Perm  permissions() const { return (Perm)info()._perm; }
      inline Perm  permissions( PermType type ) const { return (Perm)((permissions() >> (type*4)) & 0x0F); }

      inline Perm  groupPermissions() const { return permissions(PERM_TYPE_GROUP); }
      inline Perm  otherPermissions() const { return permissions(PERM_TYPE_OTHER); }
      inline Perm  ownerPermissions() const { return permissions(PERM_TYPE_OWNER); }
      inline Perm  userPermissions()  const { return permissions(PERM_TYPE_USER ); }
      inline bool  isExecutable() const;
      inline bool  isReadable() const;
      inline bool  isWritable() const;

      inline Date  created() const { return info()._ctime; }
      inline Date  lastModified() const { return info()._mtime; }

      inline size_t  size() const { return info()._size; }

      //BASE_DLL_API String  user() const;
      BASE_DLL_API uint    userID() const;
      //BASE_DLL_API String  group() const;
      BASE_DLL_API uint    groupID() const;
      //BASE_DLL_API String  owner() const;
      BASE_DLL_API uint    ownerID() const;

      inline bool  hasChildren() const { return (info()._type != TYPE_DIRECTORY); }

      BASE_DLL_API String  toStr() const;

      inline bool  remove() const;

   protected:

      /*----- data types -----*/

      /*==============================================================================
        CLASS EntryInfo
      ==============================================================================*/
      class EntryInfo
      {
      public:
         EntryInfo( const Path& path );

         void  refresh( const Path& path );

         int       _type;

         //Date    _atime;
         Date      _ctime;
         Date      _mtime;

         int       _perm;

         size_t    _size;
      };


      /*----- methods -----*/

      inline       EntryInfo&  info()       { if( _info == NULL ) refresh(); return *_info; }
      inline const EntryInfo&  info() const { if( _info == NULL ) refresh(); return *_info; }


      /*----- data members -----*/

      Path                _path;  //!< The current path to the Entry.
      mutable EntryInfo*  _info;  //!< The cached entry info structure.

   }; //class Entry


   // Some utility routines.
   BASE_DLL_API bool  searchUp( Entry& dst, const Path& dir, const char* patterns[] );
   BASE_DLL_API bool  searchUp( Entry& dst, const Type type, const Path& dir, const char* patterns[] );
   BASE_DLL_API bool  createDirectory( const Path& path );
   BASE_DLL_API bool  createDirectories( const Path& path );
   //BASE_DLL_API bool createFile( const Path& path, Mode mode );
   //BASE_DLL_API bool createDirectory( const Path& path );
   BASE_DLL_API bool remove( const Path& path );
   //BASE_DLL_API bool delete( const Directory& dir, bool recursive = false );
   //BASE_DLL_API bool move( const Entry& src, const Path& dst );
   //BASE_DLL_API bool move( const Path& src, const Path& dst );

   //BASE_DLL_API String  user( uint id );
   //BASE_DLL_API uint    userToID( const String& name );
   //BASE_DLL_API String  group( uint id );
   //BASE_DLL_API uint    groupToID( const String& name );

//------------------------------------------------------------------------------
//!
inline bool  Entry::remove() const
{
   return FS::remove( path() );
}


} // namespace FS

NAMESPACE_END

#endif //BASE_FILE_SYSTEM_H
