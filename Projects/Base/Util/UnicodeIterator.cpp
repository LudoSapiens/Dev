/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#include <Base/Util/UnicodeIterator.h>

#include <Base/Dbg/Defs.h>

#include <Base/Util/Bits.h>

#include <cstdio>

USING_NAMESPACE

/*==============================================================================
  UNNAMED NAMESPACE
==============================================================================*/
UNNAMESPACE_BEGIN



UNNAMESPACE_END


/*==============================================================================
  CLASS UTF8Iterator
==============================================================================*/

char32_t
UTF8Iterator::operator*() const
{
   uint8_t c = *_cur;
   
   switch( (c >> 6) )
   {
      case 0x0: // 8'b00xxxxxx
      case 0x1: // 8'b01xxxxxx
         // Valid ASCII subset.
         return c;
      case 0x2: // 8'b10xxxxxx
         // Invalid sequence.
         return INVALID_CODEPOINT; //0x80000000 | _cur[0]; //0xFFFD; //0xE0000 + c;
      case 0x3: // 8'b11xxxxxx
      {
         char32_t code = 0;
         uint8_t  b;
         uint8_t  nBytes = 1; // We handle the first byte after the loop.
         c <<= 1; // Get rid of the leading 1.
         while( (c >> 7) != 0x00 ) // 8'b1xxxxxxx
         {
            b = _cur[nBytes];
            ++nBytes;
            if( (b >> 6) == 0x02 ) // 8'b10xxxxxx
            {
               // Proper continuation code.
               code <<= 6;
               code |= (b & 0x3F); // Insert next 6 bits.
            }
            else
            {
               // Improper continuation; pack the bytes in an invalid sequence.
               return INVALID_CODEPOINT; //_packInvalid( cur, nBytes );
            }
            c <<= 1;
            //c = (c << 1) & 0xFF;
         }
         if( nBytes > 4 )
         {
            return INVALID_CODEPOINT;
         }
         // Here, c should have a leading 0.
         // Insert the bits of the original byte in the MSBs.
         // Equivalent to shifting left 6*cont_bytes, then shift right the number of 1s we shift above.
         code |= char32_t(c) << (6*(nBytes-1) - nBytes);
         return code;
      }
      default:
         CHECK( false );
         return INVALID_CODEPOINT;
   }
}
