/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#include <Plasma/Action/TKAction.h>
#include <Plasma/Particle/BaseParticles.h>
#include <Plasma/World/World.h>
#include <Plasma/World/SkeletalEntity.h>
#include <Plasma/Geometry/LineMeshGeometry.h>
#include <Plasma/Geometry/MeshGeometry.h>
#include <Plasma/Intersector.h>
#include <Plasma/Resource/ResManager.h>

#include <MotionBullet/Collision/BasicShapes.h>

#include <CGMath/Noise.h>

#include <Base/ADT/StringMap.h>

/*==============================================================================
  UNNAMED NAMESPACE
==============================================================================*/
UNNAMESPACE_BEGIN

//------------------------------------------------------------------------------
//!
ConstString _typeTKAction_str;

enum
{
   ATTRIB_ENTITY,
   ATTRIB_FLOW,
   ATTRIB_FORCE,
   ATTRIB_MAX_DISTANCE,
   ATTRIB_POSITION,
   ATTRIB_SHIELD,
   ATTRIB_SHIELD_OFFSET,
   ATTRIB_SHIELD_RADIUS,
   ATTRIB_SHOCKWAVE,
   ATTRIB_THROW
};

StringMap _attributes(
   "entity",       ATTRIB_ENTITY,
   "flow",         ATTRIB_FLOW,
   "force",        ATTRIB_FORCE,
   "maxDistance",  ATTRIB_MAX_DISTANCE,
   "position",     ATTRIB_POSITION,
   "shield",       ATTRIB_SHIELD,
   "shieldOffset", ATTRIB_SHIELD_OFFSET,
   "shieldRadius", ATTRIB_SHIELD_RADIUS,
   "shockwave",    ATTRIB_SHOCKWAVE,
   "throw",        ATTRIB_THROW,
   ""
);

//------------------------------------------------------------------------------
//!
Map< RigidEntity*, RCP<TKAttractor> >  _tkAttractors;

//------------------------------------------------------------------------------
//!
Vec3f effectCurve( float t, float o, float s )
{
   Vec3f p( 0.0f, 0.0f, t );
   Vec3f off = Vec3f(
      CGM::perlinNoise1( 0.2f*(p + o*0.15f) )*0.5f,
      CGM::perlinNoise1( 0.2f*(p + o*2.50f) )*0.5f,
      CGM::perlinNoise1( 0.2f*(p + o*1.20f) )*0.2f
   );
   off += Vec3f(
      CGM::perlinNoise1( 2.0f*(p + o*1.15f) )*0.2f,
      CGM::perlinNoise1( 2.0f*(p + o*1.50f) )*0.2f,
      CGM::perlinNoise1( 2.0f*(p + o*1.20f) )*0.1f
   );
   return p + off*s;
}

/*==============================================================================
   FUNCTIONS for beam
==============================================================================*/

RCP<RigidEntity>
createBeam()
{
   RCP<RigidEntity> beam = new RigidEntity( RigidBody::STATIC );
   beam->castsShadows( false );
   // Geometry.
   RCP<LineMeshGeometry> geom = new LineMeshGeometry();
   beam->geometry( geom.ptr() );
   geom->allocateSegments( 31 );
   geom->allocateVertices( 31+1 );
   geom->addPatch( 0, geom->numIndices() );

   // Set material.
   RCP<CustomMaterial> mat = new CustomMaterial();
   mat->programName( "shader/program/line/colorTex" );
   RCP<Image> img = data( ResManager::getImage( "image/blueEnergy" ) );
   mat->addImage( img.ptr() );
   RCP<Table> tex( new Table() );
   mat->variants().set( "colorTex", tex.ptr() );

   RCP<MaterialSet> matSet = new MaterialSet();
   matSet->add( mat.ptr() );
   beam->materialSet( matSet.ptr() );

   return beam;
}

//------------------------------------------------------------------------------
//!
void
updateBeam( RigidEntity& beam, const Vec3f& spos, const Vec3f& tpos, float radius, float time )
{
   // Compute starting and ending position of the effect.
   Vec3f pos  = spos + Vec3f(0.0f,0.9f,0.0f);
   Vec3f dir  = normalize( tpos-pos );

   // Create referential (matrix) for tk function.
   Reff ref = Reff::lookAt( pos, tpos, Vec3f(0.0f,1.0f,0.0f) );

   // Update effect position and orientation.
   beam.referential( ref );

   LineMeshGeometry* lines = beam.geometry()->linemesh();
   uint ns = lines->numSegments();

   // Indices.
   auto s = lines->segment(0);
   for( uint i = 0; i < ns; ++i, ++s ) s.set( i, i+1 );

   // Unique offset for function.
   float offset = float(StdHash<void*>()(&beam) % 1000);

   // Vertices.
   auto v  = lines->vertex(0);
   float l = -length(tpos-pos);
   for( uint i = 0; i <= ns; ++i, ++v )
   {
      float t = float(i)/float(ns);
      float s = CGM::smoothRampC1( 1.0f-2.0f*CGM::abs(t-0.5f) );
      Vec3f p = effectCurve( t*l, time*4.0f + offset, s );
      v.position( p );
      v.t( t );
      v.radius( radius );
      v.color( Vec4f(1.0f,1.0f,1.1f,0.9f)*CGM::min(1.0f,s*4.0f) );
   }
   lines->updateAdjacency();
   //lines->updateProperties();
   lines->invalidateRenderableGeometry();
}

UNNAMESPACE_END

NAMESPACE_BEGIN


/*==============================================================================
   CLASS TKShockwave
==============================================================================*/
class TKShockwave:
   public Attractor
{
public:

   /*----- methods -----*/

   TKShockwave( const Vec3f& center, float speed, float force ):
      _center( center ), _radius(0.1f), _speed( speed ), _force( force )
   {
      // Creating shockwave entity.
      _entity = new RigidEntity( RigidBody::STATIC );
      _entity->castsShadows( false );
      _entity->position( center + Vec3f( 0.0f, 0.1f, 0.0f ) );
      // Geometry.
      const int attribs[] = {
         MeshGeometry::POSITION,
         MeshGeometry::MAPPING,
         0
      };
      uint numArcs     = 32;
      uint numVertices = numArcs*2;
      uint numIndices  = numArcs*6;
      RCP<MeshGeometry> geom = new MeshGeometry( MeshGeometry::TRIANGLES, attribs, numIndices, numVertices );
      _entity->geometry( geom.ptr() );
      geom->addPatch( 0, geom->numIndices() );

      float* v      = geom->vertices();
      uint32_t* ind = geom->indices();
      for( uint i = 0; i < numArcs; ++i )
      {
         float t = float(i) / float(numArcs);
         float a = CGM::cirToRad(t);
         // Position 0.
         *v++ = CGM::cos( a ); *v++ = 0.0f; *v++ = CGM::sin( a );
         // Mapping 0.
         *v++ = t; *v++ = 0.0f;
         // Position 1.
         *v++ = CGM::cos( a ); *v++ = 0.0f; *v++ = CGM::sin( a );
         // Mapping 1.
         *v++ = t; *v++ = 1.0f;

         // Triangle 0.
         uint v0 = i*2;
         uint v2 = (v0+2)%numVertices;
         *ind++ = v0;
         *ind++ = v2;
         *ind++ = v2+1;
         // Triangle 1.
         *ind++ = v0;
         *ind++ = v2+1;
         *ind++ = v0+1;
      }

      // Set material.
      RCP<CustomMaterial> mat = new CustomMaterial();
      mat->programName( "shader/program/shockwave" );
      mat->variants().set( "radius", _radius );
      mat->variants().set( "time", 0.0f );

      RCP<MaterialSet> matSet = new MaterialSet();
      matSet->add( mat.ptr() );
      _entity->materialSet( matSet.ptr() );
   }

   PLASMA_DLL_API virtual void addForce( const Vector< RCP<RigidBody> >& bodies )
   {
      const float t = (float)_world->simulationDelta();

      // Update radius.
      _radius += _speed*t;

      // Add force.
      float shock = t*_force*_speed/(_radius*_radius);

      for( auto cur = bodies.begin(); cur != bodies.end(); ++cur )
      {
         Vec3f dir   = (*cur)->centerPosition() - _center;
         float dist  = length( dir );

         if( dist > 0.1f && dist < _radius )
         {
            float f     = CGM::smoothRampC1( dist/_radius );
            Vec3f force = normalize( dir )*f*shock;
            (*cur)->addForce( force );
         }
      }
   }

   void update()
   {
      CustomMaterial* mat = (CustomMaterial*)_entity->materialSet()->material(0);
      mat->setConstant( "radius", &_radius );
   }

   bool finished() const
   {
      return _radius > 20.0f;
   }

   RigidEntity* entity() const { return _entity.ptr(); }

protected:

   /*----- data members -----*/

   Vec3f            _center;
   float            _radius;
   float            _speed;
   float            _force;
   RCP<RigidEntity> _entity;
};

/*==============================================================================
   CLASS TKShield
==============================================================================*/
class TKShield:
   public RigidEntity
{
public:

   /*----- methods -----*/

   TKShield(
      const Vec3f& offset = Vec3f(0.0f, 0.3f, 0.0f),
      float radius        = 1.0f,
      float delay         = 0.5f
   ):
      RigidEntity( RigidBody::DYNAMIC ),
      _offset( offset ), _radius( radius ),
      _delay( delay ), _dt( 0.0f ), _anim( 0.0f )
   {
      //state( Entity::GHOST );
      exists( 1<<2 ); // Only exists in world2, a.k.a. shield world.

      auto geomRes = ResManager::getGeometry( "geometry/shield", nullptr );
      geomRes->callOnLoad( makeDelegate( this, &TKShield::geometryLoadedCB ) );
   }

   const Vec3f&  offset() const { return _offset; }
   float         radius() const { return _radius; }
   float         delay () const { return _delay;  }

   void  offset( const Vec3f& v ) { _offset = v; }
   void  radius( float v )        { _radius = v; }
   void  delay ( float v )        { _delay  = v; }

   SphereShape*  shape() const { return geometry() ? (SphereShape*)geometry()->collisionShape() : nullptr; }

   bool  execute( float t, float d )
   {
      CustomMaterial* mat = (CustomMaterial*)materialSet()->material(0);
      mat->setConstant( "time", &t );

      if( _anim != 0.0f )
      {
         _dt += d;
         // From 0 to 1 when raising (i.e. _anim = 1.0f),
         // but from 1 to 0 when lowering (i.e. _anim = -1.0f).
         float f  = (_anim*-0.5f + 0.5f) + _anim * _dt / _delay;
         float fc = CGM::clamp( f, 0.0f, 1.0f );
         updateRadius( _radius * fc );
         if( fc != f )
         {
            _anim = 0.0f; // Stop animation when we clamp.
            return true;
         }
      }

      return false;
   }

   void  raise()
   {
      _dt   = (_anim == 0.0f) ? 0.0f : _delay - _dt; // Reverse anim when necessary.
      _anim = 1.0f;
   }

   void  lower()
   {
      _dt   = (_anim == 0.0f) ? 0.0f : _delay - _dt; // Reverse anim when necessary.
      _anim = -1.0f;
   }

   void  addToWorld( void* e )
   {
      SkeletalEntity* se = (SkeletalEntity*)e;
      CHECK( _joint.isNull() );
      position( se->position() + _offset );
      _joint = new FixJoint( body(), se->body() );
      _joint->anchor( Reff( se->position() ) );
      World* w = se->world();
      w->addEntity( this );
      w->addConstraint( _joint.ptr() );
   }

   void  removeFromWorld()
   {
      CHECK( _joint.isValid() );
      World* w = world();
      w->removeConstraint( _joint.ptr() );
      w->removeEntity( this );
      _joint = nullptr; // FIXME: Try to reuse same joint all the time.
   }

protected:

   /*----- methods -----*/

   void  geometryLoadedCB( Resource<Geometry>* res )
   {
      // Start with a radius of zero to avoid popping,
      // since execute() hasn't been called, yet.
      res->data()->collisionShape( new SphereShape(0.0f) );
      geometry( res->data() );

      RCP<CustomMaterial> mat = new CustomMaterial();
      mat->programName( "shader/program/shield" );
      mat->variants().set( "radius", 0.0f );
      mat->variants().set( "time", 0.0f );

      RCP<MaterialSet> matSet = new MaterialSet();
      matSet->add( mat.ptr() );
      materialSet( matSet.ptr() );
   }

   void  updateRadius( float r )
   {
      Geometry* geom = geometry();
      if( geom )
      {
         CustomMaterial* mat = (CustomMaterial*)materialSet()->material(0);
         mat->setConstant( "radius", &r );

         SphereShape* shape = (SphereShape*)geom->collisionShape();
         shape->radius(r);
      }
   }

   /*----- data members -----*/

   Vec3f          _offset;
   float          _radius;
   float          _delay;  // Duration of the raise/lower animation.
   float          _dt;     // Time of current raise/lower animation.
   float          _anim;   // -1.0f: lowering, 0.0f: raised, 1.0f: raising.
   RCP<FixJoint>  _joint;
};

/*==============================================================================
   CLASS TKSource
==============================================================================*/

//------------------------------------------------------------------------------
//!
TKSource::TKSource():
   _entity( nullptr ),
   _followOrientation( false ),
   _localPos( 0.0f ),
   _targetPos( 0.0f ),
   _targetOrient( Quatf::identity() ),
   _maxSqrDist( 10.0f*10.0f ),
   _maxForce( 40.0f ),
   _cachedForce( 0.0f )
{
}

//------------------------------------------------------------------------------
//!
float
TKSource::force( const Vec3f& pos )
{
   float sqrDist = sqrLength(_entity->position() - pos);
   float d       = sqrDist / _maxSqrDist;
   if( d > 1.0f )
   {
      d = 1.0f;
      //tooFar( true );
   }
   float f      = (2.0f*d - 3)*d*d + 1.0f;
   _cachedForce = _maxForce * f;

   return _cachedForce;
}

//------------------------------------------------------------------------------
//!
void
TKSource::targetPosition( const Vec3f& pos )
{
   if( _followOrientation )
      _localPos = _entity->referential().globalToLocal() * pos;
   else
      _localPos = pos - _entity->position();
}


/*==============================================================================
   CLASS TKAction
==============================================================================*/

//------------------------------------------------------------------------------
//!
void
TKAction::initialize()
{
   _typeTKAction_str = "tkAction";
   Action::registerAction( _typeTKAction_str, TKAction::create );
}

//------------------------------------------------------------------------------
//!
void
TKAction::terminate()
{
   _typeTKAction_str = ConstString();
}

//------------------------------------------------------------------------------
//!
Action*
TKAction::create()
{
   return new TKAction();
}

//------------------------------------------------------------------------------
//!
TKAction::TKAction():
   _mode( MODE_NONE ),
   _throwVelocity( 0.0f ),
   _offset( 0.0f, 0.2f, 0.0f ),
   _target( nullptr ),
   _attractor( nullptr )
{
   _shield = new TKShield();
   _beam   = createBeam();
}

//------------------------------------------------------------------------------
//!
TKAction::~TKAction()
{
   grabEntity( nullptr );
}

//------------------------------------------------------------------------------
//!
const ConstString&
TKAction::type() const
{
   return _typeTKAction_str;
}

//------------------------------------------------------------------------------
//!
bool
TKAction::execute( Entity& entity, double time, double delta )
{
   World* w = entity.world();
   if( !w ) return false;

   switch( _mode )
   {
      case MODE_NONE:
         break;
      case MODE_FLOW:
      {
         // Find target.
         Vector<Rayf> rays;
         const Vec3f& pos = entity.position() + Vec3f(0.0f,0.9f,0.0f);
         Vec3f dir        = _flowDir * 10.0f;
         Vec3f tpos       = pos + dir;
         rays.pushBack( Rayf( pos, dir ) );
         MotionWorld::IntersectionData data;
         w->motionWorld()->raycast( rays, RigidBody::ANY_MASK, ~0x0, data );
         if( data.hit(0) )
         {
            tpos = data.location(0);
            // Add force.
            if( data.body(0)->type() == RigidBody::DYNAMIC )
            {
               data.body(0)->addForce( normalize(dir)*80.0f, tpos-data.body(0)->centerPosition() );
            }
         }
         // Update effect position and geometry.
         updateBeam( *_beam, entity.position(), tpos, 0.4f, float(time) );
      }  break;
      case MODE_GRAB:
      {
         if( _source._followOrientation )
            _source._targetPos = entity.transform() * _source._localPos;
         else
            _source._targetPos = entity.position() + _source._localPos;

#if 1
         // Prevent objects from traversing obstacles.
         Vector<Rayf> rays;
         const Vec3f& pos = _target->position();
         //Vec3f pos = entity.position() + Vec3f(0.0f,0.9f,0.0f);
         Vec3f dir = (_source._targetPos - pos);
         Vec3f n   = normalize(dir);
         dir += n*0.2f; // TEMP: Relative to object (bounding sphere?).
         rays.pushBack( Rayf( pos, dir ) );
         MotionWorld::IntersectionData data;
         w->motionWorld()->raycast( rays, RigidBody::STATIC_MASK, ~0x0, data );
         if( data.hit(0) )
         {
            //StdErr << _source._targetPos << " -> " << data.location(0) << " " << _target->position() << nl;
            // TODO: Throw a ray from _source._targetPos to _target->position()
            // to find a surface location instead of moving the center.
            // Requires casting a ray to a single collision shape, which is
            // currently a little painful under Bullet (see btDynamicWorld::rayTestSingle()).
            _source._targetPos = data.location(0) - n*0.2f;
            _attractor->resetCorrection();
         }
#endif
            // Update effect position and geometry.
            updateBeam( *_beam, entity.position(), _target->position(), 0.2f, float(time) );
      }  break;
      case MODE_SHIELD:
      {
         _shield->execute( float(time), float(delta) );
      }  break;
      case MODE_SHIELD_LOWERING:
      {
         if( _shield->execute( float(time), float(delta) ) ) _mode = MODE_NONE;
      }  break;
      case MODE_SHIELD_RAISING:
      {
         if( _shield->execute( float(time), float(delta) ) ) _mode = MODE_SHIELD;
      }  break;
      case MODE_SHOCKWAVE:
      {
         RCP<TKShockwave> sw = new TKShockwave( entity.position(), _shockwaveSpeed, _shockwaveForce );
         _shockwaves.pushBack( sw );
         _mode = MODE_NONE;
      }  break;
      case MODE_THROW:
         if( _target )
         {
            _target->linearVelocity( _throwVelocity );
            _mode = MODE_NONE;
         }
         break;
   }

   // Do the post execute.
   w->submit( CmdDelegate0( makeDelegate(this, &TKAction::postExecute) ) );

   return false;
}

//------------------------------------------------------------------------------
//!
void
TKAction::postExecute()
{
   World* w = _source._entity->world();

   // Beam.
   if( !_beam->world() )
   {
      if( _mode == MODE_GRAB || _mode == MODE_FLOW ) w->addEntity( _beam.ptr() );
   }
   else
   {
      if( _mode != MODE_GRAB && _mode != MODE_FLOW ) w->removeEntity( _beam.ptr() );
   }

   // Shield.
   if( !_shield->world() )
   {
      if( _mode >= MODE_SHIELD && _mode <= MODE_SHIELD_RAISING ) _shield->addToWorld( _source._entity );
   }
   else
   {
      if( _mode < MODE_SHIELD || _mode > MODE_SHIELD_RAISING ) _shield->removeFromWorld();
   }

   // Shockwave.
   for( auto s = _shockwaves.begin(); s != _shockwaves.end(); ++s )
   {
      if( !(*s)->entity()->world() )
      {
         w->motionWorld()->addAttractor( (*s).ptr() );
         w->addEntity( (*s)->entity() );
      }
      (*s)->update();
      if( (*s)->finished() )
      {
         w->motionWorld()->removeAttractor( (*s).ptr() );
         w->removeEntity( (*s)->entity() );
         *s = nullptr;
      }
   }
   _shockwaves.removeAll( nullptr );

   // Grabbing.
   if( _target && _mode != MODE_GRAB ) grabEntity( nullptr );
   if( _attractor && !_attractor->world() ) w->motionWorld()->addAttractor( _attractor );
   TKAttractor::cleanAttractors();
}

//------------------------------------------------------------------------------
//!
void
TKAction::grabEntity( RigidEntity* target )
{
   // Already grabbed?
   if( target == _target ) return;

   // Remove current target.
   if( _target )
   {
      // Disable the TK attractor.
      RCP<TKAttractor> attractor = TKAttractor::getAttractor( _target );
      _attractor->removeSource( &_source );
      _attractor = nullptr;
      _mode = MODE_NONE;
   }

   _target = target;

   // Set new target.
   if( target )
   {
      _mode = MODE_GRAB;
      _attractor = TKAttractor::getAttractor( target );
      _attractor->addSource( &_source );

      // Compute start parameters.
      Vec3f y               = target->orientation().getAxisY();
      Quatf q               = Quatf::twoVecs( y, Vec3f(0.0f, 1.0f, 0.0f) );
      _source._targetOrient = q * target->orientation();
      // Only add offset if we are the first one to grab.
      if( _attractor->numSources() == 1 )  _attractor->targetPosition( target->position() + _offset );
      else                                 _source.targetPosition( target->position() );
   }
}

//------------------------------------------------------------------------------
//!
void
TKAction::targetPosition( const Vec3f& desPos )
{
#if 1
   // Prevent objects from traversing obstacles.
   Vec3f pos = desPos;
   Vec3f dir = (desPos - _target->position());
   //Vec3f n   = normalize(dir);
   //dir += n*0.2f;
   Vector<Rayf> rays;
   rays.pushBack( Rayf( _target->position(), dir ) );
   MotionWorld::IntersectionData data;
   _target->world()->motionWorld()->raycast( rays, RigidBody::STATIC_MASK, ~0x0, data );
   if( data.hit(0) )
   {
      //StdErr << "*** ";
      pos = data.location(0); //-n*0.2f;
   }
   //StdErr << _target->position().y << " des=" << desPos.y << " --> " << pos.y << nl;
#else
   const Vec3f& pos = desPos;
#endif

   _source.targetPosition( pos );
}

//------------------------------------------------------------------------------
//!
void
TKAction::flow( const Vec3f& dir )
{
   _mode    = MODE_FLOW;
   _flowDir = dir;
}

//------------------------------------------------------------------------------
//!
void
TKAction::flowStop()
{
   if( _mode == MODE_FLOW ) _mode = MODE_NONE;
}

//------------------------------------------------------------------------------
//!
void
TKAction::shield( bool raising )
{
   if( raising )
   {
      _mode = MODE_SHIELD_RAISING;
      _shield->raise();
   }
   else
   {
      if( _mode == MODE_SHIELD || _mode == MODE_SHIELD_RAISING )
      {
         _mode = MODE_SHIELD_LOWERING;
         _shield->lower();
      }
   }
}

//------------------------------------------------------------------------------
//!
void
TKAction::shieldToggle()
{
   if( _mode == MODE_SHIELD || _mode == MODE_SHIELD_RAISING )
   {
      _mode = MODE_SHIELD_LOWERING;
      _shield->lower();
   }
   else
   {
      _mode = MODE_SHIELD_RAISING;
      _shield->raise();
   }
}

//------------------------------------------------------------------------------
//!
void
TKAction::shockwave( float speed, float force )
{
   _mode           = MODE_SHOCKWAVE;
   _shockwaveSpeed = speed;
   _shockwaveForce = force;
}

//------------------------------------------------------------------------------
//!
void
TKAction::throwEntity( const Vec3f& vel )
{
   _throwVelocity = vel;
   _mode          = MODE_THROW;
   // Clamp velocity else object will go through wall.
   _throwVelocity.maxLength( 16.0f );
}

//------------------------------------------------------------------------------
//!
bool
TKAction::performGet( VMState* vm )
{
   switch( _attributes[VM::toCString( vm, -1)] )
   {
      case ATTRIB_ENTITY:
         VM::pushProxy( vm, _target );
         return true;
      case ATTRIB_FLOW:
         return false;
      case ATTRIB_FORCE:
         VM::push( vm, _source._maxForce );
         return true;
      case ATTRIB_MAX_DISTANCE:
         VM::push( vm, CGM::sqrt(_source._maxSqrDist) );
         return true;
      case ATTRIB_POSITION:
      case ATTRIB_SHIELD:
      case ATTRIB_SHIELD_OFFSET:
      case ATTRIB_SHIELD_RADIUS:
      case ATTRIB_SHOCKWAVE:
      case ATTRIB_THROW:
         return false;
      default:
         break;
   }
   return false;
}

//------------------------------------------------------------------------------
//!
bool
TKAction::performSet( VMState* vm )
{
   // Retrieve entity and world.
   BrainTaskContext* context = (BrainTaskContext*)VM::userData( vm );
   _source._entity           = context->entity();

   switch( _attributes[VM::toCString( vm, -2 )] )
   {
      case ATTRIB_ENTITY:
         grabEntity( (RigidEntity*)VM::toProxy( vm, -1 ) );
         return true;
      case ATTRIB_FLOW:
         if( VM::isBoolean( vm, -1 ) )
         {
            if( VM::toBoolean( vm, -1 ) == false )
            {
               flowStop();
            }
            else
            {
               //flow();
            }
         }
         else
         {
            Vec3f dir = VM::toVec3f( vm, -1 );
            flow( dir );
         }
         return true;
      case ATTRIB_FORCE:
         _source._maxForce = VM::toFloat( vm, -1 );
         return true;
      case ATTRIB_MAX_DISTANCE:
      {
         float d = VM::toFloat( vm, -1 );
         _source._maxSqrDist = d*d;
      }  return true;
      case ATTRIB_POSITION:
         _attractor->targetPosition( VM::toVec3f( vm, -1 ) );
         return true;
      case ATTRIB_SHIELD:
         if( VM::isBoolean( vm, -1 ) )
            shield( VM::toBoolean( vm, -1 ) );
         else
            shieldToggle();
         return true;
      case ATTRIB_SHIELD_OFFSET:
         _shield->offset( VM::toVec3f(vm, -1 ) );
         return true;
      case ATTRIB_SHIELD_RADIUS:
         _shield->radius( VM::toFloat(vm, -1 ) );
         return true;
      case ATTRIB_SHOCKWAVE:
      {
         Vec2f speedForce = VM::toVec2f( vm, -1 );
         shockwave( speedForce.x, speedForce.y );
      }  return true;
      case ATTRIB_THROW:
         throwEntity( VM::toVec3f( vm, -1 ) );
         return true;
      default:
         break;
   }
   return false;
}

/*==============================================================================
   CLASS TKAttractor
==============================================================================*/

//------------------------------------------------------------------------------
//!
TKAttractor*
TKAttractor::getAttractor( RigidEntity* e )
{
   RCP<TKAttractor>& a = _tkAttractors[e];

   if( a.isNull() )
   {
      a = new TKAttractor();
      a->target( e );
   }

   return a.ptr();
}

//------------------------------------------------------------------------------
//!
TKAttractor*
TKAttractor::getCurrentAttractor( RigidEntity* e )
{
   auto it = _tkAttractors.find( e );
   return (it != _tkAttractors.end()) ? (*it).second.ptr() : nullptr;
}

//------------------------------------------------------------------------------
//!
const Map< RigidEntity*, RCP<TKAttractor> >&
TKAttractor::getAttractors()
{
   return _tkAttractors;
}

//------------------------------------------------------------------------------
//!
void
TKAttractor::cleanAttractors()
{
   for( auto cur = _tkAttractors.begin(); cur != _tkAttractors.end(); )
   {
      if( (*cur).second->numSources() == 0 )
      {
         (*cur).second->world()->removeAttractor( (*cur).second.ptr() );
         _tkAttractors.erase( cur++ );
         continue;
      }
      ++cur;
   }
}

//------------------------------------------------------------------------------
//!
void
TKAttractor::addForce( const Vector< RCP<RigidBody> >& )
{
   // Do we have source to move the target body?
   if( _sources.empty() ) return;

   // Retrieve various variables.
   RigidBody& body    = *_target->body();
   const Vec3f& p0    = body.centerPosition();
   const Quatf  o0    = body.centerOrientation();
   const Vec3f& v0    = body.linearVelocity();
   const float t      = (float)_world->simulationDelta();
   const float one_t  = (float)_world->simulationRate();
   const float m      = body.mass();


   Vec3f p1( 0.0f );
   Quatf o1( 0.0f, 0.0f, 0.0f, 0.0f );
   float totForce = 0.0f;
   float maxForce = 0.0f;

   // Compute target position.
   for( auto it = _sources.begin(); it != _sources.end(); ++it )
   {
      float force = (*it)->force(p0);
      // Don't account for energy and let players pay for lack of energy when it happens.
      p1       += (*it)->targetPosition()    * force;
      o1       += (*it)->targetOrientation() * force;
      totForce += force;
   }
   maxForce = totForce;
   if( maxForce != 0.0f )
   {
      p1 *= (1.0f/maxForce);
      o1 *= (1.0f/maxForce);
      p1 += _correction;
   }
   else
   {
      // No source is strong enough.
      p1 = p0;
      o1 = o0;
   }

   // Calculated displacement vector.
   Vec3f p01     = p1 - p0;
   float p01_len = length( p01 );
   Vec3f p01n    = p01 / p01_len;

   // Compute maximum applicable force.
   if( p01_len > 1e-5f )
   {
      maxForce = 0.0f;
      for( auto it = _sources.begin(); it != _sources.end(); ++it )
      {
         Vec3f dir    = (*it)->targetPosition() - p0;
         float dirSq  = sqrLength( dir );
         float factor = 1.0f;
         if( dirSq > 1e-5f ) // Within sqrt(1e-5), assume we're using 100% to avoid normalizing zero-length dir.
         {
            dir   *= CGM::sqrt( dirSq ); // Normalize dir.
            factor = 1.0f - 0.2f*( 1.0f - dir.dot( p01n ) );
            factor = CGM::clamp( factor, 0.0f, 1.0f );
         }
         maxForce += (*it)->force() * factor;
      }
   }

   // Maximum speed.
   float maxS = maxForce * t / m;

   // Compute the new velocity.
   Vec3f vf;

   // Are we there yet?
   if( p01_len < 1e-5f )
   {
      float v0_len = v0.length();
      vf           = v0_len > maxS ? v0 * ( 1.0f-maxS/v0_len ) : Vec3f::zero();
   }
   else
   {
      float maxDecel = maxForce / m;

      // Calculate maximum expected speed.
      float maxExpectedSpeed = CGM::sqrt( maxS*maxS + 2.0f*maxDecel*p01_len );

      // Clamp to no more than what is needed to attain p1 in less than a timestep.
      maxExpectedSpeed = CGM::min( maxExpectedSpeed, p01_len*one_t );

      // Calculate the final velocity that we will get.
      float d1, d2;
      Rayf ray = Rayf( Vec3f::zero(), p01n );
      uint n   = Intersector::trace( v0, maxS, ray, d1, d2 );

      switch( n )
      {
         case 0:
            if( v0.dot(p01) < 0.0f )
               vf = v0 - v0.getRescaled( maxS );
            else
            {
               Vec3f par, per;
               v0.decompose( p01, par, per );
               vf = v0 - per.getRescaled( maxS );
            }
            break;
         case 1:
            // Barely counters v0.
            vf = ray.point( d1 );
            break;
         case 2:
            // Has a range of possibilities.
            CGM::clamp( maxExpectedSpeed, d1, d2 );
            vf = p01.getRescaled( maxExpectedSpeed );
            break;
      }
   }

   // Calculate force using only the telekinesis component of the final velocity.
   Vec3f force = (vf - v0) * (m / t);

   // Apply force.
   body.addForce( force );

   // Orientation.
   Quatf of = o0.slerp( o1, 0.2f );
   of.normalize();
   body.referential( Reff( of, p0 ) );
   body.angularVelocity(0.0f);


#if 1
   Vec3f gravDir;
   float gravNorm;
   body.world()->getGravity( &body, gravDir, gravNorm );
   Vec3f badLinVel = body.linearVelocity() - _expLinVel - gravDir*(gravNorm*t);
   _correction += badLinVel * t;
   _expLinVel = vf; // Register expected linear velocity for next addForce().
#endif
}

//------------------------------------------------------------------------------
//! Broadcasts the target position across all of the current sources.
void
TKAttractor::targetPosition( const Vec3f& v )
{
   for( auto cur = _sources.begin(); cur != _sources.end(); ++cur )
   {
      (*cur)->targetPosition( v );
   }
}

NAMESPACE_END
