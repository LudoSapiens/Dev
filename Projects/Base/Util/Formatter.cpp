/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#include <Base/Util/Formatter.h>

NAMESPACE_BEGIN

//------------------------------------------------------------------------------
//!
ValueSuffix  humanReadableSize( size_t s, bool si )
{
   const char* unitsSI[] = { "B", "KB" , "MB" , "GB" , "TB" , "PB" , "EB"  };
   const char* unitsBI[] = { "B", "KiB", "MiB", "GiB", "TiB", "PiB", "EiB" };

   size_t scale;
   const char** units;
   if( si )
   {
      scale = 1000;
      units = unitsSI;
   }
   else
   {
      scale = 1024;
      units = unitsBI;
   }

   size_t denom = 1;
   for( size_t range = scale; range <= s; range *= scale )
   {
      denom *= scale;
      ++units;
   }

   //StdErr << "fmt s=" << s << " scale=" << scale << " range=" << range << " un=" << (units - unitsBI) << nl;
   double val = double(s) / double(denom);
   return ValueSuffix( val, *units );
}

NAMESPACE_END