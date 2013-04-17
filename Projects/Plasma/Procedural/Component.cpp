/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#include <Plasma/Procedural/Component.h>
#include <Plasma/Procedural/ProceduralGeometry.h>
#include <Plasma/Procedural/BSP2.h>

#include <Fusion/VM/VMMath.h>
#include <Fusion/VM/VMRegistry.h>

#include <CGMath/Distributions.h>
#include <CGMath/Grid.h>
#include <CGMath/Quadric.h>

#include <Base/ADT/StringMap.h>
#include <Base/Dbg/DebugStream.h>

#include <algorithm>

/*==============================================================================
   UNNAME NAMESPACE
==============================================================================*/

UNNAMESPACE_BEGIN

DBG_STREAM( os_b, "Component" );

const char* _comp_str       = "component";
const char* _region_str     = "region";
const char* _iter_str       = "iterator";
const char* _constraint_str = "constraint";

enum {
   ATTRIB_BOUNDARY,
   ATTRIB_COMPONENT,
   ATTRIB_ID,
   ATTRIB_ORIENTATION,
   ATTRIB_CONNECTOR_ORIENTATION,
   ATTRIB_CONNECTOR_POSITION,
   ATTRIB_PARENT,
   ATTRIB_POSITION,
   ATTRIB_TRANSFORM,
   ATTRIB_SIZE
};

StringMap _attributes(
   "boundary",             ATTRIB_BOUNDARY,
   "id",                   ATTRIB_ID,
   "orientation",          ATTRIB_ORIENTATION,
   "connectorOrientation", ATTRIB_CONNECTOR_ORIENTATION,
   "connectorPosition",    ATTRIB_CONNECTOR_POSITION,
   "parent",               ATTRIB_PARENT,
   "position",             ATTRIB_POSITION,
   "transform",            ATTRIB_TRANSFORM,
   "size",                 ATTRIB_SIZE,
   ""
);

StringMap _region_attr(
   "component",  ATTRIB_COMPONENT,
   "id",         ATTRIB_ID,
   "size",       ATTRIB_SIZE,
   ""
);

struct IterResult
{
   IterResult( Component* c ) { _c = c; }
   IterResult( Region* r )    { _r = r; }
   IterResult( int i )        { _i = i; }

   union {
      Component* _c;
      Region*    _r;
      int        _i;
   };
};

//------------------------------------------------------------------------------
//! 
struct ComponentGeom
{
   float  _abs;
   float  _rel;
   float  _endPos;
   int    _mode;
};

//------------------------------------------------------------------------------
//! 
struct ConstraintGeom
{
   float _min;
   float _max;
   int   _type;
};

//------------------------------------------------------------------------------
//! 
inline GeometryContext* getContext( VMState* vm )
{
   return (GeometryContext*)VM::userData(vm);
}

//------------------------------------------------------------------------------
//! 
bool leq( const ConstraintGeom& a, const ConstraintGeom& b )
{
   return a._min < b._min;
}


/*==============================================================================
   Constructor/Accessors
==============================================================================*/

//------------------------------------------------------------------------------
//! 
void adjustOrientation( VMState* vm, Component* comp )
{
   if( VM::get( vm, -1, "orientation" ) )
   {
      if( VM::isNumber( vm, -1 ) )
      {
         int ori = VM::toInt( vm, -1 );
         ori = CGM::clamp( ori, 0, 7 );
         Vec3f s = comp->size();
         Reff r  = comp->referential();

         if( comp->dimension() == 3 )
         {
            switch( ori )
            {
               case 0: break;
               case 1: 
                  r.position( r.toMatrix() * Vec3f( s.x, 0.0f, 0.0f ) );
                  r.orientation( Quatf::axisCir( Vec3f(0.0f,1.0f,0.0f), 0.75f )*r.orientation() );
                  comp->size( Vec3f(s.z, s.y, s.x) );
                  break;
               case 2:
                  r.position( r.toMatrix() * Vec3f( 0.0f, s.y, 0.0f ) );
                  r.orientation( Quatf::axisCir( Vec3f(0.0f,0.0f,1.0f), 0.5f )*Quatf::axisCir( Vec3f(0.0f,1.0f,0.0f), 0.75f )*r.orientation() );
                  comp->size( Vec3f(s.z, s.y, s.x) );
                  break;
               case 3:
                  r.position( r.toMatrix() * Vec3f( s.x, s.y, 0.0f ) );
                  r.orientation( Quatf::axisCir( Vec3f(0.0f,0.0f,1.0f), 0.5f )*r.orientation() );
                  break;
               case 4:
                  r.position( r.toMatrix() * Vec3f( 0.0f, 0.0f, s.z ) );
                  r.orientation( Quatf::axisCir( Vec3f(0.0f,1.0f,0.0f), 0.25f )*r.orientation() );
                  comp->size( Vec3f(s.z, s.y, s.x) );
                  break;
               case 5:
                  r.position( r.toMatrix() * Vec3f( s.x, 0.0f, s.z ) );
                  r.orientation( Quatf::axisCir( Vec3f(0.0f,1.0f,0.0f), 0.5f )*r.orientation() );
                  break;
               case 6:
                  r.position( r.toMatrix() * Vec3f( 0.0f, s.y, s.z ) );
                  r.orientation( Quatf::axisCir( Vec3f(1.0f,0.0f,0.0f), 0.5f )*r.orientation() );
                  break;
               case 7:
                  r.position( r.toMatrix() * Vec3f( s.x, s.y, s.z ) );
                  r.orientation( Quatf::axisCir( Vec3f(1.0f,0.0f,0.0f), 0.5f )*Quatf::axisCir( Vec3f(0.0f,1.0f,0.0f), 0.75f )*r.orientation() );
                  comp->size( Vec3f(s.z, s.y, s.x) );
                  break;
               default: break;
            }
            comp->referential( r );
         }
         else
         {
            switch( ori )
            {
               case 0: break;
               case 1:
                  r.position( r.toMatrix() * Vec3f( s.x, 0.0f, 0.0f ) );
                  r.orientation( Quatf::axisCir( Vec3f(0.0f,0.0f,1.0f), 0.25f )*r.orientation() );
                  comp->size( Vec2f( s.y, s.x ) );
                  break;
               case 2:
                  r.position( r.toMatrix() * Vec3f( s.x, s.y, 0.0f ) );
                  r.orientation( Quatf::axisCir( Vec3f(0.0f,0.0f,1.0f), 0.5f )*r.orientation() );
                  break;
               case 3:
                  r.position( r.toMatrix() * Vec3f( 0.0f, s.y, 0.0f ) );
                  r.orientation( Quatf::axisCir( Vec3f(0.0f,0.0f,1.0f), 0.75f )*r.orientation() );
                  comp->size( Vec2f( s.y, s.x ) );
                  break;
            }
         }
      }
      VM::pop( vm );
   }
}

//------------------------------------------------------------------------------
//! 
void readUserAttributes( VMState* vm, Component* comp )
{
   const char* key = VM::toCString( vm, -2 );
   switch( VM::type( vm, -1 ) )
   {
      case VM::NIL:    comp->removeAttribute( key ); break;
      case VM::BOOL:   comp->setAttribute( key, VM::toBoolean( vm, -1 ) ); break;
      case VM::NUMBER: comp->setAttribute( key, VM::toFloat( vm, -1 ) );   break;
      case VM::STRING: comp->setAttribute( key, VM::toCString( vm, -1 ) ); break;
      case VM::TABLE:  
      {
         Table* table = new Table();
         VM::toTable( vm, -1, *table );
         comp->setAttribute( key, table );
      } break;
      case VM::OBJECT:
         switch( VMMath::type( vm, -1 ) )
         {
            case VMMath::COUNTER:
            {
               VMCounter* c = VMMath::toCounter( vm, -1 );
               comp->setAttribute( key, (float)c->next() );
            }  break;
            default:
               StdErr << "Component attributes of type OBJECT is not a counter." << nl;
               break;
         }
         break;
      default:;
   }
}

//------------------------------------------------------------------------------
//! 
void readAttributes( VMState* vm, Component* comp )
{
   // Read id(s).
   if( VM::get( vm, -1, "id" ) )
   {
      if( VM::isString( vm, -1 ) )
      {
         comp->addId( VM::toCString( vm, -1 ) );
      }
      else
      {
         for( uint i = 1; VM::geti( vm, -1, i ); ++i )
         {
            comp->addId( VM::toCString( vm, -1 ) );
            VM::pop( vm );
         }
      }
      VM::pop( vm );
   }

   // User attributes.
   VM::push( vm );
   while( VM::next( vm, -2 ) )
   {
      if( VM::isNumber( vm, -2 ) || _attributes[ VM::toCString( vm, -2 ) ] != StringMap::INVALID )
      {
         VM::pop( vm );
      }
      else
      {
         readUserAttributes( vm, comp );
         VM::pop( vm );
      }
   }
}

//------------------------------------------------------------------------------
//! 
void readBoundary( VMState* vm, Component* comp, bool relative )
{
   // We have a polygon.
   if( VM::isTable( vm, -1 ) )
   {
      // Reading boundary polygon.
      BoundaryPolygon poly;
      int numVertices = VM::getTableSize( vm, -1 );
      AABBoxf box     = AABBoxf::empty();

      Vec3f direction( 0.0f, 1.0f, 0.0f );
      VM::get( vm, -1, "direction", direction );

      poly.reserveVertices( numVertices );
      for( int i = 1; i <= numVertices; ++i )
      {
         VM::geti( vm, -1, i );
         Vec3f v = VM::toVec3f( vm, -1 );
         poly.addVertex( v );
         box |= v;
         box |= v + direction;
         VM::pop( vm );
      }

      // Compute size and scale factor.
      Vec3f s(1.0f);
      if( relative ) s = comp->size();

      // Resize if needed, then transform it in global space.
      Mat4f m = comp->referential().toMatrix() * Mat4f::scaling( s );
      for( int i = 0; i < numVertices; ++i )
      {
         poly.vertex(i) = m * poly.vertex(i);
      }

      // Create boundary.
      poly.computeDerivedData();
      RCP<Boundary> b = Boundary::create( poly, direction*s );

      // Recompute component size and position.
      comp->size( box.size()*s );
      comp->position( m*box.corner(0) );

      // Read and assign face id.
      if( VM::get( vm, -1, "id" ) )
      {
         // Do we have a list of ids? One per face.
         if( VM::isTable( vm, -1 ) )
         {
            int numFaces = CGM::min( VM::getTableSize( vm, -1 ), (int)b->numFaces() );
            for( int i = 1; i <= numFaces; ++i )
            {
               VM::geti( vm, -1, i );
               ConstString id( VM::toCString( vm, -1 ) );
               VM::pop( vm );
               b->id( i-1, id );
            }
         }
         else
         {
            // One id for all faces.
            ConstString id( VM::toCString( vm, -1 ) );
            for( uint f = 0; b->numFaces(); ++f )
            {
               b->id( f, id );
            }
         }
         // Remove id table from the stack.
         VM::pop( vm );
      }

      // Clip boundary if necessary.
      if( comp->parent() )
      {
         comp->parent()->resolveBoundary();
         if( comp->parent()->boundary() )
         {
            b = Boundary::create( *b, *comp->parent()->boundary() );
         }
      }
      comp->boundary( b.ptr() );
   }
   else
   {
      // We have a face.
      int face = VM::toInt( vm, -1 );
      comp->boundary( face );
   }
   // Remove boundary table from stack.
   VM::pop( vm );
}

//------------------------------------------------------------------------------
//! 
void init( VMState* vm, Component* comp )
{
   // Parent
   if( VM::geti( vm, -1, 1 ) )
   {
      Component* parent = (Component*)VM::toProxy( vm, -1 );
      VM::pop( vm );
      if( parent ) parent->addComponent( comp );
   }

   readAttributes( vm, comp );

   // Referential.
   Reff ref = Reff::identity();
   VM::get( vm, -1, "position", ref.position() );
   VM::get( vm, -1, "orientation", ref.orientation() );

   if( !comp->parent() )
   {
      comp->referential( ref );
   }
   else
   {
      comp->inherit( comp->parent(), ref, comp->parent()->size() );
   }

   // Connector referential.
   ref = Reff::identity();
   VM::get( vm, -1, "connectorPosition", ref.position() );
   VM::get( vm, -1, "connectorOrientation", ref.orientation() );
   comp->connector().referential( ref );

   // Size and dimension.
   bool hasSize = false;
   if( VM::get( vm, -1, "size" ) )
   {
      hasSize = true;
      int dim = 3;
      if( VM::isObject( vm, -1 ) )
      {
         dim = VMMath::type( vm, -1 ) == VMMath::VEC2 ? 2 : 3;
      }
      else
      {
         dim = VM::getTableSize( vm, -1 );
      }
      if( dim == 2 )
      {
         comp->size( VM::toVec2f( vm, -1 ) );
      }
      else if( dim == 3 )
      {
         comp->size( VM::toVec3f( vm, -1 ) );
      }
      VM::pop( vm );
   }
   else if( comp->parent() )
   {
      hasSize = true;
   }

   // Boundary definition reading/creation.
   if( VM::get( vm, -1, "boundary" ) ) readBoundary( vm, comp, hasSize );

   // Adjust orientation if orientation is a number.
   adjustOrientation( vm, comp );
}

//------------------------------------------------------------------------------
//! 
int comp_create( VMState* vm )
{
   GeometryContext* context = getContext(vm);
   Component* comp          = context->_compositor->createComponent();
   init( vm, comp );
   VM::pushProxy( vm, comp );
   return 1;
}

//------------------------------------------------------------------------------
//! 
int comp_get( VMState* vm )
{
   Component* comp  = (Component*)VM::toProxy( vm, 1 );
   const char* attr = VM::toCString( vm, 2 );

   switch( _attributes[attr] )
   {
      case ATTRIB_PARENT   : VM::pushProxy( vm, comp->parent() );            return 1;
      case ATTRIB_POSITION : VM::push( vm, comp->position()    );            return 1;
      case ATTRIB_TRANSFORM: VM::push( vm, comp->referential().toMatrix() ); return 1;
      case ATTRIB_SIZE     : VM::push( vm, comp->size()        );            return 1;
      default:;
   }

   const Variant& v = comp->getAttribute( attr );
   switch( v.type() )
   {
      case Variant::NIL:    VM::push( vm );                       return 1;
      case Variant::BOOL:   VM::push( vm, v.getBoolean() );       return 1;
      case Variant::FLOAT:  VM::push( vm, v.getFloat() );         return 1;
      case Variant::VEC2:   VM::push( vm, v.getVec2() );          return 1;
      case Variant::VEC3:   VM::push( vm, v.getVec3() );          return 1;
      case Variant::VEC4:   VM::push( vm, v.getVec4() );          return 1;
      case Variant::STRING: VM::push( vm, v.getString().cstr() ); return 1;
      case Variant::TABLE:  VM::push( vm, *v.getTable() );        return 1;
         break;
   }

   return 0;
}

//------------------------------------------------------------------------------
//! 
int comp_set( VMState* vm )
{
   readUserAttributes( vm, (Component*)VM::toProxy( vm, 1 ) );
   return 0;
}

//------------------------------------------------------------------------------
//! 
int region_get( VMState* vm )
{
   Region* region   = (Region*)VM::toProxy( vm, 1 );
   const char* attr = VM::toCString( vm, 2 );

   switch( _region_attr[attr] )
   {
      case ATTRIB_COMPONENT:
         VM::pushProxy( vm, region->component() );
         return 1;
      case ATTRIB_ID:
         VM::push( vm, region->id().cstr() );
         return 1;
      case ATTRIB_SIZE:
         VM::push( vm, region->positionRange().size() );
         return 1;
      default:
         break;
   }

   return 0;
}

//------------------------------------------------------------------------------
//! 
int region_set( VMState* /*vm*/ )
{
   return 0;
}

//------------------------------------------------------------------------------
//! 
int region_create( VMState* vm )
{
   // Parent component.
   Component* comp = 0;
   if( VM::geti( vm, -1, 1 ) )
   {
      comp = (Component*)VM::toProxy( vm, -1 );
      VM::pop( vm );
   }

   // No component: region is invalid.
   if( comp == 0 ) return 0;

   Region* region = comp->createRegion();

   // Is region build around a boundary face?
   int face = -1;
   if( VM::geti( vm, -1, 2 ) )
   {
      face = VM::toInt( vm, -1 );
      VM::pop( vm );
   }

   // Id.
   if( VM::get( vm, -1, "id" ) )
   {
      region->id( VM::toCString( vm, -1 ) );
      VM::pop( vm );
   }

   Reff ref = Reff::identity();
   Vec3f size(0.0);

   // Compute region from face.
   if( face != -1 )
   {
      // We have a complexe boundary.
      if( comp->boundary() )
      {
         comp->resolveBoundary();
         comp->boundary()->computeFaceRefAndSize( face, ref, size );
         // Transform in local component space.
         ref = comp->referential().getInversed() * ref;
      }
      else
      {
         // Bounding box boundary.
         switch( face )
         {
            case 0:
               size = Vec3f( comp->size().z, comp->size().y, 0.0f );
               ref.orientation( Quatf::axisCir( Vec3f(0.0f,1.0f,0.0f), 0.75f ) );
               break;
            case 1:
               size = Vec3f( comp->size().z, comp->size().y, 0.0f );
               ref.position( comp->size().x, 0.0f, comp->size().z );
               ref.orientation( Quatf::axisCir( Vec3f(0.0f,1.0f,0.0f), 0.25f ) );
               break;
            case 2:
               size = Vec3f( comp->size().x, comp->size().z, 0.0f );
               ref.orientation( Quatf::axisCir( Vec3f(1.0f,0.0f,0.0f), 0.25f ) );
               break;
            case 3:
               size = Vec3f( comp->size().x, comp->size().z, 0.0f );
               ref.position( comp->size().x, comp->size().y, 0.0f );
               ref.orientation( Quatf::axes( Vec3f(-1.0f,0.0f,0.0f), Vec3f(0.0f,0.0f,1.0f), Vec3f(0.0f,1.0f,0.0f) ) );
               break;
            case 4:
               size = Vec3f( comp->size().x, comp->size().y, 0.0f );
               ref.position( comp->size().x, 0.0f, 0.0f );
               ref.orientation( Quatf::axisCir( Vec3f(0.0f,1.0f,0.0f), 0.5f ) );
               break;
            case 5:
               size = Vec3f( comp->size().x, comp->size().y, 0.0f );
               ref.position( 0.0f, 0.0f, comp->size().z );
               break;
            default:
               break;
         }
      }
   }
   else
   {
      size = comp->size();
   }

   // Referential.
   Reff ref2 = Reff::identity();
   VM::get( vm, -1, "position", ref2.position() );
   VM::get( vm, -1, "orientation", ref2.orientation() );
   region->referential( ref2*ref );

   // Relative bounds of the region and absolute offsets.
   Vec3f relMin(0.0f);
   Vec3f relMax(1.0f);
   Vec3f absMin(0.0f);
   Vec3f absMax(0.0f);

   if( VM::get( vm, -1, "rel" ) )
   {
      VM::geti( vm, -1, 1 );
      relMin = VM::toVec3f( vm, -1 );
      VM::geti( vm, -2, 2 );
      relMax = VM::toVec3f( vm, -1 );
      VM::pop( vm, 3 );
   }
   if( VM::get( vm, -1, "abs" ) )
   {
      VM::geti( vm, -1, 1 );
      absMin = VM::toVec3f( vm, -1 );
      VM::geti( vm, -2, 2 );
      absMax = VM::toVec3f( vm, -1 );
      VM::pop( vm, 3 );
   }

   // Range.
   AABBoxf box( size*relMin+absMin, size*relMax+absMax );
   region->positionRange( box );

   VM::pushProxy( vm, region );
   return 1;
}

//------------------------------------------------------------------------------
//! 
int pconstraint_create( VMState* vm )
{
   // linked component.
   Component* comp = 0;
   if( VM::geti( vm, -1, 1 ) )
   {
      comp = (Component*)VM::toProxy( vm, -1 );
      VM::pop( vm );
   }

   // No component: constraint is invalid.
   if( comp == 0 ) return 0;

   // Face.
   int face = 0;
   if( VM::geti( vm, -1, 2 ) )
   {
      face = VM::toInt( vm, -1 );
      VM::pop( vm );
   }

   ComponentConstraint* constraint = (ComponentConstraint*)VM::newObject( vm, sizeof(ComponentConstraint) );
   *constraint = ComponentConstraint( ComponentConstraint::PLANE, comp, face );
   VM::getMetaTable( vm, _constraint_str );
   VM::setMetaTable( vm, -2 );

   return 1;
}

//------------------------------------------------------------------------------
//! 
int fconstraint_create( VMState* vm )
{
   // linked component.
   Component* comp = 0;
   if( VM::geti( vm, -1, 1 ) )
   {
      comp = (Component*)VM::toProxy( vm, -1 );
      VM::pop( vm );
   }

   // No component: constraint is invalid.
   if( comp == 0 ) return 0;

   // Face.
   int face = 0;
   if( VM::geti( vm, -1, 2 ) )
   {
      face = VM::toInt( vm, -1 );
      VM::pop( vm );
   }

   Vec3f offset(0.0f);
   VM::get( vm, -1, "offset", offset );

   int repulsion = 0;
   if( VM::get( vm, -1, "repulse") )
   {
      VM::pop(vm);
      repulsion = ComponentConstraint::REPULSE;
   }

   ComponentConstraint* constraint = (ComponentConstraint*)VM::newObject( vm, sizeof(ComponentConstraint) );
   *constraint = ComponentConstraint( ComponentConstraint::POLYGON | repulsion, comp, face );
   constraint->offset( offset );
   VM::getMetaTable( vm, _constraint_str );
   VM::setMetaTable( vm, -2 );
   return 1;
}

//------------------------------------------------------------------------------
//! 
int vconstraint_create( VMState* vm )
{
   // linked component.
   Component* comp = 0;
   if( VM::geti( vm, -1, 1 ) )
   {
      comp = (Component*)VM::toProxy( vm, -1 );
      VM::pop( vm );
   }

   // No component: constraint is invalid.
   if( comp == 0 ) return 0;

   Vec3f offset(0.0f);
   VM::get( vm, -1, "offset", offset );

   bool repulse = false;
   VM::get( vm, -1, "repulse", repulse );
   int repulsion = repulse ? ComponentConstraint::REPULSE : 0;

   ComponentConstraint* constraint = (ComponentConstraint*)VM::newObject( vm, sizeof(ComponentConstraint) );
   *constraint = ComponentConstraint( ComponentConstraint::VOLUME | repulsion, comp );
   constraint->offset( offset );
   VM::getMetaTable( vm, _constraint_str );
   VM::setMetaTable( vm, -2 );
   return 1;
}

/*==============================================================================
   Queries
==============================================================================*/

//------------------------------------------------------------------------------
//! 
int iter_gc( VMState* vm )
{
   delete *(Vector<IterResult>**)VM::toPtr( vm, 1 );
   return 0;
}

//------------------------------------------------------------------------------
//! 
int compIterVM( VMState* vm )
{
   Vector<IterResult>* result = *(Vector<IterResult>**)VM::toPtr( vm, VM::upvalue(1) );
   uint i                     = VM::toUInt( vm, VM::upvalue(2) );

   // Iteration finish?
   if( i >= result->size() ) return 0;

   // Update counter.
   VM::push( vm, i+1 );
   VM::replace( vm, VM::upvalue(2) );

   // Return the current component.
   VM::pushProxy( vm, (*result)[i]._c );
   return 1;
}

//------------------------------------------------------------------------------
//! 
int faceIterVM( VMState* vm )
{
   Vector<IterResult>* result = *(Vector<IterResult>**)VM::toPtr( vm, VM::upvalue(1) );
   uint i                     = VM::toUInt( vm, VM::upvalue(2) );

   // Iteration finish?
   if( i >= result->size() ) return 0;

   // Update counter.
   VM::push( vm, i+1 );
   VM::replace( vm, VM::upvalue(2) );

   // Return the current face id.
   VM::push( vm, (*result)[i]._i );
   return 1;
}

//------------------------------------------------------------------------------
//! 
int regionIterVM( VMState* vm )
{
   Vector<IterResult>* result = *(Vector<IterResult>**)VM::toPtr( vm, VM::upvalue(1) );
   uint i                     = VM::toUInt( vm, VM::upvalue(2) );

   // Iteration finish?
   if( i >= result->size() ) return 0;

   // Update counter.
   VM::push( vm, i+1 );
   VM::replace( vm, VM::upvalue(2) );

   // Return the current region ptr.
   VM::pushProxy( vm, (*result)[i]._r );
   return 1;
}

//------------------------------------------------------------------------------
//!
bool isIn( const Vector<ConstString>& ids, const Set<ConstString>& set )
{
   for( uint i = 0; i < ids.size(); ++i )
   {
      if( set.has( ids[i] ) ) return true;
   }
   return false;
}

//------------------------------------------------------------------------------
//! 
int queryVM( VMState* vm )
{
   GeometryContext* context = getContext(vm);
   int numParams = VM::getTop( vm );
   int param     = 1;

   Component* comp = 0;
   if( numParams > 0 && VM::isObject( vm, 1 ) )
   {
      --numParams;
      ++param;
      comp = (Component*)VM::toProxy( vm, 1 );
      context->_compositor->scope( comp );
   }

   // Create iterator result list.
   Vector<IterResult>* result = new Vector<IterResult>();

   if( numParams == 0 )
   {
      Compositor::Iterator iter =  context->_compositor->iterator();
      for( ; iter.valid(); ++iter )
      {
         result->pushBack( *iter );
      }
   }
   else
   {
      Set<ConstString> ids;
      for( int i = 0; i < numParams; ++i )
      {
         ids.add( VM::toCString( vm, i+param ) );
      }
      Compositor::Iterator iter =  context->_compositor->iterator();
      for( ; iter.valid(); ++iter )
      {
         if( isIn( (*iter)->ids(), ids ) ) result->pushBack( *iter );
      }
   }

   if( comp ) context->_compositor->unscope();

   // Create a user data that will be destroy when the iterator is finish.
   void** data = (void**)VM::newObject( vm, sizeof(void*) );
   *data = result;
   VM::getMetaTable( vm, _iter_str );
   VM::setMetaTable( vm, -2 );

   // Push the counter.
   VM::push( vm, 0 );

   // Push the iterator closure function.
   VM::push( vm, compIterVM, 2 );

   return 1;
}

//------------------------------------------------------------------------------
//! 
int faceQueryVM( VMState* vm )
{
   int numParams   = VM::getTop( vm );
   Component* comp = (Component*)VM::toProxy( vm, 1 );

   // No component?
   if( comp == 0 )
   {
      StdErr << "No component was specified for fquery.\n";
      return 0;
   }

   // Create iterator result list.
   Vector<IterResult>* result = new Vector<IterResult>();

   // Find witch type(s) of face we need to iterate on.
   int faces = 0;
   Set<ConstString> ids;
   if( numParams == 1 || VM::isNil( vm, 2 ) )
   {
      faces = Boundary::ALL;
   }
   else
   {
      for( int i = 2; i <= numParams; ++i )
      {
         const char* type = VM::toCString( vm, i );
         switch( type[0] )
         {
            case 'B':
               if( (type[1] == 0) || (strcmp( type, "BOTTOM" ) == 0) )
               {
                  faces |= Boundary::BOTTOM; continue;
               }
               break;
            case 'E':
               if( (type[1] == 0) || (strcmp( type, "EXTRUDED" ) == 0) )
               {
                  faces |= Boundary::EXTRUDED; continue;
               }
               break;
            case 'S':
               if( (type[1] == 0 ) || (strcmp( type, "SIDE" ) == 0) )
               {
                  faces |= Boundary::SIDE; continue;
               }
               break;
            case 'T':
               if( (type[1] == 0 ) || (strcmp( type, "TOP" ) == 0) )
               {
                  faces |= Boundary::TOP; continue;
               }
               break;
            case 'X':
               if( type[1] == 0 ) { faces |= Boundary::POSX; continue; }
               break;
            case 'Y':
               if( type[1] == 0 ) { faces |= Boundary::POSY; continue; }
               break;
            case 'Z':
               if( type[1] == 0 ) { faces |= Boundary::POSZ; continue; }
               break;
            case '-':
               if( type[2] == 0 )
               {
                  switch( type[1] )
                  {
                     case 'X': faces |= Boundary::NEGX; continue;
                     case 'Y': faces |= Boundary::NEGY; continue;
                     case 'Z': faces |= Boundary::NEGZ; continue;
                  }
               }
               break;
            case '+':
               if( type[2] == 0 )
               {
                  switch( type[1] )
                  {
                     case 'X': faces |= Boundary::POSX; continue;
                     case 'Y': faces |= Boundary::POSY; continue;
                     case 'Z': faces |= Boundary::POSZ; continue;
                  }
               }
               break;
            default: break;
         }

         ids.add( ConstString( type ) );
      }
   }

   // Find all faces.
   if( comp->boundary() )
   {
      comp->resolveBoundary();
      Boundary* b = comp->boundary();
      for( uint i = 0; i < b->numFaces(); ++i )
      {
         if( (b->orientation( i, comp->referential() ) & faces) || (!ids.empty() && ids.has( b->id(i) )) ) result->pushBack(i);
      }
   }
   else
   {
      if( comp->dimension() == 2 )
      {
         result->pushBack(0);
      }
      else
      {
         for( int i = 0; i < 6; ++i )
         {
            if( faces & (1<<i) ) result->pushBack(i);
         }
      }
   }

   // Create a user data that will be destroy when the iterator is finish.
   void** data = (void**)VM::newObject( vm, sizeof(void*) );
   *data = result;
   VM::getMetaTable( vm, _iter_str );
   VM::setMetaTable( vm, -2 );

   // Push the counter.
   VM::push( vm, 0 );

   // Push the iterator closure function.
   VM::push( vm, faceIterVM, 2 );

   return 1;
}

//------------------------------------------------------------------------------
//!
int regionQueryVM( VMState* vm )
{
   GeometryContext* context = getContext(vm);
   int numParams            = VM::getTop( vm );

   // Create iterator result list.
   Vector<IterResult>* result = new Vector<IterResult>();

   Set<ConstString> ids;
   Component* comp = 0;
   int start       = 1;

   if( numParams > 0 )
   {
      // Reading component ptr.
      if( VM::isObject( vm, 1 ) )
      {
         comp  = (Component*)VM::toProxy( vm, 1 );
         start = 2;
      }

      // Reading region names.
      for( int i = start; i <= numParams; ++i )
      {
         ids.add( ConstString( VM::toCString( vm, i ) ) );
      }
   }

   // Finding regions.
   if( comp )
   {
      for( uint i = 0; i < comp->numRegions(); ++i )
      {
         if( ids.empty() || ids.has( comp->region(i)->id() ) )
            result->pushBack( comp->region(i) );
      }
   }
   else
   {
      Compositor::Iterator iter =  context->_compositor->iterator();
      for( ; iter.valid(); ++iter )
      {
         for( uint i = 0; i < (*iter)->numRegions(); ++i )
         {
            if( ids.empty() || ids.has( (*iter)->region(i)->id() ) )
               result->pushBack( (*iter)->region(i) );
         }
      }
   }

   // Create a user data that will be destroy when the iterator is finish.
   void** data = (void**)VM::newObject( vm, sizeof(void*) );
   *data = result;
   VM::getMetaTable( vm, _iter_str );
   VM::setMetaTable( vm, -2 );

   // Push the counter.
   VM::push( vm, 0 );

   // Push the iterator closure function.
   VM::push( vm, regionIterVM, 2 );

   return 1;
}

//------------------------------------------------------------------------------
//! 
int neighborQueryVM( VMState* vm )
{
   // What about children? should they be exclude?
   // TODO: Optimize with hgrid.
   GeometryContext* context = getContext(vm);
   Component* comp          = (Component*)VM::toProxy( vm, 1 );
   int numParams            = VM::getTop( vm );
   int i                    = 2;
   float dist               = 0.0f;

   if( comp == 0 )
   {
      StdErr << "No component was specified for nquery\n";
      return 0;
   }

   if( numParams > 1 && VM::isNumber( vm, 2 ) )
   {
      dist = VM::toFloat( vm, 2 );
      ++i;
   }

   Set<ConstString> ids;
   for( ; i <= numParams; ++i )
   {
      ids.add( VM::toCString( vm, i ) );
   }

   // Compute bbox and local referential needed for intersection queries.
   AABBoxf box( Vec3f(0.0f), comp->size() );
   box.grow( dist );
   Reff iref = comp->referential().getInversed();

   // Create iterator result list.
   Vector<IterResult>* result = new Vector<IterResult>();

   Compositor::Iterator iter =  context->_compositor->iterator();
   for( ; iter.valid(); ++iter )
   {
      // Do not return yourself.
      if( (*iter) == comp ) continue;

      // Does the component have the correct id.
      if( ids.empty() || isIn( (*iter)->ids(), ids ) )
      {
         // Are component intersecting?
         Reff ref = iref * (*iter)->referential();
         if( box.isOverlapping( AABBoxf( Vec3f(0.0f), (*iter)->size() ), ref ) )
         {
            result->pushBack( *iter );
         }
      }
   }

   // Create a user data that will be destroy when the iterator is finish.
   void** data = (void**)VM::newObject( vm, sizeof(void*) );
   *data = result;
   VM::getMetaTable( vm, _iter_str );
   VM::setMetaTable( vm, -2 );

   // Push the counter.
   VM::push( vm, 0 );

   // Push the iterator closure function.
   VM::push( vm, compIterVM, 2 );

   return 1;
}

//------------------------------------------------------------------------------
//! 
float occlusion( Component* comp, int face, float dist, const Vector<Component*>& comps )
{
   BSP2 bsp;
   bsp.back( dist );

   comp->resolveBoundary();
   if( comp->dimension() == 2 || face > -1 )
   {
      RCP<BoundaryPolygon> poly;
      // Create a boundary face for the face that we want to evaluate.
      if( comp->boundary() )
      {
         if( comp->face() > -1 ) face = comp->face();
         if( face > -1 )
         {
            poly = BoundaryPolygon::create( *comp->boundary(), face );
         }
         else
         {
            poly = BoundaryPolygon::create( *comp->boundary(), 0 );
         }
      }
      else
      {
         // We have an implicit boundary.
         if( face > 1 )
         {
            poly = BoundaryPolygon::create( AABBoxf( Vec3f(0.0f), comp->size() ), face, comp->referential().toMatrix() );
         }
         else
         {
            poly = BoundaryPolygon::create( AARectf( Vec2f( comp->size().x, comp->size().y ) ), comp->referential().toMatrix() );
         }
      }
      // Starting polygon area.
      bsp.build( *poly );
      float area = bsp.computeArea();

      // Remove all component faces from default polygon.
      Vector< RCP<BoundaryPolygon> > polys;
      for( uint i = 0; i < comps.size(); ++i )
      {
         // Create boundary polygons.
         Component* c = comps[i];
         c->resolveBoundary();
         polys.clear();
         if( c->boundary() )
         {
            if( c->face() > -1 )
            {
               polys.pushBack( BoundaryPolygon::create( *c->boundary(), c->face() ) );
            }
            else
            {
               BoundaryPolygon::create( *c->boundary(), polys );
            }
         }
         else
         {
            // We have an implicit boundary.
            if( c->dimension() == 2 )
            {
               polys.pushBack( BoundaryPolygon::create( AARectf( Vec2f( c->size().x, c->size().y ) ), c->referential().toMatrix() ) );
            }
            else
            {
               BoundaryPolygon::create( AABBoxf( Vec3f(0.0f), c->size() ), c->referential().toMatrix(), polys );
            }
         }
         // Remove polygons.
         for( uint j = 0; j < polys.size(); ++j )
         {
            bsp.removeProj( *polys[j] );
         }
      }
      float occ = 1.0f - (bsp.computeArea()/area);
      return occ;
   }
   else
   {
      // Multi-face query.
      // TODO...
      StdErr << "Multi-faces occlusion query not supported.\n";
   }
   return 0;
}

//------------------------------------------------------------------------------
//! 
int occlusionVM( VMState* vm )
{
   GeometryContext* context = getContext(vm);
   Component* comp          = (Component*)VM::toProxy( vm, 1 );
   int numParams            = VM::getTop( vm );
   float dist               = 1.0f/1024.0f;
   int i                    = 2;

   if( comp == 0 )
   {
      StdErr << "No component was specified for occlusion\n";
      return 0;
   }

   // Read distance.
   if( numParams > 1 && VM::isNumber( vm, 2 ) )
   {
      dist = VM::toFloat( vm, 2 );
      ++i;
   }

   // Read components.
   Vector<Component*> comps;
   if( (numParams < i) || VM::isString( vm, i ) )
   {
      // TODO: Optimize with hgrid.
      Set<ConstString> ids;
      for( ; i <= numParams; ++i )
      {
         ids.add( VM::toCString( vm, i ) );
      }
      // Compute bbox and local referential needed for intersection queries.
      AABBoxf box( Vec3f(0.0f), comp->size() );
      box.grow( dist );
      Reff iref = comp->referential().getInversed();

      Compositor::Iterator iter =  context->_compositor->iterator();
      for( ; iter.valid(); ++iter )
      {
         // Do not intersect yourself.
         if( (*iter) == comp ) continue;
         if( comp->isParent( *iter ) ) continue;

         // Does the component have the correct id.
         if( ids.empty() || isIn( (*iter)->ids(), ids ) )
         {
            // Are component intersecting?
            Reff ref = iref * (*iter)->referential();
            if( box.isOverlapping( AABBoxf( Vec3f(0.0f), (*iter)->size() ), ref ) )
            {
               comps.pushBack( *iter );
            }
         }
      }
   }
   else if( VM::isObject( vm, i ) )
   {
      // One component.
      comps.pushBack( (Component*)VM::toProxy( vm, i ) );
   }
   else
   {
      // A list of component.
      for( int j = 1; VM::geti( vm, i, j ); ++j )
      {
         comps.pushBack( (Component*)VM::toProxy( vm, -1 ) );
         VM::pop( vm );
      }
   }

   // Compute occlusion.
   float occ = occlusion( comp, -1, dist, comps );
   VM::push( vm, occ );

   return 1;
}

//------------------------------------------------------------------------------
//! 
int queryBeginVM( VMState* vm )
{
   GeometryContext* context = getContext(vm);
   Component* comp          = (Component*)VM::toProxy( vm, 1 );
   context->_compositor->scope( comp );
   return 0;
}

//------------------------------------------------------------------------------
//! 
int queryEndVM( VMState* vm )
{
   GeometryContext* context = getContext(vm);
   context->_compositor->unscope();
   return 0;
}

//------------------------------------------------------------------------------
//! 
int hasIDVM( VMState* vm )
{
   Component* comp = (Component*)VM::toProxy( vm, 1 );

   if( comp == 0 ) return 0;

   ConstString id( VM::toCString( vm, 2 ) );
   for( uint i = 0; i < comp->ids().size(); ++i )
   {
      if( id == comp->id(i) )
      {
         VM::push( vm, true );
         return 1;
      }
   }
   VM::push( vm, false );
   return 1;
}

//------------------------------------------------------------------------------
//! 
int hasParentIDVM( VMState* vm )
{
   Component* comp = (Component*)VM::toProxy( vm, 1 );

   if( comp == 0 ) return 0;

   ConstString id( VM::toCString( vm, 2 ) );

   Component* parent = comp->parent();
   while( parent )
   {
      for( uint i = 0; i < parent->ids().size(); ++i )
      {
         if( id == parent->id(i) )
         {
            VM::push( vm, true );
            return 1;
         }
      }
      parent = parent->parent();
   }
   VM::push( vm, false );
   return 1;
}

//------------------------------------------------------------------------------
//! 
int hasFaceIDVM( VMState* vm )
{
   Component* comp = (Component*)VM::toProxy( vm, 1 );

   if( comp == 0 ) return 0;

   ConstString id( VM::toCString( vm, 2 ) );

   Boundary* b = comp->boundary();
   if( b )
   {
      // 2D component with parent boundary?
      if( comp->face() > -1 )
      {
         VM::push( vm, id == b->id( comp->face() ) );
         return 1;
      }
      // Check all faces.
      for( uint f = 0; f < b->numFaces(); ++f )
      {
         if( id == b->id(f) )
         {
            VM::push( vm, true );
            return 1;
         }
      }
   }
   VM::push( vm, false );
   return 1;
}

//------------------------------------------------------------------------------
//! 
int faceIDVM( VMState* vm )
{
   Component* comp = (Component*)VM::toProxy( vm, 1 );

   if( comp == 0 ) return 0;

   int face = ( VM::getTop(vm) > 1 ) ? VM::toInt( vm, 2 ) : 0;

   if( comp->face() != -1 ) face = comp->face();

   Boundary* b = comp->boundary();
   if( b && !b->id( face ).isNull() ) VM::push( vm, b->id( face ).cstr() );
   else
      VM::push( vm );
   return 1;
}

/*==============================================================================
   Operations
==============================================================================*/

//------------------------------------------------------------------------------
//! 
int connectVM( VMState* vm )
{
   GeometryContext* context = getContext(vm);
   Component* compFrom      = (Component*)VM::toProxy( vm, 1 );
   Region*    regionTo      = (Region*)VM::toProxy( vm, 2 );

   if( compFrom == 0 )
   {
      StdErr << "No component was specified for connect.\n";
      return 0;
   }
   if( regionTo == 0 )
   {
      StdErr << "No region was specified for connect.\n";
      return 0;
   }

   int numParams = VM::getTop(vm);

   // Position.
   Vec3f pos(0.5f);
   if( numParams > 2 ) pos = VM::toVec3f( vm, 3 );
   // Offset.
   Vec3f offset(0.0f);
   if( numParams > 3 ) offset = VM::toVec3f( vm, 4 );

   // Orientation.
   Quatf ori( Quatf::identity() );
   if( numParams > 4 ) ori = VM::toQuatf( vm, 5 );

   // Connect region.
   pos = pos * regionTo->positionRange().size() + regionTo->positionRange().corner(0) + offset;
   context->_compositor->connect( compFrom, regionTo, regionTo->connection( pos, ori ) );

   return 0;
}

//------------------------------------------------------------------------------
//! 
int disconnectVM( VMState* vm )
{
   GeometryContext* context = getContext(vm);
   Component* comp          = (Component*)VM::toProxy( vm, 1 );

   if( comp == 0 )
   {
      StdErr << "No component was specified for disconnect.\n";
      return 0;
   }

   context->_compositor->disconnect( comp );
   return 0;
}

//------------------------------------------------------------------------------
//! 
int moveVM( VMState* vm )
{
   GeometryContext* context = getContext(vm);
   Component* comp          = (Component*)VM::toProxy( vm, 1 );

   if( comp == 0 )
   {
      StdErr << "No component was specified for move.\n";
      return 0;
   }

   Vec3f pos = VM::toVec3f( vm, 2 );
   if( VM::getTop( vm ) == 2 )
   {
      context->_compositor->move( comp, pos );
   }
   else
   {
      Quatf q = VM::toQuatf( vm, 3 );
      context->_compositor->move( comp, Reff( q, pos ) );
   }

   return 0;
}

//------------------------------------------------------------------------------
//! 
struct BoundaryVertex {

   const Vec3f& vertex() const { return _b->vertex( _v ); }
   const Vec3f& vertex2() const { return _b->vertex( _v+1 ); }

   Boundary* _b;
   uint      _v;
};

//------------------------------------------------------------------------------
//! 
void joinBoundaries( const Vector<Component*>& comps )
{
   float error = 1.0f/1024.0f;
   Vector<BoundaryVertex*> vertices;

   // Add vertices to a grid for fast queries.
   Grid<BoundaryVertex> grid( 128, (1.0f/128.0f) );
   for( uint c = 0; c < comps.size(); ++c )
   {
      Boundary* b = comps[c]->boundary();
      //if( !b ) continue; // FIXME:BIG HACK.
      BoundaryVertex bv;
      bv._b = b;
      uint numVertices = b->numVertices(0);
      for( uint v = 0; v < numVertices; ++v )
      {
         bv._v = b->vertexID( 0, v );
         grid.add( grid.cellID( b->vertex(bv._v) ), bv );
         b->flag( bv._v, 0 );
      }
   }

   // Compute new extruded vertices positions.
   for( uint c = 0; c < comps.size(); ++c )
   {
      // Compute for each vertices.
      Boundary* b      = comps[c]->boundary();
      //if( !b ) continue; // FIXME:BIG HACK.
      uint numVertices = b->numVertices(0);
      for( uint v = 0; v < numVertices; ++v )
      {
         vertices.clear();
         uint v0 = b->vertexID( 0, v );

         // Already done?
         if( b->flag( v0 ) ) continue;

         Vec3f dir(0.0f);
         const Vec3f& pos = b->vertex(v0);
         // Find all identical vertices.
         Vec3i c0 = grid.cellCoord( pos-error );
         Vec3i c1 = grid.cellCoord( pos+error );
         for( int i = c0.x; i <= c1.x; ++i )
         {
            for( int j = c0.y; j <= c1.y; ++j )
            {
               for( int k = c0.z; k <= c1.z; ++k )
               {
                  Grid<BoundaryVertex>::Link* l = grid.cell( Vec3i(i,j,k) );
                  for( ; l; l = l->_next )
                  {
                     if( sqrLength(l->_obj.vertex()-pos) < (error*error) )
                     {
                        vertices.pushBack( &l->_obj );
                        dir += l->_obj._b->face(1).normal();
                     }
                  }
               }
            }
         }
         // Recompute extruded vertices.
         if( vertices.size() > 1 )
         {
            // Compute the new vertex position by resolving the quadric build by all extruded plane.
            // If the quadric is not defined we had another plane constraint perpendicular to all normal.
            Quadricf q = Quadricf::zero();
            for( uint i = 0; i < vertices.size(); ++i )
            {
               Planef plane( vertices[i]->_b->face(1).normal(), vertices[i]->vertex2() );
               q += Quadricf( plane, 1.0f );
            }
            Vec3f epos;
            if( !q.optimize( epos ) )
            {
               Vec3f pln = cross( vertices[0]->_b->face(1).normal(), dir );
               if( !equal( sqrLength( pln ), 0.0f ) )
               {
                  Planef plane( pln, pos );
                  plane.normalize();
                  q += Quadricf( plane, 1.0f );
                  q.optimize( epos );
               }
               else
               {
                  epos = vertices[0]->vertex2();
               }
            }
            // Set the new extruded position to all vertices.
            for( uint i = 0; i < vertices.size(); ++i )
            {
               vertices[i]->_b->vertex( vertices[i]->_v+1 ) = epos;
               vertices[i]->_b->flag( vertices[i]->_v, 1 );
            }
         }
      }
      // Update boundary planes.
      b->computeDerivedData();

      // Recompute size and position of component.
      Reff ref;
      Vec3f size;
      b->computeRefAndSize( comps[c]->referential(), ref, size );
      comps[c]->referential( ref );
      comps[c]->size( size );
   }
}

//------------------------------------------------------------------------------
//! 
int extrudeVM( VMState* vm )
{
   GeometryContext* context = getContext(vm);
   int numParams            = VM::getTop(vm);

   // Extrusion depth.
   float defaultDepth = 1.0f;
   if( numParams > 1 ) defaultDepth = VM::toFloat( vm, 2 );

   Vector<Component*> comps;
   Vector<Component*> newcomps;

   // Reads components to extrude.

   // Single component.
   if( VM::isObject( vm, 1 ) )
   {
      Component* comp = (Component*)VM::toProxy( vm, 1 );
      if( comp && comp->dimension() == 2 ) comps.pushBack( comp );
   }
   else
   {
      // Many components.
      for( int i = 1; VM::geti( vm, 1, i ); ++i )
      {
         Component* comp = (Component*)VM::toProxy( vm, -1 );
         if( comp && comp->dimension() == 2 ) comps.pushBack( comp );
         VM::pop( vm );
      }
   }

   // Do we have component to extrude?
   if( comps.empty() ) return 0;

   // Create extruded components.
   ConstString depthStr( "depth" );
   for( uint i = 0; i < comps.size(); ++i )
   {
      Component* comp = comps[i];

      // Do we have a specific depth for this component.
      const Variant& depthAtt = comp->getLocalAttribute( depthStr );
      float depth = ( depthAtt.type() == Variant::FLOAT ) ? depthAtt.getFloat() : defaultDepth;

      // Create component.
      Component* c = context->_compositor->createComponent();
      comp->addComponent( c );
      newcomps.pushBack( c );
   
      // Create extruded boundary.
      comp->resolveBoundary();
      if( comp->boundary() )
      {
         if( comp->face() != -1 )
         {
            // Boundary is defined by a face of an 3D boundary.
            c->boundary( Boundary::create( *comp->boundary(), comp->face(), depth ).ptr() );
         }
         else
         {
            // Boundary is normal.
            c->boundary( Boundary::create( *comp->boundary(), depth ).ptr() );
         }
      }
      else
      {
         // Boundary is defined by a face of its bounding box.
         RCP<BoundaryPolygon> poly = BoundaryPolygon::create(
            AARectf( Vec2f(comp->size().x, comp->size().y) ),
            comp->referential().toMatrix()
         );
         c->boundary( Boundary::create( *poly, depth ).ptr() );
      }
      c->size( Vec3f( comp->size().x, comp->size().y, CGM::abs( depth ) ) );

      // Position the referential at the corner of the boundary.
      Reff ref = comp->referential();
      if( depth < 0.0f ) ref.translateLocal( Vec3f( 0.0f, 0.0f, depth ) );
      c->referential( ref );

      // Reads attributes if defined else inherits parents ids.
      if( numParams > 2 )
      {
         readAttributes( vm, c );
      }
      else
      {
         c->addId( comp->ids() );
      }
   }
   
   // Merge edge/vertices.
   if( newcomps.size() > 1 ) joinBoundaries( newcomps );

   // Ajust the orientation with user request.
   for( uint i = 0; i < comps.size(); ++i )
   {
      adjustOrientation( vm, newcomps[i] );
   }

   return 0;
}


//------------------------------------------------------------------------------
//! 
int intersectVM( VMState* vm )
{
   GeometryContext* context = getContext(vm);

   if( !VM::isTable( vm, 1 ) )
   {
      StdErr << "A list of component was not specified for intersect.\n";
      return 0;
   }

   // Reads components to merge together.
   Vector<Component*> comps;
   for( int i = 1; VM::geti( vm, 1, i ); ++i )
   {
      comps.pushBack( (Component*)VM::toProxy( vm, -1 ) );
      VM::pop( vm );
   }

   if( comps.empty() ) return 0;

   // Create new component.
   Component* c = context->_compositor->createComponent();
   Component* parent = context->_compositor->scope();
   if( parent ) parent->addComponent( c );
   readAttributes( vm,c );

   // Create new boundary.
   c->boundary( Boundary::intersect( comps ).ptr() );

   // Size and referential.
   Reff ref = Reff::identity();
   Vec3f size;
   c->boundary()->computeRefAndSize( ref, ref, size );
   c->referential( ref );
   c->size( size );
   adjustOrientation( vm, c );

   return 0;
}

//------------------------------------------------------------------------------
//! 
int mergeVM( VMState* vm )
{
   GeometryContext* context = getContext(vm);

   if( !VM::isTable( vm, 1 ) )
   {
      StdErr << "A list of component was not specified for merge.\n";
      return 0;
   }
  
   // Reads components to merge together.
   Vector<Component*> comps;
   for( int i = 1; VM::geti( vm, 1, i ); ++i )
   {
      comps.pushBack( (Component*)VM::toProxy( vm, -1 ) );
      VM::pop( vm );
   }

   if( comps.empty() ) return 0;

   // Create new component.
   Component* c = context->_compositor->createComponent();
   Component* parent = context->_compositor->scope();
   if( parent ) parent->addComponent( c );
   readAttributes( vm,c );

   // Create new boundary.
   c->boundary( Boundary::merge( comps ).ptr() );

   // Size and referential.
   Reff ref = Reff::identity();
   Vec3f size;
   c->boundary()->computeRefAndSize( ref, ref, size );
   c->referential( ref );
   c->size( size );
   adjustOrientation( vm, c );

   return 0;
}

//------------------------------------------------------------------------------
//!
int roofVM( VMState* vm )
{
   GeometryContext* context = getContext(vm);
   int numParams            = VM::getTop(vm);

   Vector<Component*> comps;

   // Reads components to extrude.

   // Single component.
   if( VM::isObject( vm, 1 ) )
   {
      Component* comp = (Component*)VM::toProxy( vm, 1 );
      if( comp && comp->dimension() == 2 ) comps.pushBack( comp );
   }
   else
   {
      // Many components.
      for( int i = 1; VM::geti( vm, 1, i ); ++i )
      {
         Component* comp = (Component*)VM::toProxy( vm, -1 );
         if( comp && comp->dimension() == 2 ) comps.pushBack( comp );
         VM::pop( vm );
      }
   }

   // Do we have component to extrude?
   if( comps.empty() ) return 0;

   Vec3f xyh    = VM::toVec3f( vm, 2 );
   float height = xyh.z;

   // Create extruded components.
   for( uint i = 0; i < comps.size(); ++i )
   {
      Component* comp = comps[i];

      // Create component.
      Component* c = context->_compositor->createComponent();
      comp->addComponent( c );

      // Create extruded boundary.
      RCP<BoundaryPolygon> srcPoly;
      comp->resolveBoundary();
      if( comp->boundary() )
      {
         if( comp->face() != -1 )
         {
            // Boundary is defined by a face of an 3D boundary.
            srcPoly = BoundaryPolygon::create( *comp->boundary(), comp->face() );
         }
         else
         {
            // Boundary is normal.
            srcPoly = BoundaryPolygon::create( *comp->boundary(), 0 );
         }
      }
      else
      {
         Mat4f m = comp->referential().toMatrix();
         srcPoly = BoundaryPolygon::create(
            AARectf( Vec2f(comp->size().x, comp->size().y) ), m
         );
      }
      RCP<BoundaryPolygon> dstPoly = BoundaryPolygon::create( *srcPoly, xyh, comp->referential() );
      c->boundary( Boundary::create( *srcPoly, *dstPoly ).ptr() );
      c->size( Vec3f( comp->size().x, comp->size().y, CGM::abs( height ) ) );

      // Position the referential at the corner of the boundary.
      Reff ref = comp->referential();
      if( height < 0.0f ) ref.translateLocal( Vec3f( 0.0f, 0.0f, height ) );
      c->referential( ref );

      // Reads attributes if defined else inherits parents ids.
      if( numParams > 2 )
      {
         readAttributes( vm, c );
      }
      else
      {
         c->addId( comp->ids() );
      }
      adjustOrientation( vm, c );
   }

   return 0;
}

//------------------------------------------------------------------------------
//! 
int shrinkVM( VMState* /*vm*/ )
{
   // TODO
   return 0;
}

//------------------------------------------------------------------------------
//! 
void solveConstraints(
   Component*              /*comp*/,
   Vector<ComponentGeom>&  geom,
   Vector<ConstraintGeom>& constraints
)
{
   // Contraints cleanup.
   // Merge overlapping repulsion constraints.
   for( uint c = 0; c < constraints.size(); )
   {
      if( constraints[c]._type != 1 )
      {
         ++c;
      }
      else
      {
         uint curC = c;
         for( ++c; c < constraints.size(); ++c )
         {
            // Contraints don't overlaps?
            if( constraints[c]._min > constraints[curC]._max ) break;
            if( constraints[c]._type == 1 )
            {
               constraints[c]._type = -1;
               constraints[curC]._max = CGM::max( constraints[curC]._max, constraints[c]._max );
            }
         }
      }
   }

   // Remove snapping constraint contained inside repulsion constraints.
   // TODO

   // first pass: remove planes from repulsion.
   uint curC = 0;
   for( uint c = 0; c < geom.size()-1; ++c )
   {
      float plane = geom[c]._endPos;
      // Find the starting constraints (first constraint with repulsion that its 
      // maximum is higher thant the current plane).
      for( ; curC < constraints.size(); ++curC )
      {
         if( constraints[curC]._type == 1 && constraints[curC]._max > plane ) break;
      }
      // No more contraints?
      if( curC >= constraints.size() ) break;

      float cmin = constraints[curC]._min;
      float cmax = constraints[curC]._max;

      // If the minimum is higher than the current plane: skip current component.
      if( cmin >= plane ) continue;

      // Solve constraint.
      // Can we move to the left?
      if( (c == 0 && cmin > 0.0f) || (c > 0 && cmin >= geom[c-1]._endPos) )
      {
         // Can we move to the right?
         if( cmax <= geom[c+1]._endPos )
         {
            // Select the nearest one.
            if( plane-cmin < cmax-plane )
            {
               geom[c]._endPos = cmin;
            }
            else
            {
               geom[c]._endPos = cmax;
            }
         }
         else
         {
            geom[c]._endPos = cmin;
         }
      }
      // Can we move to the right?
      else if( cmax <= geom[c+1]._endPos )
      {
         geom[c]._endPos = cmax;
      }
      // Nothing to do: we can't solve the constraint.
   }

   // Second pass: snapping.
   curC = 0;
   for( uint c = 0; c < geom.size()-1; ++c )
   {
      float plane = geom[c]._endPos;

      // Find search region for constraints.
      // FIXME: Constraints should resolved in order of their distance (i.e. the nearest first).
      //float rmin = (c == 0) ? 0.0f : (geom[c-1]._endPos+plane)*0.5f;
      //float rmax = (c == geom.size()-2) ? geom[c+1]._endPos : (geom[c+1]._endPos+plane)*0.5f;
      float rmin = (c == 0) ? 0.0f : geom[c-1]._endPos+CGConstf::epsilon(64);
      float rmax = (c == geom.size()-2) ? geom[c+1]._endPos : geom[c+1]._endPos-CGConstf::epsilon(64);

      // Find the constraint to apply.
      int ac = -1;
      float newPlane = CGConstf::infinity();
      for( uint sc = curC; sc < constraints.size(); ++sc )
      {
         if( constraints[sc]._type != 0 ) continue;

         float cmin = constraints[sc]._min;
         if( cmin >= rmax ) break;
         if( cmin >= rmin )
         {
            if( CGM::abs( cmin-plane ) < CGM::abs( newPlane-plane ) )
            {
               ac       = sc;
               newPlane = cmin;
            }
         }
         float cmax = constraints[sc]._max;
         if( cmax >= rmin && cmax < rmax )
         {
            if( CGM::abs( cmax-plane ) < CGM::abs( newPlane-plane ) )
            {
               ac       = sc;
               newPlane = cmax;
            }
         }
      }
      // Apply constraint.
      if( ac != -1 )
      {
         curC            = ac;
         geom[c]._endPos = newPlane;
      }
   }
}

//------------------------------------------------------------------------------
//! 
void applyConstraints( 
   Component*                          comp, 
   int                                 axis, 
   Vector<ComponentGeom>&              geom, 
   const Vector<ComponentConstraint*>& constraints
)
{
   // We need at least 2 components to apply constraints, since the first and last plane
   // can't be moved.
   if( geom.size() < 2 ) return;

   static const int boxc[6][4] = {{0,2,4,6},{1,3,5,7},{0,1,4,5},{2,3,6,7},{0,1,2,3},{4,5,6,7}};
   // 1. compute projected constraints.
   Mat4f m = comp->referential().globalToLocal();

   Vec3f pos = comp->size()/2.0f;
   Vec3f dir(0.0f);
   pos( axis ) = 0.0f;
   dir( axis ) = 1.0f;

   Rayf ray( pos, dir );
   ray = comp->referential().toMatrix() * ray;

   Vector<ConstraintGeom> cgeom( constraints.size() );
   for( uint i = 0; i < cgeom.size(); ++i )
   {
      float min    = CGConstf::infinity();
      float max    = 0;
      Component* c = constraints[i]->component();
      Vec3f offset = constraints[i]->offset();
      c->resolveBoundary();
      int face     = c->dimension() == 3 ? constraints[i]->face() : c->face();

      switch( constraints[i]->type() & 0x3 )
      {
         case ComponentConstraint::PLANE:
         {
            Planef plane;
            if( c->boundary() )
            {
                plane = c->boundary()->face( face ).plane();
            }
            else
            {
               if( c->dimension() == 2 )
               {
                  plane = Planef( c->referential().orientation().getAxisZ(), c->referential().position() );
               }
               else
               {
                  switch( constraints[i]->face() )
                  {
                     case 0: pos = Vec3f(0.0f); dir = Vec3f( -1.0f,  0.0f,  0.0f ); break;
                     case 1: pos = c->size();   dir = Vec3f(  1.0f,  0.0f,  0.0f ); break;
                     case 2: pos = Vec3f(0.0f); dir = Vec3f(  0.0f, -1.0f,  0.0f ); break;
                     case 3: pos = c->size();   dir = Vec3f(  0.0f,  1.0f,  0.0f ); break;
                     case 4: pos = Vec3f(0.0f); dir = Vec3f(  0.0f,  0.0f, -1.0f ); break;
                     case 5: pos = c->size();   dir = Vec3f(  0.0f,  0.0f,  1.0f ); break;
                     default: 
                        pos = Vec3f(0.0f); dir = Vec3f( -1.0f,  0.0f,  0.0f ); break;
                  }
                  Mat4f m2 = c->referential().toMatrix();
                  dir      = m2 ^ dir;
                  pos      = m2 * pos;
                  plane    = Planef( dir, pos );
               }
            }
            float distance = CGConstf::infinity();
            if( Intersector::trace( plane, ray, distance ) )
            {
               min = distance;
               max = distance;
            }
         }  break;
         case ComponentConstraint::VOLUME:
            if( c->boundary() )
            {
               for( uint v = 0; v < c->boundary()->numVertices(); ++v )
               {
                  Vec3f p = m * c->boundary()->vertex(v);
                  min = CGM::min( min, p(axis) );
                  max = CGM::max( max, p(axis) );
               }
               // Offset.
               Mat4f m2   = m * c->referential().localToGlobal();
               float poff = dot( CGM::abs(m2.col3(axis)), offset );
               min -= poff;
               max += poff;
            }
            else
            {
               Mat4f m2 = m * c->referential().localToGlobal();
               AABBoxf b( -offset, c->size()+offset );
               for( uint v = 0; v < 8; ++v )
               {
                  Vec3f p = m2 * b.corner(v);
                  min = CGM::min( min, p(axis) );
                  max = CGM::max( max, p(axis) );
               }
            }
            break;
         case ComponentConstraint::POLYGON:
            if( c->boundary() )
            {
               for( uint v = 0; v < c->boundary()->numVertices(face); ++v )
               {
                  Vec3f p = m * c->boundary()->vertex( face, v );
                  min = CGM::min( min, p(axis) );
                  max = CGM::max( max, p(axis) );
               }
               // Offset.
               Mat4f m2   = m * c->referential().localToGlobal();
               float poff = dot( CGM::abs(m2.col3(axis)), offset );
               min -= poff;
               max += poff;
            }
            else
            {
               Mat4f m2 = m * c->referential().localToGlobal();
               AABBoxf b( -offset, c->size()+offset );
               int face = c->dimension() == 2 ? 5 : constraints[i]->face();
               for( uint v = 0; v < 4; ++v )
               {
                  Vec3f p = m2 * b.corner( boxc[face][v] );
                  min = CGM::min( min, p(axis) );
                  max = CGM::max( max, p(axis) );
               }
            }
            break;
      }

      cgeom[i]._min  = min;
      cgeom[i]._max  = max;
      cgeom[i]._type = constraints[i]->type() & ComponentConstraint::REPULSE ? 1 : 0;
   }

   // Sort constraints by min.
   std::sort( cgeom.begin(), cgeom.end(), leq );

   // 2. solve.
   solveConstraints( comp, geom, cgeom );
}

//------------------------------------------------------------------------------
//! 
int sliceVM( VMState* vm )
{
   GeometryContext* context = getContext(vm);
   Component* comp          = (Component*)VM::toProxy( vm, 1 );
   int numParams            = VM::getTop(vm);

   if( comp == 0 )
   {
      StdErr << "No component was specified for slice.\n";
      return 0;
   }

   // Find split axis.
   int axis;
   switch( VM::toCString( vm, 2 )[0] )
   {
      case 'X': axis = 0; break;
      case 'Y': axis = 1; break;
      case 'Z': axis = 2; break;
      default:  axis = 0;
   }

   // Only accepts Z axis in 3D.
   if( comp->dimension() <= axis )
   {
      StdErr << "Axis not supported for this dimension.\n";
      axis = comp->dimension()-1;
   }

   // Leftover distribution mode.
   int mode = 1;
   if( numParams >= 4 && VM::isNumber( vm, 4 ) ) mode = VM::toInt( vm, 4 );

   // Constraints.
   Vector<ComponentConstraint*> constraints;
   if( numParams > 3 && VM::isTable( vm, -1 ) )
   {
      for( int i = 1; VM::geti( vm, -1, i ); ++i )
      {
         constraints.pushBack( (ComponentConstraint*)VM::toPtr( vm, -1 ) );
         VM::pop( vm );
      }
   }

   // Compute number of children and their size.
   // Default children size.
   bool hasLength = false;
   float length   = 1.0f;
   if( VM::geti( vm, 3, 1 ) )
   {
      length    = (float)VM::toNumber( vm, -1 );
      hasLength = true;
      VM::pop( vm );
   }

   Vector<ComponentGeom> geom;
   Vec3f size = comp->size();

   // Slicing by constraint or by size?
   if( constraints.empty() || hasLength )
   {
      // Slicing by size.

      // Compute left over and number of children.
      int numComps      = CGM::max( 1, CGM::floori( (size(axis)+1e-6) / length ) );
      float leftOver    = size(axis) - length*numComps;
      float firstLength = length;
      float lastLength  = length;
      size(axis)        = length;

      switch( mode )
      {
         case -1:
            firstLength += leftOver;
            break;
         case 0:
            firstLength += leftOver*0.5f;
            lastLength  += leftOver*0.5f;
            break;
         case 1:
            lastLength  += leftOver;
            break;
         case 2:
            firstLength = lastLength = length = comp->size()(axis)/(float)numComps;
            break;
         default:
            firstLength += leftOver;
      }

      if( numComps == 1 ) firstLength = comp->size()(axis);

      // 2. Figure sizes and positions.
      geom.resize( numComps );
      float pos = 0.0f;
      for( int i = 0; i < numComps; ++i )
      {
         if( i == 0 )               pos += firstLength;
         else if( i == numComps-1 ) pos += lastLength;
         else                       pos += length;
         geom[i]._endPos = pos;
         geom[i]._mode   = 0;
      }

      //  Apply constraints.
      if( !constraints.empty() )
      {
         applyConstraints( comp, axis, geom, constraints );
      }
   }
   else
   {
      // Slicing by constraints.

      // Compute intersections.
      Vec3f pos = size/2.0f;
      Vec3f dir(0.0f);
      pos( axis ) = 0.0f;
      dir( axis ) = 1.0f;

      Rayf ray( pos, dir );
      ray = comp->referential().toMatrix() * ray;

      Vector<float> distances;
      const float delta = 1.0f/1024.0f;
      for( uint i = 0; i < constraints.size(); ++i )
      {
         if( constraints[i]->type() == ComponentConstraint::PLANE )
         {
            // Retrieve constraint plane.
            Planef plane;
            Component* c = constraints[i]->component();
            c->resolveBoundary();
            if( c->boundary() )
            {
               int face = c->dimension() == 3 ? constraints[i]->face() : c->face();
               plane = c->boundary()->face( face ).plane();
            }
            else
            {
               if( c->dimension() == 2 )
               {
                  plane = Planef( c->referential().orientation().getAxisZ(), c->referential().position() );
               }
               else
               {
                  switch( constraints[i]->face() )
                  {
                     case 0: pos = Vec3f(0.0f); dir = Vec3f( -1.0f,  0.0f,  0.0f ); break;
                     case 1: pos = c->size();   dir = Vec3f(  1.0f,  0.0f,  0.0f ); break;
                     case 2: pos = Vec3f(0.0f); dir = Vec3f(  0.0f, -1.0f,  0.0f ); break;
                     case 3: pos = c->size();   dir = Vec3f(  0.0f,  1.0f,  0.0f ); break;
                     case 4: pos = Vec3f(0.0f); dir = Vec3f(  0.0f,  0.0f, -1.0f ); break;
                     case 5: pos = c->size();   dir = Vec3f(  0.0f,  0.0f,  1.0f ); break;
                     default: 
                        pos = Vec3f(0.0f); dir = Vec3f( -1.0f,  0.0f,  0.0f ); break;
                  }
                  Mat4f m = c->referential().toMatrix();
                  dir     = m ^ dir;
                  pos     = m * pos;
                  plane   = Planef( dir, pos );
               }
            }

            // Intersect plane with ray.
            float distance = size(axis)-delta;
            if( Intersector::trace( plane, ray, distance ) )
            {
               distances.pushBack( distance );
            }
         }
      }
      distances.pushBack( size(axis) );

      // Order and remove duplicates.
      std::sort( distances.begin(), distances.end() );
      int num       = 0;
      float curDist = 0.0f;
      for( uint i = 0; i < distances.size(); ++i )
      {
         if( distances[i] > curDist+delta )
         {
            distances[num++] = distances[i];
            curDist = distances[i];
         }
      }

      // Create geom size.
      geom.resize( num );
      for( int i = 0; i < num; ++i )
      {
         geom[i]._endPos = distances[i];
      }
   }

   // Create components.

   // Push attributes table to the top of the stack.
   VM::pushValue( vm, 3 );
   Reff ref( Reff::identity() );
   for( uint i = 0; i < geom.size(); ++i )
   {
      size(axis) = geom[i]._endPos - ref.position()(axis);

      if( size(axis) > 0.0f )
      {
         Component* c = context->_compositor->createComponent();
         c->inherit( comp, ref, size );
         readAttributes( vm, c );
         adjustOrientation( vm, c );
         // FIXME: this is a HACK.
         c->resolveBoundary();
         if( c->boundary() && c->boundary()->numVertices() == 0 )
         {
            StdErr << "component remove\n";
            comp->removeComponent( c );
            context->_compositor->removeComponent( c );
         }
      }
      ref.position()(axis) = geom[i]._endPos;
   }

   // Remove attributes table.
   VM::pop( vm );

   return 0;
}

//------------------------------------------------------------------------------
//! 
int splitVM( VMState* vm )
{
   GeometryContext* context = getContext(vm);
   Component* comp          = (Component*)VM::toProxy( vm, 1 );
   int numParams            = VM::getTop( vm );

   if( comp == 0 )
   {
      StdErr << "No component was specified for split.\n";
      return 0;
   }

   // Find split axis.
   int axis;
   switch( VM::toCString( vm, 2 )[0] )
   {
      case 'X': axis = 0; break;
      case 'Y': axis = 1; break;
      case 'Z': axis = 2; break;
      default:  axis = 0;
   }

   // Only accepts Z axis in 3D.
   if( comp->dimension() <= axis )
   {
      StdErr << "Axis not supported for this dimension.\n";
      axis = comp->dimension()-1;
   }

   // Read constraints.
   Vector<ComponentConstraint*> constraints;
   if( VM::isTable( vm, -1 ) )
   {
      // Do we have contraints?
      if( VM::geti( vm, -1, 1 ) )
      {
         if( VM::isObject( vm, -1 ) )
         {
            constraints.pushBack( (ComponentConstraint*)VM::toPtr( vm, -1 ) );
            VM::pop( vm );
            for( int i = 2; VM::geti( vm, -1, i ); ++i )
            {
               constraints.pushBack( (ComponentConstraint*)VM::toPtr( vm, -1 ) );
               VM::pop( vm );
            }
            --numParams;
         }
         else
         {
            VM::pop( vm );
         }
      }
   }

   // Get number of requested components.
   int numComps = numParams-2;
   Vector<ComponentGeom> geom( numComps );

   // 1. First pass on shapes.
   // Compute absolute and relative size.
   //bool haveConstraint = false;
   float totalRel      = 0.0f;
   float totalAbs      = 0.0f;
   for( int i = 0; i < numComps; ++i )
   {
      geom[i]._rel = 0.0f;
      geom[i]._abs = 0;

      // Relative size.
      if( VM::get( vm, i+3, "rel", geom[i]._rel ) )  totalRel += geom[i]._rel;

      // Absolute size.
      if( VM::geti( vm, i+3, 1 ) )
      {
         if( VM::isNumber( vm, -1 ) )
         {
            geom[i]._abs = VM::toFloat( vm, -1 );
            totalAbs    += geom[i]._abs;
         }
         VM::pop( vm );
      }

      // Component snapping constraints mode.
      geom[i]._mode = 0; // Face snapping.
      VM::get( vm, i+3, "mode", geom[i]._mode );
   }

   // 2. Figure size and position.
   Vec3f size        = comp->size();
   float relSizeLeft = CGM::max( 0.0f, size(axis) - totalAbs );
   float meanRelSize = totalRel > 0.0f ? relSizeLeft / totalRel : 0.0f;
   float pos         = 0.0f;
   for( int i = 0; i < numComps; ++i )
   {
      geom[i]._endPos = pos + geom[i]._abs + meanRelSize * geom[i]._rel;
      pos = geom[i]._endPos;
   }

   // 3. Apply constraints.
   if( !constraints.empty() )
   {
      applyConstraints( comp, axis, geom, constraints );
   }

   // 4. Second pass on shapes.
   // Create compenents.
   Reff ref( Reff::identity() );
   for( int i = 0; i < numComps; ++i )
   {
      // Base shape size.
      size(axis) = geom[i]._endPos - ref.position()(axis);

      // Create shape.
      if( size(axis) > 0.0f )
      {
         // Push attributes table to the top of the stack.
         VM::pushValue( vm, i+3 );

         Component* c = context->_compositor->createComponent();
         c->inherit( comp, ref, size );
         readAttributes( vm, c );
         adjustOrientation( vm, c );
         // FIXME: this is a HACK.
         c->resolveBoundary();
         if( c->boundary() && c->boundary()->numVertices() == 0 )
         {
            StdErr << "component remove\n";
            comp->removeComponent( c );
            context->_compositor->removeComponent( c );
         }

         // Remove attributes table.
         VM::pop( vm );
      }
      ref.position()(axis) = geom[i]._endPos;
   }

   return 0;
}

//------------------------------------------------------------------------------
//! 
int alternateVM( VMState* vm )
{
   GeometryContext* context = getContext(vm);
   Component* comp          = (Component*)VM::toProxy( vm, 1 );
   int numParams            = VM::getTop(vm);

   if( comp == 0 )
   {
      StdErr << "No component was specified for slice.\n";
      return 0;
   }

   // Find split axis.
   int axis;
   switch( VM::toCString( vm, 2 )[0] )
   {
      case 'X': axis = 0; break;
      case 'Y': axis = 1; break;
      case 'Z': axis = 2; break;
      default:  axis = 0;
   }

   // Only accepts Z axis in 3D.
   if( comp->dimension() <= axis )
   {
      StdErr << "Axis not supported for this dimension.\n";
      axis = comp->dimension()-1;
   }

   // Leftover distribution mode.
   int mode = 2;
   if( numParams >= 5 && VM::isNumber( vm, 5 ) ) mode = VM::toInt( vm, 5 );

   // Compute number of children and their size.
   // Default children size.
   float a_length = 1.0f;
   if( VM::geti( vm, 3, 1 ) )
   {
      a_length = (float)VM::toNumber( vm, -1 );
      VM::pop( vm );
   }
   float b_length = 1.0f;
   if( VM::geti( vm, 4, 1 ) )
   {
      b_length = (float)VM::toNumber( vm, -1 );
      VM::pop( vm );
   }
   float ab_length = a_length + b_length;

   Vector<ComponentGeom> geom;
   Vec3f size = comp->size();

   // Slicing by size.

   // Results can be:
   //   |      A      |    if A > L/2 (can't even fit 2xA).
   //   |A           A|    if A <= L/2 but A+B+A > L (can't fit the B inside 2xA).
   //   |A     B     A|    from here on, all have the form:
   //   |A  B  A  B  A|       A + n*(A+B)
   //   |A B A B A B A|

   // Compute left over and number of children.
   int   numAs;
   int   numBs;
   float leftOver;
   float length = size(axis);
   float a_length_2 = a_length * 2.0f;
   if( a_length_2 > length || length == 0.0f )
   {
      // Can't even fit A+A, so just A.
      numAs = 1;
      numBs = 0;
      leftOver = length - a_length;
   }
   else
   if( a_length_2 + b_length > length )
   {
      // Can fit A+A, but not A+B+A.
      numAs = 2;
      numBs = 0;
      leftOver = length - a_length_2;
   }
   else
   {
      // Can fit A+n*(A+B).
      float l_minus_a = length - a_length;
      numBs = CGM::floori( l_minus_a / ab_length );
      numAs = numBs + 1;
      leftOver = l_minus_a - ab_length*numBs;
   }

   //StdErr << length << " = " << numAs << " * " << a_length << " + " << numBs << " * " << b_length << " + " << leftOver << nl;


   float firstLength = a_length;
   //float lastLength  = a_length;

   switch( mode )
   {
      case -1:
         // Give to first.
         firstLength += leftOver;
         break;
      case 0:
         // Split across first and last.
         leftOver *= 0.5f;
         firstLength += leftOver;
         //lastLength  += leftOver;
         break;
      case 1:
         // Give to last.
         //lastLength  += leftOver;
         break;
      case 2:
         // Spread across all.
         leftOver /= (numAs+numBs);
         firstLength += leftOver;
         //lastLength  += leftOver;
         a_length += leftOver;
         b_length += leftOver;
         break;
      case 3:
         // Spread across As.
         leftOver /= numAs;
         firstLength += leftOver;
         //lastLength  += leftOver;
         a_length += leftOver;
         break;
      case 4:
         // Spread across Bs.
         leftOver /= numBs;
         b_length += leftOver;
         break;
      default:
         firstLength += leftOver;
   }

   if( numAs == 1 ) firstLength = length;

   // 2. Figure sizes and positions.
   geom.resize( numAs + numBs );
   float pos = 0.0f;
   size_t lastIndex = geom.size() - 1;
   for( size_t i = 0; i <= lastIndex; ++i )
   {
      if( i == 0 )              pos += firstLength;
      else if( i == lastIndex ) pos = length; //pos += lastLength; might not be precise enough.
      else
      {
         bool useB = ((i & 0x01) == 0x01) && (numBs != 0);
         if( useB )  pos += b_length;
         else        pos += a_length;
      }
      geom[i]._endPos = pos;
      geom[i]._mode   = 0;
   }

   // Create components.

   Reff ref( Reff::identity() );
   // Push A's attributes table to the top of the stack.
   VM::pushValue( vm, 3 );
   for( uint i = 0; i < geom.size(); ++i )
   {
      bool useB = ((i & 0x01) == 0x01) && (numBs != 0);
      if( useB )  VM::pushValue( vm, 4 ); // Push B's attributes table on top of A's.
      size(axis) = geom[i]._endPos - ref.position()(axis);

      if( size(axis) > 0.0f )
      {
         Component* c = context->_compositor->createComponent();
         c->inherit( comp, ref, size );
         readAttributes( vm, c );
         adjustOrientation( vm, c );
         // FIXME: this is a HACK.
         c->resolveBoundary();
         if( c->boundary() && c->boundary()->numVertices() == 0 )
         {
            StdErr << "component remove\n";
            comp->removeComponent( c );
            context->_compositor->removeComponent( c );
         }
      }
      ref.position()(axis) = geom[i]._endPos;
      if( useB )  VM::pop( vm ); // Remove B's attributes table.
   }

   // Remove A's attributes table.
   VM::pop( vm );

   return 0;
}

//------------------------------------------------------------------------------
//! 
int subtractVM( VMState* vm )
{
   GeometryContext* context = getContext(vm);
   Component* comp = (Component*)VM::toProxy( vm, 1 );

   if( comp == 0 )
   {
      StdErr << "No component was specified for subtract.\n";
      return 0;
   }

   // Components to subtract.
   Vector< Component* > scomps;

   // One component?
   if( VM::isObject( vm, 2 ) )
   {
      scomps.pushBack( (Component*)VM::toProxy( vm, 2 ) );
   }
   // A list of component?
   else if( VM::isTable( vm, 2 ) )
   {
      for( int i = 1; VM::geti( vm, 2, i ); ++i )
      {
         scomps.pushBack( (Component*)VM::toProxy( vm, -1 ) );
         VM::pop( vm );
      }
   }
   // An attribute to compare.
   else if( VM::isString( vm, 2 ) )
   {
      ConstString key( VM::toCString( vm, 2 ) );
      const Variant& val = comp->getAttribute( key );
      float cval = val.type() == Variant::FLOAT ? val.getFloat() : -CGConstf::infinity();

      // TODO: optimize with hgrid.
      // Find component to subtract.
      Compositor::Iterator iter =  context->_compositor->iterator();
      for( ; iter.valid(); ++iter )
      {
         const Variant& val2 = (*iter)->getAttribute( key );
         if( (val2.type() == Variant::FLOAT) && val2.getFloat() > cval ) scomps.pushBack( *iter );
      }
   }
   else
   {
      StdErr << "Invalid parameter for subtraction\n";
      return 0;
   }

   // Create new component.
   Component* c = context->_compositor->createComponent();
   comp->addComponent(c);
   readAttributes( vm,c );

   // Create new boundary.
   c->boundary( Boundary::subtract( comp, scomps ).ptr() );

   // Size and referential.
   Reff ref = Reff::identity();
   Vec3f size;
   c->boundary()->computeRefAndSize( comp->referential(), ref, size );
   c->referential( ref );
   c->size( size );
   adjustOrientation( vm, c );

   return 0;
}

//------------------------------------------------------------------------------
//!
int pickVM( VMState* vm )
{
   Compositor* comp = getContext(vm)->_compositor;

   // Get number of elements in table.
   int size = VM::getTableSize( vm, 1 );

   // Choose a random key.
   int key;
   if( VM::getTop( vm ) == 1 )
   {
      // Equally distributed keys.
      key = (comp->rng().getUInt() % size)+1;
   }
   else
   {
      // Weighted keys.
      // Compute normalization factor (in doubles, since that is what the RNGs use).
      double total = 0.0;
      int wSize = VM::getTableSize( vm, 2 );
      if( size < wSize )  wSize = size;
      for( int i = 1; i <= wSize; ++i )
      {
         VM::geti( vm, 2, i );
         total += VM::toNumber( vm, -1 );
         VM::pop( vm );
      }
      // Generate a random number.
      double r = comp->rng()() * total;
      // Retrieve the associated key.
      total = 0.0;
      for( key = 1; key <= wSize; ++key )
      {
         VM::geti( vm, 2, key );
         total += VM::toNumber( vm, -1 );
         VM::pop( vm );
         if( total > r )  break; // Favor the latter when hitting boundary cases.
      }
   }

   // Return value attached to the selected key.
   VM::geti( vm, 1, key );

   return 1;
}

//------------------------------------------------------------------------------
//!
int randomVM( VMState* vm )
{
   Compositor* comp = getContext(vm)->_compositor;
   switch( VM::getTop(vm) )
   {
      case 0:
      {
         // No parameters: Generate a number in [0..1]
         VM::push( vm, comp->rng()( UniformFloat( 0.0f, 1.0f ) ) );
         return 1;
      }
      case 1:
      {
         // One parameter: Generate a number in [0..p1]
         float p1 = (float)VM::toNumber( vm, 1 );
         VM::push( vm, comp->rng()( UniformFloat( 0.0f, p1 ) ) );
         return 1;
      }
      case 2:
      {
         // Two parameters: Generate a number in [p1..p2]
         float p1 = (float)VM::toNumber( vm, 1 );
         float p2 = (float)VM::toNumber( vm, 2 );
         VM::push( vm, comp->rng()( UniformFloat( p1, p2 ) ) );
         return 1;
      }
      default:
      {
         return 0;
      }
   }
}

//------------------------------------------------------------------------------
//!
int seedVM( VMState* vm )
{
   Compositor* comp = getContext(vm)->_compositor;
   comp->rng().seed( VM::toUInt( vm, -1 ) );
   return 0;
}


//------------------------------------------------------------------------------
//! 
int outputVM( VMState* vm )
{
   Component* comp = (Component*)VM::toProxy( vm, 1 );

   if( comp == 0 )
   {
      StdErr << "No component was specified for output.\n";
      return 0;
   }
   comp->dump();
   return 0;
}

//------------------------------------------------------------------------------
//! 
const VM::Reg funcs[] = {
   // Creation.
   { "component",        comp_create        },
   { "region",           region_create      },
   { "planeConstraint",  pconstraint_create },
   { "faceConstraint",   fconstraint_create },
   { "volumeConstraint", vconstraint_create },
   // Query.
   { "query",            queryVM            },
   { "fquery",           faceQueryVM        },
   { "rquery",           regionQueryVM      },
   { "nquery",           neighborQueryVM    },
   { "occlusion",        occlusionVM        },
   { "queryBegin",       queryBeginVM       },
   { "queryEnd",         queryEndVM         },
   // IDs.
   { "hasID",            hasIDVM            },
   { "hasParentID",      hasParentIDVM      },
   { "hasFaceID",        hasFaceIDVM        },
   { "faceID",           faceIDVM           },
   // Connection.
   { "connect",          connectVM          },
   { "disconnect",       disconnectVM       },
   { "move",             moveVM             },
   // Operation.
   { "extrude",          extrudeVM          },
   { "intersect",        intersectVM        },
   { "merge",            mergeVM            },
   { "roof",             roofVM             },
   { "shrink",           shrinkVM           },
   { "slice",            sliceVM            },
   { "split",            splitVM            },
   { "alternate",        alternateVM        },
   { "subtract",         subtractVM         },
   // Random.
   { "pick",             pickVM             },
   { "random",           randomVM           },
   { "seed",             seedVM             },
   // Debugging.
   { "output",           outputVM           },
   { 0, 0 }
};

//------------------------------------------------------------------------------
//! 
void initVM( VMState* vm, uint /*mask*/ )
{
   VM::registerFunctions( vm, "_G", funcs );

   // Constraint metatable.
   VM::newMetaTable( vm, _constraint_str );
   VM::pop( vm );
}

UNNAMESPACE_END


NAMESPACE_BEGIN

/*==============================================================================
   CLASS Region
==============================================================================*/

//------------------------------------------------------------------------------
//! 
const char*
Region::meta() const
{
   return _region_str;
}

//------------------------------------------------------------------------------
//! 
Reff 
Region::connection( RNG_WELL& rng ) const
{
   Reff ref;
   ref.scale(1.0f);

   // Generate position.
   ref.position(
      rng( UniformFloat( _box.slabX()(0), _box.slabX()(1) ) ),
      rng( UniformFloat( _box.slabY()(0), _box.slabY()(1) ) ),
      rng( UniformFloat( _box.slabZ()(0), _box.slabZ()(1) ) )
   );

   // Generate orientation.
   Vec3f angles(
      rng( UniformFloat( _oriRange[0].x, _oriRange[0].y ) ),
      rng( UniformFloat( _oriRange[1].x, _oriRange[1].y ) ),
      rng( UniformFloat( _oriRange[2].x, _oriRange[2].y ) )
   );
   ref.orientation( _eulerQuat * Quatf::eulerXYZ( angles ) );

   // Transform into region space.
   return _ref * ref;
}

//------------------------------------------------------------------------------
//! 
Reff 
Region::connection( RNG_WELL& rng, const Vec3f& pos ) const
{
   Reff ref;
   ref.scale(1.0f);

   // Set position.
   ref.position( pos );

   // Generate orientation.
   Vec3f angles(
      rng( UniformFloat( _oriRange[0].x, _oriRange[0].y ) ),
      rng( UniformFloat( _oriRange[1].x, _oriRange[1].y ) ),
      rng( UniformFloat( _oriRange[2].x, _oriRange[2].y ) )
   );
   ref.orientation( _eulerQuat * Quatf::eulerXYZ( angles ) );

   // Transform into region space.
   return _ref * ref;
}

//------------------------------------------------------------------------------
//! 
Reff 
Region::connection( const Vec3f& pos ) const
{
   Reff ref;
   ref.scale(1.0f);

   // Set position.
   ref.position( pos );

   // Set orientation.
   ref.orientation( Quatf::identity() );

   // Transform into region space.
   return _ref * ref;
}

//------------------------------------------------------------------------------
//! 
Reff 
Region::connection( const Vec3f& pos, const Quatf& orient ) const
{
   Reff ref;
   ref.scale(1.0f);

   // Set position.
   ref.position( pos );

   // Set orientation.
   ref.orientation( _eulerQuat * orient );

   // Transform into region space.
   return _ref * ref;
}


/*==============================================================================
   CLASS Component
==============================================================================*/

//------------------------------------------------------------------------------
//! 
void
Component::initialize()
{
   VMRegistry::add( _comp_str, NULL, comp_get, comp_set, VM_CAT_WORLD | VM_CAT_GEOM );
   VMRegistry::add( _region_str, NULL, region_get, region_set, VM_CAT_WORLD | VM_CAT_GEOM );
   VMRegistry::add( initVM, VM_CAT_WORLD | VM_CAT_GEOM );
   VMRegistry::add( _iter_str, iter_gc, NULL, NULL, VM_CAT_WORLD | VM_CAT_GEOM );
}

//------------------------------------------------------------------------------
//! 
Component::Component() :
   _parent(0), _children(0), _sibling(0),
   _ref( Reff::identity() ),
   _size(1.0f), _dimension(3), _face(-1), _bdirty(true)
{
}

//------------------------------------------------------------------------------
//! 
Component::~Component()
{
}

//------------------------------------------------------------------------------
//! 
const char*
Component::meta() const
{
   return _comp_str;
}

//------------------------------------------------------------------------------
//! 
void
Component::addId( const char* id )
{
   _ids.pushBack( id );
}

//------------------------------------------------------------------------------
//! 
void
Component::addId( const Vector<ConstString>& ids )
{
   for( uint i = 0; i < ids.size(); ++i )
   {
      _ids.pushBack( ids[i] );
   }
}

//------------------------------------------------------------------------------
//! 
void
Component::removeId( const char* id )
{
   _ids.removeSwap( id );
}

//------------------------------------------------------------------------------
//! 
void
Component::addComponent( Component* comp )
{
   if( comp->_parent ) comp->_parent->removeComponent( comp );
   comp->_parent  = this;
   comp->_sibling = _children;
   _children      = comp;
}

//------------------------------------------------------------------------------
//! 
void 
Component::removeComponent( Component* comp )
{
   if( comp->_parent != this ) return;
   if( _children == comp )
   {
      _children = comp->_sibling;
      comp->_sibling  = 0;
      return;
   }
   Component* cur = _children;
   while( cur->_sibling != comp ) cur = cur->_sibling;
   cur->_sibling  = comp->_sibling;
   comp->_sibling = 0;
}

//------------------------------------------------------------------------------
//! 
void 
Component::inherit( Component* parent, const Reff& ref, const Vec3f& size )
{
   parent->addComponent( this );
   _bdirty    = true;
   _face      = parent->_face;
   _boundary  = parent->boundary();
   _size      = size;
   _dimension = parent->dimension();
   _ref       = parent->referential() * ref;
}

//------------------------------------------------------------------------------
//! 
bool
Component::isParent( Component* parent )
{
   for( Component* p = _parent; p; p = p->_parent )
   {
      if( p == parent ) return true;
   }
   return false;
}

//------------------------------------------------------------------------------
//! 
void 
Component::transform( const Reff& xform )
{
   _ref = xform * _ref;

   if( _boundary.isValid() ) _boundary->transform( xform );

   for( Component* c = _children; c; c = c->_sibling )
   {
      c->transform( xform );
   }
}

//------------------------------------------------------------------------------
//! 
void
Component::resolveBoundary()
{
   // Do we need to resolve the boundary?
   if( _boundary.isNull() || (!_bdirty) ) return;

   if( _dimension == 3 )
   {
      boundary( Boundary::create( *_boundary, AABBoxf( Vec3f(0.0f), _size ), _ref ).ptr() );
   }
   else
   {
      if( _face != -1 )
      {
         // Create boundary with face boundary + bb.
         boundary( Boundary::create( *_boundary, _face, AARectf( Vec2f( _size.x, _size.y ) ), _ref ).ptr() );
      }
      else
      {
         // Clip boundary with bb.
         boundary( Boundary::create( *_boundary, AARectf( Vec2f( _size.x, _size.y ) ), _ref ).ptr() );
      }
   }
}

//------------------------------------------------------------------------------
//! 
void 
Component::boundary( Boundary* b )
{
   _bdirty   = false;
   _boundary = b;
   _face     = -1;
}

//------------------------------------------------------------------------------
//! 
void
Component::boundary( int face )
{
   // We need a parent since we are defining our boundary with one of its face.
   if( !_parent || (_parent->dimension() == 2) ) return;

   _face      = face;
   _dimension = 2;
   _bdirty    = false;

   // We have a complexe boundary.
   if( _parent->boundary() )
   {
      _parent->resolveBoundary();
      _boundary = _parent->boundary();
      _boundary->computeFaceRefAndSize( face, _ref, _size );
   }
   else
   {
      // Bounding box boundary.
      Reff ref = Reff::identity();
      switch( face )
      {
         case 0:
            _size = Vec3f( _parent->size().z, _parent->size().y, 0.0f );
            ref.orientation( Quatf::axisCir( Vec3f(0.0f,1.0f,0.0f), 0.75f ) );
            break;
         case 1:
            _size = Vec3f( _parent->size().z, _parent->size().y, 0.0f );
            ref.position( _parent->size().x, 0.0f, _parent->size().z );
            ref.orientation( Quatf::axisCir( Vec3f(0.0f,1.0f,0.0f), 0.25f ) );
            break;
         case 2:
            _size = Vec3f( _parent->size().x, _parent->size().z, 0.0f );
            ref.orientation( Quatf::axisCir( Vec3f(1.0f,0.0f,0.0f), 0.25f ) );
            break;
         case 3:
            _size = Vec3f( _parent->size().x, _parent->size().z, 0.0f );
            ref.position( _parent->size().x, _parent->size().y, 0.0f );
            ref.orientation( Quatf::axes( Vec3f(-1.0f,0.0f,0.0f), Vec3f(0.0f,0.0f,1.0f), Vec3f(0.0f,1.0f,0.0f) ) );
            break;
         case 4:
            _size = Vec3f( _parent->size().x, _parent->size().y, 0.0f );
            ref.position( _parent->size().x, 0.0f, 0.0f );
            ref.orientation( Quatf::axisCir( Vec3f(0.0f,1.0f,0.0f), 0.5f ) );
            break;
         case 5:
            _size = Vec3f( _parent->size().x, _parent->size().y, 0.0f );
            ref.position( 0.0f, 0.0f, _parent->size().z );
            break;
         default:
            _size = Vec3f(0.0);
            break;
      }
      _ref = _parent->referential() * ref;
   }
}

//------------------------------------------------------------------------------
//! 
Region* 
Component::createRegion()
{
   Region* r = new Region();
   r->component( this );
   _regions.pushBack( r );
   return r;
}

//------------------------------------------------------------------------------
//! 
void 
Component::removeRegion( Region* r )
{
   // FIXME: do we need to disconnect all sibling connected to those region?
   _regions.removeSwap( r );
}

//------------------------------------------------------------------------------
//! 
void 
Component::removeAllRegions()
{
   // FIXME: do we need to disconnect all sibling connected to those region?
   _regions.clear();
}

//------------------------------------------------------------------------------
//! Returns the attribute with inheritance.
const Variant&
Component::getAttribute( const ConstString& key ) const
{
   // Iterative code instead of recursive.
   const Component* cur = this;
   do
   {
      const Variant& v = cur->_attributes.isValid() ? cur->_attributes->get( key ) : Variant::null();
      if( !v.isNil() )  return v;
      cur = cur->parent();
   } while( cur );
   return Variant::null();
}

//------------------------------------------------------------------------------
//! Returns the attribute without inheritance.
const Variant&
Component::getLocalAttribute( const ConstString& key ) const
{
   return _attributes.isValid() ? _attributes->get( key ) : Variant::null();
}

//------------------------------------------------------------------------------
//! 
void
Component::dump() const
{
   StdErr << "Component\n";
   StdErr << "  ids: ";
   for( uint i = 0; i < ids().size(); ++i ) StdErr << id(i).cstr() << " ";

   Boundary* b = boundary();

   StdErr << "\n";
   StdErr << "  dim: "      << dimension() << "\n";
   StdErr << "  size: "     << size() << "\n";
   StdErr << "  ref: "      << referential() << "\n";
   StdErr << "  boundary: " << (b ? "yes" : "no") << "\n";
   if( b ) b->print( StdErr ); 
}

/*==============================================================================
   CLASS Compositor::Iterator
==============================================================================*/

//------------------------------------------------------------------------------
//! 
Compositor::Iterator::Iterator( Compositor* compositor ) :
   _compositor( compositor ), _ipos(0), _comp(0)
{
   if( _compositor->scope() )
   {
      _comp = _compositor->scope();
      for( Component* child = _comp->children(); child; child = child->sibling() )
      {
         _stack.pushBack( child );
      }
   }
}

//------------------------------------------------------------------------------
//! 
void 
Compositor::Iterator::operator++()
{
   if( _compositor->scope() )
   {
      if( _stack.empty() )
      {
         _comp = 0;
      }
      else
      {
         _comp = _stack.back();
         _stack.popBack();
         for( Component* child = _comp->children(); child; child = child->sibling() )
         {
            _stack.pushBack( child );
         }
      }
   }
   else
   {
      ++_ipos;
   }
}

//------------------------------------------------------------------------------
//! 
bool 
Compositor::Iterator::valid() const
{
   if( _compositor->scope() ) return _comp != 0;
   return _ipos < _compositor->numComponents();
}

//------------------------------------------------------------------------------
//! 
Component* 
Compositor::Iterator::operator*()
{
   if( _compositor->scope() ) return _comp;
   return _compositor->component( _ipos );
}

/*==============================================================================
   CLASS Compositor
==============================================================================*/

//------------------------------------------------------------------------------
//! 
Compositor::Compositor()
{
}

//------------------------------------------------------------------------------
//! 
Compositor::~Compositor()
{
}

//------------------------------------------------------------------------------
//! 
Component* 
Compositor::createComponent()
{
   RCP<Component> comp( new Component() );
   _components.pushBack( comp );
   return comp.ptr();
}

//------------------------------------------------------------------------------
//! 
void 
Compositor::removeComponent( Component* comp )
{
   _components.removeSwap( comp );
}

//------------------------------------------------------------------------------
//! 
void 
Compositor::move( Component* comp, const Vec3f& pos )
{
   Reff ref( comp->referential().orientation(), pos );
   move( comp, ref );
}

//------------------------------------------------------------------------------
//! 
void 
Compositor::move( Component* comp, const Reff& ref )
{
   Reff xform = ref * comp->referential().getInversed();
   comp->transform( xform );
   comp->referential( ref );
}

//------------------------------------------------------------------------------
//! 
void 
Compositor::connect( Component* from, Component* to )
{
   if( to->numRegions() > 0 )
   {
      connect( from, to->region( _rng.getUInt( to->numRegions() ) ) );
   }
}

//------------------------------------------------------------------------------
//! 
void 
Compositor::connect( Component* from, Region* to )
{
   connect( from, to, to->connection( _rng ) );
}

//------------------------------------------------------------------------------
//! 
void 
Compositor::connect( Component* from, Region* to, const Reff& ref )
{
   // Already connected?
   if( from->parent() ) return;

   // Compute the new referential of the current component.
   Component* toC = to->component();
   Reff xform = toC->referential() * ref * 
      (from->referential() * from->connector().referential()).getInversed();

   // Move the current component and all its neighbors to the new referential.
   from->transform( xform );

   // Connect component.
   toC->addComponent( from );
}

//------------------------------------------------------------------------------
//! 
void 
Compositor::disconnect( Component* comp )
{
   if( comp->parent() ) comp->parent()->removeComponent( comp );
}

//------------------------------------------------------------------------------
//! 
void
Compositor::bind( MetaNode* from, Component* to )
{
   _geomConnections.pushBack( Pair<Component*,MetaNode*>( to, from ) );
}

//------------------------------------------------------------------------------
//! 
void 
Compositor::updateTransform( MetaBuilder& builder )
{
   for( uint i = 0; i < _geomConnections.size(); ++i )
   {
      MetaNode* node = _geomConnections[i].second;
      switch( node->type() )
      {
         case MetaNode::META_COMPOSITE:
         {
            MetaComposite* c = (MetaComposite*)node;
            builder.set( c, _geomConnections[i].first->referential().toMatrix() );
         }  break;
         case MetaNode::META_BLOCKS:
         {
            MetaBlocks* c = (MetaBlocks*)node;
            builder.set( c, _geomConnections[i].first->referential().toMatrix() );
         }  break;
         default:;
      }
   }
}

NAMESPACE_END

