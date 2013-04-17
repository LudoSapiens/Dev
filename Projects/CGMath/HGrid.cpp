/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#include <CGMath/HGrid.h>

NAMESPACE_BEGIN

/*==============================================================================
   CLASS HGrid
==============================================================================*/

//------------------------------------------------------------------------------
//!
HGrid::HGrid( float maxCellSize ) :
   _maxCellSize( maxCellSize )
{
}

//------------------------------------------------------------------------------
//!
void
HGrid::clear()
{
   _links.clear();
   _cells.clear();
}

//------------------------------------------------------------------------------
//!
void
HGrid::add( void* obj, const AABBoxf& box )
{
   Vec3f c0 = box.corner(0);
   Vec3f c1 = box.corner(7);

   uint level;
   float cellSize;

   // Compute level and cell size for the object.
   computeLevel( box.maxSize(), level, cellSize );

   // Compute cell id for lower left corner.
   CellID id( c0, level, cellSize );

   // Add object to the cell.
   Cell& cell    = _cells[id];
   Link* link    = _links.alloc();
   link->_obj    = obj;
   link->_next   = cell._objects;
   cell._objects = link;

   // Create compress bounding box.
   link->_min[0] = CGM::floori( (c0.x*128.0f)/cellSize ) - (id._x<<7);
   link->_min[1] = CGM::floori( (c0.y*128.0f)/cellSize ) - (id._y<<7);
   link->_min[2] = CGM::floori( (c0.z*128.0f)/cellSize ) - (id._z<<7);
   link->_max[0] = CGM::floori( (c1.x*128.0f)/cellSize ) - (id._x<<7);
   link->_max[1] = CGM::floori( (c1.y*128.0f)/cellSize ) - (id._y<<7);
   link->_max[2] = CGM::floori( (c1.z*128.0f)/cellSize ) - (id._z<<7);

   // Update parents occupancy.
   while( id._level > 0 )
   {
      uint64_t mask = computeMask( id );
      downLevel( id );
      Cell& pcell = _cells[id];
      if( pcell._occupancy & mask ) break;
      pcell._occupancy |= mask;
   }
}

//------------------------------------------------------------------------------
//!
void
HGrid::add( void* obj, const Vec3f& corner, float size )
{
   uint level;
   float cellSize;

   // Compute level and cell size for the object.
   computeLevel( size, level, cellSize );

   // Compute cell id for lower left corner.
   CellID id( corner, level, cellSize );

   // Add object to the cell.
   Cell& cell    = _cells[id];
   Link* link    = _links.alloc();
   link->_obj    = obj;
   link->_next   = cell._objects;
   cell._objects = link;

   // Create compress bounding box.
   link->_min[0] = CGM::floori( (corner.x*128.0f)/cellSize )          - (id._x<<7);
   link->_min[1] = CGM::floori( (corner.y*128.0f)/cellSize )          - (id._y<<7);
   link->_min[2] = CGM::floori( (corner.z*128.0f)/cellSize )          - (id._z<<7);
   link->_max[0] = CGM::floori( ((corner.x + size)*128.0f)/cellSize ) - (id._x<<7);
   link->_max[1] = CGM::floori( ((corner.y + size)*128.0f)/cellSize ) - (id._y<<7);
   link->_max[2] = CGM::floori( ((corner.z + size)*128.0f)/cellSize ) - (id._z<<7);

   // Update parents occupancy.
   while( id._level > 0 )
   {
      uint64_t mask = computeMask( id );
      downLevel( id );
      Cell& pcell = _cells[id];
      if( pcell._occupancy & mask ) break;
      pcell._occupancy |= mask;
   }
}

//------------------------------------------------------------------------------
//!
void
HGrid::remove( void* obj, const Vec3f& corner, float size )
{
   uint level;
   float cellSize;

   // Compute level and cell size for the object.
   computeLevel( size, level, cellSize );

   // Compute cell id for lower left corner.
   CellID id( corner, level, cellSize );

   // Remove object from cell.
   // 1. Get cell.
   Container::Iterator cellIt = _cells.find(id);

   // 2. Cell not found.
   if( cellIt == _cells.end() ) return;
   Cell& cell = cellIt.data();

   // 3. remove from list.
   Link* prev = 0;
   Link* cur  = cell._objects;
   for( ; cur != 0; prev = cur, cur = cur->_next )
   {
      if( cur->_obj == obj )
      {
         if( prev == 0 ) cell._objects = cur->_next;
         else            prev->_next   = cur->_next;
         _links.free( cur );
         break;
      }
   }

   // Remove cell if empty.
   if( cell._objects == NULL )
   {
      _cells.erase( cellIt );
      // Update occupancy.
      while( id._level > 0 )
      {
         uint64_t mask = computeMask( id );
         downLevel( id );

         Container::Iterator pcellIt = _cells.find(id);
         Cell& pcell       = pcellIt.data();
         pcell._occupancy ^= mask;

         // Stop recursion?
         if( (pcell._occupancy != 0) || (pcell._objects != NULL) ) return;

         // Cell is empty: erase and continue to parent.
         _cells.erase( pcellIt );
      }
   }
}

//------------------------------------------------------------------------------
//!
void
HGrid::findAllCollisions( void* data, collisionFunc func ) const
{
   Container::ConstIterator cellIt = _cells.begin();
   Container::ConstIterator end    = _cells.end();

   // Iterate over all cells.
   for( ; cellIt != end; ++cellIt )
   {
      const Cell& cell = cellIt.data();

      // Iterate over all objects in cell.
      for( Link* l = cell._objects; l; l = l->_next )
      {
         // Compute collisions with objects in the current cell.
         for( Link* l2 = l->_next; l2; l2 = l2->_next )
         {
            if( overlap( *l, *l2 ) ) func( data, l->_obj, l2->_obj );
         }

         // Compute collisions with objects in neighboring cells.
         const CellID id  = cellIt.key();
         int dx = l->_max[0]>>7;
         int dy = l->_max[1]>>7;
         int dz = l->_max[2]>>7;
         int cellsOverlap = (dx) | (dy<<1) | (dz<<2);

         switch( cellsOverlap )
         {
            case 0: break;
            case 1:
               findCollisions( l, id,  1,  0,  0, data, func ); // 14
               break;
            case 2:
               findCollisions( l, id, -1,  1,  0, data, func ); // 15
               findCollisions( l, id,  0,  1,  0, data, func ); // 16
               break;
            case 3:
               findCollisions( l, id,  1,  0,  0, data, func ); // 14
               findCollisions( l, id, -1,  1,  0, data, func ); // 15
               findCollisions( l, id,  0,  1,  0, data, func ); // 16
               findCollisions( l, id,  1,  1,  0, data, func ); // 17
               break;
            case 4:
               findCollisions( l, id, -1, -1,  1, data, func ); // 18
               findCollisions( l, id,  0, -1,  1, data, func ); // 19
               findCollisions( l, id, -1,  0,  1, data, func ); // 21
               findCollisions( l, id,  0,  0,  1, data, func ); // 22
               break;
            case 5:
               findCollisions( l, id,  1,  0,  0, data, func ); // 14
               findCollisions( l, id, -1, -1,  1, data, func ); // 18
               findCollisions( l, id,  0, -1,  1, data, func ); // 19
               findCollisions( l, id,  1, -1,  1, data, func ); // 20
               findCollisions( l, id, -1,  0,  1, data, func ); // 21
               findCollisions( l, id,  0,  0,  1, data, func ); // 22
               findCollisions( l, id,  1,  0,  1, data, func ); // 23
               break;
            case 6:
               findCollisions( l, id, -1,  1,  0, data, func ); // 15
               findCollisions( l, id,  0,  1,  0, data, func ); // 16
               findCollisions( l, id, -1, -1,  1, data, func ); // 18
               findCollisions( l, id,  0, -1,  1, data, func ); // 19
               findCollisions( l, id, -1,  0,  1, data, func ); // 21
               findCollisions( l, id,  0,  0,  1, data, func ); // 22
               findCollisions( l, id, -1,  1,  1, data, func ); // 24
               findCollisions( l, id,  0,  1,  1, data, func ); // 25
               break;
            case 7:
               findCollisions( l, id,  1,  0,  0, data, func ); // 14
               findCollisions( l, id, -1,  1,  0, data, func ); // 15
               findCollisions( l, id,  0,  1,  0, data, func ); // 16
               findCollisions( l, id,  1,  1,  0, data, func ); // 17
               findCollisions( l, id, -1, -1,  1, data, func ); // 18
               findCollisions( l, id,  0, -1,  1, data, func ); // 19
               findCollisions( l, id,  1, -1,  1, data, func ); // 20
               findCollisions( l, id, -1,  0,  1, data, func ); // 21
               findCollisions( l, id,  0,  0,  1, data, func ); // 22
               findCollisions( l, id,  1,  0,  1, data, func ); // 23
               findCollisions( l, id, -1,  1,  1, data, func ); // 24
               findCollisions( l, id,  0,  1,  1, data, func ); // 25
               findCollisions( l, id,  1,  1,  1, data, func ); // 26
               break;
         }

         // Compute collisions with objects in parents cells.
         CellID c0ID = id;
         CellID c1ID( id._x+dx, id._y+dy, id._z+dz, id._level );
         while( c0ID._level > 0 )
         {
            downLevel( c0ID );
            downLevel( c1ID );

            cellsOverlap = (c1ID._x-c0ID._x) | ((c1ID._y-c0ID._y)<<1) | ((c1ID._z-c0ID._z)<<2);

            findCollisions( l, id, c0ID, -1, -1, -1, data, func );
            findCollisions( l, id, c0ID,  0, -1, -1, data, func );
            findCollisions( l, id, c0ID, -1,  0, -1, data, func );
            findCollisions( l, id, c0ID,  0,  0, -1, data, func );
            findCollisions( l, id, c0ID, -1, -1,  0, data, func );
            findCollisions( l, id, c0ID,  0, -1,  0, data, func );
            findCollisions( l, id, c0ID, -1,  0,  0, data, func );
            findCollisions( l, id, c0ID,  0,  0,  0, data, func );

            switch( cellsOverlap )
            {
               case 0: break;
               case 1:
                  findCollisions( l, id, c0ID,  1, -1, -1, data, func );
                  findCollisions( l, id, c0ID,  1,  0, -1, data, func );
                  findCollisions( l, id, c0ID,  1, -1,  0, data, func );
                  findCollisions( l, id, c0ID,  1,  0,  0, data, func );
                  break;
               case 2:
                  findCollisions( l, id, c0ID, -1,  1, -1, data, func );
                  findCollisions( l, id, c0ID,  0,  1, -1, data, func );
                  findCollisions( l, id, c0ID, -1,  1,  0, data, func );
                  findCollisions( l, id, c0ID,  0,  1,  0, data, func );
                  break;
               case 3:
                  findCollisions( l, id, c0ID,  1, -1, -1, data, func );
                  findCollisions( l, id, c0ID,  1,  0, -1, data, func );
                  findCollisions( l, id, c0ID,  1, -1,  0, data, func );
                  findCollisions( l, id, c0ID,  1,  0,  0, data, func );
                  findCollisions( l, id, c0ID, -1,  1, -1, data, func );
                  findCollisions( l, id, c0ID,  0,  1, -1, data, func );
                  findCollisions( l, id, c0ID, -1,  1,  0, data, func );
                  findCollisions( l, id, c0ID,  0,  1,  0, data, func );
                  findCollisions( l, id, c0ID,  1,  1, -1, data, func );
                  findCollisions( l, id, c0ID,  1,  1,  0, data, func );
                  break;
               case 4:
                  findCollisions( l, id, c0ID, -1, -1,  1, data, func );
                  findCollisions( l, id, c0ID,  0, -1,  1, data, func );
                  findCollisions( l, id, c0ID, -1,  0,  1, data, func );
                  findCollisions( l, id, c0ID,  0,  0,  1, data, func );
                  break;
               case 5:
                  findCollisions( l, id, c0ID,  1, -1, -1, data, func );
                  findCollisions( l, id, c0ID,  1,  0, -1, data, func );
                  findCollisions( l, id, c0ID,  1, -1,  0, data, func );
                  findCollisions( l, id, c0ID,  1,  0,  0, data, func );
                  findCollisions( l, id, c0ID, -1, -1,  1, data, func );
                  findCollisions( l, id, c0ID,  0, -1,  1, data, func );
                  findCollisions( l, id, c0ID, -1,  0,  1, data, func );
                  findCollisions( l, id, c0ID,  0,  0,  1, data, func );
                  findCollisions( l, id, c0ID,  1, -1,  1, data, func );
                  findCollisions( l, id, c0ID,  1,  0,  1, data, func );
                  break;
               case 6:
                  findCollisions( l, id, c0ID, -1,  1, -1, data, func );
                  findCollisions( l, id, c0ID,  0,  1, -1, data, func );
                  findCollisions( l, id, c0ID, -1,  1,  0, data, func );
                  findCollisions( l, id, c0ID,  0,  1,  0, data, func );
                  findCollisions( l, id, c0ID, -1, -1,  1, data, func );
                  findCollisions( l, id, c0ID,  0, -1,  1, data, func );
                  findCollisions( l, id, c0ID, -1,  0,  1, data, func );
                  findCollisions( l, id, c0ID,  0,  0,  1, data, func );
                  findCollisions( l, id, c0ID, -1,  1,  1, data, func );
                  findCollisions( l, id, c0ID,  0,  1,  1, data, func );
                  break;
               case 7:
                  findCollisions( l, id, c0ID,  1, -1, -1, data, func );
                  findCollisions( l, id, c0ID,  1,  0, -1, data, func );
                  findCollisions( l, id, c0ID, -1,  1, -1, data, func );
                  findCollisions( l, id, c0ID,  0,  1, -1, data, func );
                  findCollisions( l, id, c0ID,  1,  1, -1, data, func );
                  findCollisions( l, id, c0ID,  1, -1,  0, data, func );
                  findCollisions( l, id, c0ID,  1,  0,  0, data, func );
                  findCollisions( l, id, c0ID, -1,  1,  0, data, func );
                  findCollisions( l, id, c0ID,  0,  1,  0, data, func );
                  findCollisions( l, id, c0ID,  1,  1,  0, data, func );
                  findCollisions( l, id, c0ID, -1, -1,  1, data, func );
                  findCollisions( l, id, c0ID,  0, -1,  1, data, func );
                  findCollisions( l, id, c0ID,  1, -1,  1, data, func );
                  findCollisions( l, id, c0ID, -1,  0,  1, data, func );
                  findCollisions( l, id, c0ID,  0,  0,  1, data, func );
                  findCollisions( l, id, c0ID,  1,  0,  1, data, func );
                  findCollisions( l, id, c0ID, -1,  1,  1, data, func );
                  findCollisions( l, id, c0ID,  0,  1,  1, data, func );
                  findCollisions( l, id, c0ID,  1,  1,  1, data, func );
                  break;
            }
         }
      }
   }
}
//------------------------------------------------------------------------------
//!
void
HGrid::findCollisions( void* data, void* objB, const AABBoxf& box, collisionFunc func ) const
{
   int min[3];
   int max[3];
   computeBoundingBox( box, min, max );

   CellID id0( min[0]>>27, min[1]>>27, min[2]>>27, 0 );
   CellID id1( max[0]>>27, max[1]>>27, max[2]>>27, 0 );
   --id0._x;
   --id0._y;
   --id0._z;

   for( int x = id0._x; x <= id1._x; ++x )
   {
      for( int y = id0._y; y <= id1._y; ++y )
      {
         for( int z = id0._z; z <= id1._z; ++z )
         {
            // Find cell.
            CellID id1( x, y, z, 0 );
            Container::ConstIterator cellIt = _cells.find(id1);

            // Cell exist?
            if( cellIt != _cells.end() )
            {
               // Compute collisions with objects in the cell.
               const Cell& cell = cellIt.data();
               for( Link* l = cell._objects; l; l = l->_next )
               {
                  if( overlap( min, max, id1, *l ) ) func( data, l->_obj, objB );
               }
               // Compute overlap mask.
               uint64_t mask  = computeMask( id1, min, max );
               uint64_t vmask = cell._occupancy & mask;

               // Test children.
               if( vmask == 0 ) continue;
               upLevel( id1 );
               for( int zz = 0; zz < 4; ++zz )
               {
                  if( vmask & 0xffff )
                  {
                     for( int yy = 0; yy < 4; ++yy )
                     {
                        if( vmask & 0xf )
                        {
                           for( int xx = 0; xx < 4; ++xx, vmask >>= 1 )
                           {
                              CellID id2( id1._x + xx, id1._y + yy, id1._z + zz, id1._level );
                              if( vmask & 1 ) findCollisions( id2, min, max, data, objB, func );
                           }
                        } else vmask >>= 4;
                     }
                  } else vmask >>= 16;
               }
            }
         }
      }
   }
}

//------------------------------------------------------------------------------
//!
void
HGrid::findCollisions(
   Link*         l,
   const CellID& id,
   int           dx,
   int           dy,
   int           dz,
   void*         data,
   collisionFunc func
) const
{
   // Find cell.
   CellID id2( id._x+dx, id._y+dy, id._z+dz, id._level );
   Container::ConstIterator cellIt = _cells.find(id2);

   // Cell not found?
   if( cellIt == _cells.end() ) return;

   // Compute collisions with objects in the cell.
   const Cell& cell = cellIt.data();
   for( Link* l2 = cell._objects; l2; l2 = l2->_next )
   {
      if( overlapSameLevel( id, *l, id2, *l2 ) ) func( data, l->_obj, l2->_obj );
   }
}

//------------------------------------------------------------------------------
//!
void
HGrid::findCollisions(
   Link* l,
   const CellID& id0,
   const CellID& id1,
   int           dx,
   int           dy,
   int           dz,
   void*         data,
   collisionFunc func
) const
{
   // Find cell.
   CellID id2( id1._x+dx, id1._y+dy, id1._z+dz, id1._level );
   Container::ConstIterator cellIt = _cells.find(id2);

   // Cell not found?
   if( cellIt == _cells.end() ) return;

   // Compute collisions with objects in the cell.
   const Cell& cell = cellIt.data();
   for( Link* l2 = cell._objects; l2; l2 = l2->_next )
   {
      if( overlap( id0, *l, id2, *l2 ) ) func( data, l->_obj, l2->_obj );
   }
}

//------------------------------------------------------------------------------
//! 
void 
HGrid::findCollisions( 
   const CellID& id, 
   int           min[3], 
   int           max[3], 
   void*         data, 
   void*         obj, 
   collisionFunc func
) const
{
   Container::ConstIterator cellIt = _cells.find(id);

   // Compute collisions with objects in the cell.
   const Cell& cell = cellIt.data();
   for( Link* l = cell._objects; l; l = l->_next )
   {
      if( overlap( min, max, id, *l ) ) func( data, l->_obj, obj );
   }

   // Compute overlap mask.
   uint64_t mask  = computeMask( id, min, max );
   uint64_t vmask = cell._occupancy & mask;

   // Test children.
   if( vmask == 0 ) return;
   CellID id1 = id;
   upLevel( id1 );
            
   for( int z = 0; z < 4; ++z )
   {
      if( vmask & 0xffff )
      {
         for( int y = 0; y < 4; ++y )
         {
            if( vmask & 0xf )
            {
               for( int x = 0; x < 4; ++x, vmask >>= 1 )
               {
                  CellID id2( id1._x + x, id1._y + y, id1._z + z, id1._level );
                  if( vmask & 1 ) findCollisions( id2, min, max, data, obj, func );
               }
            } else vmask >>= 4;
         }
      } else vmask >>= 16;
   }
}

NAMESPACE_END
