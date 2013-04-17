/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#include <Base/Dbg/UnitTest.h>

#include <Base/Dbg/DebugStream.h>
#include <Base/IO/Path.h>
#include <Base/IO/TextStream.h>
#include <Base/MT/Thread.h>
#include <Base/Util/RCObject.h>
#include <Base/Util/RCP.h>
#include <Base/Util/Timer.h>

USING_NAMESPACE

DBG_STREAM( os_main , "Main"  );
DBG_STREAM( os_main2, "Main2" );

//------------------------------------------------------------------------------
//!
void func2()
{
   DBG_BLOCK( os_main, "func2" );

   DBG_MSG( os_main, "interieur de func2" );
}

//Let's use the Main2 trace group

//------------------------------------------------------------------------------
//!
void func()
{
   DBG_BLOCK( os_main2, "func" );

   DBG_MSG( os_main2, "value: " << 5 );
   DBG( int a = 10 );
   DBG_MSG( os_main2, "a = " << a );

   func2();
}


class SubRCObject
   : public RCObject
{
public:
   void setVal( int val ) { _val = val; }
   int getVal() const { return _val; }
private:
   int _val;
};

//------------------------------------------------------------------------------
//!
class TestTask : public Task
{
   public:

   /*----- methods -----*/

   TestTask( int id ) :
      _id( id )
   {
      printf( "creating task: %d\n", _id );
   }

   virtual void execute()
   {
      printf( "running task %d\n", _id );
      for( uint i = 0; i < 100; ++i )
      {
         printf( "computing: %d\n", i );
      }
   }

   virtual ~TestTask()
   {
      printf( "deleting task %d\n", _id );
   }

   /*----- data members -----*/

   int _id;
};

//------------------------------------------------------------------------------
//!
void old_tests( Test::Result& )
{
   Vector<int> vimp;
   std::vector<int> vstl;
   
   Timer timer;
   for( uint i = 0; i < 100000; ++i )
   {
      vstl.push_back( i );
   }
   
   double tstl = timer.elapsed();
   timer.restart();
   
   for( uint i = 0; i < 100000; ++i )
   {
      vimp.push_back( i );
   }
   double timp = timer.elapsed();
   
   printf( "time for stl vector: %f\n", tstl );
   printf( "time for imp vector: %f\n", timp );
   
   
   DBG_MSG( os_main2, "Avant function" );

   func();

   DBG_MSG( os_main2, "Apres function" );

   // *** test of ConstRCP ***

   RCP<SubRCObject>      nonConstObj = new SubRCObject();
   RCP<const SubRCObject> constObj   = new SubRCObject();

   SubRCObject*       nonConstPtr = new SubRCObject();
   const SubRCObject* constPtr    = new SubRCObject();

   bool test;

   // Following line should work with RCP< const T > but doesn't.
   // it works with ConstRCP<T>
   //   test = (nonConstObj == constObj);
   test = (constObj == nonConstObj);
   test = (constObj == constObj);
   test = (nonConstObj == nonConstObj);

   // Following line should work with RCP< const T > but doesn't.
   // it works with ConstRCP<T>
   //   test = (nonConstObj != constObj);
   test = (constObj != nonConstObj);
   test = (constObj != constObj);
   test = (nonConstObj != nonConstObj);

   // Next line shouldn't compile (invalid const->nonConst conversion)
   // nonConstObj = constObj;
   constObj = nonConstObj;
   constObj = constObj;
   nonConstObj = nonConstObj;

   // Next line shouldn't compile (invalid const->nonConst conversion)
   // RCP<SubRCObject>      test1( constObj );
   RCP<const SubRCObject> test2( nonConstObj );
   RCP<const SubRCObject> test3( constObj );
   RCP<SubRCObject>      test4( nonConstObj );

   // Next line shouldn't compile (invalid const->nonConst conversion)
   // RCP<SubRCObject>      test5( constPtr );
   RCP<const SubRCObject> test6( nonConstPtr );
   RCP<const SubRCObject> test7( constPtr );
   RCP<SubRCObject>      test8( nonConstPtr );

   RCP<const SubRCObject> nonConstObj2 = new SubRCObject();
   RCP<const SubRCObject> test9( nonConstObj2 );


   Path path( "c:/" );
   path /= "test/dir/";
   path /= "path.cpp";
   StdOut << "path: " << path.string() << nl;

   ///////////////////////////////////////////////////////////////////////
   // Test multi-threading.

   // Non auto-free.
   Thread thread1( new TestTask( 1 ) );
   printf( "Before thread\n" );
   thread1.wait();
   printf( "After thread\n" );

   // Auto-free
   Thread* thread2 = new Thread( new TestTask( 2 ), true );
   unused( thread2 );

   Thread::sleep( 1.0 );

   CHECK( 10 > 40 );
}

void  init_old_tests()
{
   Test::special().add( new Test::Function( "old_tests", "A bunch of old tests (deprecated)", old_tests ) );
}
