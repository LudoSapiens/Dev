/*=============================================================================
   Copyright (c) 2006, Ludo Sapiens Inc.
   All rights reserved.

   These coded instructions, statements, and computer programs contain
   unpublished, proprietary information and are protected by Federal copyright
   law. They may not be disclosed to third parties or copied or duplicated in
   any form, in whole or in part, without prior written consent.
=============================================================================*/
#include "c_library/myCLib.h"
#include "cpp_library/myCppLib.h"

int main( int argc, char** argv )
{
    myCLibFoo();
    myCppLibFoo();
    //#error Test
    return 0;
}
