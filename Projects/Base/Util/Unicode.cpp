/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#include <Base/Util/Unicode.h>

#include <Base/Dbg/Defs.h>

#include <Base/Util/CPU.h>
#include <Base/Util/EndianSwapper.h>

/*==============================================================================
  UNNAMED NAMESPACE
==============================================================================*/
UNNAMESPACE_BEGIN

//-----------------------------------------------------------------------------
//! Packs an invalid number of UTF-8 bytes in a 32b character which is invalid,
//! but which can be used to retrieve the original invalid bits.
char32_t _packInvalid( const char* cur, const uint8_t nBytes )
{
   char32_t code = cur[0];
   for( uint8_t i = 1; i < nBytes; ++i )
   {
      code <<= 8;
      code |= cur[i];
   }
   code |= 0x80000000; // Since cur[0] started with 1, we're overriding nothing even at 4 bytes.
   return code;
}

//-----------------------------------------------------------------------------
//! Packs an invalid UTF-16 character in a 32b character which is invalid,
//! but which can be used to retrieve the original invalid bits.
char32_t _packInvalid1( const char16_t byte )
{
   char32_t code = byte;
   code |= 0x80000000; // Guarantee we have a high bit set to 1.
   return code;
}

//-----------------------------------------------------------------------------
//! Packs an invalid UTF-16 pair in a 32b character which is invalid,
//! but which can be used to retrieve the original invalid bits.
char32_t _packInvalid2( const char16_t w0, const char16_t w1 )
{
   char32_t code = w0;
   code <<= 16;
   code |= w1;
   code |= 0x80000000; // Normally, byte0 should have it too, but just in case.
   return code;
}

UNNAMESPACE_END

NAMESPACE_BEGIN

//-----------------------------------------------------------------------------
//! Interprets the BOM at the beginning of a file.
//! Returns the number of characters taken by the BOM, and stores the UTF type in utf.
const char* readBOM( const char* cur, UTF& utf )
{
   // 00 00 FE FF  UTF-32BE
   // FF FE 00 00  UTF-32LE
   // FE FF        UTF-16BE
   // FF FE        UTF-16LE
   // EF BB BF     UTF-8

   uint16_t w0 = uint8_t(cur[0]);
   w0 <<= 8;
   w0 |= uint8_t(cur[1]);

   uint16_t w1 = uint8_t(cur[2]);
   w1 <<= 8;
   w1 |= uint8_t(cur[3]);

   if( w0 == 0xFFFE )
   {
      // Possibly UTF32LE or UTF16LE.
      if( w1 == 0x0000 )
      {
         utf = UTF32LE;
         return cur + 4;
      }
      else
      {
         utf = UTF16LE;
         return cur + 2;
      }
   }
   else
   if( w0 == 0xFEFF )
   {
      utf = UTF16BE;
      return cur + 2;
   }
   else
   if( w0 == 0x0000 && w1 == 0xFEFF )
   {
      utf = UTF32BE;
      return cur + 4;
   }
   else
   if( w0 == 0xEFBB && uint8_t(cur[2]) == 0xBF )
   {
      utf = UTF8;
      return cur + 3;
   }
   else
   {
      utf = UTF_UNKNOWN; // UTF8?
      return cur;
   }
}

//-----------------------------------------------------------------------------
//! Interprets the UTF-8 character pointed to by cur and stores it in code.
//! Also returns where the next character starting position (can be used to determine
//! the number of bytes the UTF-8 encoding took).
//! This routine requires that cur starts at the beginning of the character.
//! Ill-formed characters are terminated early, and return an invalid char32_t code
//! of which has the high bit set to 1 and all of the intepreted bytes up to (and including)
//! the first ill-formed byte, e.g.:
//!    code[31:0] = {   0x80,   0x00,   0x00, cur[0] } // First byte is ill-formed.
//!    code[31:0] = {   0x80,   0x00, cur[0], cur[1] } // Second byte is ill-formed.
//!    code[31:0] = {   0x80, cur[0], cur[1], cur[2] } // Third byte is ill-formed.
//!    code[31:0] = { cur[0], cur[1], cur[2], cur[3] } // Fourth byte is ill-formed.
//! Note that all well-formed bytes returned are guaranteed to have a leading 1 (per the Unicode spec).
//! Use utf32Error( char32_t ) to detect this situation.
const char* nextUTF8( const char* cur, char32_t& code )
{
   uint8_t c = *cur;
   switch( (c >> 6) )
   {
      case 0x0: // 8'b00xxxxxx
      case 0x1: // 8'b01xxxxxx
         // Valid ASCII subset.
         code = c;
         return cur + 1;
      case 0x2: // 8'b10xxxxxx
         // Invalid sequence.
         code = 0x80000000 | cur[0]; //0xFFFD; //0xE0000 + c;
         return cur + 1;
      case 0x3: // 8'b11xxxxxx
      {
         code = 0;
         uint8_t b;
         uint8_t nBytes = 1; // We handle the first byte after the loop.
         c <<= 1; // Get rid of the leading 1.
         while( (c >> 7) != 0x00 ) // 8'b1xxxxxxx
         {
            b = cur[nBytes];
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
               code = _packInvalid( cur, nBytes );
               return cur + nBytes;
            }
            c <<= 1;
            //c = (c << 1) & 0xFF;
         }
         if( nBytes > 4 )
         {
            code = _packInvalid( cur, 4 );
            return cur + 4;
         }
         // Here, c should have a leading 0.
         // Insert the bits of the original byte in the MSBs.
         // Equivalent to shifting left 6*cont_bytes, then shift right the number of 1s we shift above.
         code |= char32_t(c) << (6*(nBytes-1) - nBytes);
         return cur + nBytes;
      }
      default:
         CHECK( false );
         code = (0x80000000 | cur[0]);
         return cur + 1;
   }
}

//-----------------------------------------------------------------------------
//! Skips the current codepoint.
//! This routine does no error checking on the validity of the skipped bytes,
//! and only interprets the first byte.
const char* nextUTF8( const char* cur )
{
   return cur + getUTF8Size(cur);
}

//-----------------------------------------------------------------------------
//! Skips the specified number of codepoints.
const char* skipUTF8( const char* cur, size_t numCodepoints )
{
   while( numCodepoints-- )
   {
      cur = nextUTF8( cur );
   }
   return cur;
}

//-----------------------------------------------------------------------------
//! Retrieves the next Unicode character in code in a UTF-16 stream,
//! and returns the position of the next character.
const char16_t* nextUTF16( const char16_t* cur, char32_t& code )
{
   uint16_t w0 = *cur;
   switch( w0 & 0xFC00 )
   {
      case 0xD800:
      {
         // High surrogate chunk.
         uint16_t w1 = cur[1]; // Next should be low surrogate chunk.
         if( (w1 & 0xFC00) == 0xDC00 )
         {
            code  = (w0 & 0x03FF);
            code <<= 10;
            code |= (w1 & 0x03FF);
            code += 0x10000; // Bring back the bias.
            return cur + 2;
         }
         else
         {
            code = _packInvalid2( w0, w1 );
            return cur + 2;
         }
      }
      case 0xDC00:
         // Low surrogate chunk.
         code = _packInvalid1( w0 );
         return cur + 1;
      default:
         // Valid.
         code = w0;
         return cur + 1;
   }
}

//-----------------------------------------------------------------------------
//! Retrieves the next Unicode character in code in a UTF-16BE stream,
//! and returns the position of the next character.
const char* nextUTF16BE( const char* cur, char32_t& code )
{
   uint16_t w0 = EndianSwapper::get16BE( cur );
   switch( w0 & 0xFC00 )
   {
      case 0xD800:
      {
         // High surrogate chunk.
         uint16_t w1 = EndianSwapper::get16BE( cur+2 );; // Next should be low surrogate chunk.
         if( (w1 & 0xFC00) == 0xDC00 )
         {
            code  = (w0 & 0x03FF);
            code <<= 10;
            code |= (w1 & 0x03FF);
            code += 0x10000; // Bring back the bias.
            return cur + 4;
         }
         else
         {
            code = _packInvalid2( w0, w1 );
            return cur + 4;
         }
      }
      case 0xDC00:
         // Low surrogate chunk.
         code = _packInvalid1( w0 );
         return cur + 2;
      default:
         // Valid.
         code = w0;
         return cur + 2;
   }
}

//-----------------------------------------------------------------------------
//! Retrieves the next Unicode character in code in a UTF-16LE stream,
//! and returns the position of the next character.
const char* nextUTF16LE( const char* cur, char32_t& code )
{
   uint16_t w0 = EndianSwapper::get16LE( cur );
   switch( w0 & 0xFC00 )
   {
      case 0xD800:
      {
         // High surrogate chunk.
         uint16_t w1 = EndianSwapper::get16LE( cur+2 );; // Next should be low surrogate chunk.
         if( (w1 & 0xFC00) == 0xDC00 )
         {
            code  = (w0 & 0x03FF);
            code <<= 10;
            code |= (w1 & 0x03FF);
            code += 0x10000; // Bring back the bias.
            return cur + 4;
         }
         else
         {
            code = _packInvalid2( w0, w1 );
            return cur + 4;
         }
      }
      case 0xDC00:
         // Low surrogate chunk.
         code = _packInvalid1( w0 );
         return cur + 2;
      default:
         // Valid.
         code = w0;
         return cur + 2;
   }
}

//-----------------------------------------------------------------------------
//! Retrieves the next Unicode character in code in a UTF-32BE stream,
//! and returns the position of the next character.
const char* nextUTF32BE( const char* cur, char32_t& code )
{
   code = EndianSwapper::get32BE( cur );
   return cur + 4;
}

//-----------------------------------------------------------------------------
//! Retrieves the next Unicode character in code in a UTF-32LE stream,
//! and returns the position of the next character.
const char* nextUTF32LE( const char* cur, char32_t& code )
{
   code = EndianSwapper::get32LE( cur );
   return cur + 4;
}

//-----------------------------------------------------------------------------
//! Converts a UTF-8 character (1 to 4 bytes) into its UTF-32 representation.
char32_t toUTF32( const char* src )
{
   char32_t tmp;
   nextUTF8( src, tmp );
   return tmp;
}

//-----------------------------------------------------------------------------
//! Converts a UTF-16 character (1 to 2 words) into its UTF-32 representation.
char32_t toUTF32( const char16_t* src )
{
   char32_t tmp;
   nextUTF16( src, tmp );
   return tmp;
}

//-----------------------------------------------------------------------------
//! Stores a UTF-32 character (src) into its UTF-8 representation (in dst).
//! Returns the number of bytes stored in dst.
size_t toUTF8( char32_t src, char* dst )
{
   size_t s = getUTF8Size(src);
   switch( s-1 )
   {
      case 0:
         dst[0] =        (src & 0x7F);
         break;
      case 1:
         dst[1] = 0x80 | (src & 0x3F);
         src >>= 6;
         dst[0] = 0xC0 | (src & 0x1F);
         break;
      case 2:
         dst[2] = 0x80 | (src & 0x3F);
         src >>= 6;
         dst[1] = 0x80 | (src & 0x3F);
         src >>= 6;
         dst[0] = 0xE0 | (src & 0x0F);
         break;
      case 3:
         dst[3] = 0x80 | (src & 0x3F);
         src >>= 6;
         dst[2] = 0x80 | (src & 0x3F);
         src >>= 6;
         dst[1] = 0x80 | (src & 0x3F);
         src >>= 6;
         dst[0] = 0xF0 | (src & 0x07);
         break;
   }
   return s;
}

//-----------------------------------------------------------------------------
//! Stores a UTF-16 character (in src) into its UTF-8 representation (in dst).
//! Returns the number of bytes stored in dst.
size_t toUTF8( const char16_t* src, char* dst )
{
   return toUTF8( toUTF32(src), dst );
}

//-----------------------------------------------------------------------------
//! Stores a UTF-32 character (src) into its UTF-16 representation (in dst).
//! Returns the number of words stored in dst.
size_t toUTF16( char32_t src, char16_t* dst )
{
   if( src < 0x10000 )
   {
      dst[0] = src;
      return 1;
   }
   else
   {
      // Low and high surrogates.
      src -= 0x10000;
      dst[1] = 0xDC00 | (src & 0x03FF);
      src >>= 10;
      dst[0] = 0xD800 | (src & 0x03FF);
      return 2;
   }
}

//-----------------------------------------------------------------------------
//! Stores a UTF-8 character (src) into its UTF-16 representation (in dst).
//! Returns the number of words stored in dst.
size_t toUTF16( const char* src, char16_t* dst )
{
   return toUTF16( toUTF32(src), dst );
}

//-----------------------------------------------------------------------------
//! Returns the number of characters in the specified UTF-8 string.
//! If the string has ill-formed characters, results are undefined.
//! This routine is still safe even on ill-formed strings.
size_t getUTF8Length( const char* str, const size_t strLen )
{
   size_t n = 0;
   const char* cur = str;
   const char* end = str + strLen;
   while( cur < end )
   {
      uint8_t c = *cur;
      if( (c >> 6) != 0x2 ) ++n; // Skip continuation chunks (starting 8'b10xxxxxx).
      ++cur;
   }
   return n;
}

//-----------------------------------------------------------------------------
//! Returns the number of bytes required to represent the specified Unicode
//! character in UTF-8.
size_t getUTF8Size( const char32_t code )
{
   // Represented       Requires
   // by ... bits       ... bytes
   // 0 to 7            1  0xxxxxxx
   // 8 to 11           2  110xxxxx 10xxxxxx
   // 12 to 16          3  1110xxxx 10xxxxxx 10xxxxxx
   // 17 to 21          4  11110xxx 10xxxxxx 10xxxxxx 10xxxxxx

   char32_t tmp;
   // Propagate bits[20:16].
   tmp = (code >> 16);
   tmp |= (tmp >> 1);  //  2
   tmp |= (tmp >> 2);  //  4
   tmp |= (tmp >> 4);  //  8
   tmp |= (tmp >> 8);  // 16
   tmp |= (tmp << 1); // Make it a 2b code, for a 3.
   uint8_t b3 = (tmp & 0x03); // Equals 3 if there is a bit in [31:16].

   // Propagate bits[15:11].
   tmp = (code >> 11) & 0x1F;
   tmp |= (tmp >> 1); // 2
   tmp |= (tmp >> 2); // 4
   tmp |= (tmp >> 1); // 5
   uint8_t b2 = (tmp & 0x01) << 1; // Equals 2 if there is a bit in [15:11].

   // Propagate bits[10:07].
   tmp = (code >> 7) & 0x0F;
   tmp |= (tmp >> 1); // 2
   tmp |= (tmp >> 2); // 4
   uint8_t b1 = (tmp & 0x01); // Equals 1 if there is a bit in [10:07].

   // Create a code equal to b3, else b2, else b1, else 0.
   uint8_t c = b3 | b2 | (b1 & ~(b2>>1));

#if 0
   printf("code=%08x %d%d%d%d%d%d%d%d%d%d%d_%d%d%d%d%d %d%d%d%d%d %d%d%d%d %d%d%d%d%d%d%d\n",
          code,
          (code>>31) & 0x01,
          (code>>30) & 0x01,
          (code>>29) & 0x01,
          (code>>28) & 0x01,
          (code>>27) & 0x01,
          (code>>26) & 0x01,
          (code>>25) & 0x01,
          (code>>24) & 0x01,
          (code>>23) & 0x01,
          (code>>22) & 0x01,
          (code>>21) & 0x01,
          (code>>20) & 0x01,
          (code>>19) & 0x01,
          (code>>18) & 0x01,
          (code>>17) & 0x01,
          (code>>16) & 0x01,
          (code>>15) & 0x01,
          (code>>14) & 0x01,
          (code>>13) & 0x01,
          (code>>12) & 0x01,
          (code>>11) & 0x01,
          (code>>10) & 0x01,
          (code>> 9) & 0x01,
          (code>> 8) & 0x01,
          (code>> 7) & 0x01,
          (code>> 6) & 0x01,
          (code>> 5) & 0x01,
          (code>> 4) & 0x01,
          (code>> 3) & 0x01,
          (code>> 2) & 0x01,
          (code>> 1) & 0x01,
          (code>> 0) & 0x01);
   printf("utf8Size: %01x %01x %01x --> %01x\n", b3, b2, b1, c);
#endif

   return c + 1;
}

//-----------------------------------------------------------------------------
//! Returns the number of bytes that the next UTF-8 character takes.
size_t getUTF8Size( const char* str )
{
   char c = str[0];
   if( ((c>>7) & 0x01) == 0x00 )
   {
      return 1;
   }
   else
   {
      size_t tmp = 1;
      c <<= 1;
      while( ((c>>7) & 0x01) == 0x01 )
      {
         ++tmp;
         c <<= 1;
      }
      return tmp;
   }
}

//-----------------------------------------------------------------------------
//! Returns the required size (in bytes) of a UTF-16 string if it were converted to UTF-8.
size_t getUTF8Size( const char16_t* str, const size_t strLen )
{
   size_t n = 0;
   const char16_t* cur = str;
   const char16_t* end = str + strLen;
   char32_t code;
   while( cur < end )
   {
      cur = nextUTF16( cur, code );
      n += getUTF8Size( code );
   }
   return n;
}

//-----------------------------------------------------------------------------
//! Returns the required size (in bytes) of a UTF-32 string if it were converted to UTF-8.
size_t getUTF8Size( const char32_t* str, const size_t strLen )
{
   size_t n = 0;
   const char32_t* cur = str;
   const char32_t* end = str + strLen;
   while( cur < end )
   {
      n += getUTF8Size( *cur );
      ++cur;
   }
   return n;
}

//-----------------------------------------------------------------------------
//! Returns the number of bytes required to represent the specified Unicode
//! character in UTF-8.
size_t getUTF16Size( const char32_t code )
{
#if 0
   // Propagate bits[20:16].
   char32_t tmp = (code >> 16);
   tmp |= (tmp >> 1);  //  2
   tmp |= (tmp >> 2);  //  4
   tmp |= (tmp >> 4);  //  8
   tmp |= (tmp >> 8);  // 16
   return (tmp & 0x01) + 1;
#else
   return (code < 0x10000) ? 1 : 2;
#endif
}

//-----------------------------------------------------------------------------
//! Returns the required size (in bytes) of a UTF-8 string if it were converted to UTF-16.
size_t getUTF16Size( const char* str, const size_t strLen )
{
   size_t n = 0;
   const char* cur = str;
   const char* end = str + strLen;
   char32_t code;
   while( cur < end )
   {
      cur = nextUTF8( cur, code );
      n += getUTF16Size( code );
   }
   return n;
}

//-----------------------------------------------------------------------------
//! Returns the required size (in bytes) of a UTF-32 string if it were converted to UTF-16.
size_t getUTF16Size( const char32_t* str, const size_t strLen )
{
   size_t n = 0;
   const char32_t* cur = str;
   const char32_t* end = str + strLen;
   while( cur < end )
   {
      n += getUTF16Size( *cur );
      ++cur;
   }
   return n;
}

//-----------------------------------------------------------------------------
//! Returns the number of characters (surrogate pairs count as one) in a
//! UTF-16 string.
size_t getUTF16Length( const char16_t* str, const size_t strLen )
{
   size_t n = 0;
   const char16_t* cur = str;
   const char16_t* end = str + strLen;
   while( cur < end )
   {
      char16_t c = *cur;
      if( (c & 0xFC00) != 0xDC00 ) ++n; // Ignore low surrogate chunks.
      ++cur;
   }
   return n;
}

//-----------------------------------------------------------------------------
//! Returns the number of characters (surrogate pairs count as one) in a
//! UTF-16BE string.
size_t getUTF16BELength( const char* str, const size_t strLen )
{
   size_t n = 0;
   const char* cur = str;
   const char* end = str + strLen;
   while( cur < end )
   {
      char16_t c = EndianSwapper::get16BE( cur );
      if( (c & 0xFC00) != 0xDC00 ) ++n; // Ignore low surrogate chunks.
      cur += 2;
   }
   return n;
}

//-----------------------------------------------------------------------------
//! Returns the number of characters (surrogate pairs count as one) in a
//! UTF-16LE string.
size_t getUTF16LELength( const char* str, const size_t strLen )
{
   size_t n = 0;
   const char* cur = str;
   const char* end = str + strLen;
   while( cur < end )
   {
      char16_t c = EndianSwapper::get16LE( cur );
      if( (c & 0xFC00) != 0xDC00 ) ++n; // Ignore low surrogate chunks.
      cur += 2;
   }
   return n;
}


NAMESPACE_END
