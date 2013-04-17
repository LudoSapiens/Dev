/*==============================================================================
   Copyright (c) 2009, Ludo Sapiens Inc.
   All rights reserved.

   These coded instructions, statements, and computer programs contain
   unpublished, proprietary information and are protected by Federal copyright
   law. They may not be disclosed to third parties or copied or duplicated in
   any form, in whole or in part, without prior written consent.
==============================================================================*/
#import <Fusion/Core/CocoaTouch/FusionUIApplication.h>

#include <Base/Dbg/DebugStream.h>

USING_NAMESPACE

/*==============================================================================
  UNNAMED NAMESPACE
==============================================================================*/
UNNAMESPACE_BEGIN

DBG_STREAM( os_ct, "CocoaTouch" );

UNNAMESPACE_END


FusionUIApplication* _mainApp = NULL;

// A class extension to declare private methods
@interface FusionUIApplication ( ext )
   // None yet...
@end


@implementation FusionUIApplication

//------------------------------------------------------------------------------
//!
- (FusionApp*)fusionApp
{
   return _app;
}

//------------------------------------------------------------------------------
//!
- (void) printInfo: (TextStream&)os
{
   os << "FusionUIApplication" << nl;
}


- (id)init
{
   DBG_BLOCK( os_ct, "FusionUIApplication init" );
   self = [super init];
   _app = (FusionApp*)sApp;
   _mainApp = self;
   return self;
}


@end

FusionUIApplication* mainApp()
{
   return _mainApp;
}
