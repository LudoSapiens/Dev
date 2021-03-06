/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#include <Fusion/Core/Animator.h>

NAMESPACE_BEGIN

/*==============================================================================
  CLASS Animator
  ==============================================================================*/

//------------------------------------------------------------------------------
//!
Animator::Animator( double period ):
   _nextTime( 0.0 ),
   _period( period )
{}

//------------------------------------------------------------------------------
//!
Animator::~Animator()
{}

/*==============================================================================
  CLASS VMAnimator
  ==============================================================================*/

//------------------------------------------------------------------------------
//!
VMAnimator::VMAnimator()
{
}

//------------------------------------------------------------------------------
//!
VMAnimator::~VMAnimator()
{}

//------------------------------------------------------------------------------
//!
bool
VMAnimator::exec( double time, double delta )
{
   if( _animRef.isValid() )
   {
      VMState* vm = _animRef.vm();
      VM::push( vm, _animRef );
      VM::push( vm, time );
      VM::push( vm, delta );
      VM::ecall( vm, 2, 1 );
      bool end = VM::toBoolean( vm, -1 );
      VM::pop( vm, 1 );
      return end;
   }

   return true;
}


NAMESPACE_END
