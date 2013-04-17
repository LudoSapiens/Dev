/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#include <Plasma/Procedural/ProceduralSkeleton.h>

#include <Fusion/VM/VMRegistry.h>

/*==============================================================================
  UNNAME NAMESPACE
==============================================================================*/

UNNAMESPACE_BEGIN

/*==============================================================================
   Procedural API/functions.
==============================================================================*/

//------------------------------------------------------------------------------
//! 
int skeletonVM( VMState* vm )
{
   ProceduralSkeleton* userData = (ProceduralSkeleton*)VM::userData( vm );
   Skeleton* skel               = userData->skeleton();

   // Only accept a table as argument.
   if( !VM::isTable( vm, 1 ) )
   {
      StdErr << "Missing arguments to skeleton()." << nl;
      return 0;
   }

   bool endpointsSpecified = false;

   // Reading bones.
   skel->reserveBones( VM::getTableSize( vm, 1 ) );
   for( int i = 1; VM::geti( vm, -1, i ); ++i )
   {
      const char* name;
      VM::get( vm, -1, "name", name );

      int parent = -1;
      VM::get( vm, -1, "parent", parent );
      
      int dof = 0;
      VM::get( vm, -1, "dof", dof );

      Reff ref( Reff::identity() );
      VM::get( vm, -1, "p", ref.position() );
      VM::get( vm, -1, "q", ref.orientation() );

      Vec3f endpoint(0, 0, 0);
      if( VM::get( vm, -1, "e", endpoint ) )
      {
         endpointsSpecified = true;
      }

      skel->addBone( name, ref, parent, endpoint );
      skel->bones().back().dof( dof );
      VM::pop( vm, 1 );
   }
   
   // Read limbs description.
   if( VM::get( vm, 1, "limbs" ) )
   {
      skel->reserveLimbs( VM::getTableSize( vm, -1 ) );
      for( int i = 1; VM::geti( vm, -1, i ); ++i )
      {
         skel->addLimb( VM::toInt( vm, -1 ) );
         VM::pop( vm, 1 );
      }
      VM::pop( vm, 1 );
   }

   skel->computeDerivedData();
   if( !endpointsSpecified ) skel->computeEndpoints();

   return 0;
}

//------------------------------------------------------------------------------
//! 
const VM::Reg funcs[] = {
   // Instantiation.
   { "skeleton", skeletonVM },
   { 0,0 }
};

//------------------------------------------------------------------------------
//! 
void initVM( VMState* vm, uint /*mask*/ )
{
   VM::registerFunctions( vm, "_G", funcs );
}

UNNAMESPACE_END

NAMESPACE_BEGIN

/*==============================================================================
   CLASS ProceduralSkeleton
==============================================================================*/

//------------------------------------------------------------------------------
//! 
void
ProceduralSkeleton::initialize()
{
   VMRegistry::add( initVM, VM_CAT_SKEL );
}

//------------------------------------------------------------------------------
//! 
ProceduralSkeleton::ProceduralSkeleton( Resource<Skeleton>* res, const String& path ): 
   _res( res ),
   _path( path )
{
}

//------------------------------------------------------------------------------
//! 
void
ProceduralSkeleton::execute()
{
   _skel = new Skeleton();

   // Prepare to build the skeleton.
   VMState* vm = VM::open( VM_CAT_SKEL | VM_CAT_MATH, true );

   VM::userData( vm, this );
   VM::doFile( vm, _path, 0 );

   VM::close( vm );

   _res->data( _skel.ptr() );
}

NAMESPACE_END
