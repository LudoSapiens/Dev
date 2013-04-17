/*==============================================================================
   Copyright (c) 2009, Ludo Sapiens Inc.
   All rights reserved.

   These coded instructions, statements, and computer programs contain
   unpublished, proprietary information and are protected by Federal copyright
   law. They may not be disclosed to third parties or copied or duplicated in
   any form, in whole or in part, without prior written consent.
==============================================================================*/
#import <Fusion/Core/CocoaTouch/FusionEAGLView.h>

#include <Fusion/Core/CocoaTouch/FusionAppDelegate.h>

#include <Base/Dbg/DebugStream.h>

#import <QuartzCore/QuartzCore.h>

USING_NAMESPACE


/*==============================================================================
  UNNAMED NAMESPACE
==============================================================================*/
UNNAMESPACE_BEGIN

DBG_STREAM( os_ct, "CocoaTouch" );

FusionEAGLView* _mainView = NULL;

UNNAMESPACE_END


// A class extension to declare private methods
@interface FusionEAGLView ()

@end


@implementation FusionEAGLView

// This method is mandatory.
+ (Class)layerClass
{
   return [CAEAGLLayer class];
}

- (id)init
{
   DBG_BLOCK( os_ct, "FusionEAGLView -init" );
   self = [super init];
   _mainView = self;
   return self;
}

//The GL view is stored in the nib file. When it's unarchived it is sent an -initWithCoder: message.
- (id)initWithCoder:(NSCoder*)coder
{
   DBG_BLOCK( os_ct, "FusionEAGLView -initWithCoder:" );
   self = [super initWithCoder:coder];
   _mainView = self;
   return self;
}

- (void)layoutSubviews
{
   DBG_BLOCK( os_ct, "FusionEAGLView -layoutSubviews" );
   CGRect rect = self.bounds;
   [mainDelegate() resize:rect.size];
}

- (void) printInfo: (TextStream&)os
{
   CGRect bounds = self.bounds;
   os << "FusionEAGLView:"
      << " "
      << bounds.size.width << "x" << bounds.size.height
      << " @ "
      << "(" << bounds.origin.x << "," << bounds.origin.y << ")"
      << nl;
}


//=============================================================================
//= Touch Events ==============================================================
//=============================================================================

- (void)touchesBegan:(NSSet *)touches withEvent:(UIEvent *)event
{
   //DBG_BLOCK( os_ct, "FusionEAGLView touchesBegan:withEvent:" );
   [mainDelegate() touchesBegan:touches withEvent:event];
}

- (void)touchesCancelled:(NSSet *)touches withEvent:(UIEvent *)event
{
   //DBG_BLOCK( os_ct, "FusionEAGLView touchesCancelled:withEvent:" );
   [mainDelegate() touchesCancelled:touches withEvent:event];
}

- (void)touchesEnded:(NSSet *)touches withEvent:(UIEvent *)event
{
   //DBG_BLOCK( os_ct, "FusionEAGLView touchesEnded:withEvent:" );
   [mainDelegate() touchesEnded:touches withEvent:event];
}

- (void)touchesMoved:(NSSet *)touches withEvent:(UIEvent *)event
{
   //DBG_BLOCK( os_ct, "FusionEAGLView touchesMoved:withEvent:" );
   [mainDelegate() touchesMoved:touches withEvent:event];
}


//=============================================================================
//= Motion Events =============================================================
//=============================================================================

- (void)motionBegan:(UIEventSubtype)motion withEvent:(UIEvent *)event
{
   //DBG_BLOCK( os_ct, "FusionEAGLView motionBegan:withEvent:" );
   [mainDelegate() motionBegan:motion withEvent:event];
}

- (void)motionCancelled:(UIEventSubtype)motion withEvent:(UIEvent *)event
{
   //DBG_BLOCK( os_ct, "FusionEAGLView motionCancelled:withEvent:" );
   [mainDelegate() motionCancelled:motion withEvent:event];
}

- (void)motionEnded:(UIEventSubtype)motion withEvent:(UIEvent *)event
{
   //DBG_BLOCK( os_ct, "FusionEAGLView motionEnded:withEvent:" );
   [mainDelegate() motionEnded:motion withEvent:event];
}

@end

FusionEAGLView*  mainView()
{
   return _mainView;
}
