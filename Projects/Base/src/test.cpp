/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#include <Base/Dbg/UnitTest.h>

USING_NAMESPACE

void init_adt();
void init_dbg();
void init_io();
void init_msg();
void init_mt();
void init_net();
void init_util();
void init_old_tests();

void test_types( Test::Result& res )
{
   TEST_ADD( res, sizeof(int8_t ) == 1 );
   TEST_ADD( res, sizeof(int16_t) == 2 );
   TEST_ADD( res, sizeof(int32_t) == 4 );
   TEST_ADD( res, sizeof(int64_t) == 8 );

   TEST_ADD( res, sizeof(uint8_t ) == 1 );
   TEST_ADD( res, sizeof(uint16_t) == 2 );
   TEST_ADD( res, sizeof(uint32_t) == 4 );
   TEST_ADD( res, sizeof(uint64_t) == 8 );

   TEST_ADD( res, sizeof(uchar ) == 1 );
   TEST_ADD( res, sizeof(ushort) == 2 );
   TEST_ADD( res, sizeof(uint  ) == 4 );

   TEST_ADD( res, sizeof(float ) == 4 );
   TEST_ADD( res, sizeof(double) == 8 );
}


//------------------------------------------------------------------------------
//!
int
main( int argc, char* argv[] )
{
   RCP<Test::Collection> col = new Test::Collection( "Base", "Collection for Base" );
   col->add( new Test::Function( "types", "Tests that the basic types are of the proper size", test_types ) );
   Test::standard().add( col.ptr() );

   init_adt();
   init_dbg();
   init_io();
   init_msg();
   init_mt();
   init_net();
   init_util();
   init_old_tests();

   return Test::main( argc, argv );
}
