/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#ifndef PLASMA_MESH_GEOMETRY_H
#define PLASMA_MESH_GEOMETRY_H

#include <Plasma/StdDefs.h>

#include <Plasma/Geometry/Geometry.h>

NAMESPACE_BEGIN

class Viewport;

/*==============================================================================
  CLASS MeshGeometry
==============================================================================*/
class MeshGeometry:
   public Geometry
{
public:

   /*----- types and enumerations -----*/
   enum
   {
      INVALID  = 0,
      POSITION,   // XYZ
      NORMAL,     // XYZ
      COLOR,      // RGB
      MAPPING,    // XY
      GENERIC_1,  // X
      GENERIC_2,  // XY
      GENERIC_3,  // XYZ
      GENERIC_4,  // XYZW
   };

   enum
   {
      POINTS    = 1,
      LINES     = 2,
      TRIANGLES = 3
   };

   /*----- static methods -----*/

   static PLASMA_DLL_API const char*  attrToStr( uint attrType );

   /*----- methods -----*/

   PLASMA_DLL_API MeshGeometry( int primType = TRIANGLES );
   PLASMA_DLL_API MeshGeometry( int primType, const int* attrList, uint numIndices, uint numVertices );
   virtual ~MeshGeometry();

   void primitiveType( int t );
   inline int primitiveType() const { return _primType; }

   // Memory allocation.
   inline void  allocateIndices( uint numIndices );
   inline void  allocateVertices( uint numVertices );
   inline void  deallocate();

   // Creation/copying of data.
   PLASMA_DLL_API void  setAttributes( const int* attrList );
   PLASMA_DLL_API void  setAttributes( MeshGeometry& );
   PLASMA_DLL_API void  addAttributes( const int* attrList );
   PLASMA_DLL_API void  copyIndices( const uint32_t* srcPtr );
   PLASMA_DLL_API void  copyAttributes(
      const float* srcPtr,
      uint         srcAttrSize,
      size_t       srcStride,
      uint16_t     dstOffset
   );

   // Patches ranges.
   inline void addPatch( uint indices, uint numIndices, uint matID = 0 );
   inline void clearPatches();
   inline void materialID( uint i, uint matID );

   // Quantities.
   inline uint  numPrimitives() const      { return (_numIndices!=0) ? _numIndices/_primType : _numVertices/_primType; }
   inline uint  numIndices()    const      { return _numIndices; }
   inline uint  numVertices()   const      { return _numVertices; }
   inline uint  numAttributes() const      { return _numAttr; }
   inline uint  primitiveSize() const      { return _primType; }

   // Raw data access.
   inline       uint32_t*  indices()       { return _iData; }
   inline const uint32_t*  indices() const { return _iData; }
   inline size_t       indicesSize() const { return numIndices(); }

   inline       float*   vertices()        { return _vData; }
   inline const float*   vertices() const  { return _vData; }
   inline size_t     vertexStride() const  { return _vStride; }
   inline size_t     verticesSize() const  { return numVertices()*_vStride; }

   // Accessor for default attributes.
   inline bool  hasPosition()  const       { return _pOffset != INVALID_OFFSET; }
   inline bool  hasNormal()    const       { return _nOffset != INVALID_OFFSET; }
   inline bool  hasMapping()   const       { return _mOffset != INVALID_OFFSET; }

   inline const Vec3f&  position( uint vID ) const;
   inline const Vec3f&  normal(   uint vID ) const;
   inline const Vec2f&  mapping(  uint vID ) const;

   // Accessor for generic attributes.
   inline float         getAttributeAsFloat( uint attrID, uint vID ) const;
   inline const Vec2f&  getAttributeAsVec2 ( uint attrID, uint vID ) const;
   inline const Vec3f&  getAttributeAsVec3 ( uint attrID, uint vID ) const;
   inline const Vec4f&  getAttributeAsVec4 ( uint attrID, uint vID ) const;

   // Attributes informations.
   inline uint      attributeType  ( uint attrID ) const;
   inline uint16_t  attributeOffset( uint attrID ) const;
   inline void  attributeInfo( uint attrID, int& type, uint16_t& offset ) const;

   PLASMA_DLL_API  void updateProperties();
   PLASMA_DLL_API  void print( TextStream& os = StdErr ) const;
   PLASMA_DLL_API  void printInfo( TextStream& os = StdErr ) const;

   // Skinning.
   virtual void computeBonesWeights();

   // Picking.
   PLASMA_DLL_API bool pick( const Viewport& vp, const Vec2f& pos, float d, uint32_t& id ) const;

protected:

   /*----- types -----*/

   struct AttrInfo
   {
      uint16_t  _type;
      uint16_t  _offset;
   };

   enum
   {
      INVALID_OFFSET = (1<<16)-1
   };

   /*----- methods -----*/

   virtual void computeRenderableGeometry();
   void setAttributes( const int* attrList, AttrInfo* );

   /*----- data members -----*/

   int           _primType;

   uint          _numIndices;  //!< Total number of indices.
   uint          _numVertices; //!< Total number of vertices.
   uint32_t*     _iData;       //!< Index buffer data.
   float*        _vData;       //!< Interleaved vertex data.
   size_t        _vStride;     //!< Stride (in floats) for the vertex data.
   uint16_t      _pOffset;     //!< Offset of the position in the vertex structure.
   uint16_t      _nOffset;     //!< Offset of the normal in the vertex structure.
   uint16_t      _mOffset;     //!< Offset of the mapping in the vertex structure.
   uint16_t      _numAttr;     //!< Total number of attributes in a vertex structure.
   AttrInfo*     _attrInfo;    //!< All attribute informations (including position/normal/mapping).

private:
};

//------------------------------------------------------------------------------
//!
inline void
MeshGeometry::addPatch( uint indices, uint numIndices, uint matID )
{
   Geometry::addPatch( indices, numIndices, matID );
}

//------------------------------------------------------------------------------
//!
inline void
MeshGeometry::clearPatches()
{
   Geometry::clearPatches();
}

//------------------------------------------------------------------------------
//!
inline void
MeshGeometry::materialID( uint i, uint matID )
{
   if( i < numPatches() ) _patches[i].materialID( matID );
}

//------------------------------------------------------------------------------
//!
inline void
MeshGeometry::allocateIndices( uint numIndices )
{
   delete [] _iData;
   _numIndices = numIndices;
   _iData      = (_numIndices != 0) ? new uint32_t[numIndices] : nullptr;
}

//------------------------------------------------------------------------------
//!
inline void
MeshGeometry::allocateVertices( uint numVertices )
{
   delete [] _vData;
   _numVertices = numVertices;
   _vData       = (_numVertices != 0 ) ? new float[verticesSize()] : nullptr;
}

//------------------------------------------------------------------------------
//!
inline void
MeshGeometry::deallocate()
{
   _numIndices              = 0;
   _numVertices             = 0;
   delete [] _iData; _iData = nullptr;
   delete [] _vData; _vData = nullptr;
}

//------------------------------------------------------------------------------
//!
inline const Vec3f&
MeshGeometry::position( uint vID ) const
{
   return reinterpret_cast<const Vec3f&>(_vData[vID*_vStride + _pOffset]);
}

//------------------------------------------------------------------------------
//!
inline const Vec3f&
MeshGeometry::normal( uint vID ) const
{
   return reinterpret_cast<const Vec3f&>(_vData[vID*_vStride + _nOffset]);
}

//------------------------------------------------------------------------------
//!
inline const Vec2f&
MeshGeometry::mapping(  uint vID ) const
{
   return reinterpret_cast<const Vec2f&>(_vData[vID*_vStride + _mOffset]);
}

//------------------------------------------------------------------------------
//!
inline float
MeshGeometry::getAttributeAsFloat( uint attrID, uint vID ) const
{
   return _vData[vID*_vStride + attributeOffset(attrID)];
}

//------------------------------------------------------------------------------
//!
inline const Vec2f&
MeshGeometry::getAttributeAsVec2 ( uint attrID, uint vID ) const
{
   return reinterpret_cast<const Vec2f&>( _vData[vID*_vStride + attributeOffset(attrID)] );
}

//------------------------------------------------------------------------------
//!
inline const Vec3f&
MeshGeometry::getAttributeAsVec3 ( uint attrID, uint vID ) const
{
   return reinterpret_cast<const Vec3f&>( _vData[vID*_vStride + attributeOffset(attrID)] );
}

//------------------------------------------------------------------------------
//!
inline const Vec4f&
MeshGeometry::getAttributeAsVec4 ( uint attrID, uint vID ) const
{
   return reinterpret_cast<const Vec4f&>( _vData[vID*_vStride + attributeOffset(attrID)] );
}

//------------------------------------------------------------------------------
//!
inline uint
MeshGeometry::attributeType( uint attrID ) const
{
   return _attrInfo[attrID]._type;
}

//------------------------------------------------------------------------------
//!
inline uint16_t
MeshGeometry::attributeOffset( uint attrID ) const
{
   return _attrInfo[attrID]._offset;
}

//------------------------------------------------------------------------------
//!
inline void
MeshGeometry::attributeInfo( uint attrID, int& type, uint16_t& offset ) const
{
   type = _attrInfo[attrID]._type; offset = _attrInfo[attrID]._offset;
}


NAMESPACE_END

#endif
