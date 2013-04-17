/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#ifndef BASE_ID_POOL_H
#define BASE_ID_POOL_H

#include <Base/StdDefs.h>

#include <Base/Dbg/Defs.h>
#include <Base/IO/TextStream.h>
#include <Base/ADT/Vector.h>

NAMESPACE_BEGIN


/*==============================================================================
  CLASS IDPool
==============================================================================*/
//!< A class distributing ID while keeping track of available ones and reusing
//!< them as much as possible in order to have compact IDs.
//!< The implementation simply pushes the available IDs at the end in order to
//!< keep the list in decreasing order.  That makes the last one always the
//!< value to return.
//!< Also, if only one element is present in the list, then it marks the
//!< beginning of the open-ended space (everything beyond that point has to be
//!< available).
class IDPool
{
public:

   /*----- methods -----*/

   inline IDPool( const uint firstID = 0 );

   inline uint  next();

   inline void  release( const uint id );

protected:

   /*----- data members -----*/

   Vector<uint>  _pool;

   /*----- methods -----*/

   /* methods... */

private:
}; //class IDPool


//------------------------------------------------------------------------------
//!
inline
IDPool::IDPool( const uint firstID )
{
   _pool.pushBack( firstID );
}

//------------------------------------------------------------------------------
//!
uint
IDPool::next()
{
   //StdErr << "IDP::next()";
   //StdErr << " [ n=" << _pool.size();
   //for( uint i = 0; i < _pool.size(); ++i )
   //{
   //   StdErr << " " << _pool[i];
   //}
   //StdErr << " ]";

   uint id = _pool.back();
   if( _pool.size() > 1 )
   {
      _pool.popBack();
   }
   else
   {
      _pool[0] = id + 1;
   }

   //StdErr << " >> " << id;
   //StdErr << " [ n=" << _pool.size();
   //for( uint i = 0; i < _pool.size(); ++i )
   //{
   //   StdErr << " " << _pool[i];
   //}
   //StdErr << " ]";
   //StdErr << nl;
   return id;
}

//------------------------------------------------------------------------------
//!
void
IDPool::release( const uint id )
{
   //StdErr << "IDP::release(" << id << ")";
   //StdErr << " [ n=" << _pool.size();
   //for( uint i = 0; i < _pool.size(); ++i )
   //{
   //   StdErr << " " << _pool[i];
   //}
   //StdErr << " ]";

   // Find insertion position.
   Vector<uint>::Iterator pos = _pool.end();
   if( !_pool.empty() )
   {
      Vector<uint>::Iterator cur = pos;
      Vector<uint>::Iterator end = _pool.begin();
      while( cur != end )
      {
         --cur;
         if( (*cur) < id )
         {
            pos = cur;
            // Continue trickling towards the beginning.
         }
         else
         {
            // Found it!
            // Sanity check to make sure we don't release the ID twice.
            CHECK( (*cur) != id );
            break;
         }
      }
   }
   else
   {
      printf( "ERROR: IDPool::release() called on an empty pool." );
      CHECK( false );  // There should always be at least one element.
   }

   //StdErr << " pos=" << (size_t)(pos - _pool.begin());
   // Insert (equivalent to doing a bubble sort with the least number of swaps).
   _pool.insert( pos, id );

   // Compact representation if all values are contiguous.
   uint s1 = (uint)_pool.size() - 1;
   if( s1 > 0 && (_pool[0] - _pool[s1]) == s1 )
   {
      _pool.resize(1);
      _pool[0] -= s1;
   }

   //StdErr << " >> " << id;
   //StdErr << " [ n=" << _pool.size();
   //for( uint i = 0; i < _pool.size(); ++i )
   //{
   //   StdErr << " " << _pool[i];
   //}
   //StdErr << " ]";
   //StdErr << nl;
}


NAMESPACE_END

#endif //BASE_ID_POOL_H
