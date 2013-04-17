/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#ifndef BASE_BYTES_H
#define BASE_BYTES_H

NAMESPACE_BEGIN

/*==============================================================================
  CLASS Bytes
==============================================================================*/
class Bytes
{
public:

   /*----- types -----*/
   typedef char*        Iterator;
   typedef const char*  ConstIterator;

   /*----- methods -----*/

   Bytes(): _size(0), _data(NULL) {}
   Bytes( size_t s ): _size(s), _data( new char[s] ) {}
   ~Bytes() { delete [] _data; }

   void  allocate( size_t s ) { deallocate(); _size = s; _data = new char[_size]; }
   void  deallocate() { delete [] _data; _size = 0; _data = NULL; }
   void  resize( size_t s );

   inline void  swap( Bytes& b );

   size_t  size() const { return _size; }

         char*  data()       { return _data; }
   const char*  data() const { return _data; }

   operator       char*()       { return _data; }
   operator const char*() const { return _data; }

   operator       void*()       { return _data; }
   operator const void*() const { return _data; }

   //      char&  operator[]( size_t i )       { return _data[i]; }
   //const char&  operator[]( size_t i ) const { return _data[i]; }

        Iterator  begin()       { return _data; }
   ConstIterator  begin() const { return _data; }
        Iterator  end()         { return _data + _size; }
   ConstIterator  end()   const { return _data + _size; }

protected:

   /*----- Bytes members -----*/

   size_t  _size;
   char*   _data;

}; //class Bytes

//-----------------------------------------------------------------------------
//!
void
Bytes::swap( Bytes& b )
{
   size_t tmps = _size;
   _size = b._size;
   b._size = tmps;
   char* tmpd = _data;
   _data = b._data;
   b._data = tmpd;
}

//-----------------------------------------------------------------------------
//!
void
Bytes::resize( size_t s )
{
   char* tmpd  = new char[s];
   size_t mins = _size;
   if( s < mins )  mins = s;
   memcpy( tmpd, _data, mins );
   delete [] _data;
   _data = tmpd;
   _size = s;
}

NAMESPACE_END

#endif //BASE_BYTES_H
