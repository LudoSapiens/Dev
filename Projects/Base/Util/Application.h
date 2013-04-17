/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#ifndef BASE_APPLICATION_H
#define BASE_APPLICATION_H

#include <Base/StdDefs.h>

#include <Base/IO/Path.h>
#include <Base/Util/Arguments.h>
#include <Base/Util/RCObject.h>
#include <Base/Util/RCP.h>

NAMESPACE_BEGIN

/*==============================================================================
  CLASS Application
==============================================================================*/
class Application:
   public RCObject
{
public:

   /*----- methods -----*/

   static BASE_DLL_API RCP<Application>  create( int argc, char* argv[] );

   BASE_DLL_API virtual ~Application();

   BASE_DLL_API virtual int  run();

   inline       Arguments&  args()       { return _args; }
   inline const Arguments&  args() const { return _args; }

   inline const Path&  executable() const { return _executable; }

   inline const String&  name() const { return _name; }
   inline void  name( const String& v ) { _name = v; }

   inline const String&  version() const { return _version; }
   inline void  version( const String& v ) { _version = v; }

protected:

   /*----- members -----*/
   Arguments  _args;        //!< Arguments specified at creation time.
   Path       _executable;  //!< The path to the running executable.
   String     _name;        //!< A string representing the name of the application.
   String     _version;     //!< A string representing the version.

   /*----- methods -----*/

   BASE_DLL_API Application( int argc, char* argv[], const String& name = "" );

private:
}; //class Application

//------------------------------------------------------------------------------
//!
BASE_DLL_API extern Application*  sApp;

NAMESPACE_END

#endif //BASE_APPLICATION_H
