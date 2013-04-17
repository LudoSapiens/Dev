/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#include <Base/IO/Path.h>

#include <map>

#ifdef __GNUC__
#include <unistd.h>
#else
#include <direct.h>
#endif

/*==============================================================================
  UNNAME NAMESPACE
==============================================================================*/

UNNAMESPACE_BEGIN

struct ltstr
{
  bool operator()( const String& s1, const String& s2 ) const
  {
     return s1.compare( s2 ) < 0;
  }
};

std::map<String, String, ltstr> _tokens;

UNNAMESPACE_END

/*==============================================================================
  CLASS Path
==============================================================================*/

NAMESPACE_BEGIN

//------------------------------------------------------------------------------
//!
void
Path::setToken
( const String& token, const String& p )
{
   _tokens[token] = p;
}

//------------------------------------------------------------------------------
//!
String
Path::getPath( const String& token )
{
   const String::SizeType tokenSize = token.size();
   if( token[0] == '<' && token[ tokenSize-1 ] == '>' )
   {
      return _tokens[ token.sub( 1, tokenSize-2 ) ];
   }

   return _tokens[token];
}

//------------------------------------------------------------------------------
//!
String
Path::getCurrentDirectory()
{
   char tmp[1024];
#if _MSC_VER
   const char* ret = _getcwd(tmp, sizeof(tmp));;
#else
   const char* ret = getcwd(tmp, sizeof(tmp));
#endif
   if( ret == nullptr ) return String();
   bs2fs(tmp, 1024);
#if _MSC_VER
   // Native file dialogs use uppercase drive letter, so make them consistent.
   if( tmp[1] == ':' )  tmp[0] = toupper( tmp[0] );
#endif
   return String(tmp);
}

//------------------------------------------------------------------------------
//! Converts backward slashes to forward slashes.
void
Path::bs2fs( char* tmp, size_t s )
{
   while( *tmp && s )
   {
      if( *tmp == '\\' ) *tmp = '/';
      ++tmp;
      --s;
   }
}

//------------------------------------------------------------------------------
//! Converts backward slashes to forward slashes.
void
Path::fs2bs( char* tmp, size_t s )
{
   while( *tmp && s )
   {
      if( *tmp == '/' ) *tmp = '\\';
      ++tmp;
      --s;
   }
}

//------------------------------------------------------------------------------
//!
Path::Path()
{}

//------------------------------------------------------------------------------
//!
Path::Path( const String& p )
{
   append( p );
}

//------------------------------------------------------------------------------
//!
Path::Path( const char* p )
{
   append( p );
}

//------------------------------------------------------------------------------
//!
Path&
Path::set( const String& p )
{
   clear();
   append( p );
   return *this;
}

//------------------------------------------------------------------------------
//!
Path&
Path::set( const char* p )
{
   clear();
   append( p );
   return *this;
}

//------------------------------------------------------------------------------
//!
String
Path::getLongExt() const
{
   String::SizeType pos = _path.findLastOf("/", _path.size() - 2);
   if( pos != String::npos )
   {
      ++pos; // Start right after the "/".
   }
   else
   {
      pos = 0; // Start at the beginning of the string.
   }

   pos = _path.find( '.', pos );
   if( pos != String::npos ) return _path.sub( pos+1 );

   return String();
}

//------------------------------------------------------------------------------
//!
String
Path::nextExt( const String& ext )
{
   String::SizeType pos = ext.find( '.' );
   if( pos != String::npos )
   {
      return ext.sub( pos + 1 );
   }
   return String();
}

//------------------------------------------------------------------------------
//!
bool
Path::goUp()
{
   String::SizeType s = _path.size();
   if( s == 0 ) return false;
   if( s == 1 && _path[0] == '/' )  return false;
   String::SizeType pos = _path.findLastOf("/", s - 2);
   if( pos != String::npos )
   {
      ++pos; // Keep the '/'.
      String::SizeType size = s - pos;
      _path.erase( pos, size );
      return true;
   }
   return false;
}

//------------------------------------------------------------------------------
//!
bool
Path::goUp( uint n )
{
   bool ok = true;
   for( uint i = 0; i < n; ++i )
   {
      ok &= goUp();
   }
   return ok;
}

//------------------------------------------------------------------------------
//! Separates the directory and base portion of a path.
//! Sets the current path as the directory and return the base name.
Path
Path::split()
{
   String::SizeType pos = _path.findLastOf("/", _path.size() - 2);
   if( pos != String::npos )
   {
      Path file = _path.sub( pos+1, String::npos );
      _path.erase( _path.size() - file._path.size() - 1, String::npos );
      return file;
   }
   else
   {
      return Path();
   }
}

//------------------------------------------------------------------------------
//! Returns the last part of the path (i.e. the value returned by split()).
Path
Path::basename() const
{
   String::SizeType siz = _path.size();
   String::SizeType pos = _path.findLastOf("/", siz - 2);
   if( pos != String::npos )
   {
      String::SizeType off = (_path[siz-1] == '/') ? 2 : 1;
      Path base = _path.sub( pos+1, siz-pos-off );
      return base;
   }
   else
   {
      return Path();
   }
}

//------------------------------------------------------------------------------
//!
Path
Path::removeExt() const
{
   String::SizeType pos = _path.findLastOf("/", _path.size() - 2);
   if( pos != String::npos )
   {
      ++pos; // Start right after the "/".
   }
   else
   {
      pos = 0; // Start at the beginning of the string.
   }

   pos = _path.find( '.', pos );

   // No extension?
   if( pos == String::npos ) return _path;
   // Without extension.
   return _path.sub( 0, pos+1 );
}

//------------------------------------------------------------------------------
//!
void
Path::append( const String& p )
{
   // TODO
   // We need special case handling.
   if( !_path.empty() && _path[_path.size()-1] != '/' )
   {
      _path += '/';
   }
   _path += p;

   normalize();
}

//------------------------------------------------------------------------------
//!
void
Path::normalize()
{
   // Replace tokens.
   String::SizeType p = 0;
   while( ( p = _path.find( '<', p ) ) < _path.size() )
   {
      String::SizeType e = _path.find( '>', p );
      _path.replace( p, e-p+1, _tokens[_path.sub( p + 1, e-p-1 )] );
   }

   // FIXME: We could do a in place algorithm!
   // Remove ., .. and //

   // 1. Do we need normalization?
   if( _path.size() <= 2 ) return;

   for( p = 0; ; ++p )
   {
      p = _path.find( '.', p );
      if( p >= _path.size() ) return;
      char c = _path[p+1];
      if( c == '.' || c == '/' || c == 0 ) break;
   }

   // 2. Create a normalized path in another string.
   String npath;
   String::SizeType s = 0;
   bool done = false;
   for( p = 0; !done; s=p )
   {
      // Find the end of the current section.
      p = _path.find( '/', p+1 );
      // No more /, then we are at the last section.
      if( p >= _path.size() )
      {
         p = _path.size()-1;
         done = true;

         // Copy the last section.
         if( _path[p] != '.' && _path[p] != '/' )
         {
            npath += _path.sub( s, p-s+1 );
            continue;
         }
      }
      // Remove one /
      if( p-s == 1 ) continue;
      // We have either . or ..
      if( _path[s+1] == '.' )
      {
         // We have ..
         if( _path[s+2] == '.' )
         {
            // Remove the previous section.
            String::SizeType c = npath.rfind( '/' );
            if( c >= npath.size() )
               npath.clear();
            else
               npath.resize( c );
         }
         continue;
      }
      // Add the current section to the new path.
      npath += _path.sub( s, p-s );
   }
   // 3. Replace string.
   _path = npath;
}

NAMESPACE_END

