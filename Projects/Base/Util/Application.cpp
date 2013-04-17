/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#include <Base/Util/Application.h>

#include <Base/Util/Bits.h>

/*==============================================================================
  UNNAMED NAMESPACE
==============================================================================*/
UNNAMESPACE_BEGIN



UNNAMESPACE_END


NAMESPACE_BEGIN

//------------------------------------------------------------------------------
//!
Application*  sApp = NULL;

//------------------------------------------------------------------------------
//!
RCP<Application>
Application::create( int argc, char* argv[] )
{
   return RCP<Application>( new Application(argc, argv) );
}

//------------------------------------------------------------------------------
//!
Application::Application( int argc, char* argv[], const String& name ):
   _args( argc, argv ),
   _name( name )
{
   if( argc > 0 )
   {
      if( argv[0][0] == '/' )
      {
         _executable = Path( argv[0] );
      }
      else
      {
         _executable = Path::getCurrentDirectory();
         _executable /= Path( argv[0] );
      }
   }

   if( sApp == NULL )
   {
      sApp = this;
      // but do not assign if one is already assigned (it could be the one running).
   }
}

//------------------------------------------------------------------------------
//!
Application::~Application()
{
   sApp = NULL;
}

//------------------------------------------------------------------------------
//!
int
Application::run()
{
   sApp = this;
   return 0;
}

NAMESPACE_END
