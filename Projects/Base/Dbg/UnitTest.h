/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#ifndef BASE_UNIT_TEST_H
#define BASE_UNIT_TEST_H

#include <Base/StdDefs.h>

#include <Base/ADT/String.h>
#include <Base/ADT/Vector.h>
#include <Base/Util/RCObject.h>
#include <Base/Util/RCP.h>

NAMESPACE_BEGIN

namespace Test
{

/*==============================================================================
  CLASS Result
==============================================================================*/
class Result
{
public:
   /*----- methods -----*/

   Result(): _nSuccesses( 0 ), _nFailures( 0 ) { }

   inline void  add( bool success ) { if( success ) ++_nSuccesses; else ++_nFailures; }
   inline void  add( const Result& r ) { _nSuccesses += r._nSuccesses; _nFailures += r._nFailures; _failureInfos.append(r._failureInfos); }

   inline uint  successes() const { return _nSuccesses; }
   inline uint  failures() const { return _nFailures; }
   inline uint  total() const { return _nSuccesses + _nFailures; }

   inline void  reset() { _nSuccesses = 0; _nFailures = 0; _failureInfos.clear(); }

   inline void  addFailureInfo( const String& s ) { _failureInfos.pushBack(s); }
   BASE_DLL_API void  printFailureInfos( bool clear = true );

protected:
   /*----- data members -----*/

   uint  _nSuccesses;
   uint  _nFailures;

   Vector< String >  _failureInfos;

private:
}; //class Result

#define TEST_ADD( res, test ) \
   do \
   { \
      bool _ok_private = (test); \
      Test::Result& _res_private = (res); \
      _res_private.add( _ok_private ); \
      if( !_ok_private ) \
      { \
         _res_private.addFailureInfo( String().format("FAILED: '" #test "' file: %s line: %d\n", __FILE__, __LINE__) ); \
      } \
   } while( false )


/*==============================================================================
  CLASS Test
==============================================================================*/
class Test:
   public RCObject
{
public:
   /*----- methods -----*/

   BASE_DLL_API Test( const String& name, const String& desc = String() );
   BASE_DLL_API virtual ~Test();

   BASE_DLL_API void  run( Result& result );

   inline  const String&  name() const { return _name; }
   inline  const String&  desc() const { return _desc; }

   BASE_DLL_API virtual Test*  find( const String& name );

   BASE_DLL_API virtual void  printInfo( const String& prefix = String() ) const;

protected:
   /*----- data members -----*/

   String  _name;  //!< A name for the test.
   String  _desc;  //!< An small description.

   /*----- methods -----*/

   BASE_DLL_API virtual void  doRun( Result& result ) = 0;

private:
}; //class Test


/*==============================================================================
  CLASS Function
==============================================================================*/

// Typedef to ease reading.
typedef void (*TestFunc)( Result& );

class Function:
   public Test
{
public:

   /*----- methods -----*/

   BASE_DLL_API Function( const String& name, const String& desc, TestFunc func );
   BASE_DLL_API virtual ~Function();

protected:

   /*----- data members -----*/

   TestFunc  _func;

   /*----- methods -----*/

   BASE_DLL_API virtual void  doRun( Result& result );

private:
}; //class Function


/*==============================================================================
  CLASS Collection
==============================================================================*/
class Collection:
   public Test
{
public:

   /*----- typedefs -----*/
   typedef Vector< RCP<Test> >  TestContainer;

   /*----- methods -----*/

   BASE_DLL_API Collection( const String& name, const String& desc );
   BASE_DLL_API ~Collection();

   BASE_DLL_API void  add( Test* test );
   BASE_DLL_API Test*  find( const String& name );

   BASE_DLL_API virtual void  printInfo( const String& prefix = String() ) const;

   inline const TestContainer&  tests() const { return _tests; }

protected:

   /*----- data members -----*/

   TestContainer  _tests;
   size_t         _maxNameLen;

   /*----- methods -----*/

   BASE_DLL_API void  doRun( Result& result );


private:
}; //class Collection


//------------------------------------------------------------------------------
//!
BASE_DLL_API Collection&  standard();

//------------------------------------------------------------------------------
//!
BASE_DLL_API Collection&  special();

//------------------------------------------------------------------------------
//!
BASE_DLL_API void  init( const String& name, const String& desc );

//------------------------------------------------------------------------------
//!
BASE_DLL_API int  main( int argc, char* argv[] );

} // Test

NAMESPACE_END

#endif //BASE_UNIT_TEST_H
