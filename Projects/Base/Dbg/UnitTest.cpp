/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#include <Base/Dbg/UnitTest.h>

#include <Base/Dbg/Defs.h>
#include <Base/IO/StreamIndent.h>

/*==============================================================================
  UNNAMED NAMESPACE
==============================================================================*/
UNNAMESPACE_BEGIN

//------------------------------------------------------------------------------
//!
size_t  _maxNameSize = 0;

//------------------------------------------------------------------------------
//!
String  _fmtName = String( "%1s" );

//------------------------------------------------------------------------------
//!
String  _fmtResRatio = String( "%11d/%-11d" );

//------------------------------------------------------------------------------
//!
String  _fmtResPercent = String( "(%g %%)" );

//------------------------------------------------------------------------------
//!
String  _fmtResEmpty = String( "(empty)" );

//------------------------------------------------------------------------------
//!
//StreamIndent  _indent;

//------------------------------------------------------------------------------
//!
inline  void  updateNameSize( const size_t s )
{
   if( s > _maxNameSize )
   {
      _fmtName = '%';
      _fmtName += String( s );
      _fmtName += 's';
      _maxNameSize = s;
   }
}

//------------------------------------------------------------------------------
//!
RCP<Test::Collection>  _std;

//------------------------------------------------------------------------------
//!
RCP<Test::Collection>  _spc;


UNNAMESPACE_END


NAMESPACE_BEGIN

namespace Test
{


/*==============================================================================
  CLASS Result
==============================================================================*/
//------------------------------------------------------------------------------
//!
void
Result::printFailureInfos( bool clear )
{
   for( Vector< String >::ConstIterator cur = _failureInfos.begin();
        cur != _failureInfos.end();
        ++cur )
   {
      printf( "%s", (*cur).cstr() );
      printf( "\n" );
   }
   if( clear )
   {
      _failureInfos.clear();
   }
}


/*==============================================================================
  CLASS Test
==============================================================================*/
//------------------------------------------------------------------------------
//!
Test::Test( const String& name, const String& desc ):
   _name( name ),
   _desc( desc )
{
   updateNameSize( _name.size() );
}

//------------------------------------------------------------------------------
//!
Test::~Test()
{
}

//------------------------------------------------------------------------------
//!
void
Test::run( Result& result )
{
   //printf( (const char*)_indent );
   printf( _fmtName.cstr(), name().cstr() );
   //printf( ":" );
   fflush( stdout );
   doRun( result );
   uint tot = result.total();
   //printf( " " );
   printf( _fmtResRatio.cstr(), result.successes(), tot );
   //printf( " " );
   if( tot != 0 )
   {
      double rate = 100.0 * result.successes()/tot;
      printf( _fmtResPercent.cstr(), rate );
   }
   else
   {
      printf( "%s", _fmtResEmpty.cstr() );
   }
   if( result.failures() != 0 )
   {
      printf( " %d failures", result.failures() );
   }
   printf( "\n" );
   result.printFailureInfos();
   fflush( stdout );
}

//------------------------------------------------------------------------------
//!
Test*
Test::find( const String& name )
{
   if( _name == name )
   {
      return this;
   }
   else
   {
      return NULL;
   }
}

//------------------------------------------------------------------------------
//!
void
Test::printInfo( const String& prefix ) const
{
   if( name().startsWith( prefix ) )
   {
      printf( _fmtName.cstr(), name().cstr() );
      printf( ": " );
      printf( "%s", _desc.cstr() );
      printf( "\n" );
   }
}


/*==============================================================================
  CLASS Function
==============================================================================*/

//------------------------------------------------------------------------------
//!
Function::Function( const String& name, const String& desc, TestFunc func ):
   Test( name, desc ),
   _func( func )
{
}

//------------------------------------------------------------------------------
//!
Function::~Function()
{
}

//------------------------------------------------------------------------------
//!
void
Function::doRun( Result& result )
{
   (*_func)( result );
}


/*==============================================================================
  CLASS Collection
==============================================================================*/

//------------------------------------------------------------------------------
//!
Collection::Collection( const String& name, const String& desc ):
   Test( name, desc )
{
}

//------------------------------------------------------------------------------
//!
Collection::~Collection()
{
}

//------------------------------------------------------------------------------
//!
void
Collection::add( Test* test )
{
   _tests.pushBack( test );
}

//------------------------------------------------------------------------------
//!
Test*
Collection::find( const String& name )
{
   if( _name == name )  return this;

   for( TestContainer::Iterator cur = _tests.begin(), end = _tests.end();
        cur != end;
        ++cur )
   {
      Test* tmp = (*cur)->find( name );
      if( tmp )  return tmp;
   }
   return NULL;
}

//------------------------------------------------------------------------------
//!
void
Collection::printInfo( const String& prefix ) const
{
   Test::printInfo( prefix );
   for( TestContainer::ConstIterator cur = _tests.begin(), end = _tests.end();
        cur != end;
        ++cur )
   {
      (*cur)->printInfo( prefix );
   }
}

//------------------------------------------------------------------------------
//!
void
Collection::doRun( Result& result )
{
   //printf( ":" );
   printf( " " );
   printf( ":::::::::::::::::::::::::::::" );
   //printf( "%s", _name.cstr() );
   printf( "\n" );
   //++_indent;
   for( TestContainer::Iterator cur = _tests.begin(), end = _tests.end();
        cur != end;
        ++cur )
   {
      Result r;
      (*cur)->run( r );
      result.add( r );
   }
   //--_indent;
   //printf( (const char*)_indent );
   printf( _fmtName.cstr(), "" );
   printf( " -----------------------------\n" );
   printf( _fmtName.cstr(), _name.cstr() );
   //printf( ":" );
   // The the AbsRun() will put the total on the same line.
}

/*==============================================================================
  Public Routines
==============================================================================*/

//------------------------------------------------------------------------------
//!
Collection&  standard()
{
   if( _std.isNull() )
   {
      _std = new Collection( "standard", "Default tests ran by default" );
   }

   return *_std;
}

//------------------------------------------------------------------------------
//!
Collection&  special()
{
   if( _spc.isNull() )
   {
      _spc = new Collection( "special", "Some special tests that can only be run when explicitly specified" );
   }

   return *_spc;
}

//------------------------------------------------------------------------------
//!
void  init( const String& name, const String& desc )
{
   CHECK( _std.isNull() );
   _std = new Collection( name, desc );
}

//------------------------------------------------------------------------------
//!
int  main( int argc, char* argv[] )
{
   // Make sure all of the required collections exist.
   standard();
   special();
   RCP<Collection>  sel = new Collection( "", "" );

   Result res;

   if( argc > 1 )
   {
      for( int argi = 1; argi < argc; ++argi )
      {
         String arg = String( argv[argi] );
         if( arg == "-list" || arg == "-l" )
         {
            String prefix;
            if( argc > argi )
            {
               prefix = argv[argi+1];
            }
            _std->printInfo( prefix );
            printf("\n");
            _spc->printInfo( prefix );
            printf("\n");
            return 0;
         }
         else
         {
            Test* test = _std->find( arg );
            if( test == NULL )
            {
               test = _spc->find( arg );
            }
            if( test != NULL )
            {
               sel->add( test );
            }
            else
            {
               printf( "ERROR - '%s' not found\n", arg.cstr() );
               return 2;
            }
         }
      }
      if( !sel->tests().empty() )
      {
         sel->run( res );
      }
   }
   else
   {
      _std->run( res );
   }

#if _MSC_VER
   // Prevent VS from closing terminal window.
   fflush( NULL );
   getchar();
#endif

   return (res.failures() == 0) ? 0 : 1;
}

} // namespace Test

NAMESPACE_END
