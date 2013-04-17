/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#ifndef BASE_PATH_H
#define BASE_PATH_H

#include <Base/StdDefs.h>

#include <Base/ADT/String.h>

NAMESPACE_BEGIN

/*==============================================================================
  CLASS Path
==============================================================================*/

//! The design of this class is highly based on the boost filesystem.

class Path
{

public:

   /*----- static methods -----*/

   BASE_DLL_API static void setToken( const String& token, const String& p );
   BASE_DLL_API static String getPath( const String& token );
   BASE_DLL_API static String getCurrentDirectory();

   BASE_DLL_API static void bs2fs( char* tmp, size_t s = (size_t)-1 );
   BASE_DLL_API static void fs2bs( char* tmp, size_t s = (size_t)-1 );

   /*----- methods -----*/

   BASE_DLL_API Path();
   BASE_DLL_API Path( const String& p );
   BASE_DLL_API Path( const char* p );

   BASE_DLL_API Path& set( const String& p );
   BASE_DLL_API Path& set( const char* p );

   inline Path& operator/=( const Path& rhs );
   inline Path operator/( const Path& rhs );

   inline Path& operator+=( const String& str );
   inline Path  operator+ ( const String& str );

   inline const String& string() const;
   inline const char* cstr() const;

   inline bool empty() const;
   inline void clear();

   inline       String getExt()     const;
   BASE_DLL_API String getLongExt() const;
   static BASE_DLL_API String nextExt( const String& ext );

   BASE_DLL_API bool goUp();
   BASE_DLL_API bool goUp( uint n );
   BASE_DLL_API Path split();
   inline       Path dirname() const;
   BASE_DLL_API Path basename() const;
   BASE_DLL_API Path removeExt() const;

   inline Path&  toDir();

   inline bool  isAbsolute() const;
   inline bool  isRelative() const { return !isAbsolute(); }

private:

   /*----- methods -----*/

   BASE_DLL_API void append( const String& p );
   BASE_DLL_API void normalize();

   /*----- data members -----*/

   String _path;
};

//------------------------------------------------------------------------------
//!
inline bool  operator==( const Path& lhs, const Path& rhs )
{
   return lhs.string() == rhs.string();
}

//------------------------------------------------------------------------------
//!
inline bool  operator!=( const Path& lhs, const Path& rhs )
{
   return lhs.string() != rhs.string();
}

//------------------------------------------------------------------------------
//!
inline bool  operator<( const Path& lhs, const Path& rhs )
{
   return lhs.string() < rhs.string();
}

//------------------------------------------------------------------------------
//!
inline bool  operator<=( const Path& lhs, const Path& rhs )
{
   return lhs.string() <= rhs.string();
}

//------------------------------------------------------------------------------
//!
inline Path&
Path::operator/=( const Path& rhs )
{
   append( rhs._path );
   return *this;
}

//------------------------------------------------------------------------------
//!
inline Path
Path::operator/( const Path& rhs )
{
   return Path( *this ) /= rhs;
}

//------------------------------------------------------------------------------
//!
inline Path&
Path::operator+=( const String& str )
{
   _path += str;
   return *this;
}

//------------------------------------------------------------------------------
//!
inline Path
Path::operator+( const String& str )
{
   return Path( _path + str );
}

//------------------------------------------------------------------------------
//!
inline const String&
Path::string() const
{
   return _path;
}

//------------------------------------------------------------------------------
//!
inline const char*
Path::cstr() const
{
   return _path.cstr();
}

//------------------------------------------------------------------------------
//!
inline bool
Path::empty() const
{
   return _path.empty();
}

//------------------------------------------------------------------------------
//!
inline void
Path::clear()
{
   _path.clear();
}

//------------------------------------------------------------------------------
//!
inline String
Path::getExt() const
{
   return _path.getExt();
}

//------------------------------------------------------------------------------
//!
inline Path
Path::dirname() const
{
   Path tmp = *this;
   tmp.goUp();
   return tmp;
}

//------------------------------------------------------------------------------
//! Adds a trailing '/', but only if none is there already.
inline Path&
Path::toDir()
{
   if( !_path.empty() && _path[_path.size()-1] != '/' ) _path += '/';
   return *this;
}

//------------------------------------------------------------------------------
//!
inline bool
Path::isAbsolute() const
{
   // Empty paths are considered relative.
   if( _path.empty() )  return false;

   // Classic POSIX path starting with a slash.
   if( _path[0] == '/' )  return true;

   // Classing Windows path of the form: 'C:/Something'.
   if( _path.size() > 2 && _path[1] == ':' &&  _path[2] == '/' )  return true;

   return false;
}


/*==============================================================================
  Non-member functions
==============================================================================*/

//------------------------------------------------------------------------------
//!
inline Path
operator/( const char* lhs, const Path& rhs )
{
   return Path( lhs ) /= rhs;
}

//------------------------------------------------------------------------------
//!
inline Path
operator/( const String& lhs, const Path& rhs )
{
   return Path( lhs ) /= rhs;
}

NAMESPACE_END

#endif

