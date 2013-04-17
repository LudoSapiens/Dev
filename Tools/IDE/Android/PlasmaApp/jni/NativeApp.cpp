/*=============================================================================
   Copyright (c) 2011, Ludo Sapiens Inc.
   All rights reserved.

   These coded instructions, statements, and computer programs contain
   unpublished, proprietary information and are protected by Federal copyright
   law. They may not be disclosed to third parties or copied or duplicated in
   any form, in whole or in part, without prior written consent.
=============================================================================*/
#include <Plasma/Plasma.h>

/*==============================================================================
  UNNAMED NAMESPACE
==============================================================================*/
UNNAMESPACE_BEGIN

RCP<PlasmaApp>  _sApp = NULL;

UNNAMESPACE_END

extern "C" {

extern void  nativeAppDummy();

//=============================================================================
// Application callbacks
//=============================================================================

//-----------------------------------------------------------------------------
//!
void  nativeAppCreate()
{
   nativeAppDummy();
   char* args[] = {
      // "arg1",
   };
   _sApp = PlasmaApp::create( sizeof(args)/sizeof(args[0]), args );
}

//-----------------------------------------------------------------------------
//!
void  nativeAppStart()
{
   _sApp->run();
}

} // extern "C"
