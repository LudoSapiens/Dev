/*==============================================================================
   Copyright (c) 2009, Ludo Sapiens Inc.
   All rights reserved.

   These coded instructions, statements, and computer programs contain
   unpublished, proprietary information and are protected by Federal copyright
   law. They may not be disclosed to third parties or copied or duplicated in
   any form, in whole or in part, without prior written consent.
==============================================================================*/
#import <Fusion/Core/CocoaTouch/FusionEAGLViewController.h>

#include <Fusion/Core/CocoaTouch/FusionAppDelegate.h>

#include <Base/Dbg/DebugStream.h>

#import <QuartzCore/QuartzCore.h>

USING_NAMESPACE


/*==============================================================================
  UNNAMED NAMESPACE
==============================================================================*/
UNNAMESPACE_BEGIN

DBG_STREAM( os_ct, "CocoaTouch" );

FusionEAGLViewController* _mainViewController = NULL;

//------------------------------------------------------------------------------
//!
inline Core::Orientation convert( UIInterfaceOrientation ori )
{
   switch( ori )
   {
      case UIInterfaceOrientationPortrait:
         return Core::ORIENTATION_DEFAULT;
      case UIInterfaceOrientationPortraitUpsideDown:
         return Core::ORIENTATION_UPSIDE_DOWN;
      case UIInterfaceOrientationLandscapeLeft:
         return Core::ORIENTATION_CCW_90;
      case UIInterfaceOrientationLandscapeRight:
         return Core::ORIENTATION_CW_90;
      default:
         CHECK( false );
         return Core::ORIENTATION_DEFAULT;
   }
}

UNNAMESPACE_END


// A class extension to declare private methods
@interface FusionEAGLViewController ()

@end


@implementation FusionEAGLViewController

- (void) printInfo: (TextStream&)os
{
   os << "FusionEAGLViewController:"
      << " "
      << (int)self.interfaceOrientation
      << nl;
}

- (void)viewDidLoad {
    [super viewDidLoad];
    CHECK( _mainViewController == NULL );
    _mainViewController = self;
}

//=============================================================================
//= Orientation Events ========================================================
//=============================================================================

- (BOOL)shouldAutorotateToInterfaceOrientation:(UIInterfaceOrientation)interfaceOrientation
{
   //StdErr << Core::orientationLocked() << " " << Core::orientation() << " vs. " << convert(interfaceOrientation) << " Should?";
   if( Core::orientationLocked() )
   {
      if( convert(interfaceOrientation) == Core::orientation() )
      {
         // Guarantee we are in sync.
         return YES;
      }
      else
      {
         return NO;
      }
   }
   else
   {
      return YES;
   }
}

- (void)willAnimateRotationToInterfaceOrientation:(UIInterfaceOrientation)newInterfaceOrientation duration:(NSTimeInterval)duration
{
   DBG_BLOCK( os_ct, "willAnimateRotationToInterfaceOrientation:" << (int)newInterfaceOrientation << " duration:" << (float)duration );
   Core::orientation( convert(newInterfaceOrientation), true ); // Forcing to guarantee sync in states.
   unused( duration );
}

/*
- (void)didRotateFromInterfaceOrientation:(UIInterfaceOrientation)oldInterfaceOrientation
{
   DBG_BLOCK( os_ct, "didRotateFromInterfaceOrientation:" << (int)oldInterfaceOrientation );
}
*/


@end

FusionEAGLViewController*  mainViewController()
{
   return _mainViewController;
}
