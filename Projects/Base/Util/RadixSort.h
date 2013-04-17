/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#ifndef BASE_RADIX_SORT_H
#define BASE_RADIX_SORT_H

#include <Base/StdDefs.h>

#include <Base/ADT/RCVector.h>
#include <Base/Util/RCP.h>

NAMESPACE_BEGIN

/*==============================================================================
  CLASS RadixSort
==============================================================================*/
class RadixSort
{
public:

   /*----- methods -----*/

   static BASE_DLL_API void  initializeIndices( const uint nValues, uint* indices );

   BASE_DLL_API RadixSort();
   BASE_DLL_API ~RadixSort();

   BASE_DLL_API RadixSort&  sort( const uint nValues, const uint* inputValues );
   BASE_DLL_API RadixSort&  sort( const uint nValues, const uint* inputValues, uint* sortedIndices );
   BASE_DLL_API RadixSort&  sort( const uint nValues, const uint* inputValues, uint* sortedIndices, uint* tempIndices );

   BASE_DLL_API RadixSort&  sort( const uint nValues, const float* inputValues );
   BASE_DLL_API RadixSort&  sort( const uint nValues, const float* inputValues, uint* sortedIndices );
   BASE_DLL_API RadixSort&  sort( const uint nValues, const float* inputValues, uint* sortedIndices, uint* tempIndices );

         uint*  sortedIndices()       { return _sortedIndices; }
   const uint*  sortedIndices() const { return _sortedIndices; }

   uint  sortedIndex( const uint i ) const { return _sortedIndices[i]; }

   RadixSort&  clear() { _indices[0] = NULL; _indices[1] = NULL; _sortedIndices = NULL; return *this; }

protected:

   /*----- data members -----*/

   RCP< RCVector<uint> >  _indices[2];   //!< Potentially-allocated temporary buffers (algo uses _sortedIndices and _tempIndices below).

   uint     _histograms[4][256];  // The number of occurences of every radix, for every byte.  Note that the byte index depends on endianness.
   uint*    _sortedIndices;       // The sorted indices (points to _indices[0]->data() when the user didn't specify a sortedIndices pointer).

   /*----- methods -----*/

   void  createHistograms( const uint nValues, const void* inputValues );
   void  guaranteeIndices( const uint nValues, const uint id );
   RadixSort&  commonSort( const uint nValues, const uint* inputValues, const bool allUnsigned, uint* sortedIndices, uint* tempIndices );

private:
}; //class RadixSort


NAMESPACE_END

#endif //BASE_RADIX_SORT_H
