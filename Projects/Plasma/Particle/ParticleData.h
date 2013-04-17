/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#ifndef PLASMA_PARTICLE_DATA_H
#define PLASMA_PARTICLE_DATA_H

#include <Plasma/StdDefs.h>

#include <CGMath/Quat.h>
#include <CGMath/Random.h>
#include <CGMath/Vec3.h>

#include <Base/ADT/RCVector.h>
#include <Base/Dbg/Defs.h>
#include <Base/Util/Enum.h>

NAMESPACE_BEGIN


typedef RNG_WELL  ParticleRNG;


/*==============================================================================
  CLASS ParticleScratchData
==============================================================================*/
class ParticleScratchData
{
public:
   float*     _distances;
   uint32_t*  _indicesA;
   uint32_t*  _indicesB;

   ParticleScratchData( float* d, uint32_t* iA, uint32_t* iB ):
      _distances(d), _indicesA(iA), _indicesB(iB) {}
}; //class ParticleScratchData


/*==============================================================================
  CLASS ParticleData
==============================================================================*/
//!< A geometry used to cache in software the particle data used in both the
//!< particle simulation and the particle rendering.
//!< The data is spread across 2 float buffers:
//!<   _vData = Pos(XYZ) Size Color(RGBA)* Ori(Quat)*
//!<   _eData = Energy linVel(XYZ)*
//!< Attributes marked with * indicate that they are optional.
//!< The _vData (vertex data) is also used directly for rendering,
//!< whereas _eData (extra data) is solely used by the generator/animator.
//!< An iterator class is used to both iterate over all of the current particles
//!< as well as have easy and efficient access to the data.
//!< Iteration is done starting from the end so that removal of dying particles
//!< is more efficient; relative order between iterations (i.e. different frames)
//!< is not guarateed (uses a removeSwap() removal for efficiency reasons).
class ParticleData
{
public:

   /*----- types and enumerations -----*/
   enum DataType
   {
      INVALID         = 0x00,
      SIZE            = 0x01,
      COLOR           = 0x02,
      ORIENTATION     = 0x04,
      LINEAR_VELOCITY = 0x08,
   };

   /*==============================================================================
     CLASS Iterator
   ==============================================================================*/
   class Iterator
   {
   public:
      /*----- methods -----*/

      inline bool  operator()() const { return _curV >= _data->_vData; }

      inline void  next() { _curV -= _data->_vStride; _curE -= _data->_eStride; }

      inline const Vec3f&  position() const                 { return Vec3f::as( _curV ); }
      inline       Vec3f&  position()                       { return Vec3f::as( _curV ); }
      inline         void  position( const Vec3f& v )       { Vec3f::as( _curV ) = v;    }

      inline       float   size() const                     { return *( _curV + 3 ); }
      inline       float&  size()                           { return *( _curV + 3 ); }
      inline         void  size( float v )                  { *( _curV + 3 ) = v;    }

      inline const Vec4f&  color() const                    { return Vec4f::as( _curV + _data->_cOffset ); }
      inline       Vec4f&  color()                          { return Vec4f::as( _curV + _data->_cOffset ); }
      inline         void  color( const Vec4f& v )          { Vec4f::as( _curV + _data->_cOffset ) = v;    }

      inline const Quatf&  orientation() const              { return Quatf::as( _curV + _data->_oOffset ); }
      inline       Quatf&  orientation()                    { return Quatf::as( _curV + _data->_oOffset ); }
      inline         void  orientation( const Quatf& v )    { Quatf::as( _curV + _data->_oOffset ) = v;    }

      inline       float   energy() const                   { return *( _curE ); }
      inline       float&  energy()                         { return *( _curE ); }
      inline         void  energy( float v )                { *( _curE ) = v;    }

      inline const Vec3f&  linearVelocity() const           { return Vec3f::as( _curE + 1 ); }
      inline       Vec3f&  linearVelocity()                 { return Vec3f::as( _curE + 1 ); }
      inline         void  linearVelocity( const Vec3f& v ) { Vec3f::as( _curE + 1 ) = v;    }

   protected:
      friend class ParticleData;

      inline Iterator( ParticleData* data, int idx ):
         _data( data )
      {
         _curV = _data->_vData + idx*_data->_vStride;
         _curE = _data->_eData + idx*_data->_eStride;
      }

      /*----- data members -----*/
      ParticleData*  _data;
      float*         _curV;
      float*         _curE;

   }; //class Iterator

   /*----- static methods -----*/

   /*----- methods -----*/

   PLASMA_DLL_API ParticleData();
   PLASMA_DLL_API ParticleData( DataType reqs, uint32_t cap );
   PLASMA_DLL_API ~ParticleData();

   inline bool  hasSize()           const { return (_types & SIZE           ) != 0x0; }
   inline bool  hasColor()          const { return (_types & COLOR          ) != 0x0; }
   inline bool  hasOrientation()    const { return (_types & ORIENTATION    ) != 0x0; }
   inline bool  hasLinearVelocity() const { return (_types & LINEAR_VELOCITY) != 0x0; }

   inline     uint32_t  size()              const { return _size;     }
   inline     uint32_t  capacity()          const { return _capacity; }
   inline     uint32_t  available()         const { return _capacity - _size; }
   inline const float*  vertexData()        const { return _vData;    }
   inline const float*  extraData()         const { return _eData;    }
   inline     uint16_t  vertexStride()      const { return _vStride;  }
   inline     uint16_t  extraStride()       const { return _eStride;  }
   inline     uint16_t  colorOffset()       const { return _cOffset;  }
   inline     uint16_t  orientationOffset() const { return _oOffset;  }

   // Utility routines.

   inline Iterator  iter() { return Iterator(this, _size-1); }

   inline Iterator  addParticle();

   PLASMA_DLL_API void  removeParticle( const Iterator& it );

   inline void  clear() { _size = 0; }
   inline bool  empty() { return _size == 0; }
   inline bool  full()  { return _size == _capacity; }

   PLASMA_DLL_API void  print( TextStream& os = StdErr ) const;
   PLASMA_DLL_API void  printData( TextStream& os = StdErr ) const;
   PLASMA_DLL_API void  printInfo( TextStream& os = StdErr ) const;

   PLASMA_DLL_API void  allocate( DataType reqs, uint32_t cap );
   PLASMA_DLL_API void  deallocate();

protected:

   /*----- types -----*/

   /*----- methods -----*/

   /*----- data members -----*/

   DataType  _types;       //!< The extra data types that are present.
   uint32_t  _size;        //!< Current number of elements.
   uint32_t  _capacity;    //!< Maximum number of elements.
   float*    _vData;       //!< Interleaved vertex data (position, size, color, orientation).
   float*    _eData;       //!< Interleaved extra data (energy, linear velocity).
   uint16_t  _vStride;     //!< Stride (in floats) for the vertex data.
   uint16_t  _eStride;     //!< Stride (in floats) for the extra data.
   uint16_t  _cOffset;     //!< Offset of the color in the vertex structure.
   uint16_t  _oOffset;     //!< Offset of the orientation in the vertex structure.

private:
};

GEN_ENUM_BITWISE_OPS( ParticleData::DataType )

//------------------------------------------------------------------------------
//!
inline ParticleData::Iterator
ParticleData::addParticle()
{
   CHECK( _size < _capacity );
   Iterator iter( this, _size );
   ++_size;
   return iter;
}


NAMESPACE_END

#endif //PLASMA_PARTICLE_DATA_H
