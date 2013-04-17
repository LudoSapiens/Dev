/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#ifndef CALIBRATOR_APP_H
#define CALIBRATOR_APP_H

#include <Game/Calibrator/StdDefs.h>

#include <Plasma/Plasma.h>

NAMESPACE_BEGIN

/*==============================================================================
  CLASS CalibratorApp
==============================================================================*/
class CalibratorApp:
   public PlasmaApp
{
public:

   /*----- methods -----*/

   static RCP<CalibratorApp>  create( int argc, char* argv[] );

   ~CalibratorApp();

protected:

   /*----- methods -----*/

   CalibratorApp( int argc, char* argv[] );

private:
}; //class CalibratorApp

NAMESPACE_END

#endif //CALIBRATOR_APP_H
