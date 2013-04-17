/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#include <Base/Dbg/UnitTest.h>

#include <Base/ADT/Bytes.h>
#include <Base/ADT/Cache.h>
#include <Base/ADT/DEQueue.h>
#include <Base/ADT/DynArray.h>
#include <Base/ADT/HashTable.h>
#include <Base/ADT/Heap.h>
#include <Base/ADT/Map.h>
#include <Base/ADT/MapWithDefault.h>
#include <Base/ADT/Pair.h>
#include <Base/ADT/Queue.h>
#include <Base/ADT/RCVector.h>
#include <Base/ADT/Set.h>
#include <Base/ADT/String.h>
#include <Base/ADT/Vector.h>
#include <Base/ADT/ConstString.h>

#include <Base/Util/Platform.h>
#include <Base/Util/RCP.h>

USING_NAMESPACE

typedef bool(*Func)(uint, uint);

bool lessUInt( uint a, uint b )
{
   return a < b;
}

void adt_bytes( Test::Result& res )
{
   Bytes bytesA;
   TEST_ADD( res, bytesA.size() == 0 );
   TEST_ADD( res, bytesA.data() == NULL );

   Bytes bytesB(4);
   TEST_ADD( res, bytesB.size() == 4 );
   TEST_ADD( res, bytesB.data() != NULL );

   bytesA.swap( bytesB );
   TEST_ADD( res, bytesA.size() == 4 );
   TEST_ADD( res, bytesA.data() != NULL );
   TEST_ADD( res, bytesB.size() == 0 );
   TEST_ADD( res, bytesB.data() == NULL );

   TEST_ADD( res, (      char*)bytesA == bytesA.data() );
   TEST_ADD( res, (const char*)bytesA == bytesA.data() );
   TEST_ADD( res, (      void*)bytesA == bytesA.data() );
   TEST_ADD( res, (const void*)bytesA == bytesA.data() );

   bytesA[0] = 'a';
   bytesA[1] = 'b';
   bytesA[2] = 'c';
   bytesA[3] = '\0';
   Bytes::Iterator cur = bytesA.begin();
   TEST_ADD( res, cur != bytesA.end() && *cur == 'a' );
   ++cur;
   TEST_ADD( res, cur != bytesA.end() && *cur == 'b' );
   ++cur;
   TEST_ADD( res, cur != bytesA.end() && *cur == 'c' );
   ++cur;
   TEST_ADD( res, cur != bytesA.end() && *cur == '\0' );
   ++cur;
   TEST_ADD( res, cur == bytesA.end() );

   bytesA.resize( 2 );
   TEST_ADD( res, bytesA.size() == 2 );
   TEST_ADD( res, bytesA[0] == 'a' && bytesA[1] == 'b' );
}

void adt_cache( Test::Result& res )
{
   typedef LRUCache< uint, String, NoHash<uint> >  CacheType1;

   // Testing basic functionality.
   CacheType1  cache( 3 );
   String*     ptr;
   TEST_ADD( res, cache.capacity() == 3 );
   TEST_ADD( res, cache.size() == 0 );
   TEST_ADD( res, cache.find(0) == NULL );
   cache.set( 0, "zero" );
   TEST_ADD( res, cache.get(0, ptr) );
   TEST_ADD( res, *ptr == "zero" );
   TEST_ADD( res, cache.size() == 1 );
   cache.set( 1, "one" );
   TEST_ADD( res, cache.size() == 2 );
   cache.set( 2, "two" );
   TEST_ADD( res, cache.size() == 3 );
   cache.set( 3, "three" );
   TEST_ADD( res, cache.size() == 3 );
   TEST_ADD( res,  cache.find(0) == NULL   );
   TEST_ADD( res, *cache.find(1) == "one"  );
   TEST_ADD( res, *cache.find(2) == "two"  );
   TEST_ADD( res, *cache.find(3) == "three");
   TEST_ADD( res, cache.get(1, ptr) );
   TEST_ADD( res, *ptr == "one"     );
   TEST_ADD( res, cache.get(2, ptr) );
   TEST_ADD( res, *ptr == "two"     );
   TEST_ADD( res, cache.get(3, ptr) );
   TEST_ADD( res, *ptr == "three"   );
   // Refresh 1 so that it doesn't get evicted.
   cache.set( 1, "oneB" );
   // Evict 2.
   cache.set( 0, "zero" );
   TEST_ADD( res, cache.get(0, ptr) && *ptr == "zero" );
   TEST_ADD( res, cache.get(1, ptr) && *ptr == "oneB" );
   TEST_ADD( res, cache.find(2) == NULL               );
   TEST_ADD( res, cache.get(3, ptr) && *ptr == "three");
   cache.touch( 0 ); // Refresh 0, so that it's not evicted.
   cache.set( 2, "two" );
   TEST_ADD( res, *cache.find(0) == "zero" );
   TEST_ADD( res,  cache.find(1) == NULL   );
   TEST_ADD( res, *cache.find(2) == "two"  );
   TEST_ADD( res, *cache.find(3) == "three");
   cache.set( 1, "one" );
   TEST_ADD( res, *cache.find(0) == "zero" );
   TEST_ADD( res, *cache.find(1) == "one"  );
   TEST_ADD( res, *cache.find(2) == "two"  );
   TEST_ADD( res,  cache.find(3) == NULL   );
   cache.set( 3, "three" );
   TEST_ADD( res,  cache.find(0) == NULL   );
   TEST_ADD( res, *cache.find(1) == "one"  );
   TEST_ADD( res, *cache.find(2) == "two"  );
   TEST_ADD( res, *cache.find(3) == "three");

   // Test erase and clear.
   cache.erase( 1 );
   TEST_ADD( res,  cache.find(0) == NULL   );
   TEST_ADD( res,  cache.find(1) == NULL   );
   TEST_ADD( res, *cache.find(2) == "two"  );
   TEST_ADD( res, *cache.find(3) == "three");
   TEST_ADD( res, cache.size() == 2 );
   cache.clear();
   TEST_ADD( res, cache.size() == 0 );
   TEST_ADD( res, cache.find(0) == NULL );
   TEST_ADD( res, cache.find(1) == NULL );
   TEST_ADD( res, cache.find(2) == NULL );
   TEST_ADD( res, cache.find(3) == NULL );
   cache.set( 0, "zero" );
   TEST_ADD( res, cache.get(0, ptr) && *ptr == "zero" );
   TEST_ADD( res, cache.size() == 1 );

   // Test add.
   TEST_ADD( res, *cache.add(1, "one") == "one" );
   TEST_ADD( res, cache.size() == 2 );
}

void adt_dynarray( Test::Result& res )
{
   {
   DynArray<int>  arr;

   // Testing basic access and state.
   TEST_ADD( res, arr.size() == 0 );
   TEST_ADD( res, arr.empty() );
   arr[2] = -4;
   TEST_ADD( res, arr.size() == 3 );
   TEST_ADD( res, !arr.empty() );
   TEST_ADD( res, arr[2] == -4 );
   TEST_ADD( res, arr.size() == 3 );
   arr[0] = -1;
   TEST_ADD( res, arr.size() == 3 );
   TEST_ADD( res, arr[0] == -1 );
   TEST_ADD( res, arr[2] == -4 );
   arr[1] = -2;
   TEST_ADD( res, arr.size() == 3 );
   TEST_ADD( res, arr[0] == -1 );
   TEST_ADD( res, arr[1] == -2 );
   TEST_ADD( res, arr[2] == -4 );

   // Testing iterators.
   DynArray<int>::Iterator it = arr.begin();
   TEST_ADD( res, arr.begin() != arr.end() );
   TEST_ADD( res, (arr.end() - arr.begin()) == 3 );
   TEST_ADD( res, it != arr.end() && (*it) == -1 );
   ++it;
   TEST_ADD( res, it != arr.end() && (*it) == -2 );
   ++it;
   TEST_ADD( res, it != arr.end() && (*it) == -4 );
   ++it;
   TEST_ADD( res, it == arr.end() );
   TEST_ADD( res, *(arr.begin()+0) == -1 );
   TEST_ADD( res, *(arr.begin()+1) == -2 );
   TEST_ADD( res, *(arr.begin()+2) == -4 );
   TEST_ADD( res, (arr.begin()+3) == arr.end() );

   // Testing reverse iterators.
   DynArray<int>::ReverseIterator rit = arr.rbegin();
   TEST_ADD( res, arr.rbegin() != arr.rend() );
   TEST_ADD( res, (arr.rend() - arr.rbegin()) == 3 );
   TEST_ADD( res, rit != arr.rend() && (*rit) == -4 );
   ++rit;
   TEST_ADD( res, rit != arr.rend() && (*rit) == -2 );
   ++rit;
   TEST_ADD( res, rit != arr.rend() && (*rit) == -1 );
   ++rit;
   TEST_ADD( res, rit == arr.rend() );
   TEST_ADD( res, *(arr.rbegin()+0) == -4 );
   TEST_ADD( res, *(arr.rbegin()+1) == -2 );
   TEST_ADD( res, *(arr.rbegin()+2) == -1 );
   TEST_ADD( res, (arr.rbegin()+3) == arr.rend() );

   arr.resize(2);
   TEST_ADD( res, arr.size() == 2 );
   TEST_ADD( res, arr[0] == -1 );
   TEST_ADD( res, arr[1] == -2 );

   arr.clear();
   TEST_ADD( res, arr.size() == 0 );

   arr.resize(2, -3);
   TEST_ADD( res, arr.size() == 2 );
   TEST_ADD( res, arr[0] == -3 );
   TEST_ADD( res, arr[1] == -3 );

   arr.resize(5, -5);
   TEST_ADD( res, arr.size() == 5 );
   TEST_ADD( res, arr[0] == -3 );
   TEST_ADD( res, arr[1] == -3 );
   TEST_ADD( res, arr[2] == -5 );
   TEST_ADD( res, arr[3] == -5 );
   TEST_ADD( res, arr[4] == -5 );

   TEST_ADD( res, arr.get(0, -7) == -3 );
   TEST_ADD( res, arr.get(2, -7) == -5 );
   TEST_ADD( res, arr.get(5, -7) == -7 );
   }
}

void adt_hash( Test::Result& res )
{
#if PLAT_ANDROID
   // The code below is rather slow.
   printf(" *** Skipping under Android *** ");
   return;
#endif

   NoHash<uint> nh_u;
   for( uint i = 0; i < 10; ++i )
   {
      TEST_ADD( res, nh_u(i) == i );
   }

   Vector<uint>  histogram;
   size_t  histogram_size;
   size_t  h;

   uint avg;
   uint dev;

#define CONSIDER( print, hash, value ) \
   h = hash(value); \
   ++histogram[ h % histogram_size ]; \
   if( print ) StdErr << String().format("0x%08x", h) << ": '" << value << "'" << nl

   StdHash<String> h_str;
   histogram.clear();
   histogram.resize( 4, 0 );
   histogram_size = histogram.size();
   avg = uint(8 / histogram_size);
   dev = 1;  // Tolerate only an error of 1, since we only have 8 values.
   if( dev == 0 )  dev = 1;

   CONSIDER( false, h_str, "" );
   CONSIDER( false, h_str, "a" );
   CONSIDER( false, h_str, "b" );
   CONSIDER( false, h_str, "c" );
   CONSIDER( false, h_str, "I have a dream" );
   CONSIDER( false, h_str, "This is not a string" );
   CONSIDER( false, h_str, "Whatever" );
   CONSIDER( false, h_str, "Whyever" );
   //StdErr << "Accepted dev: " << dev << nl;
   for( size_t i = 0; i < histogram_size; ++i )
   {
      int d = histogram[i] - avg;
      TEST_ADD( res, (uint)abs(d) <= dev );
      //StdErr << "h[" << i << "]: " << histogram[i] << " " << d << " (" << (abs(d) <= dev) << ")" << nl;
   }

   StdHash<uint> h_u;
   histogram.clear();
   histogram.resize( 65, 0 );
   histogram_size = histogram.size();
   uint n = (1 << 25);
   avg = uint(n / histogram_size);
   dev = avg * 1 / 100;  // Tolerate 1% deviation.
   if( dev == 0 )  dev = 1;

   for( uint i = 0; i < n; ++i )
   {
      CONSIDER( false, h_u, i );
   }

   //StdErr << "Accepted dev: " << dev << nl;
   for( size_t i = 0; i < histogram_size; ++i )
   {
      int d = histogram[i] - avg;
      TEST_ADD( res, (uint)abs(d) <= dev );
      //StdErr << "h[" << i << "]: " << histogram[i] << " " << d << " (" << (abs(d) <= dev) << ")" << nl;
   }

#undef CONSIDER
}

void adt_hash_table( Test::Result& res )
{
   typedef HashTable< uint, String, NoHash<uint> >  HashTable1;

   // Testing basic functionality.
   HashTable1  ht( 8, 0.0, 2.0f );
   HashTable1::Iterator it = ht.find(0);
   TEST_ADD( res, ht.capacity() == 8 );
   TEST_ADD( res, !ht.has(0) );
   TEST_ADD( res, it == ht.end() );
   ht[0] = "zero";
   it = ht.find(0);
   TEST_ADD( res, ht.has(0) );
   TEST_ADD( res, ht[0] == "zero" );
   TEST_ADD( res, it != ht.end() );
   TEST_ADD( res, it.data() == "zero" );

   // Testing collision.
   ht[8] = "eight";
   it = ht.find(8);
   TEST_ADD( res, ht.has(0) );
   TEST_ADD( res, ht.has(8) );
   TEST_ADD( res, ht[0] == "zero" );
   TEST_ADD( res, ht[8] == "eight" );
   it = ht.find(16);
   TEST_ADD( res, !ht.has(16) );
   TEST_ADD( res, it == ht.end() );
   ht[2] = "two";
   TEST_ADD( res, ht.has(2) );

   // Testing iterators.
   HashTable1::Iterator it1;
   HashTable1::Iterator it2;

   // the table contains 0, 2, and 8.
   it1 = ht.begin();
   it2 = ht.end();
   TEST_ADD( res, it1.key() == 8 );
   TEST_ADD( res, it1 != it2 );

   it2 = it1;
   TEST_ADD( res, it1 == it2 );

   it2 = it1++;
   TEST_ADD( res, it1.key() == 0 );
   TEST_ADD( res, it2.key() == 8 );

   it2 = ++it1;
   TEST_ADD( res, it1.key() == 2 );
   TEST_ADD( res, it2.key() == 2 );

   // Testing one collision, and that detection still works fine.
   for( uint i = 0; i <= 8; ++i )
   {
      ht[i] = String().format("n_%d", i);
   }
   for( uint i = 0; i < 128; ++i )
   {
      bool should = (i <= 8);
      TEST_ADD( res, ht.has(i) == should );
   }

   // Testing iterator returned by add().
   it1 = ht.find( 0 );
   TEST_ADD( res, it1 != ht.end() );
   TEST_ADD( res, it1.key()  == 0 );
   TEST_ADD( res, it1.data() == "n_0" );  // Contains old data from above.
   it2 = ht.add( 0, "zero" );
   TEST_ADD( res, it1 != ht.end() );
   TEST_ADD( res, it1.key()  == 0 );
   TEST_ADD( res, it1.data() == "zero" ); // Should have changed data.
   TEST_ADD( res, it2 != ht.end() );
   TEST_ADD( res, it2.key()  == 0 );
   TEST_ADD( res, it2.data() == "zero" );
   TEST_ADD( res, it1 == it2 );

   // Testing removals and clear.
   TEST_ADD( res, ht.has(0) && ht.has(8) );
   ht.erase( 0 );
   TEST_ADD( res, !ht.has(0) && ht.has(8) );
   ht.add( 0, "zero" );
   TEST_ADD( res, ht.has(0) && ht.has(8) );
   it1 = ht.find( 0 );
   ht.erase( it1 );
   ht.erase( 8 );
   TEST_ADD( res, !ht.has(0) && !ht.has(8) );
   ht.clear();
   for( uint i = 0; i <= 9; ++i )
   {
      TEST_ADD( res, !ht.has(i) );
   }

   // Since it is empty, check that begin() gives end().
   TEST_ADD( res, ht.begin() == ht.end() );

   // Testing merging of hash tables.
   HashTable1 ht2(4, 0.0, 1.0f);
   ht[2]   = "2";
   ht[3]   = "3a";
   ht[8]   = "8";
   ht[24]  = "24";
   ht2[6]  = "6";
   ht2[4]  = "4";
   ht2[1]  = "1";
   ht2[9]  = "9"; // hash conflict with 1
   ht2[3]  = "3b";
   ht2[16] = "16";
   ht2[32] = "32";
   TEST_ADD( res, ht[3] == "3a" );
   ht.merge( ht2 );
   // Rudimentary sanity on ht2.
   TEST_ADD( res, !ht2.has(2) );
   TEST_ADD( res,  ht2.has(3) );
   // Check whole content of ht.
   TEST_ADD( res, ht[1]  ==  "1" );
   TEST_ADD( res, ht[2]  ==  "2" );
   TEST_ADD( res, ht[3]  == "3b" );
   TEST_ADD( res, ht[4]  ==  "4" );
   TEST_ADD( res, ht[6]  ==  "6" );
   TEST_ADD( res, ht[8]  ==  "8" );
   TEST_ADD( res, ht[9]  ==  "9" );
   TEST_ADD( res, ht[16] == "16" );
   TEST_ADD( res, ht[24] == "24" );
   TEST_ADD( res, ht[32] == "32" );

   // Check histogram.
   Vector<size_t> histogram;
   size_t tot = ht.getUsageHistogram( histogram );
   TEST_ADD( res, tot == 10 );
   TEST_ADD( res, histogram.size() == 8 );
   TEST_ADD( res, histogram[0] == 4 ); //8, 16, 24, 32
   TEST_ADD( res, histogram[1] == 2 ); //1, 9
   TEST_ADD( res, histogram[2] == 1 ); //2
   TEST_ADD( res, histogram[3] == 1 ); //3b
   TEST_ADD( res, histogram[4] == 1 ); //4
   TEST_ADD( res, histogram[5] == 0 ); //-
   TEST_ADD( res, histogram[6] == 1 ); //6
   TEST_ADD( res, histogram[7] == 0 ); //-

   //StdErr << "it1:"; it1.print(StdErr); StdErr << nl;
   //StdErr << "---" << nl;
   //ht.print();
}

void adt_heap( Test::Result& res )
{
   // Decreasing order (using less)
   Heap<uint> heapD;
   TEST_ADD( res, heapD.empty() );
   heapD.push(4);
   TEST_ADD( res, heapD.size() == 1 );
   heapD.push(3);
   TEST_ADD( res, heapD.size() == 2 );
   heapD.push(5);
   TEST_ADD( res, heapD.size() == 3 );
   TEST_ADD( res, heapD.root() == 5 );
   heapD.pop();
   TEST_ADD( res, heapD.root() == 4 );
   heapD.pop();
   TEST_ADD( res, heapD.root() == 3 );
   TEST_ADD( res, !heapD.empty() );
   heapD.pop();
   TEST_ADD( res, heapD.empty() );

   Vector<uint> v;
   v.push_back(3);
   v.push_back(5);
   v.push_back(1);
   v.push_back(8);
   heapD = Heap<uint>(v);
   TEST_ADD( res, heapD.size() == 4 );
   TEST_ADD( res, heapD.root() == 8 );
   heapD.push(4);
   heapD.push(9);
   TEST_ADD( res, heapD.root() == 9 );
   heapD.pop();
   TEST_ADD( res, heapD.root() == 8 );
   heapD.pop();
   TEST_ADD( res, heapD.root() == 5 );
   heapD.pop();
   TEST_ADD( res, heapD.root() == 4 );
   heapD.pop();
   TEST_ADD( res, heapD.root() == 3 );
   heapD.pop();
   TEST_ADD( res, heapD.root() == 1 );
   heapD.pop();
   TEST_ADD( res, heapD.empty() );


   // Increasing order (using greater)
   Heap< uint, std::greater<uint> > heapI;
   TEST_ADD( res, heapI.empty() );
   heapI.push(4);
   TEST_ADD( res, heapI.size() == 1 );
   heapI.push(3);
   TEST_ADD( res, heapI.size() == 2 );
   heapI.push(5);
   TEST_ADD( res, heapI.size() == 3 );
   TEST_ADD( res, heapI.root() == 3 );
   heapI.pop();
   TEST_ADD( res, heapI.root() == 4 );
   heapI.pop();
   TEST_ADD( res, heapI.root() == 5 );
   TEST_ADD( res, !heapI.empty() );
   heapI.pop();
   TEST_ADD( res, heapI.empty() );

   //V is already set
   heapI = Heap<uint>(v);
   TEST_ADD( res, heapI.size() == 4 );
   TEST_ADD( res, heapI.root() == 1 );
   heapI.push(4);
   heapI.push(9);
   TEST_ADD( res, heapI.root() == 1 );
   heapI.pop();
   TEST_ADD( res, heapI.root() == 3 );
   heapI.pop();
   TEST_ADD( res, heapI.root() == 4 );
   heapI.pop();
   TEST_ADD( res, heapI.root() == 5 );
   heapI.pop();
   TEST_ADD( res, heapI.root() == 8 );
   heapI.pop();
   TEST_ADD( res, heapI.root() == 9 );
   heapI.pop();
   TEST_ADD( res, heapI.empty() );


   // FHeap
   // Increasing order (using less)
   FHeap<uint> fheapD;
   TEST_ADD( res, fheapD.empty() );
   fheapD.push(4);
   TEST_ADD( res, fheapD.size() == 1 );
   fheapD.push(3);
   TEST_ADD( res, fheapD.size() == 2 );
   fheapD.push(5);
   TEST_ADD( res, fheapD.size() == 3 );
   TEST_ADD( res, fheapD.root() == 3 );
   fheapD.pop();
   TEST_ADD( res, fheapD.root() == 4 );
   fheapD.pop();
   TEST_ADD( res, fheapD.root() == 5 );
   TEST_ADD( res, !fheapD.empty() );
   fheapD.pop();
   TEST_ADD( res, fheapD.empty() );

   for( uint i = 0; i < 100; ++i )
   {
      fheapD.push( (i + 13) % 100 );
   }
   TEST_ADD( res, fheapD.size() == 100 );
   for( uint i = 0; i < 100; ++i )
   {
      TEST_ADD( res, fheapD.root() == i );
      fheapD.pop();
   }

   fheapD.clear();
   FHeap<uint>::Node* node = NULL;
   for( uint i = 0; i < 100; ++i )
   {
      uint v = (i+13) % 100;
      if( v == 50 )
      {
         node = fheapD.push( v );
      }
      else
      {
         fheapD.push( v );
      }
   }
   TEST_ADD( res, fheapD.size() == 100 );
   TEST_ADD( res, node->data() == 50 );
   fheapD.pop(); // Remove the current 0.
   TEST_ADD( res, fheapD.size() == 99 );
   TEST_ADD( res, fheapD.root() == 1 );
   node->data( 0 );
   fheapD.update( node );
   TEST_ADD( res, fheapD.root() == 0 );

   fheapD.clear();
   fheapD.push( 1 );
   fheapD.push( 2 );
   fheapD.push( 3 );
   node = fheapD.push( 4 );
   fheapD.push( 5 );
   fheapD.push( 6 );
   fheapD.push( 7 );
   fheapD.pop( node );
   TEST_ADD( res, fheapD.root() == 1 );
   fheapD.pop();
   TEST_ADD( res, fheapD.root() == 2 );
   fheapD.pop();
   TEST_ADD( res, fheapD.root() == 3 );
   fheapD.pop();
   TEST_ADD( res, fheapD.root() == 5 );
   fheapD.pop();
   TEST_ADD( res, fheapD.root() == 6 );
   fheapD.pop();
   TEST_ADD( res, fheapD.root() == 7 );
   fheapD.pop();
   TEST_ADD( res, fheapD.empty() );

   // Trying to yield problems with deallocated Node*.
   fheapD.clear();
   FHeap<uint>::Node* nodes[100];
   for( uint i = 0; i < 100; ++i )
   {
      nodes[i] = fheapD.push( i );
   }
   TEST_ADD( res, fheapD.size() == 100 );
   for( uint i = 0; i < 100; ++i )
   {
      uint v = (i+13) % 100;
      fheapD.pop( nodes[v] );
   }
   TEST_ADD( res, fheapD.empty() );

   // Compile checks.
   FHeap<uint, Func> h(0, lessUInt);
   h.push(1);
   h.push(3);
   h.push(2);
   TEST_ADD( res, h.size() == 3 );
   TEST_ADD( res, h.root() == 1 );
   h.pop();
   TEST_ADD( res, h.root() == 2 );
   h.pop();
   TEST_ADD( res, h.root() == 3 );
   h.pop();
}

void adt_map( Test::Result& res )
{
   Map< int, String >  map;

   TEST_ADD( res, map.empty() );
   TEST_ADD( res, map.size() == 0 );
   TEST_ADD( res, map.find(1) == map.end() );

   map[1] = "one";
   TEST_ADD( res, !map.empty() );
   TEST_ADD( res, map.size() == 1 );
   TEST_ADD( res, map.find(1) != map.end() );
   TEST_ADD( res, map.find(1)->second == "one" );
   TEST_ADD( res, map[1] == "one" );

   map[2] = "two";
   TEST_ADD( res, map.size() == 2 );
   TEST_ADD( res, map.find(1) != map.find(2) );
   TEST_ADD( res, map[1] == "one" );
   TEST_ADD( res, map[2] == "two" );

   map.erase(1);
   TEST_ADD( res, map.size() == 1 );
   TEST_ADD( res, map.find(1) == map.end() );
   TEST_ADD( res, map.find(2) != map.end() );
   TEST_ADD( res, map[2] == "two" );
   TEST_ADD( res, !map.has(1) && map.has(2) );
}

void adt_map_default( Test::Result& res )
{
   MapWithDefault< int, String >  map( "default" );

   TEST_ADD( res, map.empty() );
   TEST_ADD( res, map.size() == 0 );
   TEST_ADD( res, map.find(1) == map.end() );
   TEST_ADD( res, map[1] == "default" );
   TEST_ADD( res, map.empty() );
   TEST_ADD( res, map.size() == 0 );
   TEST_ADD( res, map.getDefault() == "default" );

   map.set( 1, "one" );
   TEST_ADD( res, !map.empty() );
   TEST_ADD( res, map.size() == 1 );
   TEST_ADD( res, map.find(1) != map.end() );
   TEST_ADD( res, map.find(1)->second == "one" );
   TEST_ADD( res, map[1] == "one"     );
   TEST_ADD( res, map[2] == "default" );
   TEST_ADD( res, map.get(1) == "one"     );
   TEST_ADD( res, map.get(2) == "default" );

   map.set( 2, "two" );
   TEST_ADD( res, map.size() == 2 );
   TEST_ADD( res, map.find(1) != map.find(2) );
   TEST_ADD( res, map[1] == "one" );
   TEST_ADD( res, map[2] == "two" );

   map.setDefault( "new default" );
   map.erase(1);
   TEST_ADD( res, map.size() == 1 );
   TEST_ADD( res, map.find(1) == map.end() );
   TEST_ADD( res, map.find(2) != map.end() );
   TEST_ADD( res, map[2] == "two" );
   TEST_ADD( res, !map.has(1) && map.has(2) );
   TEST_ADD( res, map[1] == "new default" );
   TEST_ADD( res, map.size() == 1 );
}

void adt_pair( Test::Result& res )
{
   Pair<uint, String> pair( 1, "one" );
   TEST_ADD( res, pair.first  ==     1 );
   TEST_ADD( res, pair.second == "one" );
   pair.first = 2;
   TEST_ADD( res, pair.first  ==     2 );
   TEST_ADD( res, pair.second == "one" );
   pair.second = "two";
   TEST_ADD( res, pair.first  ==     2 );
   TEST_ADD( res, pair.second == "two" );
}

void adt_queue( Test::Result& res )
{
   Queue<uint>  q(2);

   TEST_ADD( res, q.empty() );
   TEST_ADD( res, q.capacity() == 2 );
   q.push(1);
   TEST_ADD( res, q.size() == 1 );
   TEST_ADD( res, q.capacity() == 2 );
   q.push(2);
   TEST_ADD( res, q.size() == 2 );
   TEST_ADD( res, q.capacity() == 2 );
   q.push(3);
   TEST_ADD( res, q.size() == 3 );
   TEST_ADD( res, q.front() == 1 );
   TEST_ADD( res, q.back() == 3 );
   TEST_ADD( res, q.capacity() > 2 );
   q.pop();
   TEST_ADD( res, q.size() == 2 );
   TEST_ADD( res, q.front() == 2 );
   TEST_ADD( res, q.back() == 3 );
   q.pop();
   TEST_ADD( res, q.size() == 1 );
   TEST_ADD( res, q.front() == 3 );
   TEST_ADD( res, q.back() == 3 );
   q.pop();
   TEST_ADD( res, q.size() == 0 );
   TEST_ADD( res, q.empty() );

   // The same cases should now force a wrap case in the implementation.
   q.push(1);
   TEST_ADD( res, q.size() == 1 );
   q.push(2);
   TEST_ADD( res, q.size() == 2 );
   q.push(3);
   TEST_ADD( res, q.size() == 3 );
   TEST_ADD( res, q.front() == 1 );
   TEST_ADD( res, q.back() == 3 );
   q.pop();
   TEST_ADD( res, q.size() == 2 );
   TEST_ADD( res, q.front() == 2 );
   TEST_ADD( res, q.back() == 3 );
   q.pop();
   TEST_ADD( res, q.size() == 1 );
   TEST_ADD( res, q.front() == 3 );
   TEST_ADD( res, q.back() == 3 );
   q.pop();
   TEST_ADD( res, q.size() == 0 );
   TEST_ADD( res, q.empty() );

   for( uint i = 0; i < 100; ++i )
   {
      q.push(i);
   }
   TEST_ADD( res, q.size() == 100 );
   for( uint i = 0; i < 100; ++i )
   {
      TEST_ADD( res, q.front() ==  i );
      TEST_ADD( res, q.back()  == 99 );
      q.pop();
   }

   Queue<uint> q2(0);
   TEST_ADD( res, q2.empty() );
   TEST_ADD( res, q2.size()  == 0 );
   q2.push(1);
   TEST_ADD( res, !q2.empty() );
   TEST_ADD( res, q2.size()  == 1 );
   TEST_ADD( res, q2.front() == 1 );
   TEST_ADD( res, q2.back()  == 1 );
   q2.push(2);
   TEST_ADD( res, !q2.empty() );
   TEST_ADD( res, q2.size()  == 2 );
   TEST_ADD( res, q2.front() == 1 );
   TEST_ADD( res, q2.back()  == 2 );
   q2.pop();
   TEST_ADD( res, !q2.empty() );
   TEST_ADD( res, q2.size()  == 1 );
   TEST_ADD( res, q2.front() == 2 );
   TEST_ADD( res, q2.back()  == 2 );
   q2.pop();
   TEST_ADD( res, q2.empty() );
   TEST_ADD( res, q2.size() == 0 );
}

void adt_dequeue( Test::Result& res )
{
   DEQueue<uint>  q(2);

   TEST_ADD( res, q.empty() );
   TEST_ADD( res, q.capacity() == 2 );
   q.pushBack(1);
   TEST_ADD( res, q.size() == 1 );
   TEST_ADD( res, q.capacity() == 2 );
   q.pushBack(2);
   TEST_ADD( res, q.size() == 2 );
   TEST_ADD( res, q.capacity() == 2 );
   q.pushBack(3);
   TEST_ADD( res, q.size() == 3 );
   TEST_ADD( res, q.front() == 1 );
   TEST_ADD( res, q.back() == 3 );
   TEST_ADD( res, q.capacity() > 2 );
   q.popFront();
   TEST_ADD( res, q.size() == 2 );
   TEST_ADD( res, q.front() == 2 );
   TEST_ADD( res, q.back() == 3 );
   q.popFront();
   TEST_ADD( res, q.size() == 1 );
   TEST_ADD( res, q.front() == 3 );
   TEST_ADD( res, q.back() == 3 );
   q.popFront();
   TEST_ADD( res, q.size() == 0 );
   TEST_ADD( res, q.empty() );

   // The same cases should now force a wrap case in the implementation.
   q.pushBack(1);
   TEST_ADD( res, q.size() == 1 );
   q.pushBack(2);
   TEST_ADD( res, q.size() == 2 );
   q.pushBack(3);
   TEST_ADD( res, q.size() == 3 );
   TEST_ADD( res, q.front() == 1 );
   TEST_ADD( res, q.back() == 3 );
   q.popFront();
   TEST_ADD( res, q.size() == 2 );
   TEST_ADD( res, q.front() == 2 );
   TEST_ADD( res, q.back() == 3 );
   q.popFront();
   TEST_ADD( res, q.size() == 1 );
   TEST_ADD( res, q.front() == 3 );
   TEST_ADD( res, q.back() == 3 );
   q.popFront();
   TEST_ADD( res, q.size() == 0 );
   TEST_ADD( res, q.empty() );

   for( uint i = 0; i < 100; ++i )
   {
      q.pushBack(i);
   }
   TEST_ADD( res, q.size() == 100 );
   for( uint i = 0; i < 100; ++i )
   {
      TEST_ADD( res, q.front() ==  i );
      TEST_ADD( res, q.back()  == 99 );
      q.popFront();
   }

   DEQueue<uint> q2(0);
   TEST_ADD( res, q2.empty() );
   TEST_ADD( res, q2.size()  == 0 );
   q2.pushBack(1);
   TEST_ADD( res, !q2.empty() );
   TEST_ADD( res, q2.size()  == 1 );
   TEST_ADD( res, q2.front() == 1 );
   TEST_ADD( res, q2.back()  == 1 );
   q2.pushBack(2);
   TEST_ADD( res, !q2.empty() );
   TEST_ADD( res, q2.size()  == 2 );
   TEST_ADD( res, q2.front() == 1 );
   TEST_ADD( res, q2.back()  == 2 );
   q2.popFront();
   TEST_ADD( res, !q2.empty() );
   TEST_ADD( res, q2.size()  == 1 );
   TEST_ADD( res, q2.front() == 2 );
   TEST_ADD( res, q2.back()  == 2 );
   q2.popFront();
   TEST_ADD( res, q2.empty() );
   TEST_ADD( res, q2.size() == 0 );

   // DEQueue-specific tests.
   q.clear();
   TEST_ADD( res, q.empty() );
   q.pushFront( 1 );
   TEST_ADD( res, !q.empty() );
   TEST_ADD( res, q.size()  == 1 );
   TEST_ADD( res, q.front() == 1 );
   TEST_ADD( res, q.back()  == 1 );
   q.pushFront( 2 );
   TEST_ADD( res, q.size()  == 2 );
   TEST_ADD( res, q.front() == 2 );
   TEST_ADD( res, q.back()  == 1 );
   q.pushBack( 3 );
   TEST_ADD( res, q.size()  == 3 );
   TEST_ADD( res, q.front() == 2 );
   TEST_ADD( res, q.back()  == 3 );
   q.popFront();
   TEST_ADD( res, q.size()  == 2 );
   TEST_ADD( res, q.front() == 1 );
   TEST_ADD( res, q.back()  == 3 );
   q.popBack();
   TEST_ADD( res, !q.empty() );
   TEST_ADD( res, q.size()  == 1 );
   TEST_ADD( res, q.front() == 1 );
   TEST_ADD( res, q.back()  == 1 );
   q.popFront();
   TEST_ADD( res, q.empty() );
   TEST_ADD( res, q.size()  == 0 );
   q.pushBack( 4 );
   TEST_ADD( res, q.size()  == 1 );
   TEST_ADD( res, q.front() == 4 );
   TEST_ADD( res, q.back()  == 4 );
   q.popBack();
   TEST_ADD( res, q.empty() );
   TEST_ADD( res, q.size() == 0 );
}

void adt_rcvector( Test::Result& res )
{
   RCP< RCVector<uint> > bv1( new RCVector<uint>() );
   RCP< RCVector<uint> > bv2( new RCVector<uint>() );

   Vector<uint>& v1 = *bv1;
   Vector<uint>& v2 = *bv2;
   TEST_ADD( res, v1.size() == 0 );
   TEST_ADD( res, v1.empty() );
   v1.pushBack( 1 );
   TEST_ADD( res, v1.size() == 1 );
   TEST_ADD( res, !v1.empty() );
   v1.pushBack( 4 );
   v1.pushBack( 8 );
   TEST_ADD( res, v1.size() == 3 );
   TEST_ADD( res, v1[0] == 1 );
   TEST_ADD( res, v1[1] == 4 );
   TEST_ADD( res, v1[2] == 8 );
   v1.popBack();
   TEST_ADD( res, v1.size() == 2 );

   v1.clear();
   TEST_ADD( res, v1.size() == 0 );
   TEST_ADD( res, v1.empty() );

   v1.pushBack( 0 );
   v1.pushBack( 8 );
   v2.clear();
   v2.pushBack( 2 );
   v2.pushBack( 4 );
   v2.pushBack( 6 );
   v1.insert( v1.begin() + 1, v2.begin(), v2.end() );
   TEST_ADD( res, v1.size() == 5 );
   TEST_ADD( res, v1.dataSize() == 5*sizeof(uint) );
   TEST_ADD( res, v1.capacity() == 8 );
   TEST_ADD( res, v1[0] == 0 );
   TEST_ADD( res, v1[1] == 2 );
   TEST_ADD( res, v1[2] == 4 );
   TEST_ADD( res, v1[3] == 6 );
   TEST_ADD( res, v1[4] == 8 );

   v2.clear();
   v2.pushBack( 1 );
   v2.pushBack( 3 );
   v2.pushBack( 5 );
   v2.pushBack( 7 );
   v2.pushBack( 9 );
   v2.reverse( v2.begin(), v2.end() );
   v1.append( v2 );
   v1.remove( 8 );
   TEST_ADD( res, v1.size() == 9 );
   TEST_ADD( res, v1[0] == 0 );
   TEST_ADD( res, v1[1] == 2 );
   TEST_ADD( res, v1[2] == 4 );
   TEST_ADD( res, v1[3] == 6 );
   TEST_ADD( res, v1[4] == 9 );
   TEST_ADD( res, v1[5] == 7 );
   TEST_ADD( res, v1[6] == 5 );
   TEST_ADD( res, v1[7] == 3 );
   TEST_ADD( res, v1[8] == 1 );
   TEST_ADD( res, v1.capacity() == 16 );
}

void adt_set( Test::Result& res )
{
   Set<uint> setA;
   TEST_ADD( res, setA.size() == 0 );
   TEST_ADD( res, setA.has(0) == false );
   TEST_ADD( res, setA.has(5) == false );
   setA.add(5);
   TEST_ADD( res, setA.size() == 1 );
   TEST_ADD( res, setA.has(0) == false );
   TEST_ADD( res, setA.has(5) == true  );
   setA.add(1);
   setA.add(0);
   setA.add(8);
   TEST_ADD( res, setA.size() == 4 );
   TEST_ADD( res, setA.has(0) == true  );
   TEST_ADD( res, setA.has(1) == true  );
   TEST_ADD( res, setA.has(2) == false );
   TEST_ADD( res, setA.has(3) == false );
   TEST_ADD( res, setA.has(4) == false );
   TEST_ADD( res, setA.has(5) == true  );
   TEST_ADD( res, setA.has(6) == false );
   TEST_ADD( res, setA.has(7) == false );
   TEST_ADD( res, setA.has(8) == true  );
   TEST_ADD( res, setA.has(9) == false );
   TEST_ADD( res, *(setA.begin() + 0) == 0 );
   TEST_ADD( res, *(setA.begin() + 1) == 1 );
   TEST_ADD( res, *(setA.begin() + 2) == 5 );
   TEST_ADD( res, *(setA.begin() + 3) == 8 );

   Set<uint> setB(setA);
   setA.clear();
   TEST_ADD( res, setA.size() == 0 );
   TEST_ADD( res, setA.has(0) == false );
   TEST_ADD( res, setA.has(1) == false );
   TEST_ADD( res, setA.has(2) == false );
   TEST_ADD( res, setA.has(3) == false );
   TEST_ADD( res, setA.has(4) == false );
   TEST_ADD( res, setA.has(5) == false );
   TEST_ADD( res, setA.has(6) == false );
   TEST_ADD( res, setA.has(7) == false );
   TEST_ADD( res, setA.has(8) == false );
   TEST_ADD( res, setA.has(9) == false );
   TEST_ADD( res, setB.size() == 4 );
   TEST_ADD( res, setB.has(0) == true  );
   TEST_ADD( res, setB.has(1) == true  );
   TEST_ADD( res, setB.has(2) == false );
   TEST_ADD( res, setB.has(3) == false );
   TEST_ADD( res, setB.has(4) == false );
   TEST_ADD( res, setB.has(5) == true  );
   TEST_ADD( res, setB.has(6) == false );
   TEST_ADD( res, setB.has(7) == false );
   TEST_ADD( res, setB.has(8) == true  );
   TEST_ADD( res, setB.has(9) == false );
   TEST_ADD( res, *(setB.begin() + 0) == 0 );
   TEST_ADD( res, *(setB.begin() + 1) == 1 );
   TEST_ADD( res, *(setB.begin() + 2) == 5 );
   TEST_ADD( res, *(setB.begin() + 3) == 8 );

   Vector<uint> v;
   v.push_back(8);
   v.push_back(5);
   v.push_back(1);
   v.push_back(0);
   setA.insert( v.begin(), v.end() );
   TEST_ADD( res, setA.size() == 4 );
   TEST_ADD( res, setA.has(0) == true  );
   TEST_ADD( res, setA.has(1) == true  );
   TEST_ADD( res, setA.has(2) == false );
   TEST_ADD( res, setA.has(3) == false );
   TEST_ADD( res, setA.has(4) == false );
   TEST_ADD( res, setA.has(5) == true  );
   TEST_ADD( res, setA.has(6) == false );
   TEST_ADD( res, setA.has(7) == false );
   TEST_ADD( res, setA.has(8) == true  );
   TEST_ADD( res, setA.has(9) == false );
   TEST_ADD( res, *(setA.begin() + 0) == 0 );
   TEST_ADD( res, *(setA.begin() + 1) == 1 );
   TEST_ADD( res, *(setA.begin() + 2) == 5 );
   TEST_ADD( res, *(setA.begin() + 3) == 8 );

   setA.remove(5);
   TEST_ADD( res, setA.size() == 3 );
   TEST_ADD( res, setA.has(0) == true  );
   TEST_ADD( res, setA.has(1) == true  );
   TEST_ADD( res, setA.has(2) == false );
   TEST_ADD( res, setA.has(3) == false );
   TEST_ADD( res, setA.has(4) == false );
   TEST_ADD( res, setA.has(5) == false );
   TEST_ADD( res, setA.has(6) == false );
   TEST_ADD( res, setA.has(7) == false );
   TEST_ADD( res, setA.has(8) == true  );
   TEST_ADD( res, setA.has(9) == false );

   setA.remove(5);
   setA.remove(2);
   setA.remove(9);
   TEST_ADD( res, setA.size() == 3 );
   TEST_ADD( res, setA.has(0) == true  );
   TEST_ADD( res, setA.has(1) == true  );
   TEST_ADD( res, setA.has(2) == false );
   TEST_ADD( res, setA.has(3) == false );
   TEST_ADD( res, setA.has(4) == false );
   TEST_ADD( res, setA.has(5) == false );
   TEST_ADD( res, setA.has(6) == false );
   TEST_ADD( res, setA.has(7) == false );
   TEST_ADD( res, setA.has(8) == true  );
   TEST_ADD( res, setA.has(9) == false );

   // Reproducing a bug related to the empty string.
   // Crashes in Android when using stlport implementations.
   Set<String> strSet;
   strSet.add( "abc" );
   strSet.add( "def" );
   TEST_ADD( res, strSet.size() == 2 );
   TEST_ADD( res, strSet.has("abc") == true  );
   TEST_ADD( res, strSet.has("def") == true  );
   TEST_ADD( res, strSet.has("ghi") == false );
/**
   for( Set<String>::ConstIterator cur = strSet.begin();
        cur != strSet.end();
        ++cur )
   {
      const String& str = (*cur);
      fprintf( stderr, ">> '%s' %p\n", str.cstr(), (void*)(&str) );
   }
   fprintf( stderr, "\nAdding empty string\n" );
**/
   strSet.add( String() );
   TEST_ADD( res, strSet.size() == 3 );
   TEST_ADD( res, strSet.has("abc") == true  );
   TEST_ADD( res, strSet.has("def") == true  );
   TEST_ADD( res, strSet.has("ghi") == false );
/**
   fprintf( stderr, "\n" );
   for( Set<String>::ConstIterator cur = strSet.begin();
        cur != strSet.end();
        ++cur )
   {
      const String& str = (*cur);
      fprintf( stderr, ">> '%s' %p\n", str.cstr(), (void*)(&str) );
   }
**/
}

void adt_string( Test::Result& res )
{
   String str((const char*)NULL);
   String::SizeType pos;

   TEST_ADD( res, str.empty() );
   TEST_ADD( res, str.size() == 0 );
   str = String("abc");
   TEST_ADD( res, str.size() == 3 );
   TEST_ADD( res, str == "abc" );
   TEST_ADD( res, str != "ab" );
   TEST_ADD( res, str != "abcd" );
   str += " def";
   TEST_ADD( res, str.size() == 7 );
   TEST_ADD( res, str == "abc def" );
   TEST_ADD( res, str.findFirstOf(" ") == 3 );
   str = "ghi";
   TEST_ADD( res, str.size() == 3 );
   TEST_ADD( res, str == "ghi" );
   str = (const char*)NULL;
   TEST_ADD( res, str.empty() );

   Vector<String> parts;
   str = "abc.def.ghi";
   str.split( ".", parts );
   TEST_ADD( res, parts.size() == 3 );
   TEST_ADD( res, parts[0] == "abc" );
   TEST_ADD( res, parts[1] == "def" );
   TEST_ADD( res, parts[2] == "ghi" );

   parts.clear();
   str = "abc..def.";
   str.split( ".", parts );
   TEST_ADD( res, parts.size() == 2 );
   TEST_ADD( res, parts[0] == "abc" );
   TEST_ADD( res, parts[1] == "def" );

   parts.clear();
   str = "abc..def.";
   str.split( ".", parts, true );
   TEST_ADD( res, parts.size() == 4 );
   TEST_ADD( res, parts[0] == "abc" );
   TEST_ADD( res, parts[1].empty() );
   TEST_ADD( res, parts[2] == "def" );
   TEST_ADD( res, parts[3].empty() );

   str = "some string";
   TEST_ADD( res,  str.startsWith("so")  );
   TEST_ADD( res, !str.startsWith("so ") );
   TEST_ADD( res, !str.startsWith(" so") );
   TEST_ADD( res,  str.startsWith("some string")  );
   TEST_ADD( res, !str.startsWith("some string ") );
   TEST_ADD( res, !str.startsWith(" some string") );
   TEST_ADD( res,  str.endsWith("ing")  );
   TEST_ADD( res, !str.endsWith(" ing") );
   TEST_ADD( res, !str.endsWith("ing ") );
   TEST_ADD( res,  str.endsWith("some string")  );
   TEST_ADD( res, !str.endsWith("some string ") );
   TEST_ADD( res, !str.endsWith(" some string") );

   str = "abc..def.";

   pos = str.findFirstOf(".");
   TEST_ADD( res, pos == 3 );
   pos = str.findFirstOf(".", pos+1);
   TEST_ADD( res, pos == 4 );
   pos = str.findFirstOf(".", pos+1);
   TEST_ADD( res, pos == 8 );
   pos = str.findFirstOf(".", pos+1);
   TEST_ADD( res, pos == String::npos );

   pos = str.findFirstNotOf("abcdef");
   TEST_ADD( res, pos == 3 );
   pos = str.findFirstNotOf("abcdef", pos+1);
   TEST_ADD( res, pos == 4 );
   pos = str.findFirstNotOf("abcdef", pos+1);
   TEST_ADD( res, pos == 8 );
   pos = str.findFirstNotOf("abcdef", pos+1);
   TEST_ADD( res, pos == String::npos );
   pos = str.findFirstNotOf(".");
   TEST_ADD( res, pos == 0 );
   pos = str.findFirstNotOf(".", 7);
   TEST_ADD( res, pos == 7 );
   pos = str.findFirstNotOf(".", 8);
   TEST_ADD( res, pos == String::npos );

   pos = str.findLastOf(".");
   TEST_ADD( res, pos == 8 );
   pos = str.findLastOf(".", pos-1);
   TEST_ADD( res, pos == 4 );
   pos = str.findLastOf(".", pos-1);
   TEST_ADD( res, pos == 3 );
   pos = str.findLastOf(".", pos-1);
   TEST_ADD( res, pos == String::npos );

   pos = str.findLastNotOf("abcdef");
   TEST_ADD( res, pos == 8 );
   pos = str.findLastNotOf("abcdef", pos-1);
   TEST_ADD( res, pos == 4 );
   pos = str.findLastNotOf("abcdef", pos-1);
   TEST_ADD( res, pos == 3 );
   pos = str.findLastNotOf("abcdef", pos-1);
   TEST_ADD( res, pos == String::npos );
   pos = str.findLastNotOf(".");
   TEST_ADD( res, pos == 7 );
   pos = str.findLastNotOf(".bcdef");
   TEST_ADD( res, pos == 0 );
   pos = str.findLastNotOf("abc", 2);
   TEST_ADD( res, pos == String::npos );

   // Types.
   int64_t big = 1; // Will contain 0x00000001_00010100
   big <<= 16;      // so that we can test values larger
   big |= 1;        // than the previous size can be:
   big <<= 8;       //  64b --> >=2^32
   big |= 1;        //  32b --> >=2^16
   big <<= 8;       //  16b --> >=2^8
   big |= 1;        // (lower bit widths wrap)
   // Text formats.
   TEST_ADD( res, String() == "" );
   TEST_ADD( res, String( "a" ) == "a" );
   TEST_ADD( res, String( 'b' ) == "b" );
   // Signed formats.
   TEST_ADD( res, String( (short  )-1  ) == "-1" );
   TEST_ADD( res, String( (short  )big ) == "257" );
   TEST_ADD( res, String( (int    )-1  ) == "-1" );
   TEST_ADD( res, String( (int    )big ) == "65793" );
   TEST_ADD( res, String( (int8_t )-1  ) == "-1" );
   TEST_ADD( res, String( (int8_t )big ) == "1" );
   TEST_ADD( res, String( (int16_t)-1  ) == "-1" );
   TEST_ADD( res, String( (int16_t)big ) == "257" );
   TEST_ADD( res, String( (int32_t)-1  ) == "-1" );
   TEST_ADD( res, String( (int32_t)big ) == "65793" );
   TEST_ADD( res, String( (int64_t)-1  ) == "-1" );
   TEST_ADD( res, String( (int64_t)big ) == "4295033089" );
   // Unsigned formats.
   //TEST_ADD( res, String( (uchar)'c' ) == "c" );
   //TEST_ADD( res, String( (uchar)'c' ) == "255" );
   // uchar actually are typedef'd as uint8_t, so it's an ascii->int typecast.
   TEST_ADD( res, String( (ushort  )-1 ) == "65535" );
   TEST_ADD( res, String( (uint    )-1 ) == "4294967295" );
   TEST_ADD( res, String( (uint8_t )-1 ) == "255" );
   TEST_ADD( res, String( (uint16_t)-1 ) == "65535" );
   TEST_ADD( res, String( (uint32_t)-1 ) == "4294967295" );
   TEST_ADD( res, String( (uint64_t)-1 ) == "18446744073709551615" );
   // Floating-point formats.
   TEST_ADD( res, String( -1.5f ) == "-1.5" );
   TEST_ADD( res, String( -2.25 ) == "-2.25" );
   // Other formats.
   TEST_ADD( res, String( (size_t)512 ) == "512" );
   TEST_ADD( res, String( (ptrdiff_t)-256 ) == "-256" );
}

void adt_vector( Test::Result& res )
{
   Vector<uint> v1, v2;
   TEST_ADD( res, v1.size() == 0 );
   TEST_ADD( res, v1.empty() );
   v1.pushBack( 1 );
   TEST_ADD( res, v1.size() == 1 );
   TEST_ADD( res, !v1.empty() );
   v1.pushBack( 4 );
   v1.pushBack( 8 );
   TEST_ADD( res, v1.size() == 3 );
   TEST_ADD( res, v1[0] == 1 );
   TEST_ADD( res, v1[1] == 4 );
   TEST_ADD( res, v1[2] == 8 );

   // Testing iterators.
   Vector<uint>::Iterator it = v1.begin();
   TEST_ADD( res, (v1.end() - v1.begin()) == 3 );
   TEST_ADD( res, it != v1.end() && *it == 1 );
   ++it;
   TEST_ADD( res, it != v1.end() && *it == 4 );
   ++it;
   TEST_ADD( res, it != v1.end() && *it == 8 );
   ++it;
   TEST_ADD( res, it == v1.end() );
   Vector<uint>::ReverseIterator rit = v1.rbegin();
   TEST_ADD( res, (v1.rend() - v1.rbegin()) == 3 );
   TEST_ADD( res, rit != v1.rend() && *rit == 8 );
   ++rit;
   TEST_ADD( res, rit != v1.rend() && *rit == 4 );
   ++rit;
   TEST_ADD( res, rit != v1.rend() && *rit == 1 );
   ++rit;
   TEST_ADD( res, rit == v1.rend() );

   v1.popBack();
   TEST_ADD( res, v1.size() == 2 );

   v1.clear();
   TEST_ADD( res, v1.size() == 0 );
   TEST_ADD( res, v1.empty() );

   v1.pushBack( 0 );
   v1.pushBack( 8 );
   v2.clear();
   v2.pushBack( 2 );
   v2.pushBack( 4 );
   v2.pushBack( 6 );
   v1.insert( v1.begin() + 1, v2.begin(), v2.end() );
   TEST_ADD( res, v1.size() == 5 );
   TEST_ADD( res, v1.dataSize() == 5*sizeof(uint) );
   TEST_ADD( res, v1.capacity() == 8 );
   TEST_ADD( res, v1[0] == 0 );
   TEST_ADD( res, v1[1] == 2 );
   TEST_ADD( res, v1[2] == 4 );
   TEST_ADD( res, v1[3] == 6 );
   TEST_ADD( res, v1[4] == 8 );

   v2.clear();
   v2.pushBack( 1 );
   v2.pushBack( 3 );
   v2.pushBack( 5 );
   v2.pushBack( 7 );
   v2.pushBack( 9 );
   v2.reverse( v2.begin(), v2.end() );
   v1.append( v2 );
   v1.remove( 8 );
   TEST_ADD( res, v1.size() == 9 );
   TEST_ADD( res, v1[0] == 0 );
   TEST_ADD( res, v1[1] == 2 );
   TEST_ADD( res, v1[2] == 4 );
   TEST_ADD( res, v1[3] == 6 );
   TEST_ADD( res, v1[4] == 9 );
   TEST_ADD( res, v1[5] == 7 );
   TEST_ADD( res, v1[6] == 5 );
   TEST_ADD( res, v1[7] == 3 );
   TEST_ADD( res, v1[8] == 1 );
   TEST_ADD( res, v1.capacity() == 16 );

   v2.clear();
   v2.pushBack( 1 );
   v2.pushBack( 2 );
   v2.pushBack( 3 );
   v2.pushBack( 4 );
   TEST_ADD( res, v2.removeSwap( 2 ) );
   TEST_ADD( res, v2[0] == 1 );
   TEST_ADD( res, v2[1] == 4 );
   TEST_ADD( res, v2[2] == 3 );
   TEST_ADD( res, v2.size() == 3 );
   TEST_ADD( res, !v2.removeSwap( 2 ) );
   TEST_ADD( res, v2.size() == 3 );
   TEST_ADD( res, v2.removeSwap( 3 ) );
   TEST_ADD( res, v2[0] == 1 );
   TEST_ADD( res, v2[1] == 4 );
   TEST_ADD( res, v2.removeSwap( 1 ) );
   TEST_ADD( res, v2[0] == 4 );
   TEST_ADD( res, v2.removeSwap( 4 ) );
   TEST_ADD( res, v2.size() == 0 );
   v2.pushBack( 1 );
   v2.pushBack( 2 );
   v2.pushBack( 3 );
   v2.pushBack( 2 );
   TEST_ADD( res, v2.removeAllSwap( 2 ) );
   TEST_ADD( res, v2.size() == 2 );
   TEST_ADD( res, v2[0] == 1 );
   TEST_ADD( res, v2[1] == 3 );
   TEST_ADD( res, !v2.removeAllSwap( 2 ) );

   v1.clear();
   v1.pushBack( 1 );
   v1.grow( 3, 2 );
   TEST_ADD( res, v1.size() == 3 );
   TEST_ADD( res, v1[0] == 1 && v1[1] == 2 && v1[2] == 2 );
   v1.grow( 1, 0 ); // Doesn't change anything.
   TEST_ADD( res, v1.size() == 3 );
   TEST_ADD( res, v1[0] == 1 && v1[1] == 2 && v1[2] == 2 );
   v1.shrink( 2 );  // Drops last entry.
   TEST_ADD( res, v1.size() == 2 );
   TEST_ADD( res, v1[0] == 1 && v1[1] == 2 );
   v1.shrink( 3 );  // Doesn't change anything.
   TEST_ADD( res, v1.size() == 2 );
   TEST_ADD( res, v1[0] == 1 && v1[1] == 2 );

   v1.clear();
   v1.pushBack( 1 );
   v1.pushBack( 2 );
   v1.pushBack( 3 );
   v1.pushBack( 4 );
   TEST_ADD( res, v1[0] == 1 && v1[1] == 2 && v1[2] == 3 && v1[3] == 4 );
   v1.move( 3, 0 );
   TEST_ADD( res, v1[0] == 4 && v1[1] == 1 && v1[2] == 2 && v1[3] == 3 );
   v1.move( 0, 3 );
   TEST_ADD( res, v1[0] == 1 && v1[1] == 2 && v1[2] == 3 && v1[3] == 4 );
   for( uint i = 0; i < v1.size(); ++i )
   {
      v1.move( i, i );
      TEST_ADD( res, v1[0] == 1 && v1[1] == 2 && v1[2] == 3 && v1[3] == 4 );
   }

#if 0
   auto ait = v1.begin();
   TEST_ADD( res, ait != v1.end() && (*ait) == 1 );
   ++ait;
   TEST_ADD( res, ait != v1.end() && (*ait) == 2 );
   ++ait;
   TEST_ADD( res, ait != v1.end() && (*ait) == 3 );
   ++ait;
   TEST_ADD( res, ait != v1.end() && (*ait) == 4 );
   ++ait;
   TEST_ADD( res, ait == v1.end() );
#endif
}

void adt_constString( Test::Result& res )
{
   ConstString str1( "test" );
   TEST_ADD( res, str1.size() == 4 );
   ConstString str2( "test2" );
   TEST_ADD( res, str1 != str2 );
   TEST_ADD( res, strcmp( str2.cstr(), "test2" ) == 0 );
   ConstString str3( "test" );
   TEST_ADD( res, str1 == str3 );
}

void adt_format( Test::Result& /*res*/ )
{
   StdErr << nl;
   size_t s = 0x0123;
   s <<= 16;
   s |= 0x4567;
   s <<= 16;
   s |= 0x89AB;
   s <<= 16;
   s |= 0xCDEF;
   printf( "size_t: "FMT_SIZE_T"  hex: "FMT_HEX_SIZE_T"\n", s, s );

   ptrdiff_t d = -1;
   printf( "ptrdiff_t: "FMT_PTRDIFF_T"  hex: "FMT_HEX_PTRDIFF_T"\n", d, d );

   int64_t i64 = -1;
   printf( "uint64_t: "FMT_UINT64_T",0x"FMT_HEX_UINT64_T"\n", (uint64_t)i64, (uint64_t)i64 );
   printf( "int64_t: "FMT_INT64_T",0x"FMT_HEX_INT64_T"\n", i64, i64 );

   void* v = (void*)0xDEADBEEF;
   printf( "void*: "FMT_VOID_PTR"  0x"FMT_HEX_VOID_PTR"\n", v, v );
}

void  init_adt()
{
   RCP<Test::Collection> col = new Test::Collection( "adt", "Collection for Base/ADT" );
   col->add( new Test::Function("bytes",       "Tests the Bytes data structure",       adt_bytes       ) );
   col->add( new Test::Function("cache",       "Tests the Cache data structure",       adt_cache       ) );
   col->add( new Test::Function("dequeue",     "Tests the DEQueue data structure",     adt_dequeue     ) );
   col->add( new Test::Function("dynarray",    "Tests the DynArray data structure",    adt_dynarray    ) );
   col->add( new Test::Function("hash",        "Tests the standard hashing functions", adt_hash        ) );
   col->add( new Test::Function("hash_table",  "Tests the HashTable data structure",   adt_hash_table  ) );
   col->add( new Test::Function("heap",        "Tests the Heap data structure",        adt_heap        ) );
   col->add( new Test::Function("map",         "Tests the Map data structure",         adt_map         ) );
   col->add( new Test::Function("map_default", "Tests the Map data structure",         adt_map_default ) );
   col->add( new Test::Function("pair",        "Tests the Pair data structure",        adt_pair        ) );
   col->add( new Test::Function("queue",       "Tests the Queue data structure",       adt_queue       ) );
   col->add( new Test::Function("rcvector",    "Tests the RCVector data structure",    adt_rcvector    ) );
   col->add( new Test::Function("set",         "Tests the Pair data structure",        adt_set         ) );
   col->add( new Test::Function("string",      "Tests the String data structure",      adt_string      ) );
   col->add( new Test::Function("vector",      "Tests the Vector data structure",      adt_vector      ) );
   col->add( new Test::Function("conststring", "Tests the ConstString data structure", adt_constString ) );
   Test::standard().add( col.ptr() );

   Test::special().add( new Test::Function( "format", "Verifies some format printing defines and other things", adt_format ) );

}
