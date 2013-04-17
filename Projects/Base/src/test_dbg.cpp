/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#include <Base/Dbg/UnitTest.h>

#include <Base/Dbg/DebugStream.h>
#include <Base/Dbg/Defs.h>

USING_NAMESPACE

void dbg_CHECK( Test::Result& )
{
   CHECK( 1 == 2 );
   printf("If you read this, you are in a release build.\n");
}

void dbg_DBG( Test::Result& )
{
   DBG( printf("This should only print in debug.\n") );
   printf("(nothing special in release)\n");
}

void dbg_DBG_BEGIN_END( Test::Result& )
{
   DBG_BEGIN();
   int n = 4;
   printf("This should only print\n");
   printf("                       in debug (n=%d).\n", n);
   DBG_END();
   printf("(nothing special in release)\n");
}

void dbg_DBG_HALT( Test::Result& )
{
   printf("Before halting in the debugger\n");
   DBG_HALT();
   printf("After halting in the debugger\n");
}

void dbg_debug_stream( Test::Result& res )
{
   DebugStream ds1("test");
   ds1.activate();
   ds1 << "test 1 - (always)" << nl;
   ds1.deactivate();
   ds1 << "test 2 - (never)" << nl;
   ds1.activate();
   ds1 << "test 3 - (always)" << nl;

   DBG_STREAM( ds2, "test" );
   TEST_ADD( res, &ds1.state() == &ds2.state() );
   ds2 << "test 4 - (always)" << nl;
   ds1.deactivate();
   ds2 << "test 5 - (never)" << nl;

   DebugStream ds3("bogus");
   ds3 << "bogus 1 - (depends)" << nl;

   fprintf( stderr, "This line goes in stderr\n" );
   fprintf( stdout, "This line goes in stdout\n" );

   ds1.activate();
   ds1 << ds1.pre() << "Indent level " << ds1.depth() << nl;
   ++ds1;
   ds1 << ds1.pre() << "Indent level " << ds1.depth() << nl;
   ds1 << ds1.pre() << "Doing something" << nl;
   ++ds1;
   ds1 << ds1.pre() << "Indent level " << ds1.depth() << nl;
   ds1 << ds1.pre() << "Doing something" << nl;
   --ds1;
   ds1 << ds1.pre() << "Continuing level " << ds1.depth() << nl;
   --ds1;
   ds1 << ds1.pre() << "Continuing level " << ds1.depth() << nl;
   --ds1;
   ds1 << ds1.pre() << "Decrementing too far " << ds1.depth() << nl;

   ds1 << nl;
   {
      DebugStream::Block b( ds1.block() );
      ds1 << ds1.pre() << "Inside funcA()" << nl;
      ds1 << ds1.pre() << "funcA::part1" << nl;
      {
         DebugStream::Block b( ds1.block() );
         ds1 << ds1.pre() << "Inside funcB()" << nl;
         ds1 << ds1.pre() << "funcB::part1" << nl;
         ds1 << ds1.pre() << "funcB::part2" << nl;
      }
      ds1 << ds1.pre() << "funcA::part2" << nl;
      {
         DebugStream::Block b( ds1.block() );
         ds1 << ds1.pre() << "Inside funcC()" << nl;
         ds1 << ds1.pre() << "funcC::part1" << nl;
         ds1 << ds1.pre() << "funcC::part2" << nl;
      }
      ds1 << ds1.pre() << "funcA::part3" << nl;
   }

   ds1 << "The following 11(+3) lines should only print in debug" << nl;
   DBG_MSG( ds1, "Starting" );
   {
      DBG_BLOCK( ds1, "funcA()" );
      DBG_MSG( ds1, "funcA::part1" );
      {
         DBG_BLOCK( ds1, "funcB()" );
         DBG_MSG( ds1, "funcB::part1" );
         DBG_MSG( ds1, "funcB::part2" );
      }
      DBG_MSG( ds1, "funcA::part2" );
      {
         DBG_BLOCK( ds1, "funcC()" );
         DBG_MSG_BEGIN( ds1 );
         ds1 << "funcC";
         ds1 << "::";
         ds1 << "part1";
         DBG_MSG_END( ds1 );
         DBG_MSG( ds1, "funcC::part2" );
      }
      DBG_MSG( ds1, "funcA::part3" );
   }

   ds1 << "The following 11(+3) lines should always print" << nl;
   REL_MSG( ds1, "Starting" );
   {
      REL_BLOCK( ds1, "funcA()" );
      REL_MSG( ds1, "funcA::part1" );
      {
         REL_BLOCK( ds1, "funcB()" );
         REL_MSG( ds1, "funcB::part1" );
         REL_MSG( ds1, "funcB::part2" );
      }
      REL_MSG( ds1, "funcA::part2" );
      {
         REL_BLOCK( ds1, "funcC()" );
         REL_MSG_BEGIN( ds1 );
         ds1 << "funcC";
         ds1 << "::";
         ds1 << "part1";
         REL_MSG_END( ds1 );
         REL_MSG( ds1, "funcC::part2" );
      }
      REL_MSG( ds1, "funcA::part3" );
   }

   // Testing nesting and indentation across multiple streams.
   REL_BLOCK( ds1, "ds1 A" );
   {
      REL_BLOCK( ds3, "ds3 B" );
      {
         REL_BLOCK( ds1, "ds1 C" );
         {
            REL_BLOCK( ds3, "ds3 D" );
            {
               REL_BLOCK( ds1, "ds1 E");
               REL_MSG( ds3, "ds3 E1");
               REL_MSG( ds1, "ds1 E2");
               REL_MSG( ds3, "ds3 E3");
            }
            REL_MSG( ds3, "ds3 D1");
            REL_MSG( ds1, "ds1 D2");
            REL_MSG( ds3, "ds3 D3");
         }
         REL_MSG( ds3, "ds3 C1");
         REL_MSG( ds1, "ds1 C2");
         REL_MSG( ds3, "ds3 C3");
      }
      REL_MSG( ds3, "ds3 B1");
      REL_MSG( ds1, "ds1 B2");
      REL_MSG( ds3, "ds3 B3");
   }
   REL_MSG( ds3, "ds3 A1");
   REL_MSG( ds1, "ds1 A2");
   REL_MSG( ds3, "ds3 A3");
}

void init_dbg()
{
   Test::special().add( new Test::Function( "CHECK"        , "Verifies that CHECK() works properly"                          , dbg_CHECK         ) );
   Test::special().add( new Test::Function( "DBG"          , "Verifies that DBG() works properly"                            , dbg_DBG           ) );
   Test::special().add( new Test::Function( "DBG_BEGIN_END", "Verifies that DBG_BEGIN()..DBG_END() work properly"            , dbg_DBG_BEGIN_END ) );
   Test::special().add( new Test::Function( "DBG_HALT"     , "Verifies that DBG_HALT() properly stops in the debugger or not", dbg_DBG_HALT      ) );
}
