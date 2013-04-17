/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#ifndef CGMATH_HGRID_H
#define CGMATH_HGRID_H

#include <CGMath/StdDefs.h>
#include <CGMath/AABBox.h>

#include <Base/ADT/MemoryPool.h>
#include <Base/ADT/HashTable.h>

NAMESPACE_BEGIN

/*==============================================================================
   CLASS HGrid
 ==============================================================================*/
//! FIXME: We should be able to retreive the bounding box of an object.
class HGrid
{
public:

   /*----- classes -----*/

   struct Link {
      uchar _min[3];
      uchar _max[3];
      void* _obj;
      Link* _next;
   };

   struct CellID {
      CellID() {}
      CellID( int x, int y, int z, int level ) : _x(x), _y(y), _z(z), _level(level) {}
      CellID( const Vec3f p, uint level, float size ) :
         _x( CGM::floori(p.x/size) ), _y( CGM::floori(p.y/size) ), _z( CGM::floori(p.z/size) ),
         _level( level )
      {}

      bool operator==( const CellID& id ) const
      {
         return (_x == id._x) && (_y == id._y) && (_z == id._z) && (_level == id._level);
      }
      int _x;
      int _y;
      int _z;
      int _level;
   };

   struct Cell {
      Cell() : _occupancy(0), _objects(0) {}
      uint64_t _occupancy;
      Link*    _objects;
   };

   /*----- types -----*/

   typedef void (*collisionFunc)( void* data, void* objA, void* objB );

   /*----- methods -----*/

   CGMATH_DLL_API HGrid( float maxCellSize = 1024.0f );

   // Operations.
   CGMATH_DLL_API void clear();
   CGMATH_DLL_API void add( void*, const AABBoxf& );
   CGMATH_DLL_API void add( void*, const Vec3f& corner, float size );
   CGMATH_DLL_API void remove( void*, const AABBoxf& );
   CGMATH_DLL_API void remove( void*, const Vec3f& corner, float size );

   // Queries.
   CGMATH_DLL_API void findAllCollisions( void* data, collisionFunc ) const;
   CGMATH_DLL_API void findCollisions( void* data, void* objB, const AABBoxf&, collisionFunc ) const;

   // ray casting...

private:

   /*----- classes -----*/

   struct HashFunc {
      size_t operator()( const CellID& k )
      {
         //size_t h = k._x  * 73856093;
         //h = (h^k._y)     * 19349663; //19349669
         //h = (h^k._z)     * 83492791;
         //h = (h^k._level) * 67867979;
         size_t h = 5381;
         h = h * 33 + k._x;
         h = h * 33 + k._y;
         h = h * 33 + k._z;
         h = h * 33 + k._level;
         return h;
      }
   };

   /*----- types -----*/

   typedef HashTable<CellID,Cell,HashFunc> Container;

   /*----- static methods -----*/

   static inline bool overlap( const Link& l0, const Link& l1 );
   static inline bool overlap( const CellID& id0, const Link& l0, const CellID& id1, const Link& l1 );
   static inline bool overlapSameLevel( const CellID& id0, const Link& l0, const CellID& id1, const Link& l1 );
   static inline bool overlap( int min[3], int max[3], const CellID& id, const Link& l );

   static inline uint64_t computeMask( const CellID& id );
   static inline uint64_t computeMask( const CellID& id, int min[3], int max[3] );
   static inline void downLevel( CellID& id );
   static inline void upLevel( CellID& id );

   /*----- methods -----*/

   HGrid( const HGrid& );
   void operator=( const HGrid& );

   void findCollisions( Link*, const CellID&, int dx, int dy, int dz, void*, collisionFunc ) const;
   void findCollisions( Link*, const CellID&, const CellID&, int dx, int dy, int dz, void*, collisionFunc ) const;
   void findCollisions( const CellID&, int min[3], int max[3], void* data, void* obj, collisionFunc ) const;

   inline void computeLevel( float size, uint& level, float& cellSize ) const;
   inline void computeBoundingBox( const AABBoxf&, int min[3], int max[3] ) const;

   /*----- static data members -----*/

#if (_MSC_VER >= 1400) || defined(__clang__)
#  define _CELLFACTOR 0.25f
#else
   static const float _CELLFACTOR = 0.25f;
#endif

   /*----- data members -----*/

   float             _maxCellSize;
   MemoryPool<Link>  _links;
   Container         _cells;
};

//------------------------------------------------------------------------------
//!
inline void
HGrid::remove( void* obj, const AABBoxf& box )
{
   remove( obj, box.corner(0), box.maxSize() );
}

//------------------------------------------------------------------------------
//!
inline bool
HGrid::overlap( const Link& l0, const Link& l1 )
{
   if( l0._min[0] > l1._max[0] || l0._max[0] < l1._min[0] ) return false;
   if( l0._min[1] > l1._max[1] || l0._max[1] < l1._min[1] ) return false;
   if( l0._min[2] > l1._max[2] || l0._max[2] < l1._min[2] ) return false;
   return true;
}

//------------------------------------------------------------------------------
//!
inline bool
HGrid::overlap( const CellID& id0, const Link& l0, const CellID& id1, const Link& l1 )
{
   int ls = (id0._level-id1._level)*2;

   int xs0 = id0._x<<7;
   int x0  = xs0 + l0._min[0];
   int x1  = xs0 + (l0._max[0]+1);
   int xs1 = id1._x<<7;
   int x2  = (xs1 + l1._min[0])<<ls;
   int x3  = (xs1 + (l1._max[0]+1))<<ls;

   if( x0 > x3 || x1 < x2 ) return false;

   int ys0 = id0._y<<7;
   int y0  = ys0 + l0._min[1];
   int y1  = ys0 + (l0._max[1]+1);
   int ys1 = id1._y<<7;
   int y2  = (ys1 + l1._min[1])<<ls;
   int y3  = (ys1 + (l1._max[1]+1))<<ls;

   if( y0 > y3 || y1 < y2 ) return false;

   int zs0 = id0._z<<7;
   int z0  = zs0 + l0._min[2];
   int z1  = zs0 + (l0._max[2]+1);
   int zs1 = id1._z<<7;
   int z2  = (zs1 + l1._min[2])<<ls;
   int z3  = (zs1 + (l1._max[2]+1))<<ls;

   if( z0 > z3 || z1 < z2 ) return false;

   return true;
}

//------------------------------------------------------------------------------
//!
inline bool
HGrid::overlapSameLevel( const CellID& id0, const Link& l0, const CellID& id1, const Link& l1 )
{
   int xs0 = id0._x<<7;
   int x0  = xs0 + l0._min[0];
   int x1  = xs0 + l0._max[0];
   int xs1 = id1._x<<7;
   int x2  = xs1 + l1._min[0];
   int x3  = xs1 + l1._max[0];

   if( x0 > x3 || x1 < x2 ) return false;

   int ys0 = id0._y<<7;
   int y0  = ys0 + l0._min[1];
   int y1  = ys0 + l0._max[1];
   int ys1 = id1._y<<7;
   int y2  = ys1 + l1._min[1];
   int y3  = ys1 + l1._max[1];

   if( y0 > y3 || y1 < y2 ) return false;

   int zs0 = id0._z<<7;
   int z0  = zs0 + l0._min[2];
   int z1  = zs0 + l0._max[2];
   int zs1 = id1._z<<7;
   int z2  = zs1 + l1._min[2];
   int z3  = zs1 + l1._max[2];

   if( z0 > z3 || z1 < z2 ) return false;

   return true;
}

//------------------------------------------------------------------------------
//!
inline bool
HGrid::overlap( int min[3], int max[3], const CellID& id, const Link& l )
{
   int ls = (10-id._level)*2;

   int xs = id._x<<7;
   int x0 = (xs + l._min[0])<<ls;
   int x1 = (xs + (l._max[0]+1))<<ls;

   if( x0 > max[0] || x1 < min[0] ) return false;

   int ys = id._y<<7;
   int y0 = (ys + l._min[1])<<ls;
   int y1 = (ys + (l._max[1]+1))<<ls;

   if( y0 > max[1] || y1 < min[1] ) return false;

   int zs = id._z<<7;
   int z0 = (zs + l._min[2])<<ls;
   int z1 = (zs + (l._max[2]+1))<<ls;

   if( z0 > max[2] || z1 < min[2] ) return false;

   return true;
}

//------------------------------------------------------------------------------
//!
inline void
HGrid::computeLevel( float size, uint& level, float& cellSize ) const
{
   level               = 0;
   cellSize            = _maxCellSize;
   float nextLevelSize = cellSize*_CELLFACTOR;
   for( ; size < nextLevelSize; ++level )
   {
      cellSize       = nextLevelSize;
      nextLevelSize *= _CELLFACTOR;
   }
}

//------------------------------------------------------------------------------
//!
inline uint64_t
HGrid::computeMask( const CellID& id )
{
   uint64_t mask = 1;
   mask <<= ((id._x&0x3) | ((id._y&0x3)<<2) | ((id._z&0x3)<<4));
   return mask;
}

inline uint64_t
HGrid::computeMask( const CellID& id, int min[3], int max[3] )
{
   uint64_t _xmask0[4] = { 0xffffffffffffffffLL, 0xeeeeeeeeeeeeeeeeLL, 0xccccccccccccccccLL, 0x8888888888888888LL };
   uint64_t _xmask1[4] = { 0x1111111111111111LL, 0x3333333333333333LL, 0x7777777777777777LL, 0xffffffffffffffffLL };
   uint64_t _ymask0[4] = { 0xffffffffffffffffLL, 0xfff0fff0fff0fff0LL, 0xff00ff00ff00ff00LL, 0xf000f000f000f000LL };
   uint64_t _ymask1[4] = { 0x000f000f000f000fLL, 0x00ff00ff00ff00ffLL, 0x0fff0fff0fff0fffLL, 0xffffffffffffffffLL };
   uint64_t _zmask0[4] = { 0xffffffffffffffffLL, 0xffffffffffff0000LL, 0xffffffff00000000LL, 0xffff000000000000LL };
   uint64_t _zmask1[4] = { 0x000000000000ffffLL, 0x00000000ffffffffLL, 0x0000ffffffffffffLL, 0xffffffffffffffffLL };

   int ls = 7+(9-id._level)*2;
   int x0 = (min[0] >> ls) - (id._x<<2)-1;
   int x1 = (max[0] >> ls) - (id._x<<2);
   int y0 = (min[1] >> ls) - (id._y<<2)-1;
   int y1 = (max[1] >> ls) - (id._y<<2);
   int z0 = (min[2] >> ls) - (id._z<<2)-1;
   int z1 = (max[2] >> ls) - (id._z<<2);
   
   uint64_t x0m = (x0 < 0) ? _xmask0[0] : ((x0 > 3) ? 0 : _xmask0[x0]);
   uint64_t x1m = (x1 < 0) ? 0 : ((x1 > 3) ? _xmask1[3] : _xmask1[x1]);
   uint64_t y0m = (y0 < 0) ? _ymask0[0] : ((y0 > 3) ? 0 : _ymask0[y0]);
   uint64_t y1m = (y1 < 0) ? 0 : ((y1 > 3) ? _ymask1[3] : _ymask1[y1]);
   uint64_t z0m = (z0 < 0) ? _zmask0[0] : ((z0 > 3) ? 0 : _zmask0[z0]);
   uint64_t z1m = (z1 < 0) ? 0 : ((z1 > 3) ? _zmask1[3] : _zmask1[z1]);

   return x0m & x1m & y0m & y1m & z0m & z1m;
}

//------------------------------------------------------------------------------
//!
inline void
HGrid::downLevel( CellID& id )
{
   --id._level;
   id._x >>= 2;
   id._y >>= 2;
   id._z >>= 2;
}

//------------------------------------------------------------------------------
//!
inline void
HGrid::upLevel( CellID& id )
{
   ++id._level;
   id._x <<= 2;
   id._y <<= 2;
   id._z <<= 2;
}

//------------------------------------------------------------------------------
//! 
inline void 
HGrid::computeBoundingBox( const AABBoxf& box, int min[3], int max[3] ) const
{
   float scale = float(1<<27)/_maxCellSize;
   min[0] = CGM::floori( box.min(0)*scale );
   min[1] = CGM::floori( box.min(1)*scale );
   min[2] = CGM::floori( box.min(2)*scale );
   max[0] = CGM::floori( box.max(0)*scale );
   max[1] = CGM::floori( box.max(1)*scale );
   max[2] = CGM::floori( box.max(2)*scale );
}

NAMESPACE_END

#endif

