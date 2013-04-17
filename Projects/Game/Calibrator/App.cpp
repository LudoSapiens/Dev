/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#include <Game/Calibrator/App.h>

/*==============================================================================
  UNNAME NAMESPACE
==============================================================================*/

UNNAMESPACE_BEGIN

UNNAMESPACE_END

NAMESPACE_BEGIN

/*==============================================================================
  CLASS CalibratorApp
==============================================================================*/

//------------------------------------------------------------------------------
//!
RCP<CalibratorApp>
CalibratorApp::create( int argc, char* argv[] )
{
   return RCP<CalibratorApp>( new CalibratorApp(argc, argv) );
}

//------------------------------------------------------------------------------
//!
CalibratorApp::CalibratorApp( int argc, char* argv[] ):
   PlasmaApp( argc, argv )
{
   defaultScript( "app/game/calibrator" );

   // Initialize VM stuff here...
}

//------------------------------------------------------------------------------
//!
CalibratorApp::~CalibratorApp()
{
}

NAMESPACE_END
