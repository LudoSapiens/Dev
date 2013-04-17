/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#include <Plasma/World/Receptor.h>

#include <Plasma/World/Entity.h>
#include <Plasma/World/RigidEntity.h>
#include <Plasma/World/World.h>
#include <Plasma/World/WorldVM.h>

#include <Fusion/VM/VMRegistry.h>

#include <Base/ADT/StringMap.h>
#include <Base/Dbg/DebugStream.h>

/*==============================================================================
  UNNAMED NAMESPACE
==============================================================================*/
UNNAMESPACE_BEGIN

DBG_STREAM( os_recep, "Receptor" );

//------------------------------------------------------------------------------
//!
const char* _receptor_str_ = "receptor";

//-----------------------------------------------------------------------------
//!
int receptorVM( VMState* vm )
{
   BrainTaskContext* context = (BrainTaskContext*)VM::userData( vm );
   Entity* e = context->entity();

   ConstString typeStr = VM::toCString( vm, 1 );
   if( typeStr.isNull() )  return 0;

   int type = (typeStr.size() == 7) ? Receptor::TYPE_CONTACT : Receptor::TYPE_CONTACT_GROUP;

   RCP<Receptor> r;
#if 0
   bool reuseExisting = VM::toBoolean( vm, 3 );
   if( reuseExisting )
   {
      receptor = e->brain()->findAction( typeStr );
   }
#endif

   if( r.isNull() )
   {
      // Need to allocate a new one, and add it to the brain.
      switch( type )
      {
         case Receptor::TYPE_CONTACT:
            r = new ContactReceptor();
            break;
         case Receptor::TYPE_CONTACT_GROUP:
            r = new ContactGroupReceptor();
            break;
      }
      if( r.isNull() )
      {
         StdErr << "ERROR: receptor() - Failed to create an receptor of type '" << typeStr << "'." << nl;
         return 0;
      }

      initReceptor( vm, 2, r.ptr() );
      e->brain()->receptors().pushBack( r );

      if( e->world() )
      {
         e->world()->registerReceptor( e, r.ptr() );
      }
   }
   else
   {
      initReceptor( vm, 2, r.ptr() );
   }

   VM::pushProxy( vm, r.ptr() );
   return 1;
}

//------------------------------------------------------------------------------
//!
const VM::Reg _receptorFuncs[] = {
   { "receptor", receptorVM },
   { 0,0 }
};

//-----------------------------------------------------------------------------
//!
void initVM( VMState* vm, uint /*mask*/ )
{
   VM::registerFunctions( vm, "_G", _receptorFuncs );
}

UNNAMESPACE_END

USING_NAMESPACE

//------------------------------------------------------------------------------
//! A separate initialization routine in order to reduce coupling in Plasma.cpp.
void
Receptor::initialize()
{
   VMRegistry::add( initVM, VM_CAT_BRAIN );
   VMRegistry::add( _receptor_str_, NULL, receptor_get, receptor_set, VM_CAT_BRAIN );
}


/*==============================================================================
  CLASS Receptor
==============================================================================*/

//------------------------------------------------------------------------------
//!
Receptor::Receptor( int type ):
   _type( type )
{
}

//------------------------------------------------------------------------------
//!
Receptor::~Receptor()
{
}

//-----------------------------------------------------------------------------
//!
const char*
Receptor::meta() const
{
   return _receptor_str_;
}


/*==============================================================================
  CLASS ContactReceptor
==============================================================================*/

//------------------------------------------------------------------------------
//!
ContactReceptor::ContactReceptor():
   Receptor( TYPE_CONTACT )
{
   //fprintf( stderr, ">> ctor %p\n", this );
}

//------------------------------------------------------------------------------
//!
ContactReceptor::~ContactReceptor()
{
   //fprintf( stderr, ">> dtor %p\n", this );
}

//-----------------------------------------------------------------------------
//!
void
ContactReceptor::receivingEntity( RigidEntity* e )
{
   _receivingEntity = e;
}

//-----------------------------------------------------------------------------
//!
bool
ContactReceptor::interestedInA( const RigidEntity* a, const RigidEntity* b ) const
{
   return ( (a == receivingEntity()) && (a->senses() & b->exists()) );
}

//-----------------------------------------------------------------------------
//!
bool
ContactReceptor::interestedInB( const RigidEntity* a, const RigidEntity* b ) const
{
   return ( (b == receivingEntity()) && (b->senses() & a->exists()) );
}


/*==============================================================================
  CLASS ContactGroupReceptor
==============================================================================*/

//------------------------------------------------------------------------------
//!
ContactGroupReceptor::ContactGroupReceptor():
   Receptor( TYPE_CONTACT_GROUP ),
   _receivingGroups( ~0x0 ),
   _collidingGroups( ~0x0 )
{
}

//------------------------------------------------------------------------------
//!
ContactGroupReceptor::~ContactGroupReceptor()
{
}

//-----------------------------------------------------------------------------
//!
bool
ContactGroupReceptor::interestedInA( const RigidEntity*, const RigidEntity* ) const
{
   // FIXME
   return false;
   //return ( (a->collisionCategories() & receivingGroups()) && (b->collisionCategories() & collidingGroups()) );
}

//-----------------------------------------------------------------------------
//!
bool
ContactGroupReceptor::interestedInB( const RigidEntity*, const RigidEntity* ) const
{
   // FIXME
   return false;
   //return ( (b->collisionCategories() & receivingGroups()) && (a->collisionCategories() & collidingGroups()) );
}
