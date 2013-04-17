/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#include <Base/Util/Arguments.h>

#include <Base/Util/Bits.h>

#include <cstring>
#include <cstdio>

USING_NAMESPACE

/*==============================================================================
  UNNAMED NAMESPACE
==============================================================================*/
UNNAMESPACE_BEGIN



UNNAMESPACE_END

//------------------------------------------------------------------------------
//! Gets to the next non-masked argument.
Arguments::Iterator&
Arguments::Iterator::operator++()
{
   if( _pos < _args->argc() )
   {
      do
      {
         ++_pos;
         if( _pos == _args->argc() )
         {
            break;
         }
         else
         if( !_args->masked(_pos) )
         {
            break;
         }
      } while( true );
   }
   return *this;
}

//------------------------------------------------------------------------------
//!
Arguments::Iterator
Arguments::Iterator::operator+( int off ) const
{
   if( off < 0 )  return Iterator();

   Iterator it( *this );
   while( off-- > 0 )
   {
      ++it;
      if( !it.isValid() ) return it; // Early out.
   }
   return it;
}

//------------------------------------------------------------------------------
//!
Arguments::Arguments( int argc, char* argv[] ):
   _argc( argc ), _argv( argv )
{
   size_t s = (argc+7) >> 3;
   _mask = new uint8_t[s];
   memset( _mask, 0, s );
}

//------------------------------------------------------------------------------
//!
Arguments::~Arguments()
{
   delete [] _mask;
}

//------------------------------------------------------------------------------
//!
void
Arguments::mask( int arg )
{
   setbits( _mask, arg, 1, 0x1 );
}

//------------------------------------------------------------------------------
//!
void
Arguments::mask( int argStart, int argEnd )
{
   if( argEnd < argStart )
   {
      int tmp = argStart;
      argStart = argEnd;
      argEnd = tmp;
   }
   int d = argEnd - argStart + 1;
   while( d > 32 )
   {
      setbits( _mask, argStart, 32, 0xFFFFFFFF );
      argStart += 32;
      d -= 32;
   }
   setbits( _mask, argStart, d, 0xFFFFFFFF );
}

//------------------------------------------------------------------------------
//!
void
Arguments::unmask( int arg )
{
   setbits( _mask, arg, 1, 0x0 );
}

//------------------------------------------------------------------------------
//!
void
Arguments::setMask( int arg, int val )
{
   setbits( _mask, arg, 1, val );
}

//------------------------------------------------------------------------------
//!
bool
Arguments::masked( int arg )
{
   return getbits( _mask, arg, 1 ) != 0x0;
}

//------------------------------------------------------------------------------
//!
int
Arguments::find( const char* str )
{
   for( int i = 0; i < _argc; ++i )
   {
      if( strcmp(str, _argv[i]) == 0 )
      {
         return i;
      }
   }
   return -1;
}

//------------------------------------------------------------------------------
//!
int
Arguments::findPrefix( const char* str )
{
   size_t s = strlen(str);
   for( int i = 0; i < _argc; ++i )
   {
      if( strncmp(str, _argv[i], s) == 0 )
      {
         return i;
      }
   }
   return -1;
}
