/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#include <Fusion/Fusion.h>
#include <Fusion/Core/Core.h>
#include <Fusion/VM/VMRegistry.h>
#include <Fusion/Resource/ResManager.h>
#include <Fusion/Resource/Font.h>
#include <Fusion/Resource/Bitmap.h>

#include <Base/Dbg/DebugStream.h>
#include <Base/Util/Compiler.h>
#include <Base/Util/CPU.h>

USING_NAMESPACE

/*==============================================================================
  UNNAMED NAMESPACE
==============================================================================*/
UNNAMESPACE_BEGIN

DBG_STREAM( os_fusion, "Fusion" );

UNNAMESPACE_END

//------------------------------------------------------------------------------
//!
RCP<FusionApp>
FusionApp::create( int argc, char* argv[] )
{
   return RCP<FusionApp>( new FusionApp(argc, argv) );
}

//------------------------------------------------------------------------------
//!
FusionApp::FusionApp( int argc, char* argv[], const String& name ):
   Application( argc, argv, name ), _showInfo( false )
{
   DBG_BLOCK( os_fusion, "FusionApp::FusionApp()" );

   Core::initialize();

   defaultScript( "main" );

   parseArguments();
}

//------------------------------------------------------------------------------
//!
FusionApp::~FusionApp()
{
   DBG_BLOCK( os_fusion, "FusionApp::~FusionApp()" );
   Core::finalize();
}

//------------------------------------------------------------------------------
//!
void
FusionApp::execCB()
{
   DBG_BLOCK( os_fusion, "FusionApp::execCB()" );

   // TODO: Set tokens here (<Root>/<UserRoot>/<CacheRoot>).

   if( _showInfo )  printInfo( StdOut );

   // Execute unmasked arguments.
   execArguments( Core::vm() );
}

//------------------------------------------------------------------------------
//!
int
FusionApp::run()
{
   DBG_BLOCK( os_fusion, "FusionApp::run()" );

   Core::callWhenReadyToExec( Delegate0<>(this, &FusionApp::execCB) );

   // Start main loop.
   Core::exec();

   return 0;
}

//------------------------------------------------------------------------------
//!
void
FusionApp::parseArguments()
{
   DBG_BLOCK( os_fusion, "FusionApp::parseArguments()" );
   Arguments& args = sApp->args();
   for( Arguments::Iterator it = args.iter(); it.isValid(); ++it )
   {
      const char* arg = *it;
      if( arg[0] == '-' )
      {
         switch( arg[1] )
         {
            case 'f':
               Core::fullScreen( true );
               args.mask( it );
               break;
            case 'w':
               Core::fullScreen( false );
               args.mask( it );
               break;
            case 's':
               {
                  Arguments::Iterator arg1 = it + 1;
                  Arguments::Iterator arg2 = arg1 + 1;
                  if( arg1.isValid() && arg2.isValid() )
                  {
                     int x = atoi( *arg1 );
                     int y = atoi( *arg2 );
                     Core::size( Vec2i( x, y ) );
                     args.mask( it, 2 );
                  }
               }
               break;
            case 'i':
               _showInfo = true;
               args.mask( it );
               break;
            case 'g':
               args.mask( it );
               StdErr << "Halting to allow debugger to connect" << nl;
               getchar();
               break;
            default:
               break;
         }
      }
      else
      if( ResManager::handleRootCandidate( arg ) ) args.mask( it );
   }
}

//------------------------------------------------------------------------------
//! This routine executes all remaining unmasked arguments as a Lua file.
void
FusionApp::execArguments( VMState* vm )
{
   DBG_BLOCK( os_fusion, "FusionApp::execArguments()" );

   bool noFile = true;

   for( Arguments::Iterator it = args().iter(); it.isValid(); ++it )
   {
      DBG_MSG( os_fusion, "Arg: " << *it );
      const char* tmp = *it;
      if( tmp[0] != '-' )
      {
         ResManager::executeScript( vm, String(tmp) );
         noFile = false;
      }
      else
      {
         StdErr << "Unrecognized option: " << tmp << nl;
      }
   }

   if( noFile )
   {
      DBG_MSG( os_fusion, "No input file specified..." );
      DBG_MSG( os_fusion, "Using default script: " << defaultScript() );
      if( !defaultScript().empty() )
      {
         ResManager::executeScript( vm, defaultScript(), false );
      }
      else
      {
         StdErr << "Could not determine default file - Aborting.\n";
         exit( 1 );
      }
   }
}

//------------------------------------------------------------------------------
//!
void
FusionApp::printInfo( TextStream& os ) const
{
   os << "-------------------------------" << nl;
   os << "INFORMATION ABOUT THE PLATFORM:" << nl;
   os << "-------------------------------" << nl;

   os << "CPU:";
   os << " arch=";
   switch( CPU_ARCH )
   {
      case CPU_ARCH_UNKNOWN: os << "(\?\?\?)"; break; // Avoid trigraph warning with ??
      case CPU_ARCH_X86    : os << "x86"     ; break;
      case CPU_ARCH_PPC    : os << "ppc"     ; break;
      default              : os << "(other)" ; break;
   }
   os << " size=" << CPU_SIZE << "b";
   os << " end=";
   switch( CPU_ENDIANNESS )
   {
      case CPU_ENDIAN_UNKNOWN: os << "(\?\?\?)"; break; // Avoid trigraph warning with ??
      case CPU_ENDIAN_LITTLE : os << "little"  ; break;
      case CPU_ENDIAN_BIG    : os << "big"     ; break;
      default                : os << "(other)" ; break;
   }
   os << nl;

   os << "Compiler: ";
#if COMPILER == COMPILER_UNKNOWN
   os << "unknown";
#elif COMPILER == COMPILER_GCC
   os << "GCC(" << __VERSION__ << ")";
//   os << "GCC(" << __GNUC__ << '.' << __GNUC_MINOR__ << '.' << __GNUC_PATCHLEVEL__;
//#if defined(__llvm__)
//   os << " llvm";
//#endif //__llvm__
//   os << ')';
#elif COMPILER == COMPILER_MSVC
   os << "MSVC(" << _MSC_VER << ": ";
   switch( _MSC_VER )
   {
      case 1100: os << "5.0"; break;
      case 1200: os << "6.0"; break;
      case 1300: os << "7.0"; break;
      case 1310: os << "7.1 aka .NET 2003"; break;
      case 1400: os << "8.0 aka Express 2005"; break;
      default: break;
   }
   os << ')';
#else
   os << "(other)";
#endif
   os << nl;

   os << "-------------------------------" << nl;
   os << nl;
   os << "----------------------" << nl;
   os << "INFORMATION ABOUT GFX:" << nl;
   os << "----------------------" << nl;
   if( Core::gfx() )
   {
      Core::gfx()->printInfo( os );
   }
   else
   {
      os << "(null)" << nl;
   }
   os << "----------------------" << nl;
   os << nl;
   os << "-------------------------" << nl;
   os << "INFORMATION ABOUT FUSION:" << nl;
   os << "-------------------------" << nl;
   Core::printInfo( os );       // Cocoa/CocoaTouch/Win32/X11
   Font::printInfo( os );       // Freetype vs. STB_Truetype
   Bitmap::printInfo( os );     // LibPNG vs. STB_Image
   ResManager::printInfo( os ); // SQLite
   VM::printInfo( os );         // Lua+ext

   //Base::printInfo( os );     // Socket+Platform+etc
   //CGM::printInfo( os );      // Empty
   //Gfx::printInfo( os );      // OpenGL/DirectX
   os << "-------------------------" << nl;
}
