/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#ifndef BASE_FORMATTER_H
#define BASE_FORMATTER_H

#include <Base/StdDefs.h>

#include <Base/IO/TextStream.h>

NAMESPACE_BEGIN


/*==============================================================================
  CLASS ValueSuffix
==============================================================================*/
//!< A pair of double+suffix which is used as a transient value for formatting.
class ValueSuffix
{
public:
   inline ValueSuffix( double value, const char* suffix ):
      _value( value ), _suffix( suffix ) {}

   inline String  toStr() const
   {
      return String().format( "%g %s", _value, _suffix );
   }

   inline operator double() const { return _value; }

   inline      double  value()  const { return _value;  }
   inline const char*  suffix() const { return _suffix; }

protected:
   double        _value;
   const char*  _suffix;
};

//------------------------------------------------------------------------------
//!
inline TextStream&  operator<<( TextStream& os, const ValueSuffix& v )
{
   return os << v.toStr();
}

//------------------------------------------------------------------------------
//! Converts a size into a human-readable version.
//! The @si parameter specifies that we want base-10 units (1000 increments)
//! as opposed to the base-2 units (1024 increments).
BASE_DLL_API ValueSuffix  humanReadableSize( size_t s, bool si = false );


NAMESPACE_END

#endif //BASE_FORMATTER_H
