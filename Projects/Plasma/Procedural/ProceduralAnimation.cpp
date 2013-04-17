/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#include <Plasma/Procedural/ProceduralAnimation.h>
#include <Plasma/Resource/ResManager.h>

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
int animationVM( VMState* vm )
{
   ProceduralAnimation* userData = (ProceduralAnimation*)VM::userData( vm );
   SkeletalAnimation* anim       = userData->animation();

   // Only accept a table as argument.
   if( !VM::isTable( vm, 1 ) )
   {
      StdErr << "Missing arguments to animation()." << nl;
      return 0;
   }

   // Read parameters.
   float rate;
   if( VM::get( vm, 1, "rate", rate ) ) anim->rate( rate );

   Vec3f vel;
   if( VM::get( vm, 1, "velocity", vel ) ) anim->velocity( vel );

   Vec3f offset;
   if( VM::get( vm, 1, "offset", offset ) ) anim->offset( offset );

   bool cyclic;
   if( VM::get( vm, 1, "cyclic", cyclic ) ) anim->cyclic( cyclic );

   uint type;
   if( VM::get( vm, 1, "type", type ) ) anim->type( type );

   if( VM::get( vm, 1, "poses" ) )
   {
      anim->reservePoses( VM::getTableSize( vm, -1 ) );

      // Read all poses.
      for( int p = 1; VM::geti( vm, -1, p ); ++p )
      {
         Reff ref = Reff::identity();
         if( !VM::get( vm, -1, "p", ref.position() ) )
         {
            ref.position( Vec3f::zero() );
         }
         if( !VM::get( vm, -1, "q", ref.orientation() ) )
         {
            ref.orientation( Quatf::identity() );
         }

         RCP<SkeletalPose> pose = anim->addPose( ref );
         pose->reserveBones( VM::getTableSize( vm, -1 ) );
         // Read all bones positions in pose.
         for( int i = 1; VM::geti( vm, -1, i ); ++i )
         {
            pose->addBone( VM::toQuatf( vm, -1 ) );
            VM::pop( vm, 1 );
         }
         VM::pop( vm, 1 );
      }
      VM::pop( vm, 1 );
   }
   anim->prepare();  // Fix some corner cases.
   anim->makeRelative();
   return 0;
}

//------------------------------------------------------------------------------
//!
const VM::Reg funcs[] = {
   // Instantiation.
   { "animation", animationVM },
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
   CLASS ProceduralAnimation
==============================================================================*/

//------------------------------------------------------------------------------
//!
void
ProceduralAnimation::initialize()
{
   VMRegistry::add( initVM, VM_CAT_ANIM );
}

//------------------------------------------------------------------------------
//!
ProceduralAnimation::ProceduralAnimation(
   Resource<SkeletalAnimation>* res,
   const String&                animPath,
   const String&                skelId
):
   _res( res ),
   _animPath( animPath ),
   _skelId( skelId )
{
}

//------------------------------------------------------------------------------
//!
void
ProceduralAnimation::execute()
{
   _anim = new SkeletalAnimation();

   // Prepare to build the animation.
   VMState* vm = VM::open( VM_CAT_ANIM | VM_CAT_MATH, true );

   VM::userData( vm, this );
   VM::doFile( vm, _animPath, 0 );

   VM::close( vm );

   // Load skeleton.
   RCP< Resource<Skeleton> > skelRes = ResManager::getSkeleton( _skelId, this );
   _anim->skeleton( waitForData( skelRes.ptr() ) );

   _res->data( _anim.ptr() );

   //StdErr << "Animation: " << _animPath << " rate=" << _anim->rate() << " dur=" << _anim->duration() << nl;
}

NAMESPACE_END
