/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#import <UIKit/UIKit.h>

#include <Base/IO/TextStream.h>

@interface FusionEAGLView : UIView
{
}

- (void)printInfo: (NAMESPACE::TextStream&)os;

@end

FusionEAGLView*  mainView();
