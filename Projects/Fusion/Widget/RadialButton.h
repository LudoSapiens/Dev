/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#ifndef FUSION_RADIALBUTTON_H
#define FUSION_RADIALBUTTON_H

#include <Fusion/StdDefs.h>
#include <Fusion/Widget/Widget.h>

#include <Base/ADT/String.h>

NAMESPACE_BEGIN

/*==============================================================================
  CLASS RadialButton
==============================================================================*/

//!

class RadialButton
   : public Widget
{

public:

   /*----- static methods -----*/

   static void initialize();

   /*----- methods -----*/
   
   FUSION_DLL_API RadialButton();
   
   //! Event handlers
   //FUSION_DLL_API virtual void onClick( const Event& );
   
   FUSION_DLL_API virtual void onPointerPress( const Event& );
   FUSION_DLL_API virtual void onPointerRelease( const Event& );
   FUSION_DLL_API virtual void onPointerEnter( const Event& );
   FUSION_DLL_API virtual void onPointerLeave( const Event& );

   // VM.
   const char* meta() const;
   FUSION_DLL_API void init( VMState* vm );

protected:
  
   /*----- methods -----*/   

   virtual ~RadialButton();


private:
   
   /*----- data members -----*/

};

NAMESPACE_END

#endif
