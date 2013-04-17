/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#include <Base/Dbg/UnitTest.h>

#include <Base/Msg/Delegate.h>
#include <Base/Msg/DelegateList.h>

USING_NAMESPACE

// Delegates
void addOne( int& v ) { v += 1; }

struct A
{
   A( int v ): _var(v) {}
   int _var;
   void add( int& v ) { v += _var; }
   float addVar( int v ) { return (float)v + _var; }
};

void msg_delegates( Test::Result& res )
{
   int v;
   Delegate1<int&> fun;

   // There are no makeDelegate routines taking this one.
   fun.bind( &addOne );
   v = 2;
   fun(v);
   TEST_ADD( res, v == 3 );

   A a(2);
   v = 3;
   fun = makeDelegate(&a, &A::add);
   fun(v);
   TEST_ADD( res, v == 5 );

   Delegate1<int, float> funF( &a, &A::addVar );
   a._var = 3;
   TEST_ADD( res, funF(4) == 7 );

   Delegate1List<int&> delegates;
   fun.bind( &addOne );
   delegates.addDelegate( fun );
   fun = makeDelegate(&a, &A::add);
   delegates.addDelegate( fun );
   a._var = 2;
   v = 3;
   delegates.exec(v); // ((3 + 1) + 2) = 6
   TEST_ADD( res, v == 6 );
}

void init_msg()
{
   RCP<Test::Collection> col = new Test::Collection( "msg", "Collection for Base/Msg" );
   col->add( new Test::Function("delegates", "Tests delegates", msg_delegates) );
   Test::standard().add( col.ptr() );
}
