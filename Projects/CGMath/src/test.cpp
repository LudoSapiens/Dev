/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#include <Base/Dbg/UnitTest.h>

USING_NAMESPACE

void  init_boxes();
void  init_interpolation();
void  init_intersection();
void  init_misc();
void  init_random();
void  init_surfaces();
void  init_vmq();

//------------------------------------------------------------------------------
//!
int
main( int argc, char** argv )
{
   init_boxes();
   init_interpolation();
   init_intersection();
   init_misc();
   init_random();
   init_surfaces();
   init_vmq();
   return Test::main( argc, argv );
}
