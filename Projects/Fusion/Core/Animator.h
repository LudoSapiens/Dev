/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#ifndef FUSION_ANIMATOR_H
#define FUSION_ANIMATOR_H

#include <Fusion/StdDefs.h>
#include <Fusion/VM/VM.h>

#include <Base/Util/RCObject.h>

NAMESPACE_BEGIN

/*==============================================================================
  CLASS Animator
==============================================================================*/

//!

class Animator
   : public RCObject
{

public:

   /*----- methods -----*/

   FUSION_DLL_API Animator( double period = 0.0 );

   FUSION_DLL_API virtual bool exec( double time, double delta ) = 0;

   inline double  nextTime() const     { return _nextTime; }
   inline void    nextTime( double t ) { _nextTime = t;    }

   inline double  period() const     { return _period; }
   inline void    period( double t ) { _period = t;    }

protected:

   /*----- methods -----*/

   double  _nextTime; //!< The time at which the exec should be called next.
   double  _period;   //!< The number of seconds before re-execution.

   /*----- methods -----*/

   FUSION_DLL_API virtual ~Animator();

};

/*==============================================================================
  CLASS VMAnimator
==============================================================================*/

//! Animator for VM scripting.

class VMAnimator
   : public Animator
{

public:

   /*----- methods -----*/

   VMAnimator();

   virtual bool exec( double time, double delta );

   VMRef& ref() { return _animRef; }

protected:

   /*----- methods -----*/

   virtual ~VMAnimator();

private:

   /*----- data members -----*/

   VMRef _animRef;

};

NAMESPACE_END

#endif

