/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#import <UIKit/UIKit.h>

#include <Base/IO/TextStream.h>

@interface FusionEAGLViewController : UIViewController
{
}

- (void)printInfo: (NAMESPACE::TextStream&)os;

@end

FusionEAGLViewController*  mainViewController();
