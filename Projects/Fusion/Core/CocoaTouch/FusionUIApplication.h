/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#import <UIKit/UIKit.h>

#include <Fusion/Fusion.h>

USING_NAMESPACE

@interface FusionUIApplication : UIApplication
{

@private
   FusionApp*  _app;
}

- (FusionApp*)fusionApp;

- (void) printInfo: (TextStream&)os;

@end

FusionUIApplication*  mainApp();
