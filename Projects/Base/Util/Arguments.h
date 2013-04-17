/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#ifndef BASE_ARGUMENTS_H
#define BASE_ARGUMENTS_H

#include <Base/StdDefs.h>

NAMESPACE_BEGIN

/*==============================================================================
  CLASS Arguments
==============================================================================*/
class Arguments
{
public:

   /*==============================================================================
     CLASS Iterator
   ==============================================================================*/
   class Iterator
   {
   public:

      /*----- methods -----*/

      inline bool isValid() const { return _args && _pos < _args->argc(); }

      inline const char*  operator*() { return (*_args)[_pos]; }
      BASE_DLL_API Iterator&  operator++();

      BASE_DLL_API Iterator  operator+( int off ) const;

      inline Iterator(): _args(NULL), _pos(-1) {}
   protected:

      friend class Arguments;

      /*----- data members -----*/

      Arguments*  _args; //!< The associated Arguments.
      int         _pos;  //!< The current position.

      /*----- methods -----*/

      inline Iterator( Arguments* args ): _args( args ), _pos(0) { ++(*this); }

   private:
   }; //class Iterator
   

   /*----- methods -----*/

   BASE_DLL_API Arguments( int argc, char* argv[] );
   BASE_DLL_API ~Arguments();

   BASE_DLL_API void  mask( int arg );
   BASE_DLL_API void  mask( int startArg, int endArg );
   inline void  mask( Iterator& it ) { mask(it._pos); }
   inline void  mask( Iterator& it, int numExtraArgs ) { mask(it._pos, it._pos+numExtraArgs); }

   BASE_DLL_API void  unmask( int arg );
   BASE_DLL_API void  unmask( int startArg, int endArg );

   BASE_DLL_API void  setMask( int arg, int val );

   BASE_DLL_API bool  masked( int arg );

   BASE_DLL_API int  find( const char* str );
   BASE_DLL_API int  findPrefix( const char* str );

   inline int     argc() const { return _argc; }
   inline char**  argv() const { return _argv; }

   inline int          size() const { return _argc; }
   inline const char*  operator[]( int arg ) { return _argv[arg]; }


   inline Iterator  iter() { return Iterator(this); }

protected:

   /*----- data members -----*/

   int       _argc;  //!< The original argc specified by the user.
   char**    _argv;  //!< The original argv specified by the user.
   uint8_t*  _mask;  //!< A bitfield to hold mask bits for every argument.

   /*----- methods -----*/

   /* methods... */

private:
}; //class Arguments


NAMESPACE_END

#endif //BASE_ARGUMENTS_H
