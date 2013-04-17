/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#include <Plasma/Manipulator/MultiCharacterController.h>

#include <Plasma/Action/TKAction.h>
#include <Plasma/Geometry/MeshGeometry.h>
#include <Plasma/Intersector.h>
#include <Plasma/Stimulus/Orders.h>
#include <Plasma/World/SkeletalEntity.h>
#include <Plasma/Renderable/Renderable.h>
#include <Plasma/Resource/ResManager.h>

#include <Fusion/VM/VMObjectPool.h>
#include <Fusion/Core/Key.h>

#include <Base/ADT/StringMap.h>

/*==============================================================================
  UNNAMED NAMESPACE
==============================================================================*/
UNNAMESPACE_BEGIN

enum
{
   ATTRIB_ENTITYID,
   ATTRIB_ARROW,
   ATTRIB_ONCHAR_SELECT,
   ATTRIB_SET_MODE,
};

StringMap _attributes(
   "entityID",     ATTRIB_ENTITYID,
   "arrow",        ATTRIB_ARROW,
   "onCharSelect", ATTRIB_ONCHAR_SELECT,
   "setMode",      ATTRIB_SET_MODE,
   ""
);

const float _limitRadius = 5.0f;
const double _limitDelay = 0.5f;

//------------------------------------------------------------------------------
//!
inline void execute( const VMRef& ref, const ConstString& id )
{
   if( ref.isValid() )
   {
      VMState* vm = ref.vm();
      VM::push( vm, ref );
      VM::push( vm, id );
      VM::ecall( vm, 1, 0 );
   }
}

//------------------------------------------------------------------------------
//!
inline bool isClick( const Pointer& ptr )
{
   return ptr.withinPress( _limitRadius ) && ptr.withinDelay( _limitDelay );
}

//------------------------------------------------------------------------------
//!
inline bool hasMoved( const Pointer& ptr )
{
   return !ptr.withinPress( _limitRadius );
}

//------------------------------------------------------------------------------
//!
inline Vec3f computeTKNormal( Camera& c )
{
   // X-Z vs camera X-Y.
   Vec3f z  = c.orientation().getAxisZ();
   if( CGM::abs( z.y ) > 0.5f ) // 30 degrees of elevation.
      return Vec3f( 0.0f, 1.0f, 0.0f );
   else
   {
      z.y = 0.0f;
      z.normalize();
      return z;
   }
}

//------------------------------------------------------------------------------
//!
uint  getOtherPointerID( uint pid )
{
   for( uint i = Core::pointerBegin(); i != Core::pointerEnd(); i = Core::pointerNext(i) )
   {
      if( i == pid )  continue; // Skip current pointer.
      return i;
   }

   return pid;
}

//------------------------------------------------------------------------------
//!
const Pointer&  getOtherPointer( uint pid )
{
   return Core::pointer( getOtherPointerID(pid) );
}

//------------------------------------------------------------------------------
//!
float computePinchRatio( const Vec2f& p0_cur, const Vec2f& p0_old, const Vec2f& p1_cur, const float sizeToDouble )
{
   // Determine translation based on centroid shift.
   // This is equivalent to doing:
   //   trans = (p0_old+p1_cur)/2 - (p0_cur+p1_cur)/2
   //trans  = (p0_old - p0_cur);
   //trans *= 0.5f;

   // Determine zoom factor based on distance.
   float oldDist = length(p1_cur - p0_old);
   float newDist = length(p1_cur - p0_cur);

   return (sizeToDouble + oldDist - newDist)/sizeToDouble;
}

//------------------------------------------------------------------------------
//!
SkeletalEntity* findCharacterGrabbing( RigidEntity* e )
{
   TKAttractor* a = TKAttractor::getCurrentAttractor( e );
   if( a != nullptr )
   {
      if( a->numSources() > 0 )
      {
         TKSource* s = a->source(0);
         if( s->entity()->type() == Entity::SKELETAL )  return (SkeletalEntity*)s->entity();
      }
   }
   return nullptr;
}

int  strToCMode( const char* str )
{
   CHECK( str );
   CHECK( strlen(str) >= 4 );
   switch( tolower(str[3]) )
   {
      // Using fourth letter                               |
      //                                                   V
      case 'b':  return MultiCharacterController::CMODE_GRAB;
      case 'c':  return MultiCharacterController::CMODE_SHOCKWAVE;
      case 'e':  return MultiCharacterController::CMODE_SHIELD;
      case 'w':  return MultiCharacterController::CMODE_FLOW;
      default :  return MultiCharacterController::CMODE_GRAB;
   }
}

//------------------------------------------------------------------------------
//!
int  setModeVM( VMState* vm )
{
   MultiCharacterController* ctrl = (MultiCharacterController*)VM::thisPtr( vm );
   ConstString charID = VM::toConstString( vm, 1 );
   const char* str = VM::toCString( vm, 2 );
   int mode = strToCMode( str );
   ctrl->setMode( charID, mode );
   return 0;
}



//------------------------------------------------------------------------------
//!
struct PickedInfo
{
   PickedInfo(): _pos(0.0f), _weight(0.0f) {}

   Vec3f _pos;
   float _weight;
};

//------------------------------------------------------------------------------
//!
RCP<Gfx::DepthState> _failDepth   = new Gfx::DepthState();
RCP<Gfx::DepthState> _defDepth    = new Gfx::DepthState();
RCP<Gfx::AlphaState> _defBlending = new Gfx::AlphaState();

UNNAMESPACE_END

NAMESPACE_BEGIN

/*==============================================================================
   CLASS ArrowRenderable
==============================================================================*/

class ArrowRenderable:
   public Renderable
{
public:

   /*----- methods -----*/

   ArrowRenderable(): _target(nullptr)
   {
      _failDepth->depthTestFunc( Gfx::COMPARE_FUNC_GREATER );
      _failDepth->depthWriting( false );

      // Material.
      Vec4f color(0.0f, 0.0f, 1.0f, 1.0f);
      RCP<Gfx::ConstantBuffer> constant = Core::gfx()->createConstants( 4*sizeof(float) );
      constant->addConstant( "color", Gfx::CONST_FLOAT4, 0, color.ptr() );
      _constFront = Gfx::ConstantList::create( constant );

      color = Vec4f(0.0f, 0.0f, 0.15f, 0.15f);
      constant = Core::gfx()->createConstants( 4*sizeof(float) );
      constant->addConstant( "color", Gfx::CONST_FLOAT4, 0, color.ptr() );
      _constBack = Gfx::ConstantList::create( constant );

      _prog = data( ResManager::getProgram( "shader/program/color" ) );
   }

   void geometry( Geometry* geom ) { _geom = geom; }

   virtual void render( Gfx::Pass& pass, const Viewport& vp ) const
   {
      if( !_target || _geom.isNull() ) return;

      // Compute matrix.
      Vec3f x, y ,z;
      vp.camera()->orientation().getAxes( x, y, z );

      Vec3f nx, ny, nz;
      if( CGM::abs( z.y ) > 0.5f ) // 30 degrees of elevation.
      {
         nx = x;
         ny = Vec3f(0.0f, 1.0f, 0.0f );
         nz = Vec3f( -nx.z, 0.0f, nx.x ); // cross( nx, ny )
      }
      else
      {
         ny = normalize( Vec3f(z.x, 0.0f, z.z ) );
         nz = Vec3f( 0.0f, -1.0f, 0.0f );
         nx = Vec3f( ny.z, 0.0f, -ny.x ); // cross( ny, nz )
      }
      float disp = _target->geometry()->boundingBox().maxSize()*0.5f;
      _dist      = length( vp.camera()->position() - _target->position() );
      _pos       = _target->position() + y*(disp+_dist*0.02f);
      Reff ref;
      ref.orientation( Quatf:: axes( nx, ny, nz ) );
      ref.position( _pos );
      ref.scale( 0.04f*_dist );
      _mat = ref.toMatrix();

      // Behind pass.
      pass.setDepthState( _failDepth );
      pass.setAlphaState( _defBlending );
      pass.setConstants( _constBack );
      pass.setWorldMatrix( _mat.ptr() );
      pass.setProgram( _prog );
      pass.setSamplers(0);
      _geom->render( pass );

      // Front pass.
      pass.setDepthState( _defDepth );
      pass.setConstants( _constFront );
      _geom->render( pass );
   }

   void grab()
   {
      (*_constFront)[0]->setConstant( "color", Vec4f(1.0f, 0.0f, 0.0f, 1.0f).ptr() );
      (*_constBack)[0]->setConstant( "color", Vec4f(0.15f, 0.0f, 0.0f, 0.15f).ptr() );
   }

   void ungrab()
   {
      (*_constFront)[0]->setConstant( "color", Vec4f(0.0f, 0.0f, 1.0f, 1.0f).ptr() );
      (*_constBack)[0]->setConstant( "color", Vec4f(0.0f, 0.0f, 0.15f, 0.15f).ptr() );
   }

   void target( RigidEntity* t ) { _target = t; }
   RigidEntity* target() const   { return _target; }
   Vec3f position() const        { return _pos; }
   float distance() const        { return _dist; }

protected:

   /*----- data members -----*/

   RigidEntity*           _target;
   mutable Vec3f          _pos;
   mutable Mat4f          _mat;
   mutable float          _dist;
   RCP<Gfx::Program>      _prog;
   RCP<Gfx::ConstantList> _constFront;
   RCP<Gfx::ConstantList> _constBack;
   RCP<Geometry>          _geom;
};


/*==============================================================================
   CLASS DotRenderable
==============================================================================*/

class DotRenderable:
   public Renderable
{
public:

   /*----- methods -----*/

   DotRenderable():
      _target( nullptr )
   {
      // Material.
      Vec4f color = Vec4f(0.7f, 0.7f, 0.7f, 1.0f);
      RCP<Gfx::ConstantBuffer> constant = Core::gfx()->createConstants( 4*sizeof(float) );
      constant->addConstant( "color", Gfx::CONST_FLOAT4, 0, color.ptr() );
      _cons = Gfx::ConstantList::create( constant );

      _prog = data( ResManager::getProgram( "shader/program/dot" ) );

      RCP<MeshGeometry> geom = new MeshGeometry();
      int attr[] = {
         MeshGeometry::POSITION,
         MeshGeometry::MAPPING,
         0,
      };
      geom->setAttributes( attr );
      uint32_t idx[] = {
         0, 1, 2,  0, 2, 3,
      };
      float vtx[] = {
         // POSITION           MAPPING
         -1.0f,  0.0f, 0.0f,   0.0f, 0.0f,
          1.0f,  0.0f, 0.0f,   1.0f, 0.0f,
          1.0f,  2.0f, 0.0f,   1.0f, 1.0f,
         -1.0f,  2.0f, 0.0f,   0.0f, 1.0f,
      };
      uint ni = sizeof(idx)/sizeof(idx[0]);
      geom->allocateIndices( ni );
      geom->copyIndices( idx );
      uint nv = sizeof(vtx)/(sizeof(vtx[0])*5);
      geom->allocateVertices( nv );
      geom->copyAttributes( vtx, 5, 5, 0 );
      geom->addPatch( 0, ni );
      _geom = geom;
   }

   bool  refreshPosition() const
   {
      Vector<Rayf> rays;
      rays.pushBack( Rayf( _target->position(), Vec3f(0.0f, -1024.0f, 0.0f) ) );
      MotionWorld::IntersectionData data;
      _target->world()->motionWorld()->raycast( rays, RigidBody::ANY_MASK, ~0x0, data );
      if( data.hit(0) )
      {
         _pos = data.location(0);
         return true;
      }
      return false;
   }

   virtual void render( Gfx::Pass& pass, const Viewport& vp ) const
   {
      if( _target == nullptr ) return;

      if( refreshPosition() )
      {
         const Camera* cam = vp.camera();
         //float d = length( _pos - cam->position() );
         //float s = vp.size()( vp.fovAxis() );
         //float f = 2.0f * d / s;
         float f = 1.0f/32.0f;
         Reff ref = Reff( cam->orientation(), _pos, f );
         pass.setWorldMatrix( ref.toMatrix().ptr() );

         pass.setDepthState( _defDepth );
         pass.setAlphaState( _defBlending );
         pass.setConstants( _cons );
         pass.setProgram( _prog );
         pass.setSamplers(0);
         _geom->render( pass );
      }
   }

   void target( RigidEntity* t ) { _target = t; }
   RigidEntity* target() const   { return _target; }
   Vec3f position() const        { return _pos; }

protected:

   /*----- data members -----*/

   RigidEntity*           _target;
   mutable Vec3f          _pos;
   RCP<Gfx::Program>      _prog;
   RCP<Gfx::ConstantList> _cons;
   RCP<Geometry>          _geom;
};


/*==============================================================================
  CLASS MultiCharacterController
==============================================================================*/

//------------------------------------------------------------------------------
//!
void
MultiCharacterController::initialize()
{
   VMObjectPool::registerCreate(
      "UI",
      "multiCharacterController",
      stdCreateVM<MultiCharacterController>
   );
}

//-----------------------------------------------------------------------------
//!
MultiCharacterController::MultiCharacterController():
   _selectedCharacter( nullptr )
{
   dispatcher().onPointerPress( nullptr, makeDelegate(this, &MultiCharacterController::pointerPress) );
   dispatcher().onPointerRelease( nullptr, makeDelegate(this, &MultiCharacterController::pointerRelease) );
   dispatcher().onPointerMove( nullptr, makeDelegate(this, &MultiCharacterController::pointerMove) );
   dispatcher().onPointerScroll( nullptr, makeDelegate(this, &MultiCharacterController::pointerScroll) );

   Core::addRenderBegin( makeDelegate( this, &MultiCharacterController::renderBeginCb ) );

   _arrow = new ArrowRenderable();
   _dot   = new DotRenderable();
}

//-----------------------------------------------------------------------------
//!
MultiCharacterController::~MultiCharacterController()
{
   Core::removeRenderBegin( makeDelegate( this, &MultiCharacterController::renderBeginCb ) );
}

//------------------------------------------------------------------------------
//!
RigidEntity*
MultiCharacterController::pickTargetIndicator( const Vec2f& pos )
{
   RigidEntity* target = _arrow->target();
   if( target )
   {
      Vec3f pos2d = viewport()->cameraMatrix() | _arrow->position();
      float dist  = length( pos - pos2d(0,1) );
      if( dist < 32 ) return target;
   }
   return nullptr;
}

//------------------------------------------------------------------------------
//! Pick an entity in the world by throwing a set of rays in the scene.
Entity*
MultiCharacterController::pickEntity( const Vec2f& pos, Vec3f& hitPos )
{
   const float maxRayDist = 1024.0f;

   static const Vec2f offsets[] = {
      Vec2f(  0.0f,  0.0f),
      Vec2f( -6.0f,  0.0f), Vec2f(  0.0f, -6.0f), Vec2f(  6.0f,  0.0f), Vec2f( 0.0f,  6.0f),
      Vec2f( -6.0f, -6.0f), Vec2f(  6.0f, -6.0f), Vec2f( -6.0f,  6.0f), Vec2f( 6.0f,  6.0f),
      Vec2f(-12.0f,  0.0f), Vec2f(  0.0f,-12.0f), Vec2f( 12.0f,  0.0f), Vec2f( 0.0f, 12.0f),
   };
   static const float weights[] = {
      128.0f,
       32.0f, 32.0f, 32.0f, 32.0f,
        8.0f,  8.0f,  8.0f,  8.0f,
        1.0f,  1.0f,  1.0f,  1.0f,
   };
   CHECK( sizeof(offsets)/sizeof(offsets[0]) == sizeof(weights)/sizeof(weights[0]) );

   // Prepare a bunch of rays.
   Rayf ray;
   ray.origin( camera()->position() );
   const uint n = sizeof(offsets)/sizeof(offsets[0]);

   World::PickingData  data( n );
   for( uint i = 0; i < n; ++i )
   {
      Vec2f p = pos + offsets[i];
      ray.direction( viewport()->direction(p) * maxRayDist );
      data.setRay( i, ray );
   }

   // Cast them in the scene, each keeping best entity.
   world()->pick( data );

   // Compute importance for each candidate entity.
   Map<Entity*, PickedInfo>  importance;
   for( uint i = 0; i < n; ++i )
   {
      Entity* e = data.entry(i).entity();
      if( e )
      {
         PickedInfo& pi = importance[e];
         pi._weight    += weights[i];
         pi._pos        = data.entry(i).location();
      }
   }
   // Find the best one.
   Entity*   e = nullptr;
   float  best = -CGConstf::infinity();
   for( auto cur = importance.begin(); cur != importance.end(); ++cur )
   {
      if( (*cur).second._weight > best )
      {
         e      = (*cur).first;
         best   = (*cur).second._weight;
         hitPos = (*cur).second._pos;
      }
   }
   return e;
}

//------------------------------------------------------------------------------
//!
// Here we compute the direction by computing the intersection
// between the floor plane currently under the character (origin) and a
// second plane formed by the cross product of two vectors:
// (origin - camera()->position() )
// viewport()->direction( screenPos )
// This direction should be scale by the 2D distance between the
// pointer position and the character position.
Vec3f
MultiCharacterController::computeDirection( const Vec2f& screenPos, const Vec3f& origin, float& len )
{
   Vec3f origin2d = viewport()->cameraMatrix() | origin;
   float sf       = viewport()->size()( viewport()->fovAxis() );
   len            = length( (screenPos - origin2d(0,1)) / sf );

   Vec3f v0 = origin - camera()->position();
   Vec3f v1 = viewport()->direction( screenPos );
   Vec3f n  = cross( v0, v1 );
   Vec3f d  = cross( Vec3f(0.0f, 1.0f, 0.0f), n );

   return normalize(d);
}

//------------------------------------------------------------------------------
//!
Vec3f
MultiCharacterController::computePathPosition( const Vec2f& screenPos, const Vec3f& lastPos )
{
   Vec3f pos;
   Entity* e = pickEntity( screenPos, pos );
   if( e )  return pos;
   return lastPos;
}

//------------------------------------------------------------------------------
//!
void
MultiCharacterController::selectEntity( const ConstString& id )
{
   Entity* e = world()->entity( id );
   if( e->type() == Entity::SKELETAL )
      selectEntity( (SkeletalEntity*)e );
}

//------------------------------------------------------------------------------
//!
void
MultiCharacterController::selectEntity( SkeletalEntity* se )
{
   if( se != _selectedCharacter )
   {
      // Deselect previous character.
      if( _selectedCharacter ) _selectedCharacter->stimulate( new DeselectOrder() );
      // Select new character.
      _selectedCharacter = se;
      _selectedCharacter->stimulate( new SelectOrder() );
      // Tell camera to follow the new character.
      camera()->stimulate( new FollowOrder( se->id() ) );
      execute( _onCharSelectRef, _selectedCharacter->id() );
   }
}

//------------------------------------------------------------------------------
//!
void
MultiCharacterController::pointerPress()
{
   const Event& ev = Core::currentEvent();
   PointerData& pd = _pointerData[ev.pointerID()];

   // Camera no set? no ready to interact.
   if( !camera() ) return;

   // Clear pointer mode.
   pd._mode         = MODE_NONE;
   pd._pickedEntity = nullptr;
   pd._pickedOffset = 0.0f;
   pd._curCharacter = nullptr;
   pd._needReset    = false;

   if( Core::numPressedPointers() == 2 )
   {
      uint opid        = getOtherPointerID( ev.pointerID() );
      PointerData& opd = _pointerData[opid];

      if( opd._mode == MODE_SCENE )
      {
         pd._mode  = MODE_ZOOM;
         opd._mode = MODE_ZOOM;
         return;
      }
      // Else, let if fall below, which allows moving multiple characters simultaneously.
   }
   else
   if( Core::isKeyPressed(Key::ALT) )
   {
      pd._mode = MODE_ZOOM;
      return;
   }

   // Did we just picked the target indicator (arrow).
   RigidEntity* target = pickTargetIndicator( ev.position() );
   if( target )
   {
      _arrow->grab();
      pd._mode         = MODE_TK_INDICATOR;
      pd._pickedEntity = target;
      pd._curCharacter = findCharacterGrabbing( target );
      pd._pos          = target->position();
      pd._needReset    = true;
      return;
   }

   // Pick entity in the world under the pointer.
   Entity* pickedEntity = pickEntity( ev.position(), pd._pos );

   // Choose the interaction mode for the pointer depending of the type of the
   // picked entity.

   if( pickedEntity == nullptr ) return;

   if( pickedEntity->type() == Entity::SKELETAL )
   {
      pd._mode         = MODE_CHARACTER;
      pd._curCharacter = (SkeletalEntity*)pickedEntity;
   }
   else
   if( pickedEntity->type() == Entity::RIGID )
   {
      RigidEntity* re = (RigidEntity*)pickedEntity;
      SkeletalEntity* grabbingCharacter = nullptr;
      // 1. Check for character+object 2-finger touch.
      if( Core::numPressedPointers() == 2 )
      {
         uint opid        = getOtherPointerID( ev.pointerID() );
         PointerData& opd = _pointerData[opid];
         if( opd._mode == MODE_CHARACTER ) grabbingCharacter = opd._curCharacter;
      }
      // 2. Honor the selected character's FLOW power.
      if( _charData[_selectedCharacter->id()]._mode == CMODE_FLOW )  grabbingCharacter = _selectedCharacter;
      // 3. Check if another object already has grabbed the object.
      if( grabbingCharacter == nullptr ) grabbingCharacter = findCharacterGrabbing( re );
      // 4. Use the currently selected character as a final fallback.
      if( grabbingCharacter == nullptr ) grabbingCharacter = _selectedCharacter;
      // If any character is grabbing the object, mark it as such.
      if( grabbingCharacter != nullptr )
      {
         const CharacterData& charData = _charData[grabbingCharacter->id()];
         if( charData._mode == CMODE_FLOW )
         {
            pd._mode         = MODE_FLOW;
            pd._curCharacter = grabbingCharacter;
            Vec3f dir = normalize( pd._pos - (grabbingCharacter->position() + Vec3f(0.0f,0.9f,0.0f)) );
            grabbingCharacter->stimulate( new FlowOrder(dir) );
         }
         else
         if( charData._mode == CMODE_GRAB && re->isDynamic() )
         {
            pd._mode         = MODE_TK;
            pd._pickedEntity = re;
            pd._pickedOffset = 0.0f;
            pd._curCharacter = grabbingCharacter;
            pd._pos          = re->position();
            pd._needReset    = true;

            Planef plane( computeTKNormal( *camera() ), pd._pos );
            Rayf ray( camera()->position(), viewport()->direction(ev.position()) * 1024.0f );
            float d = CGConstf::infinity();
            if( Intersector::trace( plane, ray, d ) )
            {
               pd._pickedOffset = re->position() - ray.point(d);
            }
         }
         else
         {
            pd._mode = MODE_SCENE;
         }
      }
      else
      {
         pd._mode = MODE_SCENE;
      }
   }
   else
   {
      pd._mode = MODE_SCENE;
   }
}

//------------------------------------------------------------------------------
//!
void
MultiCharacterController::pointerMove()
{
   const Event& ev    = Core::currentEvent();
   const Pointer& ptr = ev.pointer();
   if( !ptr.pressed() )  return; // Disregard non-drag events.
   PointerData& pd    = _pointerData[ev.pointerID()];

   switch( pd._mode )
   {
      case MODE_CHARACTER:
         if( hasMoved( ptr ) )
         {
            float len;
            Vec3f dir = computeDirection( ev.position(), pd._curCharacter->position(), len );
            pd._curCharacter->stimulate( new MoveOrder( dir, 16.0f*len ) );
         }
         break;
      case MODE_FLOW:
      {
         Vec3f pos;
         Entity* e = pickEntity( ev.position(), pos );
         if( e )
         {
            Vec3f dir = normalize( pos - (pd._curCharacter->position() + Vec3f(0.0f,0.9f,0.0f)) );
            pd._curCharacter->stimulate( new FlowOrder(dir) );
         }
      }  break;
      case MODE_SCENE:
         if( hasMoved( ptr ) )
         {
            // Orbiting the camera.
            const Vec2f& curPos = ev.position();
            const Vec2f& oldPos = ptr.lastPosition();
            const Vec2f& pixels = viewport()->size();
            Vec2f v             = oldPos - curPos; // Saves negating the result.
            v                  /= pixels;
            camera()->stimulate( new OrbitToOrder(v, true) );
         }
         break;
      case MODE_TK:
         if( hasMoved( ptr ) )
         {
            Planef plane( computeTKNormal( *camera() ), pd._pos );
            Rayf ray( camera()->position(), viewport()->direction(ev.position()) * 1024.0f );
            float d = CGConstf::infinity();
            if( Intersector::trace( plane, ray, d ) )
            {
               Vec3f p = ray.point( d ) + pd._pickedOffset;
               TKAttractor* a = TKAttractor::getCurrentAttractor( pd._pickedEntity );
               if( a ) a->targetPosition( p );
               else    pd._curCharacter->stimulate( new MoveObjectOrder( pd._pickedEntity, p ) );
               handleCorrectionReset( pd ); // TEMP
            }
         }
         break;
      case MODE_TK_INDICATOR:
      {
         Vec3f dir  = computeTKNormal( *camera() );
         Vec3f pos  = _arrow->position();
         Mat4f m    = viewport()->cameraMatrix();
         Vec2f axis = normalize( (m | (pos+dir))(0,1) - (m | pos)(0,1) );
         float t    = dot( axis, ptr.deltaPressPosition() );
         Vec3f tr   = dir*t*_arrow->distance()*0.001f;
         Vec3f p    = pd._pos + tr;
         //pd._curCharacter->stimulate( new MoveObjectOrder( pd._pickedEntity, p ) );
         TKAttractor* a = TKAttractor::getAttractor( pd._pickedEntity );
         CHECK( a );
         if( a ) a->targetPosition( p );
         handleCorrectionReset( pd ); // TEMP
      }  break;
      case MODE_ZOOM:
         if( Core::numPressedPointers() == 2 || ptr.persistent() )
         {
            const Vec2f& p0_cur = ptr.position();
            const Vec2f& p0_old = ptr.lastPosition();
            const Vec2f& p1_cur = ptr.persistent() ?
                                    ptr.lastReleaseEvent().position() :
                                    getOtherPointer(ev.pointerID()).position();
            float sizeToDouble  = viewport()->size().length() * 0.5f;
            float ratio         = computePinchRatio( p0_cur, p0_old, p1_cur, sizeToDouble );
            camera()->stimulate( new DollyByRatioOrder(ratio) );
         }
         break;
      default:
         break;
   }
}

//------------------------------------------------------------------------------
//!
void
MultiCharacterController::pointerRelease()
{
   const Event&   ev  = Core::currentEvent();
   PointerData&   pd  = _pointerData[ev.pointerID()];
   const Pointer& ptr = ev.pointer();

   switch( pd._mode )
   {
      case MODE_CHARACTER:
         if( hasMoved( ptr ) )
         {
            pd._curCharacter->stimulate( new StopOrder() );
         }
         else
         {
            selectEntity( pd._curCharacter );
         }
         break;
      case MODE_FLOW:
         pd._curCharacter->stimulate( new FlowStopOrder() );
         break;
      case MODE_SCENE:
         if( pd._curCharacter )
         {
            //if( isClick( ptr ) )
            //   pd._curCharacter->stimulate( new GoToOrder( pd._pos ) );
            //else
            if( hasMoved( ptr ) )
               pd._curCharacter->stimulate( new StopOrder() );
         }
         break;
      case MODE_TK:
      {
         float speed = length( ev.pointer().getSpeed() );
         // Throw?
         if( speed > 500.0f )
         {
            float len;
            Vec3f dir = computeDirection( ev.position(), pd._pickedEntity->position(), len );
            Vec3f vel = dir * ( speed * 0.01f );
            pd._curCharacter->stimulate( new ThrowOrder( pd._pickedEntity, vel ) );
         }
         // Grab or release (toggle order).
         else
         if( isClick( ev.pointer() ) )
         {
            if( pd._curCharacter != _selectedCharacter && Core::numPressedPointers() == 1 )
            {
               // Multi-touch select/deselect.
               pd._curCharacter->stimulate( new GrabOrder( pd._pickedEntity ) );
            }
            else
            if( _charData[_selectedCharacter->id()]._mode == CMODE_GRAB )
            {
               // Normal select/deselect.
               _selectedCharacter->stimulate( new GrabOrder( pd._pickedEntity ) );
            }
            handleCorrectionReset( pd ); // TEMP
         }
      }  break;
      case MODE_TK_INDICATOR:
         _arrow->ungrab();
         break;
      case MODE_ZOOM:
         if( Core::numPressedPointers() == 1 )
         {
            // Releasing from 2 to 1 pointer leaves last pointer in "camera" mode.
            _pointerData[getOtherPointerID(ev.pointerID())]._mode = MODE_SCENE;
         }
         break;
      default:
         break;
   }
}

//------------------------------------------------------------------------------
//!
void
MultiCharacterController::pointerScroll()
{
   const Event& ev = Core::currentEvent();
   float dy        = ev.scrollValue().y;
   float scale     = CGM::max( 16.0f, CGM::abs(dy)*4.0f );
   float ratio     = (scale + dy)/scale;
   camera()->stimulate( new DollyByRatioOrder(ratio) );
}

//-----------------------------------------------------------------------------
//!
void
MultiCharacterController::onViewportChange()
{
}

//------------------------------------------------------------------------------
//!
void
MultiCharacterController::onCameraChange()
{
   if( world() )
   {
      world()->addRenderable( _arrow.ptr() );
      world()->addRenderable( _dot.ptr() );
      selectEntity( "char0" );
   }
}

//------------------------------------------------------------------------------
//!
void
MultiCharacterController::handleCorrectionReset( PointerData& pd )
{
   if( pd._needReset )
   {
      TKAttractor* at = TKAttractor::getCurrentAttractor( pd._pickedEntity );
      if( at )
      {
         at->resetCorrection();
      }
   }
}

//------------------------------------------------------------------------------
//!
void
MultiCharacterController::arrowReadyCb( Resource<Geometry>* res )
{
   _arrow->geometry( data( res ) );
}

//------------------------------------------------------------------------------
//!
bool
MultiCharacterController::renderBeginCb()
{
   if( !_selectedCharacter )
   {
      _arrow->target( nullptr );
      _dot->target( nullptr );
   }
   const auto& attractors = TKAttractor::getAttractors();
   for( auto cur = attractors.begin(); cur != attractors.end(); ++cur )
   {
      TKAttractor* attractor = cur->second.ptr();
      for( size_t i = 0; i < attractor->numSources(); ++i )
      {
         if( attractor->source(i)->entity() == _selectedCharacter )
         {
            _arrow->target( attractor->target() );
            _dot->target( attractor->target() );
            return false;
         }
      }
   }
   _arrow->target( nullptr );
   _dot->target( nullptr );
   return false;
}

//------------------------------------------------------------------------------
//!
void
MultiCharacterController::setMode( const ConstString& charID, int mode )
{
   CharacterData& data = _charData[charID];
   if( world() )
   {
      Entity* e = world()->entity(charID);
      // Clean up old mode.
      if( mode != data._mode )
      {
         switch( data._mode )
         {
            case CMODE_FLOW:
               break;
            case CMODE_GRAB:
               // Or we can maintain it until the next power (e.g. flow) is activated.
               e->stimulate( new GrabOrder(nullptr) );
               break;
            case CMODE_SHIELD:
               e->stimulate( new LowerShieldOrder() );
               break;
            case CMODE_SHOCKWAVE:
               break;
         }
      }
      // Handle new mode.
      switch( mode )
      {
         case CMODE_FLOW:
            //e->stimulate( new FlowOrder() );
            break;
         case CMODE_GRAB:
            break;
         case CMODE_SHIELD:
            e->stimulate( new ToggleShieldOrder() );
            break;
         case CMODE_SHOCKWAVE:
            break;
      }
   }
   data._mode = mode;
}

//-----------------------------------------------------------------------------
//!
bool
MultiCharacterController::performGet( VMState* vm )
{
   switch( _attributes[VM::toCString( vm, -1 )] )
   {
      case ATTRIB_ENTITYID:
         if( _selectedCharacter ) VM::push( vm, _selectedCharacter->id() );
         return true;
      case ATTRIB_ARROW:
         return true;
      case ATTRIB_ONCHAR_SELECT:
         return true;
      case ATTRIB_SET_MODE:
         VM::push( vm, this, setModeVM );
         return true;
      default:
         break;
   }
   return Controller::performGet( vm );
}

//------------------------------------------------------------------------------
//!
bool
MultiCharacterController::performSet( VMState* vm )
{
   switch( _attributes[VM::toCString( vm, -2)] )
   {
      case ATTRIB_ENTITYID:
         selectEntity( VM::toConstString( vm, -1 ) );
         return true;
      case ATTRIB_ARROW:
      {
         RCP< Resource<Geometry> > res = ResManager::getGeometry( VM::toString( vm, -1 ), nullptr );
         if( res.isValid() ) res->callOnLoad( makeDelegate( this, &MultiCharacterController::arrowReadyCb ) );
      }  return true;
      case ATTRIB_ONCHAR_SELECT:
         VM::toRef( vm, -1, _onCharSelectRef );
         return true;
      case ATTRIB_SET_MODE:
         return true;
      default:
         break;
   }
   return Controller::performSet( vm );
}

NAMESPACE_END
