/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#include <Plasma/Geometry/MeshGeometry.h>
#include <Plasma/Animation/Pinocchio.h>
#include <Plasma/World/Viewport.h>

#if MOTION_BULLET
#include <MotionBullet/World/RigidBody.h>
#else
#include <Motion/World/RigidBody.h>
#endif

#include <Fusion/Core/Core.h>

USING_NAMESPACE

/*==============================================================================
  UNNAMED NAMESPACE
==============================================================================*/
UNNAMESPACE_BEGIN

inline Gfx::AttributeType  toAttributeType( uint texID )
{
   return (Gfx::AttributeType)(Gfx::ATTRIB_TYPE_TEXCOORD0 << texID);
}

//------------------------------------------------------------------------------
//!
float distancePtPt( const Vec2f& pt0, const Vec2f& pt1 )
{
   return sqrLength( pt0-pt1 );
}

//------------------------------------------------------------------------------
//!
float distancePtSegment( const Vec2f& pt, const Vec2f& v0, const Vec2f& v1 )
{
   Vec2f e10  = v1-v0;
   Vec2f ptv0 = pt-v0;
   float e    = dot( ptv0, e10 );
   if( e <= 0.0 ) return distancePtPt( pt, v0 );
   float f    = dot( e10, e10 );
   if( e >= f ) return distancePtPt( pt, v1 );
   float t   = e / f;
   float d   = dot( ptv0, ptv0 ) - e*t;
   return d;
}

UNNAMESPACE_END

/*==============================================================================
  CLASS MeshGeometry
==============================================================================*/

//------------------------------------------------------------------------------
//!
const char*
MeshGeometry::attrToStr( uint attrType )
{
   switch( attrType )
   {
      case INVALID  : return "INVALID";
      case POSITION : return "POSITION";
      case NORMAL   : return "NORMAL";
      case COLOR    : return "COLOR";
      case MAPPING  : return "MAPPING";
      case GENERIC_1: return "GENERIC_1";
      case GENERIC_2: return "GENERIC_2";
      case GENERIC_3: return "GENERIC_3";
      case GENERIC_4: return "GENERIC_4";
      default:        return "INVALID";
   }
}

//------------------------------------------------------------------------------
//!
MeshGeometry::MeshGeometry( int type ):
   Geometry( Geometry::MESH ),
   _primType( type ),
   _numIndices( 0 ),
   _numVertices( 0 ),
   _iData( nullptr ),
   _vData( nullptr ),
   _vStride( 0 ),
   _pOffset( INVALID_OFFSET ),
   _nOffset( INVALID_OFFSET ),
   _mOffset( INVALID_OFFSET ),
   _numAttr( 0 ),
   _attrInfo( nullptr )
{
}

//------------------------------------------------------------------------------
//!
MeshGeometry::MeshGeometry( int type, const int* attrList, uint numIndices, uint numVertices ):
   Geometry( Geometry::MESH ),
   _primType( type ),
   _numIndices( 0 ),
   _numVertices( 0 ),
   _iData( nullptr ),
   _vData( nullptr ),
   _vStride( 0 ),
   _pOffset( INVALID_OFFSET ),
   _nOffset( INVALID_OFFSET ),
   _mOffset( INVALID_OFFSET ),
   _numAttr( 0 ),
   _attrInfo( nullptr )
{
   setAttributes( attrList );
   allocateIndices( numIndices );
   allocateVertices( numVertices );
}

//------------------------------------------------------------------------------
//!
MeshGeometry::~MeshGeometry()
{
   delete _attrInfo;
   delete [] _vData;
   delete [] _iData;
}

//------------------------------------------------------------------------------
//!
void
MeshGeometry::primitiveType( int t )
{
   if( _primType == t ) return;

   _primType = t;
   if( _rgeom.isValid() )
   {
      switch( _primType )
      {
         case POINTS:    _rgeom->primitiveType( Gfx::PRIM_POINTS );    break;
         case LINES:     _rgeom->primitiveType( Gfx::PRIM_LINES );     break;
         case TRIANGLES: _rgeom->primitiveType( Gfx::PRIM_TRIANGLES ); break;
         default: break;
      }
   }
}

//------------------------------------------------------------------------------
//!
void
MeshGeometry::copyIndices( const uint32_t* srcPtr )
{
   CHECK( _iData );
   memcpy( _iData, srcPtr, indicesSize()*sizeof(_iData[0]) );
}

//------------------------------------------------------------------------------
//!
void
MeshGeometry::copyAttributes( const float* srcPtr, uint srcAttrSize, size_t srcStride, uint16_t dstOffset )
{
   CHECK( _vData );
   if( srcAttrSize == 0 || srcStride == 0 )  return;

   float* dstPtr = _vData + dstOffset;
   const size_t dstStride = _vStride;
   // TODO: Support larger srcAttrSize.
   switch( srcAttrSize )
   {
      case 0:
         // Nothing to copy.
         break;
      case 1:
         for( uint i = 0; i < _numVertices; ++i )
         {
            dstPtr[0] = srcPtr[0];
            srcPtr += srcStride;
            dstPtr += dstStride;
         }
         break;
      case 2:
         for( uint i = 0; i < _numVertices; ++i )
         {
            dstPtr[0] = srcPtr[0];
            dstPtr[1] = srcPtr[1];
            srcPtr += srcStride;
            dstPtr += dstStride;
         }
         break;
      case 3:
         for( uint i = 0; i < _numVertices; ++i )
         {
            dstPtr[0] = srcPtr[0];
            dstPtr[1] = srcPtr[1];
            dstPtr[2] = srcPtr[2];
            srcPtr += srcStride;
            dstPtr += dstStride;
         }
         break;
      case 4:
         for( uint i = 0; i < _numVertices; ++i )
         {
            dstPtr[0] = srcPtr[0];
            dstPtr[1] = srcPtr[1];
            dstPtr[2] = srcPtr[2];
            dstPtr[3] = srcPtr[3];
            srcPtr += srcStride;
            dstPtr += dstStride;
         }
         break;
      case 5:
         for( uint i = 0; i < _numVertices; ++i )
         {
            dstPtr[0] = srcPtr[0];
            dstPtr[1] = srcPtr[1];
            dstPtr[2] = srcPtr[2];
            dstPtr[3] = srcPtr[3];
            dstPtr[4] = srcPtr[4];
            srcPtr += srcStride;
            dstPtr += dstStride;
         }
         break;
      case 6:
         for( uint i = 0; i < _numVertices; ++i )
         {
            dstPtr[0] = srcPtr[0];
            dstPtr[1] = srcPtr[1];
            dstPtr[2] = srcPtr[2];
            dstPtr[3] = srcPtr[3];
            dstPtr[4] = srcPtr[4];
            dstPtr[5] = srcPtr[5];
            srcPtr += srcStride;
            dstPtr += dstStride;
         }
         break;
      case 7:
         for( uint i = 0; i < _numVertices; ++i )
         {
            dstPtr[0] = srcPtr[0];
            dstPtr[1] = srcPtr[1];
            dstPtr[2] = srcPtr[2];
            dstPtr[3] = srcPtr[3];
            dstPtr[4] = srcPtr[4];
            dstPtr[5] = srcPtr[5];
            dstPtr[6] = srcPtr[6];
            srcPtr += srcStride;
            dstPtr += dstStride;
         }
         break;
      case 8:
         for( uint i = 0; i < _numVertices; ++i )
         {
            dstPtr[0] = srcPtr[0];
            dstPtr[1] = srcPtr[1];
            dstPtr[2] = srcPtr[2];
            dstPtr[3] = srcPtr[3];
            dstPtr[4] = srcPtr[4];
            dstPtr[5] = srcPtr[5];
            dstPtr[6] = srcPtr[6];
            dstPtr[7] = srcPtr[7];
            srcPtr += srcStride;
            dstPtr += dstStride;
         }
         break;
      default:
         for( uint i = 0; i < _numVertices; ++i )
         {
            memcpy( dstPtr, srcPtr, srcAttrSize*sizeof(srcPtr[0]) );
            srcPtr += srcStride;
            dstPtr += dstStride;
         }
         break;
   }
}

//------------------------------------------------------------------------------
//!
void
MeshGeometry::computeRenderableGeometry()
{
   if( _numIndices == 0 || _numVertices == 0 )
   {
      _rgeom = NULL;  // Will also deallocate the previous one, if any.
      return;
   }

   Gfx::PrimitiveType pt;
   switch( _primType )
   {
      case POINTS:    pt = Gfx::PRIM_POINTS;    break;
      case LINES:     pt = Gfx::PRIM_LINES;     break;
      case TRIANGLES: pt = Gfx::PRIM_TRIANGLES; break;
      default:        pt = Gfx::PRIM_TRIANGLES; break;
   }
   _rgeom = Core::gfx()->createGeometry( pt );

   uint numIdx = numIndices();
   uint numVtx = numVertices();

   if( numVtx < (1<<16) )
   {
      // Compact into 16b indices.
      uint16_t* iData16 = new uint16_t[numIdx];
      for( uint i = 0; i < numIdx; ++i )
      {
         iData16[i] = (uint16_t)_iData[i];
      }
      _rgeom->indexBuffer(
         Core::gfx()->createBuffer( Gfx::INDEX_FMT_16, Gfx::BUFFER_FLAGS_NONE, numIdx*sizeof(uint16_t), iData16 )
      );
      delete[] iData16;
   }
   else
   {
      _rgeom->indexBuffer(
         Core::gfx()->createBuffer( Gfx::INDEX_FMT_32, Gfx::BUFFER_FLAGS_NONE, numIdx*sizeof(uint32_t), _iData )
      );
   }

   RCP<Gfx::VertexBuffer> vBuffer = Core::gfx()->createBuffer( Gfx::BUFFER_FLAGS_NONE, verticesSize()*sizeof(float), vertices() );
   uint texID = 1;
   for( uint i = 0; i < _numAttr; ++i )
   {
      const AttrInfo& ai = _attrInfo[i];
      switch( ai._type )
      {
         case POSITION:
            vBuffer->addAttribute( Gfx::ATTRIB_TYPE_POSITION , Gfx::ATTRIB_FMT_32F_32F_32F    , ai._offset*sizeof(float) );
            break;
         case NORMAL:
            vBuffer->addAttribute( Gfx::ATTRIB_TYPE_NORMAL   , Gfx::ATTRIB_FMT_32F_32F_32F    , ai._offset*sizeof(float) );
            break;
         case COLOR:
            vBuffer->addAttribute( Gfx::ATTRIB_TYPE_COLOR    , Gfx::ATTRIB_FMT_32F_32F_32F     , ai._offset*sizeof(float) );
            break;
         case MAPPING:
            vBuffer->addAttribute( Gfx::ATTRIB_TYPE_TEXCOORD0, Gfx::ATTRIB_FMT_32F_32F        , ai._offset*sizeof(float) );
            break;
         case GENERIC_1:
            vBuffer->addAttribute( toAttributeType(texID++)  , Gfx::ATTRIB_FMT_32F            , ai._offset*sizeof(float) );
            break;
         case GENERIC_2:
            vBuffer->addAttribute( toAttributeType(texID++)  , Gfx::ATTRIB_FMT_32F_32F        , ai._offset*sizeof(float) );
            break;
         case GENERIC_3:
            vBuffer->addAttribute( toAttributeType(texID++)  , Gfx::ATTRIB_FMT_32F_32F_32F    , ai._offset*sizeof(float) );
            break;
         case GENERIC_4:
            vBuffer->addAttribute( toAttributeType(texID++)  , Gfx::ATTRIB_FMT_32F_32F_32F_32F, ai._offset*sizeof(float) );
            break;
      }
   }
   _rgeom->addBuffer( vBuffer );
}

//------------------------------------------------------------------------------
//!
void
MeshGeometry::setAttributes( const int* attrList )
{
   CHECK( _attrInfo == NULL );

   // Determine how many attributes we have.
   _numAttr = 0;
   for( const int* curAttr = attrList; *curAttr; ++curAttr )
   {
      ++_numAttr;
   }

   // Allocate the proper number of attributes.
   _attrInfo = new AttrInfo[_numAttr];

   // Fill the attribute information.
   setAttributes( attrList, _attrInfo );
}

//------------------------------------------------------------------------------
//!
void
MeshGeometry::setAttributes( MeshGeometry& mesh )
{
   CHECK( _attrInfo == NULL );
   _vStride = mesh._vStride;
   _pOffset = mesh._pOffset;
   _nOffset = mesh._nOffset;
   _mOffset = mesh._mOffset;
   _numAttr = mesh._numAttr;
   _attrInfo = new AttrInfo[_numAttr];
   for( uint i = 0; i < _numAttr; ++i )
   {
      _attrInfo[i] = mesh._attrInfo[i];
   }
}

//------------------------------------------------------------------------------
//!
void
MeshGeometry::addAttributes( const int* attrList )
{
   // If we don't have any attributes then set them.
   if( _numAttr == 0 )
   {
      setAttributes( attrList );
      return;
   }

   // Keep old buffers info.
   size_t vStride = _vStride;
   float* data    = _vData;

   // Determine how many attributes we have.
   uint16_t numAttr = 0;
   for( const int* curAttr = attrList; *curAttr; ++curAttr )
   {
      ++numAttr;
   }
   AttrInfo* attrInfo = new AttrInfo[numAttr+_numAttr];

   // Fill the attributes information.
   AttrInfo* curAttrInfo = attrInfo;
   AttrInfo* oldAttrInfo = _attrInfo;
   for( ; curAttrInfo < attrInfo+_numAttr; ++curAttrInfo, ++oldAttrInfo )
   {
      *curAttrInfo = *oldAttrInfo;
   }
   setAttributes( attrList, curAttrInfo );

   // Replace old attributes.
   delete[] _attrInfo;
   _attrInfo = attrInfo;
   _numAttr += numAttr;

   // Realloc vertex data if already allocated.
   if( _vData )
   {
      _vData = new float[verticesSize()];
      copyAttributes( data, uint(vStride), vStride, 0 );
      delete[] data;
   }
}

//------------------------------------------------------------------------------
//!
void
MeshGeometry::setAttributes( const int* attrList, AttrInfo* curAttrInfo )
{
   const int* curAttr = attrList;
   while( 1 )
   {
      switch( *curAttr )
      {
         case INVALID: // 0, end-of-list sentinel.
            return;
         case POSITION:
            curAttrInfo->_type   = *curAttr;
            curAttrInfo->_offset = uint16_t(_vStride);
                        _pOffset = uint16_t(_vStride);
            _vStride += 3;
            break;
         case NORMAL:
            curAttrInfo->_type   = *curAttr;
            curAttrInfo->_offset = uint16_t(_vStride);
                        _nOffset = uint16_t(_vStride);
            _vStride += 3;
            break;
         case COLOR:
            curAttrInfo->_type   = *curAttr;
            curAttrInfo->_offset = uint16_t(_vStride);
            _vStride += 3;
            break;
         case MAPPING:
            curAttrInfo->_type   = *curAttr;
            curAttrInfo->_offset = uint16_t(_vStride);
                        _mOffset = uint16_t(_vStride);
            _vStride += 2;
            break;
         case GENERIC_1:
            curAttrInfo->_type   = *curAttr;
            curAttrInfo->_offset = uint16_t(_vStride);
            _vStride += 1;
            break;
         case GENERIC_2:
            curAttrInfo->_type   = *curAttr;
            curAttrInfo->_offset = uint16_t(_vStride);
            _vStride += 2;
            break;
         case GENERIC_3:
            curAttrInfo->_type   = *curAttr;
            curAttrInfo->_offset = uint16_t(_vStride);
            _vStride += 3;
            break;
         case GENERIC_4:
            curAttrInfo->_type   = *curAttr;
            curAttrInfo->_offset = uint16_t(_vStride);
            _vStride += 4;
            break;
         default:
            StdErr << "MeshGeometry::setAttributes() - Invalid attribute id=" << *curAttr << " at index " << curAttrInfo - _attrInfo << ", aborting." << nl;
            return;
      }
      ++curAttr;
      ++curAttrInfo;
   }
}


//------------------------------------------------------------------------------
//!
void
MeshGeometry::updateProperties()
{
   // Bounding box.
   _bbox = AABBoxf::empty();
   for( uint i = 0; i < _numVertices; ++i )
   {
      _bbox |= position(i);
   }
   // Center of mass and inertia tensor.
   if( _primType == TRIANGLES )
   {
      RigidBody::computeShapeProperties(
         _vData + _pOffset, int(_vStride), _iData, _numIndices/3,
         _com, _inertiaTensor
      );
   }
   else
   {
      // Since we have no volume use bbox center as the com.
      _com = Reff( _bbox.center() );
   }

   updateShape();
}

//------------------------------------------------------------------------------
//!
void
MeshGeometry::computeBonesWeights()
{
   if( !skeleton() ) return;

   // Reallocate buffers for weights.
   const int attribs[] = {
      MeshGeometry::GENERIC_4,
      MeshGeometry::GENERIC_4,
      0
   };
   uint na = numAttributes();
   addAttributes( attribs );

   // Compute weights with pinocchio.
   Pinocchio::recomputeVertexWeights( this, skeleton(), na, na+1 );
}

//------------------------------------------------------------------------------
//!
bool
MeshGeometry::pick( const Viewport& vp, const Vec2f& pos, float d, uint32_t& id ) const
{
   float d2      = d*d;
   Mat4f m       = vp.cameraMatrix();

   float minDist = CGConstf::infinity();
   uint minPrim  = 0;

   // Test vertices primitives.
   if( primitiveType() == MeshGeometry::POINTS )
   {
      for( uint i = 0; i < numIndices(); ++i )
      {
         uint32_t i0 = indices()[i];
         Vec3f p     = m | position(i0);
         float dist  = distancePtPt( pos, p(0,1) );

         if( dist < minDist )
         {
            minDist = dist;
            minPrim = i;
         }
      }
   }

   // Test lines primitives.
   if( primitiveType() == MeshGeometry::LINES )
   {
      for( uint i = 0; i < numIndices(); i+=2 )
      {
         uint32_t i0 = indices()[i];
         uint32_t i1 = indices()[i+1];

         Vec3f p0   = m | position(i0);
         Vec3f p1   = m | position(i1);
         float dist = distancePtSegment( pos, p0(0,1), p1(0,1) );

         if( dist < minDist )
         {
            minDist = dist;
            minPrim = i>>1;
         }
      }
   }

   if( minDist <= d2 )
   {
      id = minPrim;
      return true;
   }
   return false;
}

//------------------------------------------------------------------------------
//!
void
MeshGeometry::print( TextStream& os ) const
{
   Geometry::print( os );

   os << numVertices() << " vertices" << nl;
   os << numIndices() << " indices (" << numPrimitives() << " primitives)" << nl;

   os << "vStride=" << _vStride << nl;
   os << "pOffset=" << _pOffset << nl;
   os << "nOffset=" << _nOffset << nl;
   os << "mOffset=" << _mOffset << nl;

   os << _numAttr << " attributes:" << nl;
   for( uint i = 0; i < _numAttr; ++i )
   {
      os << i << ": " << _attrInfo[i]._type << " off=" << _attrInfo[i]._offset << nl;
   }

   os << "Indices:" << nl;
   for( uint i = 0; i < numIndices(); i += 3 )
   {
      os << _iData[i] << " " << _iData[i+1] << " " << _iData[i+2] << nl;
   }

   os << "Vertices:" << nl;
   for( uint v = 0; v < numVertices(); ++v )
   {
      os << "[" << v << "]:";
      for( uint i = 0; i < _vStride; ++i )
      {
         os << " " << _vData[v*_vStride + i];
      }
      os << nl;
   }

   if( hasPosition() )
   {
      os << "Positions:" << nl;
      for( uint v = 0; v < numVertices(); ++v )
      {
         os << "[" << v << "]: " << position(v) << nl;
      }
   }

   if( hasNormal() )
   {
      os << "Normals:" << nl;
      for( uint v = 0; v < numVertices(); ++v )
      {
         os << "[" << v << "]: " << normal(v) << nl;
      }
   }

   if( hasMapping() )
   {
      os << "Mappings:" << nl;
      for( uint v = 0; v < numVertices(); ++v )
      {
         os << "[" << v << "]: " << mapping(v) << nl;
      }
   }
}

//------------------------------------------------------------------------------
//!
void
MeshGeometry::printInfo( TextStream& os ) const
{
   Geometry::print( os );

   os << numVertices() << " vertices" << nl;
   os << numIndices() << " indices (" << numPrimitives() << " primitives)" << nl;

   os << "vStride=" << _vStride << nl;
   os << "pOffset=" << _pOffset << nl;
   os << "nOffset=" << _nOffset << nl;
   os << "mOffset=" << _mOffset << nl;

   os << _numAttr << " attributes:" << nl;
   for( uint i = 0; i < _numAttr; ++i )
   {
      os << i << ": " << _attrInfo[i]._type << " off=" << _attrInfo[i]._offset << nl;
   }

   if( hasPosition() )
   {
      os << "Has positions..." << nl;
   }

   if( hasNormal() )
   {
      os << "Has normals..." << nl;
   }

   if( hasMapping() )
   {
      os << "Hash mappings..." << nl;
   }
}
