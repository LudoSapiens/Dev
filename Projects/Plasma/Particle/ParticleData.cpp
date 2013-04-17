/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#include <Plasma/Particle/ParticleData.h>

USING_NAMESPACE

/*==============================================================================
  UNNAMED NAMESPACE
==============================================================================*/
UNNAMESPACE_BEGIN

inline void  copy( float* dst, const float* src, size_t s )
{
#if 1
   memcpy( dst, src, s );
#else
   for( size_t i = 0; i < s; ++i )
   {
      dst[i] = src[i];
   }
#endif
}

inline String  fmt( float v )
{
   return String().format("%5g", v );
}

UNNAMESPACE_END

/*==============================================================================
  CLASS ParticleData
==============================================================================*/

//------------------------------------------------------------------------------
//!
ParticleData::ParticleData():
   _types( INVALID ),
   _size( 0 ),
   _capacity( 0 ),
   _vData( nullptr ),
   _eData( nullptr )
{
}

//------------------------------------------------------------------------------
//!
ParticleData::ParticleData( DataType reqs, uint32_t cap ):
   _types( INVALID ),
   _size( 0 ),
   _capacity( 0 ),
   _vData( nullptr ),
   _eData( nullptr )
{
   allocate( reqs, cap );
}

//------------------------------------------------------------------------------
//!
ParticleData::~ParticleData()
{
   deallocate();
}

//------------------------------------------------------------------------------
//!
void
ParticleData::allocate( DataType reqs, uint32_t cap )
{
   deallocate();

   _types    = reqs;
   _capacity = cap;

   _vStride = 3; // Position is always there.
   _eStride = 1; // Energy is always there.
   uint16_t strideAdjusts[4][2] = {
      // vertex  extra
      { 1, 0 }, // SIZE
      { 4, 0 }, // COLOR
      { 4, 0 }, // ORIENTATION
      { 0, 3 }, // LINEAR_VELOCITY
   };
   for( uint i = 0; i < 4; ++i )
   {
      if( ((_types>>i)&0x01) == 0x01 )
      {
         const uint16_t* adj = strideAdjusts[i];
         _vStride += adj[0];
         _eStride += adj[1];
      }
   }

   //++maxSize; // Overallocate to avoid crashing when we read invalid data at the last element?
   _vData = new float[ _capacity*_vStride ];
   _eData = new float[ _capacity*_eStride ];

   uint16_t curOffset = 3;
   if( hasSize() )
   {
      curOffset += 1;
   }
   if( hasColor() )
   {
      _cOffset = curOffset;
      curOffset += 4;
   }
   else
   {
      _cOffset = 0;
   }
   if( hasOrientation() )
   {
      _oOffset = curOffset;
   }
   else
   {
      _oOffset = 0;
   }
}


//------------------------------------------------------------------------------
//!
void
ParticleData::deallocate()
{
   delete [] _vData;  _vData = nullptr;
   delete [] _eData;  _eData = nullptr;
   _size     = 0;
   _capacity = 0;
}

//------------------------------------------------------------------------------
//!
void
ParticleData::removeParticle( const Iterator& it )
{
   CHECK( it._data == this );
   // Copy last entry in its place.
   --_size;
   copy( it._curV, _vData + (_size*_vStride), _vStride * sizeof(_vData[0]) );
   copy( it._curE, _eData + (_size*_eStride), _eStride * sizeof(_eData[0]) );
}

//------------------------------------------------------------------------------
//!
void
ParticleData::print( TextStream& os ) const
{
   printInfo( os );
   os << nl;
   printData( os );
}

//------------------------------------------------------------------------------
//!
void
ParticleData::printData( TextStream& os ) const
{
   // Header.
   os << "      ";
   os << "Position";
   if( hasSize() )  os << " Size";
   if( hasColor() )  os << " Color";
   if( hasOrientation() )  os << " Orientation";
   os << " |";
   os << " Energy";
   if( hasLinearVelocity() )  os << " LinearVelocity";
   os << nl;

   // Particles.
   const float* curV = _vData;
   const float* curE = _eData;
   for( uint32_t pi = 0; pi < _size; ++pi )
   {
      os << String().format("%4d", pi) << ": ";
      for( uint32_t fi = 0; fi < _vStride; ++fi )
      {
         os << " " << fmt( *curV++ );
      }
      os << " |";
      for( uint32_t fi = 0; fi < _eStride; ++fi )
      {
         os << " " << fmt( *curE++ );
      }
      os << nl;
   }
}

//------------------------------------------------------------------------------
//!
void
ParticleData::printInfo( TextStream& os ) const
{
   os << "ParticleData: " << _size << "/" << _capacity << " particles";
   os << " (";
   os << " pos:0";
   if( hasSize()        )  os << " size:3";
   if( hasColor()       )  os << " color:" << _cOffset;
   if( hasOrientation() )  os << " ori:" << _oOffset;
   os << " |";
   os << " nrg:0";
   if( hasLinearVelocity() )  os << " linVel:1";
   os << String().format(" 0x%02x )", _types);
   os << " vStride=" << _vStride << " eStride=" << _eStride << nl;
}
