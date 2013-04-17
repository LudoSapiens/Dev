/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#ifndef FUSION_H
#define FUSION_H

#include <Fusion/StdDefs.h>
#include <Fusion/VM/VM.h>

#include <Base/Util/Application.h>

NAMESPACE_BEGIN


/*==============================================================================
  CLASS FusionApp
==============================================================================*/
class FusionApp:
   public Application
{
public:

   /*----- methods -----*/

   static FUSION_DLL_API RCP<FusionApp>  create( int argc, char* argv[] );

   FUSION_DLL_API ~FusionApp();

   FUSION_DLL_API int  run();

   inline const String&  defaultScript() const { return _defaultScript; }
   inline void  defaultScript( const String& script ) { _defaultScript = script; }

   FUSION_DLL_API void  parseArguments();
   FUSION_DLL_API void  execArguments( VMState* vm );
   FUSION_DLL_API void  readConfig();

   FUSION_DLL_API virtual void  printInfo( TextStream& os ) const;

protected:

   /*----- methods -----*/

   FUSION_DLL_API FusionApp( int argc, char* argv[], const String& name = "Fusion" );

   void  execCB();

   /*----- data members -----*/

   String    _defaultScript;  //!< The default script to load in case none is specified.
   bool      _showInfo;

private:
}; //class FusionApp


NAMESPACE_END

#endif //FUSION_H
