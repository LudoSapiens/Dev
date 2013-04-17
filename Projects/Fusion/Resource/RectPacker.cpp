/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#include <Fusion/Resource/RectPacker.h>

#include <CGMath/CGMath.h>
#include <CGMath/Random.h>

#include <Base/Dbg/DebugStream.h>
#include <Base/IO/FileDevice.h>
#include <Base/IO/StreamIndent.h>
#include <Base/Util/RadixSort.h>

USING_NAMESPACE

/*==============================================================================
  UNNAMED NAMESPACE
==============================================================================*/
UNNAMESPACE_BEGIN

DBG_STREAM( os_rp, "Packer" );

//------------------------------------------------------------------------------
//!
template< typename T > inline T
cmp( const T& a, const T& b )
{
   return CGM::sign( a - b );
}

//------------------------------------------------------------------------------
//!
inline void randomColor3( uchar* dst )
{
   static RNG_WELL rng;
   uint r;
   do
   {
      r = rng.getUInt();
   } while( (r & 0x00FFFFFF) == 0 ); // Make sure it is not black.

   dst[0] = (r & 0xFF);
   r >>= 8;
   dst[1] = (r & 0xFF);
   r >>= 8;
   dst[2] = (r & 0xFF);
}

//------------------------------------------------------------------------------
//!
void fillRect3( Bitmap& bmp, const Vec2i& pos, const Vec2i& size, uchar* color )
{
   Vec2i tr = pos + size;
   for( int y = pos.y; y < tr.y; ++y )
   {
      uchar* dst = bmp.pixel( Vec2i(pos.x, y) );
      for( int x = pos.x; x < tr.x; ++x )
      {
         // Check that nothing overlaps.
         CHECK( dst[0] == 0 && dst[1] == 0 && dst[2] == 0 );
         // Copy the color.
         dst[0] = color[0];
         dst[1] = color[1];
         dst[2] = color[2];
         dst += 3;
      }
   }
}

//------------------------------------------------------------------------------
//! Grows the smallest dimension until at least |dimToAdd| pixels are added.
Vec2i growPowerOfTwo( const Vec2i& dim, const Vec2i& dimToAdd )
{
   Vec2i tmp = dim;
   int nToAdd = dimToAdd.x * dimToAdd.y;
   CHECK( nToAdd );  // If it is zero, how come we need to grow?!?
   do
   {
      int   a = tmp.minComponent();
      int& va = tmp(a);
      int nva = CGM::nextPow2( va + 1 ); // Simply doubling would work only if dim was pow2 to begin with.
      nToAdd -= (nva-va) * tmp(1-a);
      va = nva;
   } while( nToAdd > 0 );
   return tmp;
}

//------------------------------------------------------------------------------
//! Grows the smallest dimension trying to reach the largest, but only enough to
//! contain |dimToAdd| pixels.
//! If the size is square, uses the height (since Vec2i::minComponent() returns 1 when equal).
Vec2i growNonPowerOfTwo( const Vec2i& dim, const Vec2i& dimToAdd )
{
   Vec2i tmp = dim;
   int nToAdd = dimToAdd.x * dimToAdd.y;
   CHECK( nToAdd );  // If it is zero, how come we need to grow?!?
   CHECK( dim.x * dim.y != 0 ); // Could result in a divide by zero below.
   do
   {
      // va < vb.
      int   a = tmp.minComponent();
      int  vb = tmp(1-a);
      int   d = (nToAdd + vb - 1) / vb; // Make sure we add at least 1.
      tmp(a) += d;
      nToAdd -= vb * d;
   } while( nToAdd > 0 );
   return tmp;
}

int _dbg_packGreedyA             = 0;
int _dbg_packGreedyB             = 0;
int _dbg_packGreedyC             = 0;
int _dbg_packGreedyVisitChildren = 0;
int _dbg_packGreedyVisitLeaf     = 0;

#if 0
//------------------------------------------------------------------------------
//!
void dump(
   const ConstArrayAdaptor<Vec2i>&  sizes,
   const size_t  n,
   const Vec2i&  maxSize,
   const bool    forcePowerOfTwo,
   const bool    allowRotation
)
{
   TextStream os( new FileDevice("/Users/jph/src/Dev/Projects/rectPacker.txt", IODevice::MODE_WRITE) );
   os << maxSize << " " << forcePowerOfTwo << " " << allowRotation << nl;
   os << n << nl;
   for( size_t i = 0; i < n; ++i )
   {
      os << sizes[i] << nl;
   }
}

//------------------------------------------------------------------------------
//!
inline void print( const RectPackerGreedyKdTree::Node& node )
{
   TextStream& os = StdErr;
   os << "node_" << (void*)&node << " id=" << node._id << " split=" << node._split;
   if( node._split < 0 )
   {
      os << " v bot=" << node._child[0] << " top=" << node._child[1];
   }
   else
   {
      os << " h lft=" << node._child[0] << " rgt=" << node._child[1];
   }
   os << nl;
   //return os;
}

//------------------------------------------------------------------------------
//!
void print( const RectPackerGreedyKdTree::Node& node, StreamIndent& indent )
{
   TextStream& os = StdErr;
   os << indent << "node_" << (void*)&node << " id=" << node._id << " split=" << node._split;
   if( node._split < 0 )
   {
      os << " v bot=" << node._child[0] << " top=" << node._child[1];
   }
   else
   {
      os << " h lft=" << node._child[0] << " rgt=" << node._child[1];
   }
   os << nl;
   //return os;
}

//------------------------------------------------------------------------------
//!
void printSubTree( const RectPackerGreedyKdTree::Node& root, StreamIndent& indent )
{
   TextStream& os = StdErr;
   print( root, indent );
   ++indent;
   if( root._child[0] != NULL )  printSubTree( *root._child[0], indent );
   else                          os << indent << "NULL" << nl;
   if( root._child[1] != NULL )  printSubTree( *root._child[1], indent );
   else                          os << indent << "NULL" << nl;
   --indent;
}

//------------------------------------------------------------------------------
//!
void printTree( const RectPackerGreedyKdTree::Node& root )
{
   StdErr << "+++++" << nl;
   StreamIndent indent;
   printSubTree( root, indent );
   StdErr << "-----" << nl;
}
#endif

UNNAMESPACE_END


/*==============================================================================
  CLASS RectPacker
==============================================================================*/

//------------------------------------------------------------------------------
//!
RectPacker::RectPacker( size_t n )
{
   reserve( n );
}

//------------------------------------------------------------------------------
//!
RectPacker::~RectPacker()
{
}

//------------------------------------------------------------------------------
//!
void
RectPacker::clear()
{
   _rects.clear();
}

//------------------------------------------------------------------------------
//!
Vec2i
RectPacker::guessStartSize( const ConstArrayAdaptor<Vec2i>& sizes, const size_t n, const bool forcePowerOfTwo )
{
   Vec2i  max = Vec2i::zero();  // The maximum width and height.
   int    tot = 0;              // The total number of pixels.
   for( size_t i = 0; i < n; ++i )
   {
      const Vec2i& size = sizes[i];
      max = max.max( size );
      tot += size.x * size.y;
   }
   int edge = CGM::ceili( CGM::sqrt( (float)tot ) );
   Vec2i size = max.max( Vec2i(edge) );

   if( forcePowerOfTwo )
   {
      // Force power-of-2.
      size = Vec2i( CGM::nextPow2(size.x), CGM::nextPow2(size.y) );
      // But don't grow too much needlessly.
      if( size.x*size.y >= tot*2 )
      {
         uint a = size.maxComponent();
         if( size(a)/2 >= max(a) ) size(a) /= 2;
         else if( size(1-a)/2 >= max(1-a) ) size(1-a) /= 2;
      }
   }

   return size;
}

//------------------------------------------------------------------------------
//!
void
RectPacker::print( TextStream& os ) const
{
   os << "RectPacker solution:" << nl;
   os << _rects.size() << " rects packing in " << _size << nl;
   for( uint i = 0; i < _rects.size(); ++i )
   {
      const RectInfo& ri = _rects[i];
      os << i << "> " << ri << nl;
   }
}

//------------------------------------------------------------------------------
//!
RCP<Bitmap>
RectPacker::getBitmap() const
{
   RCP<Bitmap> bmp = new Bitmap( _size, Bitmap::BYTE, 3 );
   // Fill in with black.
   memset( bmp->pixels(), 0, bmp->size() );
   uchar color[3];

   // Iterate over all rectangles in solution, and fill in pixels.
   for( uint i = 0; i < _rects.size(); ++i )
   {
      const RectInfo& ri = _rects[i];
      randomColor3( color );
      fillRect3( *bmp, ri._position, ri._size, color );
   }

   return bmp;
}


/*==============================================================================
  CLASS RectPackerGreedyKdTree
==============================================================================*/

//------------------------------------------------------------------------------
//!
RectPackerGreedyKdTree::RectPackerGreedyKdTree( size_t n ):
   RectPacker( n )
{
}

//------------------------------------------------------------------------------
//!
RectPackerGreedyKdTree::~RectPackerGreedyKdTree()
{
#if 0
   StdErr << "packGreedyA() called " << _dbg_packGreedyA << nl;
   StdErr << "packGreedyB() called " << _dbg_packGreedyB << nl;
   StdErr << "packGreedyC() called " << _dbg_packGreedyC << nl;
   StdErr << "packGreedyVisitChildren() called " << _dbg_packGreedyVisitChildren << nl;
   StdErr << "packGreedyVisitLeaf() called " << _dbg_packGreedyVisitLeaf << nl;
#endif
}

//------------------------------------------------------------------------------
//!
void
RectPackerGreedyKdTree::clear()
{
   _pool.clear();
   RectPacker::clear();
}

//------------------------------------------------------------------------------
//!
bool
RectPackerGreedyKdTree::pack( const Vector<Vec2i>& sizes, const Vec2i& texSize )
{
   ++_dbg_packGreedyA;
   clear();

   _size = texSize;  // Update it prior to the loop in case it fails (we can get the result so far).
   int s = int(sizes.size());
   resize( s );
   Node* root = _pool.construct();
   for( int id = 0; id < s; ++id )
   {
      AARecti rect = AARecti( texSize );
      const Vec2i& s = sizes[id];
      if( search(s, id, root, rect, false) )
      {
         RectInfo& ri = _rects[id];
         ri._size     = s;
         ri._position = rect.position();
         ri._flipped  = false;
         //StdErr << "Fit: " << ri << nl;
         //StdErr << ri._size << " vs. " << rect.size() << nl;
         CHECK( ri._size == rect.size() );
         //StdErr << nl;
      }
      else
      {
         _pool.clear();
         return false;
      }
   }
   _pool.clear();
   return true;
}

//------------------------------------------------------------------------------
//!
bool
RectPackerGreedyKdTree::pack(
   const ConstArrayAdaptor<Vec2i>&  sizes,
   const size_t  n,
   const Vec2i&  maxSize,
   const bool    forcePowerOfTwo,
   const bool    allowRotation
)
{
   ++_dbg_packGreedyB;
   //dump( sizes, n, maxSize, forcePowerOfTwo, allowRotation );

   _size =  guessStartSize( sizes, n, forcePowerOfTwo );
   int nid = (int)n;

   //StdErr << "Guessed a size of " << _size << nl;

   bool ok = false;
   do
   {
      if( _size.x > maxSize.x || _size.y > maxSize.y )
      {
         // Busted allowed size, stop.
         break;
      }

      // Start with clean structures.
      clear();
      _pool.clear();
      resize( n );

      bool failed = false;
      Node* root = _pool.construct();
      for( int id = 0; id < nid; ++id )
      {
         //StdErr << nl;
         AARecti rect = AARecti( _size );
         const Vec2i& s = sizes[id];
         //StdErr << "Entry #" << id << ": " << s << nl;
         bool nonSquare = (s.x != s.y);
         if( search(s, id, root, rect, allowRotation && nonSquare) )
         {
            RectInfo& ri = _rects[id];
            ri._size     = rect.size();
            ri._position = rect.position();
            ri._flipped  = (s.x != rect.width());
            //StdErr << "Fit: " << ri << nl;
            //StdErr << ri._size << " vs. " << rect.size() << nl;
            CHECK( (!ri._flipped && (ri._size == s)) ||
                   ( ri._flipped && (ri._size == Vec2i(s.y, s.x)) ) );
            //StdErr << nl;
            //printTree( *root );
         }
         else
         {
            // Failed at packing one rect.
            if( forcePowerOfTwo )  _size = growPowerOfTwo( _size, s );
            else                   _size = growNonPowerOfTwo( _size, s );
            // Loop again...
            failed = true;
            break;
         }
      }

      if( !failed )
      {
         // We found a solution.
         ok = true;
         break;
      }
   } while( true );

   // Clear temporary memory.
   _pool.clear();

   return ok;
}

//------------------------------------------------------------------------------
//! Does a hierarchical descent into the kd-tree trying to fit the specified
//! sizeToInsert into an available leaf.
//! The rect parameter represents the bounding box of the current node.
bool
RectPackerGreedyKdTree::search( const Vec2i& sizeToInsert, int id, Node* node, AARecti& rect, bool allowRotation )
{
   ++_dbg_packGreedyC;
   //StdErr << "PackGreedy(" << sizeToInsert << ", " << id << ", " << (void*)node << ", " << rect << ")" << nl;
   //::print( *node );
   CHECK( node );
   if( node->_id == Node::USED_BY_CHILDREN )  return false;
   if( node->_child[0] != NULL )
   {
      if( visitChildren(sizeToInsert, id, node, rect, allowRotation) )
      {
         return true;
      }
      else
      if( allowRotation )
      {
         Vec2i flippedSize = Vec2i( sizeToInsert.y, sizeToInsert.x );
         return visitChildren( flippedSize, id, node, rect, false ); // Don't allow rotation this time.
      }
      else
      {
         return false;
      }
   }
   else
   {
      if( visitLeaf( sizeToInsert, id, node, rect, allowRotation ) )
      {
         return true;
      }
      else
      if( allowRotation )
      {
         Vec2i flippedSize = Vec2i( sizeToInsert.y, sizeToInsert.x );
         return visitLeaf( flippedSize, id, node, rect, false ); // Don't allow rotation this time.
      }
      else
      {
         return false;
      }
   }
}

//------------------------------------------------------------------------------
//!
bool
RectPackerGreedyKdTree::visitChildren(
   const Vec2i&  sizeToInsert,
   int           id,
   Node*   node,
   AARecti&      rect,
   bool          allowRotation
)
{
   ++_dbg_packGreedyVisitChildren;
   int tmp;
   if( node->_split < 0 )
   {
      // Vertical split.
      //StdErr << "V:" << -(node->_split) << nl;

      // Try bottom child.
      tmp = rect.top();
      rect.top( -node->_split );
      if( search(sizeToInsert, id, node->_child[0], rect, allowRotation) )
      {
         node->propagateChildUsage();
         return true;
      }
      rect.top( tmp ); // Restore rect.

      // Try top child.
      tmp = rect.bottom();
      rect.bottom( -node->_split );
      if( search(sizeToInsert, id, node->_child[1], rect, allowRotation) )
      {
         node->propagateChildUsage();
         return true;
      }
      rect.bottom( tmp ); // Restore rect.
   }
   else
   {
      // Horizontal split.
      //StdErr << "H:" << node->_split << nl;

      // Try left child.
      tmp = rect.right();
      rect.right( node->_split );
      if( search(sizeToInsert, id, node->_child[0], rect, allowRotation) )
      {
         node->propagateChildUsage();
         return true;
      }
      rect.right( tmp ); // Restore rect.

      // Try right child.
      tmp = rect.left();
      rect.left( node->_split );
      if( search(sizeToInsert, id, node->_child[1], rect, allowRotation) )
      {
         node->propagateChildUsage();
         return true;
      }
      rect.left( tmp ); // Restore rect.
   }

   return false;
}

//------------------------------------------------------------------------------
//!
bool
RectPackerGreedyKdTree::visitLeaf(
   const Vec2i&  sizeToInsert,
   int           id,
   Node*   node,
   AARecti&      rect,
   bool          allowRotation
)
{
   ++_dbg_packGreedyVisitLeaf;
   //StdErr << "LeafNode: " << node->_id << nl;

   if( node->_id >= 0 )  return false;  // Leaf is already in use.

   int cmpY = 1 + cmp( sizeToInsert.y, rect.size().y );  // 0: Fits, 1: Exact, 2: Too big
   int cmpX = 1 + cmp( sizeToInsert.x, rect.size().x );
   uint cmpCode = 3*cmpY + cmpX;
   //StdErr << "cmpCode=" << cmpCode << " size=" << sizeToInsert << " into rect=" << rect << nl;
   switch( cmpCode )
   {              // WIDTH       HEIGHT
      case 0x00:   // Fits        Fits
         // Decide whether we split horizontally or vertically.
         //       a    b            a    b
         //    +-----+---+       +-----+---+
         //  c |     |   |       |     |   | c
         //    +-----+---+  vs.  +-----+   +
         //  d |         |       |     |   | d
         //    |         |       |     |   |
         //    +-----+---+       +-----+---+
         //     (v first)         (h first)
         // We try to minimize the perimeters of the resulting rectangles:
         //   2*(b+c  ) + 2*(a+b+d)
         //   2*(b+c+d) + 2*(a  +d)
         // which ends up comparing b vs. d.
         // If b < d, than the first perimeter sum is less, and we want to split
         // vertically first.
         if( (rect.width() - sizeToInsert.x) < (rect.height() - sizeToInsert.y) )
         {
            // Better split vertically first.
            //StdErr << ">> Prefer vertical" << nl;
            int top = rect.bottom() + sizeToInsert.y;
            node->_split    = -top;
            node->_child[0] = _pool.construct();
            node->_child[1] = _pool.construct();
            int tmp = rect.top();
            rect.top( top );
            bool ok = search( sizeToInsert, id, node->_child[0], rect, allowRotation );
            if( ok )
            {
               node->propagateChildUsage();
               return true;
            }
            //if( ok )  return true; // No need to propagateChildUsage, since it can't happen.
            rect.top( tmp ); // Restore rect.
            return false;
         }
         else
         {
            // Better split horizontally first.
            //StdErr << ">> Prefer horizontal" << nl;
            int right = rect.left() + sizeToInsert.x;
            node->_split    = right;
            node->_child[0] = _pool.construct();
            node->_child[1] = _pool.construct();
            int tmp = rect.right();
            rect.right( right );
            bool ok = search( sizeToInsert, id, node->_child[0], rect, allowRotation );
            if( ok )
            {
               node->propagateChildUsage();
               return true;
            }
            //if( ok )  return true; // No need to propagateChildUsage, since it can't happen.
            rect.right( tmp ); // Restore rect.
            return false;
         }
      case 0x01:  // Exact       Fits
         // Use bottom part.
         //StdErr << ">> Use bottom." << nl;
         node->_split    = -(rect.bottom() + sizeToInsert.y);
         node->_child[0] = _pool.construct();
         node->_child[1] = _pool.construct();
         node->_child[0]->_id = id;
         rect.top( -node->_split );
         return true;
      case 0x02:  // Too big     Fits
         return false;
      case 0x03:  // Fits        Exact
         // Use left part.
         //StdErr << ">> Use left." << nl;
         node->_split    = rect.left() + sizeToInsert.x;
         node->_child[0] = _pool.construct();
         node->_child[1] = _pool.construct();
         node->_child[0]->_id = id;
         rect.right( node->_split );
         return true;
      case 0x04:  // Exact       Exact
         // Just use it.
         //StdErr << ">> Use exact." << nl;
         node->_id = id;
         return true;
      case 0x05:  // Too big     Exact
      case 0x06:  // Fits        Too big
      case 0x08:  // Too big     Too big
      case 0x07:  // Exact       Too big
         return false;
      default:
         CHECK( false );
         return false;
   }

}

//------------------------------------------------------------------------------
//!
void
RectPackerGreedyKdTree::destroy( Node* node )
{
   if( node )
   {
      destroy( node->_child[0] );
      destroy( node->_child[1] );
      _pool.destroy( node );
   }
}


/*==============================================================================
  CLASS RectPackerGreedy
==============================================================================*/

//------------------------------------------------------------------------------
//!
RectPackerGreedy::RectPackerGreedy( size_t n ):
   RectPacker( n ),
   _smallest( NULL ),
   _largest( NULL ),
   _latest( NULL )
{
}

//------------------------------------------------------------------------------
//!
RectPackerGreedy::~RectPackerGreedy()
{
}

//------------------------------------------------------------------------------
//!
void
RectPackerGreedy::clear()
{
   _clsPool.clear();
   _grpPool.clear();
   _regPool.clear();
   RectPacker::clear();
   _largest = _smallest = _latest = NULL;
}

//------------------------------------------------------------------------------
//!
bool
RectPackerGreedy::pack( const Vector<Vec2i>& sizes, const Vec2i& texSize )
{
   return pack( ConstArrayAdaptor<Vec2i>(sizes.data(), sizeof(sizes[0])), sizes.size(), texSize, true, true );
}

//------------------------------------------------------------------------------
//!
bool
RectPackerGreedy::pack(
   const ConstArrayAdaptor<Vec2i>&  sizes,
   const size_t                     n,
   const Vec2i&                     maxSize,
   const bool                       forcePowerOfTwo,
   const bool                       allowRotation
)
{
   DBG_BLOCK( os_rp, "RectPackerGreedy::pack( ..., " << n << ", max=" << maxSize << ", fp2=" << forcePowerOfTwo << ", ar=" << allowRotation << ")" );
   CHECK( allowRotation );  // Mandatory.

   if( n == 0 )  return true;

   //dump( sizes, n, maxSize, forcePowerOfTwo, allowRotation );

   resize( n );
   _maxSize = maxSize;
   _curSize = Vec2i( 0 );

   // Sort the input sizes by maximum size, minimum size.
   RadixSort  radix;
   Vec2i*     maxMinSizes = new Vec2i[n];
   uint*      keys        = new uint[n];
   uint*      sortedMin   = new uint[n];
   uint*      sorted      = new uint[n];
   // 1. Extract min and max sizes.
   for( size_t i = 0; i < n; ++i )
   {
      const Vec2i& src = sizes[i];
      Vec2i& dst = maxMinSizes[i];
      int maxComp = src.maxComponent();
      dst.x = src(   maxComp );
      dst.y = src( 1-maxComp );
   }

   // 2. Sort by minimum size (secondary key).
   // 2.1 Set the minimum size in the sorting table.
   for( size_t i = 0; i < n; ++i )
   {
      keys[i] = (uint)maxMinSizes[i].y;
   }
   // 2.2 Do the sort.
   radix.initializeIndices( uint(n), sortedMin );
   radix.sort( uint(n), keys, sortedMin );

   // 3. Sort by maximum size (primary key).
   // 3.1 Set the maximum size using the order yielded by the previous sort.
   for( size_t i = 0; i < n; ++i )
   {
      keys[i] = (uint)maxMinSizes[sortedMin[i]].x;
   }
   // 3.2 Do the sort.
   radix.clear();
   radix.sort( uint(n), keys );
   // 3.3 Remap the sorted indices into original sizes.
   for( size_t i = 0; i < n; ++i )
   {
      sorted[i] = sortedMin[radix.sortedIndices()[i]];
   }

#if 0
   StdErr << "Sorted indices:" << nl;
   for( size_t i = 0; i < n; ++i )
   {
      StdErr << i << " --> " << sortedMin[i] << " --> " << radix.sortedIndices()[i] << " --> " << sorted[i] << nl;
   }
#endif

#if 1
   // Bootstrap with the first size in power-of-two.
   uint si = sorted[n-1];
   _curSize = maxMinSizes[si];
   // Make current size a power of 2.
   //_curSize.x = CGM::nextPow2( _curSize.x );
   //_curSize.y = CGM::nextPow2( _curSize.y );
   Region* startReg = createRegion( Vec2i(0, 0), false, NULL );
   addRegion( startReg, _curSize );
   DBG_MSG( os_rp, "Bootstrapping with sizes[" << si << "]=" << _curSize );

   uint cur = uint(n); // Start by biggest size.
   RegionClass*  cls;
   RegionGroup*  grp;
   Region*       reg;
   do
   {
      DBG_MSG( os_rp, "" );
      --cur;
      si = sorted[cur];
      const Vec2i& size  = sizes[si];
      const Vec2i& ssize = maxMinSizes[si];

      DBG_MSG( os_rp, "Placing #" << cur << " (si=" << si << "): size=" << size << " ssize=" << ssize );

      if( !findCandidate( ssize, cls, grp, reg ) )
      {
         DBG_MSG( os_rp, "Need to create a region of " << ssize );
         // Create a new region to hold the size.
         if( !createNewRegion( ssize, cls, grp, reg ) )
         {
            // TODO: Allow for multiple textures.
            StdErr << "ERROR - Could not create a new region for a size #" << si << " (size=" << size << ", cur=" << cur << ")." << nl;
            return false;
         }
      }

      bool oriFlipped = size.x != ssize.x;
      bool flipped    = reg->_flipped;
      DBG_MSG( os_rp, "reg: " << cls->_max << "x" << grp->_min << " @ " << reg->_pos << " flipped=" << flipped );

      // Place current size into the region.
      // 1. Register solution.
      RectInfo& rect = _rects[si];
      rect._position = reg->_pos;
      rect._size     = flipped ? ssize( 1, 0 ) : ssize;
      rect._flipped  = flipped != oriFlipped;
      DBG_MSG( os_rp, "Solution: " << rect );

      int x, y;
      if( flipped )  { x = 1; y = 0; }
      else           { x = 0; y = 1; }

      // 2. Split leftover space into up to 2 groups.
      if( ssize.x < cls->_max )
      {
         // H split.
         if( ssize.y < grp->_min )
         {
            DBG_MSG( os_rp, "H & V split" );
            // H & V split.
            // +---+-----+
            // |top|     |
            // |   |right|
            // +---+     +
            // |###|     |
            // +---+-----+
            Region* regT   = createRegion();
            regT->_pos     = reg->_pos;
            regT->_pos(y) += ssize.y;
            Vec2i sizeT    = Vec2i( ssize.x, grp->_min-ssize.y );
            DBG_MSG( os_rp, "sizeT: " << sizeT );
            regT->_flipped = (sizeT.y > sizeT.x);
            if( regT->_flipped )  sizeT = sizeT( 1, 0 );
            regT->_flipped ^= flipped;
            addRegion( regT, sizeT );

            Region* regR   = reg;
            //regR->_pos   = reg->_pos;
            regR->_pos(x) += ssize.x;
            Vec2i sizeR    = Vec2i( cls->_max-ssize.x, grp->_min );
            DBG_MSG( os_rp, "sizeR: " << sizeR );
            regR->_flipped = (sizeR.y > sizeR.x);
            if( regR->_flipped )  sizeR = sizeR( 1, 0 );
            regR->_flipped ^= flipped;
            addRegion( regR, sizeR );
         }
         else
         {
            DBG_MSG( os_rp, "H-only split" );
            // H-only split.
            // +---+-----+
            // |###|     |
            // |###|right|
            // |###|     |
            // |###|     |
            // +---+-----+
            Region* regR   = reg;
            //regR->_pos   = reg->_pos;
            regR->_pos(x) += ssize.x;
            Vec2i sizeR    = Vec2i( cls->_max-ssize.x, ssize.y );
            regR->_flipped = (sizeR.y > sizeR.x);
            if( regR->_flipped )  sizeR = sizeR( 1, 0 );
            regR->_flipped ^= flipped;
            addRegion( regR, sizeR );
         }
      }
      else
      {
         // Tight in H.
         if( ssize.y < grp->_min )
         {
            DBG_MSG( os_rp, "V-only split" );
            // V-only split.
            // +---------+
            // |  top    |
            // |         |
            // +---------+
            // |#########|
            // +---------+
            Region* regT   = reg;
            //regT->_pos   = reg->_pos;
            regT->_pos(y) += ssize.y;
            Vec2i sizeT    = Vec2i( ssize.x, grp->_min-ssize.y );
            DBG_MSG( os_rp, "sizeT: " << sizeT );
            regT->_flipped = (sizeT.y > sizeT.x);
            if( regT->_flipped )  sizeT = sizeT( 1, 0 );
            regT->_flipped ^= flipped;
            addRegion( regT, sizeT );
         }
         else
         {
            DBG_MSG( os_rp, "Perfect" );
            // Perfect fit.
            // +---------+
            // |#########|
            // |#########|
            // |#########|
            // |#########|
            // +---------+
            // Nothing to do.

         }
      }

      //printUnusedRegions();

   } while( cur != 0 );
#endif

   delete [] sorted;
   delete [] sortedMin;
   delete [] keys;
   delete [] maxMinSizes;

   _size = _curSize;
   if( forcePowerOfTwo )
   {
      _size.x = CGM::nextPow2( _size.x );
      _size.y = CGM::nextPow2( _size.y );
   }
   //print();
   //printUnusedRegions();

   //RCP<Bitmap> bmp = getBitmap();
   //bmp->saveFile( "/Users/jph/src/Dev/Projects/pack.png" );

   return true;
}

//------------------------------------------------------------------------------
//! Returns the class of the specified size (creates it if necessary).
//! This one starts at the last position, and search in the proper direction
//! based on the required size.
RectPackerGreedy::RegionClass&
RectPackerGreedy::getClass( int max )
{
   if( _latest != NULL )
   {
      switch( 1 + cmp(max, _latest->_max) )
      {
         case 0:  // max <  _latest->_max
            _latest = getClassLTE( max, _latest );
            return *_latest;
         case 1:  // max == _latest->_max
            return *_latest;
         case 2:  // max >  _latest->_max
            _latest = getClassGTE( max, _latest );
            return *_latest;
         default:
            CHECK( false );
            return *_latest;
      }
   }
   else
   {
      // Create the first class.
      _latest = createClass( max, NULL, NULL );
      _smallest = _latest;
      _largest  = _latest;
      return *_latest;
   }
}

//------------------------------------------------------------------------------
//!
RectPackerGreedy::RegionClass*
RectPackerGreedy::getClassLTE( int max, RegionClass* cur )
{
   do
   {
      switch( 1+cmp(cur->_max, max) )
      {
         case 0:  // cur->_max <  max
         {  // Passed the desired max, add it right after cur.
            RegionClass* cls = createClass( max, cur->_next, cur );
            cur->_next = cls;
            if( cls->_next )  cls->_next->_prev = cls;
            else              _largest = cls;
           return cls;
         }
         case 1:  // cur->_max == max
            // Found it.
            return cur;
         case 2:  // cur->_max >  max
            // Keep going.
            cur = cur->_prev;
            break;
         default:
            CHECK( false );
            break;
      }
   } while( cur != NULL );

   // Reached the end of the list.
   RegionClass* cls = createClass( max, _smallest, NULL );
   if( _smallest )  _smallest->_prev = cls;
   else             _largest = cls;
   _smallest = cls;
   return cls;
}

//------------------------------------------------------------------------------
//!
RectPackerGreedy::RegionClass*
RectPackerGreedy::getClassGTE( int max, RegionClass* cur )
{
   do
   {
      switch( 1+cmp(cur->_max, max) )
      {
         case 0:  // cur->_max <  max
            // Keep going.
            cur = cur->_next;
            break;
         case 1:  // cur->_max == max
            // Found it.
            return cur;
         case 2:  // cur->_max >  max
         {
            // Passed the desired max, add it right before cur.
            RegionClass* cls = createClass( max, cur, cur->_prev );
            cur->_prev = cls;
            if( cls->_prev )  cls->_prev->_next = cls;
            else              _smallest = cls;
            return cls;
         }
         default:
            CHECK( false );
            break;
      }
   } while( cur != NULL );

   // Reached the end of the list.
   RegionClass* cls = createClass( max, NULL, _largest );
   if( _largest )  _largest->_next = cls;
   else            _smallest = cls;
   _largest = cls;
   return cls;
}

//------------------------------------------------------------------------------
//!
RectPackerGreedy::RegionGroup&
RectPackerGreedy::getGroup( RegionClass& cls, int min )
{
   RegionGroup* cur  = cls._first;
   if( cur != NULL )
   {
      switch( 1+cmp(cur->_min, min) )
      {
         case 0:  // cur->_min <  min
            // Keep going (below).
            break;
         case 1:  // cur->_min == min
            // Found it.
            return *cur;
         case 2:  // cur->_min >  min
         {
            // Passed the desired min, add it right before cur.
            RegionGroup* grp = createGroup( min, cur );
            cls._first = grp;
            return *grp;
         }
         default:
            CHECK( false );
            break;
      }

      RegionGroup* prev = cur;
      cur = cur->_next;
      while( cur != NULL )
      {
         switch( 1+cmp(cur->_min, min) )
         {
            case 0:  // cur->_min <  min
               // Keep going.
               prev = cur;
               cur  = cur->_next;
               break;
            case 1:  // cur->_min == min
               // Found it.
               return *cur;
            case 2:  // cur->_min >  min
            {
               // Passed the desired min, add it right before cur.
               RegionGroup* grp = createGroup( min, cur );
               prev->_next = grp;
               return *grp;
            }
            default:
               CHECK( false );
               break;
         }
      }

      // Searched the whole list and didn't find it.
      // Add it at the end.
      RegionGroup* grp = createGroup( min, NULL );
      prev->_next = grp;
      return *grp;
   }
   else
   {
      // No group present.
      // Add it as the first one.
      RegionGroup* grp = createGroup( min, NULL );
      cls._first = grp;
      return *grp;
   }
}

//------------------------------------------------------------------------------
//!
void
RectPackerGreedy::addRegion( Region* reg, const Vec2i& size )
{
   DBG_BLOCK( os_rp, "addRegion size=" << size << " reg=" << (void*)reg << "_pos=" << reg->_pos << "_flipped=" << reg->_flipped );
   RegionClass& cls = getClass( size.x );
   RegionGroup& grp = getGroup( cls, size.y );
   reg->_next = grp._first;
   grp._first = reg;
}

//------------------------------------------------------------------------------
//!
void
RectPackerGreedy::addRegion( RegionGroup& grp, Region* reg )
{
   DBG_BLOCK( os_rp, "addRegion grp._min=" << grp._min << " reg=" << (void*)reg << "_pos=" << reg->_pos << "_flipped=" << reg->_flipped );
   reg->_next = grp._first;
   grp._first = reg;
}

//------------------------------------------------------------------------------
//!
bool
RectPackerGreedy::createNewRegion( const Vec2i& requiredSize, RegionClass*& cls, RegionGroup*& grp, Region*& reg )
{
   unused( requiredSize );
   DBG_BLOCK( os_rp, "createNewRegion " << requiredSize << ") _curSize=" << _curSize );
   // Double the current size.
   if( _curSize.x < _curSize.y )
   {
      // Horizontally.
      int s = _curSize.x * 2;
      if( s > _maxSize.x )
      {
         StdErr << "Cannot grow bigger than " << _maxSize << " from " << _curSize << nl;
         return false;
      }
      cls = &getClass( _curSize.y );
      grp = &getGroup( *cls, _curSize.x );
      reg = createRegion( Vec2i(_curSize.x, 0), true, grp->_first );
      //grp->_first = reg;
      _curSize.x = s;
   }
   else
   {
      // Vertically.
      int s = _curSize.y * 2;
      if( s > _maxSize.y )
      {
         StdErr << "Cannot grow bigger than " << _maxSize << " from " << _curSize << nl;
         return false;
      }
      cls = &getClass( _curSize.x );
      grp = &getGroup( *cls, _curSize.y );
      reg = createRegion( Vec2i(0, _curSize.y), false, grp->_first );
      //grp->_first = reg;
      _curSize.y = s;
   }

   DBG_MSG( os_rp, "_curSize ends up being " << _curSize );
   return true;
}

//------------------------------------------------------------------------------
//!
bool
RectPackerGreedy::findCandidate( const Vec2i& reqSize, RegionClass*& cls, RegionGroup*& grp, Region*& reg )
{
   DBG_BLOCK( os_rp, "findCandidate(" << reqSize << ")" );
   if( _latest == NULL )
   {
      DBG_MSG( os_rp, "No candidate - FAILING." );
      return false;
   }

   // Search smallest candidate.
   while( reqSize.x < _latest->_max )
   {
      if( _latest->_prev )  _latest = _latest->_prev;
      else                  break;
   }
   DBG_MSG( os_rp, "Smallest is: " << _latest->_max );

   // Search large enough candidate.
   while( _latest->_max < reqSize.x )
   {
      if( _latest->_next )
      {
         _latest = _latest->_next;
      }
      else
      {
         DBG_MSG( os_rp, "Largest possible (" << _latest->_max << ") is insufficient - FAILING." );
         return false;
      }
   }
   DBG_MSG( os_rp, "Large enough is: " << _latest->_max );

   // Here, _latest->_max <= reqSize.x

   // Try to find a big enough candidate.
   do
   {
      DBG_MSG( os_rp, "Checking class _max=" << _latest->_max );
      grp = _latest->_first;
      DBG_MSG( os_rp, "  First group is " << (void*)grp );
      while( grp )
      {
         DBG_MSG( os_rp, "Checking group _min=" << grp->_min );
         if( grp->_min >= reqSize.y )
         {
            if( grp->_first )
            {
               reg = grp->_first;
               grp->_first = reg->_next;
               cls = _latest;
               DBG_MSG( os_rp, "  Found candidate: " << cls->_max << "x" << grp->_min << " @ " << reg->_pos << " flipped=" << reg->_flipped );
               return true;
            }
         }
         // Could not find any valid candidate in group, pick next one.
         grp = grp->_next;
      }
      // Could not find any valid candidate in class, pick next one.
      _latest = _latest->_next;
      DBG_MSG( os_rp, "  Next class is " << (void*)_latest );
   }
   while( _latest != NULL );

   DBG_MSG( os_rp, "Could not find any valid candidate - FAILING." );
   return false;
}

//------------------------------------------------------------------------------
//!
void
RectPackerGreedy::destroy( RegionClass* cls, RegionGroup* grp, Region* reg )
{
   CHECK( grp->_first == reg );
   grp->_first = reg->_next;
   if( grp->_first == NULL )
   {
      // Group no longer needed.
      // Find it in the class, and remove it.
      if( cls->_first == grp )
      {
         cls->_first = grp->_next;
         if( cls->_first == NULL )
         {
            // Class no longer needed.
            if( cls->_prev == NULL )
            {
               _smallest = cls->_next;
            }
            else
            {
               cls->_prev->_next = cls->_next;
            }
            if( cls->_next == NULL )
            {
               _largest = cls->_prev;
            }
            else
            {
               cls->_next->_prev = cls->_prev;
            }
            if( _latest == cls )
            {
               _latest = (cls->_next != NULL) ? cls->_next : cls->_prev;
            }
            destroyClass( cls );
         }
      }
      else
      {
         RegionGroup* prev = cls->_first;
         RegionGroup* cur  = prev->_next;
         while( cur != grp )
         {
            prev = cur;
            cur  = cur->_next;
            CHECK( cur != NULL );  // Could not find grp in the class.
         }
         prev->_next = cur->_next;
      }
      destroyGroup( grp );
   }
   destroyRegion( reg );
}

//------------------------------------------------------------------------------
//!
void
RectPackerGreedy::printUnusedRegions() const
{
   DBG_BLOCK( os_rp, "Unused regions:" );
   RegionClass* cls = _smallest;
   while( cls )
   {
      RegionGroup* grp = cls->_first;
      while( grp )
      {
         Region* reg = grp->_first;
         while( reg )
         {
            DBG_MSG_BEGIN( os_rp );
            if( reg->_flipped ) os_rp << grp->_min << "x" << cls->_max;
            else                os_rp << cls->_max << "x" << grp->_min;
            os_rp << " @ " << reg->_pos << " flipped=" << reg->_flipped;
            DBG_MSG_END( os_rp );
            reg = reg->_next;
         }
         grp = grp->_next;
      }
      cls = cls->_next;
   }
}


/*==============================================================================
  CLASS IncrementalRectPacker
==============================================================================*/
//-----------------------------------------------------------------------------
//!
Vec2i
IncrementalRectPacker::add( const Vec2i& size )
{
   uint r = _curX + size.x + _border;
   if( r < (_maxWidth-_border) )
   {
      // Add to the current row.
      Vec2i pos = Vec2i( _curX+_border, _curY+_border );
      _curX = r;
      _curH = CGM::max( _curH, uint(size.y+_border) );
      return pos;
   }
   else
   {
      // Change row.
      _curY += _curH;
      _curX = size.x+_border;
      _curH = size.y+_border;
      return Vec2i( _border, _curY+_border );
   }
}
