#import <Cocoa/Cocoa.h>

#import "Cocoa/GfxOpenGLView.h"

int main(int argc, char *argv[])
{
   // Force GfxOpenGLView class to be linked by faking usage.
   [GfxOpenGLView _keepAtLinkTime];
   return NSApplicationMain(argc,  (const char **) argv);
}
