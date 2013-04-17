/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#include <Base/Dbg/UnitTest.h>

#include <Base/Util/Application.h>
#include <Base/Util/Arguments.h>
#include <Base/Util/ArrayAdaptor.h>
#include <Base/Util/Bits.h>
#include <Base/Util/Compiler.h>
#include <Base/Util/CPU.h>
#include <Base/Util/Date.h>
#include <Base/Util/EndianSwapper.h>
#include <Base/Util/Formatter.h>
#include <Base/Util/Half.h>
#include <Base/Util/Hash.h>
#include <Base/Util/IDPool.h>
#include <Base/Util/Memory.h>
#include <Base/Util/Packed.h>
#include <Base/Util/RadixSort.h>
#include <Base/Util/RCObject.h>
#include <Base/Util/RCObjectNA.h>
#include <Base/Util/RCP.h>
#include <Base/Util/SHA.h>
#include <Base/Util/Time.h>
#include <Base/Util/Timer.h>
#include <Base/Util/Unicode.h>
#include <Base/Util/UnicodeIterator.h>
#include <Base/Util/Union.h>


#include <Base/Dbg/DebugStream.h>
#include <Base/IO/TextStream.h>
#include <Base/Util/Platform.h>

USING_NAMESPACE

class SampleApplication:
   public Application
{
public:
   static RCP<SampleApplication>  create( int argc, char* argv[], int& theInt )
   {
      return RCP<SampleApplication>( new SampleApplication(argc, argv, theInt) );
   }
   virtual ~SampleApplication()
   {
      //StdErr << "Destroying application" << nl;
      value += 4;
   }
   int run()
   {
      value += 2;
      //StdErr << "Running application" << nl;
      return Application::run();
   }
   int& value;
protected:
   SampleApplication( int argc, char* argv[], int& theInt ):
      Application( argc, argv ),
      value( theInt )
   {
      //StdErr << "Creating application" << nl;
      value += 1;
   }
}; //class SampleApplication


void util_application( Test::Result& res )
{
   const char* argvc[6];
   argvc[0] = "app.exe";
   argvc[1] = "-option1";
   argvc[2] = "value1";
   argvc[3] = "-option2";
   argvc[4] = "value2";
   argvc[5] = "extraArg";
   int    argc = 6;
   char** argv = (char**)argvc;

   int value = 0;

   TEST_ADD( res, sApp == NULL );
   TEST_ADD( res, value == 0 );
   RCP<SampleApplication> app = SampleApplication::create( argc, argv, value );
   TEST_ADD( res, app.ptr() == sApp );
   TEST_ADD( res, value == 1 );
   TEST_ADD( res, app->run() == 0 );
   TEST_ADD( res, value == 3 );
   app = NULL;
   TEST_ADD( res, sApp == NULL );
   TEST_ADD( res, value == 7 );

   // We'll typically just call:
   //   return SampleApplication::create(argc, argv)->run();
   TEST_ADD( res, SampleApplication::create(argc, argv, value)->run() == 0 );
   TEST_ADD( res, value == 14 );
}

void util_args( Test::Result& res )
{
   const char* tmp[] = {
      "app",
      "-option1",
      "-option2", "two",
      "file1",
      "-option3=three",
      "file2"
   };
   Arguments  args( 7, (char**)tmp ); // Typecast since static strings are now const char*.
   TEST_ADD( res, args.size() == 7 );
   TEST_ADD( res, String(args[0]) == "app" );
   TEST_ADD( res, String(args[1]) == "-option1" );
   TEST_ADD( res, String(args[2]) == "-option2" );
   TEST_ADD( res, String(args[3]) == "two" );
   TEST_ADD( res, String(args[4]) == "file1" );
   TEST_ADD( res, String(args[5]) == "-option3=three" );
   TEST_ADD( res, String(args[6]) == "file2" );

   Arguments::Iterator it, it2;
   it = args.iter();
   TEST_ADD( res, it.isValid() );
   TEST_ADD( res, String(*it) == "-option1" );
   TEST_ADD( res, !it2.isValid() );
   it2 = it;
   TEST_ADD( res, it2.isValid() );
   ++it;
   TEST_ADD( res, it.isValid() );
   TEST_ADD( res, String(*it) == "-option2" );
   ++it;
   TEST_ADD( res, String(*it) == "two" );
   ++it;
   TEST_ADD( res, String(*it) == "file1" );
   ++it;
   TEST_ADD( res, String(*it) == "-option3=three" );
   ++it;
   TEST_ADD( res, String(*it) == "file2" );
   TEST_ADD( res, it.isValid() );
   ++it;
   TEST_ADD( res, !it.isValid() );
   ++it;
   TEST_ADD( res, !it.isValid() ); // Test robustness

   args.mask(1);
   it = args.iter();
   //TEST_ADD( res, String(*it) == "-option1" );
   //++it;
   TEST_ADD( res, String(*it) == "-option2" );
   ++it;
   TEST_ADD( res, String(*it) == "two" );
   ++it;
   TEST_ADD( res, String(*it) == "file1" );
   ++it;
   TEST_ADD( res, String(*it) == "-option3=three" );
   ++it;
   TEST_ADD( res, String(*it) == "file2" );
   TEST_ADD( res, it.isValid() );
   ++it;
   TEST_ADD( res, !it.isValid() );

   args.mask(3, 2); // Also tests that flipping order works.
   it = args.iter();
   //TEST_ADD( res, String(*it) == "-option1" );
   //++it;
   //TEST_ADD( res, String(*it) == "-option2" );
   //++it;
   //TEST_ADD( res, String(*it) == "two" );
   //++it;
   TEST_ADD( res, String(*it) == "file1" );
   ++it;
   TEST_ADD( res, String(*it) == "-option3=three" );
   ++it;
   TEST_ADD( res, String(*it) == "file2" );
   TEST_ADD( res, it.isValid() );
   ++it;
   TEST_ADD( res, !it.isValid() );

   TEST_ADD( res, args.find("-option1") ==  1 );
   TEST_ADD( res, args.find("-option3") == -1 );
   TEST_ADD( res, args.findPrefix("-option3") == 5 );
   TEST_ADD( res, args.masked(1) );
   args.unmask( 1 );
   TEST_ADD( res, !args.masked(1) );
   TEST_ADD( res, !args.masked(5) );
   args.mask( 5 );
   TEST_ADD( res, args.masked(5) );

   it = args.iter();
   TEST_ADD( res, String(*it) == "-option1" );
   it2 = it + 1;
   TEST_ADD( res, String(*it2) == "file1" );
   it2 = it + 2;
   TEST_ADD( res, String(*it2) == "file2" );
}

struct IntInt
{
   int a;
   int b;
};

void util_array_adaptor( Test::Result& res )
{
   int array[] = { 11, 22, 33, 44, 55, 66, 77, 88, 99 };
   ArrayAdaptor<int> aa( array, sizeof(int) );
   TEST_ADD( res, aa[0] == 11 );
   TEST_ADD( res, aa[1] == 22 );
   TEST_ADD( res, aa[2] == 33 );
   aa.data( array + 3 );
   aa.stride( 2*sizeof(int) );
   TEST_ADD( res, aa[0] == 44 );
   TEST_ADD( res, aa[1] == 66 );
   TEST_ADD( res, aa[2] == 88 );
   TEST_ADD( res, aa[-1] == 22 );

   ConstArrayAdaptor<int> caa( aa.data(), aa.stride() );
   TEST_ADD( res, caa[0] == 44 );
   TEST_ADD( res, caa[1] == 66 );
   TEST_ADD( res, caa[2] == 88 );
   TEST_ADD( res, caa[-1] == 22 );

   ArrayAdaptor<IntInt> iiaa( array, 3*sizeof(int) );
   TEST_ADD( res, iiaa[0].a == 11 );
   TEST_ADD( res, iiaa[0].b == 22 );
   TEST_ADD( res, iiaa[1].a == 44 );
   TEST_ADD( res, iiaa[1].b == 55 );

   TEST_ADD( res, array[4] == 55 );
   ++(iiaa[1].b);
   TEST_ADD( res, array[4] == 56 );
}

void util_bits( Test::Result& res )
{
   TEST_ADD( res, alignTo( 0, 32) ==  0 );
   TEST_ADD( res, alignTo( 3, 32) == 32 );
   TEST_ADD( res, alignTo(31, 32) == 32 );
   TEST_ADD( res, alignTo(33, 32) == 64 );
   TEST_ADD( res, alignTo(34, 32) == 64 );
   TEST_ADD( res, alignTo(63, 32) == 64 );
   TEST_ADD( res, alignTo(64, 32) == 64 );
   TEST_ADD( res, alignTo(65, 32) == 96 );

   TEST_ADD( res, bitmask( 0) == 0x00000000 );
   TEST_ADD( res, bitmask( 1) == 0x00000001 );
   TEST_ADD( res, bitmask( 5) == 0x0000001F );
   TEST_ADD( res, bitmask(16) == 0x0000FFFF );
   TEST_ADD( res, bitmask(31) == 0x7FFFFFFF );
   TEST_ADD( res, bitmask(32) == 0xFFFFFFFF );
   TEST_ADD( res, getbits((uint32_t)0x00FF0000, 15, 5) == 0x1E );
   TEST_ADD( res, getbits((uint32_t)0x00000001,  0, 1) == 0x01 );
   TEST_ADD( res, getbits((uint32_t)0x00000001,  1, 1) == 0x00 );
   TEST_ADD( res, getbits((uint32_t)0x80000000, 31, 1) == 0x01 );
   TEST_ADD( res, getbits((uint32_t)0x80000000, 32, 1) == 0x00 );
   TEST_ADD( res, getbits((uint32_t)0xFFFFFFFF,  0, 0) == 0x00 );
   TEST_ADD( res, setbits((uint32_t)0x00000000, 1,  4, 0x0000FFFF) == 0x0000001E );
   TEST_ADD( res, setbits((uint32_t)0x00000000, 0, 32, 0xFFFFFFFE) == 0xFFFFFFFE );
   TEST_ADD( res, setbits((uint32_t)0x00000000, 0, 32, 0xFFFFFFFF) == 0xFFFFFFFF );
   TEST_ADD( res, setbits((uint32_t)0xFFFFFFFF, 0, 32, 0x00000000) == 0x00000000 );
   TEST_ADD( res, setbits((uint32_t)0xFFFFFFFF, 5, 12, 0x000000F0) == 0xFFFE1E1F );
   TEST_ADD( res, setbits((uint32_t)0xFFFFFFFF, 5, 12, 0x000008F1) == 0xFFFF1E3F );
   TEST_ADD( res, flipbits((uint32_t)0x00000000,  0,  1) == 0x00000001 );
   TEST_ADD( res, flipbits((uint32_t)0x000C3000,  9, 14) == 0x0073CE00 );
   TEST_ADD( res, flipbits((uint32_t)0x00000000, 31,  1) == 0x80000000 );
   TEST_ADD( res, flipbits((uint32_t)0x00000000,  5,  5) == 0x000003E0 );
   TEST_ADD( res, flipbits((uint32_t)0x1234DEAD,  0, 32) == 0xEDCB2152 );
   TEST_ADD( res, flipbits((uint32_t)0xEDCB2152,  0, 32) == 0x1234DEAD );

   TEST_ADD( res, getbit((uint32_t)0x00000002,  0) == false );
   TEST_ADD( res, getbit((uint32_t)0x00000002,  1) == true  );
   TEST_ADD( res, getbit((uint32_t)0x00000002,  2) == false );
   TEST_ADD( res, getbit((uint32_t)0x80000000, 30) == false );
   TEST_ADD( res, getbit((uint32_t)0x80000000, 31) == true  );
   TEST_ADD( res, setbit((uint32_t)0x00000000,  0, true ) == 0x00000001 );
   TEST_ADD( res, setbit((uint32_t)0x00000000,  1, true ) == 0x00000002 );
   TEST_ADD( res, setbit((uint32_t)0x00000000, 30, true ) == 0x40000000 );
   TEST_ADD( res, setbit((uint32_t)0x00000000, 31, true ) == 0x80000000 );
   TEST_ADD( res, setbit((uint32_t)0xFFFFFFFF,  0, false) == 0xFFFFFFFE );
   TEST_ADD( res, setbit((uint32_t)0xFFFFFFFF,  1, false) == 0xFFFFFFFD );
   TEST_ADD( res, setbit((uint32_t)0xFFFFFFFF, 30, false) == 0xBFFFFFFF );
   TEST_ADD( res, setbit((uint32_t)0xFFFFFFFF, 31, false) == 0x7FFFFFFF );

   uint32_t dwords[] = { 0x00000000, 0xFC000000, 0x0000000F };
   TEST_ADD( res, getbits(dwords,  57, 10) == 0x000003FE );
   TEST_ADD( res, getbits(dwords,  58, 10) == 0x000003FF );
   TEST_ADD( res, getbits(dwords,  59, 10) == 0x000001FF );
   dwords[1] = 0x00000000;
   dwords[2] = 0x00000000;
   setbits(dwords, 57, 12, 0x00000ABC);
   TEST_ADD( res, dwords[2] == 0x00000015 && dwords[1] == 0x78000000 && dwords[0] == 0x00000000 );
   dwords[1] = 0x00000000;
   dwords[2] = 0x00000000;
   setbits(dwords, 58, 12, 0x00000ABC);
   TEST_ADD( res, dwords[2] == 0x0000002A && dwords[1] == 0xF0000000 && dwords[0] == 0x00000000 );
   dwords[1] = 0x00000000;
   dwords[2] = 0x00000000;
   setbits(dwords, 59, 12, 0x00000ABC);
   TEST_ADD( res, dwords[2] == 0x00000055 && dwords[1] == 0xE0000000 && dwords[0] == 0x00000000 );

   uchar bytes[] = {
      //  0, 4, 8      1, 5, 9     2, 6, 10     3, 7, 11
      (uchar)0x00, (uchar)0x00, (uchar)0x00, (uchar)0x00,
      (uchar)0x00, (uchar)0x00, (uchar)0x00, (uchar)0xFC,
      (uchar)0x0F, (uchar)0x00, (uchar)0x00, (uchar)0x00
   };
   TEST_ADD( res, getbits(bytes,  57, 10) == 0x000003FE );
   TEST_ADD( res, getbits(bytes,  58, 10) == 0x000003FF );
   TEST_ADD( res, getbits(bytes,  59, 10) == 0x000001FF );
   bytes[7] = (uchar)0x00;
   bytes[8] = (uchar)0x00;
   TEST_ADD( res, getbits(bytes,  57, 10) == 0x00000000 );
   TEST_ADD( res, getbits(bytes,  58, 10) == 0x00000000 );
   TEST_ADD( res, getbits(bytes,  59, 10) == 0x00000000 );

   bytes[7] = (uchar)0x00;
   bytes[8] = (uchar)0x00;
   setbits(bytes, 57, 12, 0x00000ABC);
   TEST_ADD( res, bytes[8] == (uchar)0x15 && bytes[7] == (uchar)0x78 );

   bytes[7] = (uchar)0x00;
   bytes[8] = (uchar)0x00;
   setbits(bytes, 58, 12, 0x00000ABC);
   TEST_ADD( res, bytes[8] == (uchar)0x2A && bytes[7] == (uchar)0xF0 );

   bytes[7] = (uchar)0x00;
   bytes[8] = (uchar)0x00;
   setbits(bytes, 59, 12, 0x00000ABC);
   TEST_ADD( res, bytes[8] == (uchar)0x55 && bytes[7] == (uchar)0xE0 );

   float f = 1.0f;
   uint  b = toBits( f );
   TEST_ADD( res, b == 0x3F800000 );
   f = toFloat( b );
   TEST_ADD( res, f == 1.0f );
   f = toFloat( 0x7F800000 ); // +INF
   TEST_ADD( res, f == f );

   f = toFloat( 0x7F800001 ); // sNaN
   b = toBits( f );
   TEST_ADD( res, f != f );
   TEST_ADD( res, b == 0x7F800001 );
   f = toFloat( 0x7FC00001 ); // qNaN
   b = toBits( f );
   TEST_ADD( res, f != f );
   TEST_ADD( res, b == 0x7FC00001 );

   f = +1.0f; //0x3F800000
   b = toMonotonousBits(f);
   TEST_ADD( res, b == 0xBF800000 );
   TEST_ADD( res, f == fromMonotonousBits(b) );
   f = -1.0f; //0xBF800000
   b = toMonotonousBits(f);
   TEST_ADD( res, b == 0x407FFFFF );
   TEST_ADD( res, f == fromMonotonousBits(b) );

   f = toFloat( 0xDEADBEEF );
   uint32_t s, e, m;
   toBits( f, s, e, m );
   TEST_ADD( res, s == 0x1 );
   TEST_ADD( res, e == 0xBD );
   TEST_ADD( res, m == 0x2DBEEF );

   // nlz
   for( uint i = 0; i < 32; ++i )
   {
      TEST_ADD( res, nlz(uint32_t(1<<i)) == (31-i) );
   }

#define TEST_RANGE( type, minV, maxV, refFunc, refV ) \
   for( int i = minV; i <= maxV; ++i )                \
   {                                                  \
      type tmp = type(i);                             \
      TEST_ADD( res, refFunc( tmp ) == refV );        \
   }

   {
      TEST_RANGE( uint8_t, 0x00, 0x00, nlz, 8 );
      TEST_RANGE( uint8_t, 0x01, 0x01, nlz, 7 );
      TEST_RANGE( uint8_t, 0x02, 0x03, nlz, 6 );
      TEST_RANGE( uint8_t, 0x04, 0x07, nlz, 5 );
      TEST_RANGE( uint8_t, 0x08, 0x0F, nlz, 4 );
      TEST_RANGE( uint8_t, 0x10, 0x1F, nlz, 3 );
      TEST_RANGE( uint8_t, 0x20, 0x3F, nlz, 2 );
      TEST_RANGE( uint8_t, 0x40, 0x7F, nlz, 1 );
      TEST_RANGE( uint8_t, 0x80, 0xFF, nlz, 0 );
   }

#undef TEST_RANGE

   // 64b routines.
   uint64_t u64;
   u64 = 0xDEADBEEF;
   u64 <<= 32;
   u64 |= 0xABCDEF12;
   TEST_ADD( res, msb(u64) == 0xDEADBEEF );
   TEST_ADD( res, lsb(u64) == 0xABCDEF12 );
   TEST_ADD( res, to64(0xDEADBEEF, 0xABCDEF12) == u64 );

   // read16/read32/read64 (second line of tmp is for 64b only).
   uint8_t tmp8[] = { 0x01, 0x23, 0x45, 0x67, 0x89, 0xAB, 0xCD, 0xEF,
                      0x10, 0x32, 0x54, 0x76, 0x98, 0xBA, 0xDC, 0xFE };
   TEST_ADD( res, read16(tmp8+0) == 0x2301 );
   TEST_ADD( res, read16(tmp8+1) == 0x4523 );
   TEST_ADD( res, read16(tmp8+2) == 0x6745 );
   TEST_ADD( res, read16(tmp8+3) == 0x8967 );
   TEST_ADD( res, read16(tmp8+4) == 0xAB89 );
   TEST_ADD( res, read16(tmp8+5) == 0xCDAB );
   TEST_ADD( res, read16(tmp8+6) == 0xEFCD );
   TEST_ADD( res, read32(tmp8+0) == 0x67452301 );
   TEST_ADD( res, read32(tmp8+1) == 0x89674523 );
   TEST_ADD( res, read32(tmp8+2) == 0xAB896745 );
   TEST_ADD( res, read32(tmp8+3) == 0xCDAB8967 );
   TEST_ADD( res, read32(tmp8+4) == 0xEFCDAB89 );
   uint16_t tmp16[] = { 0x0123, 0x4567, 0x89AB, 0xCDEF,
                        0x3210, 0x7654, 0xBA98, 0xFEDC };
   TEST_ADD( res, read32(tmp16+0) == 0x45670123 );
   TEST_ADD( res, read32(tmp16+1) == 0x89AB4567 );
   TEST_ADD( res, read32(tmp16+2) == 0xCDEF89AB );
   // read64 (this one is tricky to check, since 0xFFFFFFFFFFFFFFFF isn't supported everywhere).
   TEST_ADD( res, msb(read64(tmp8+8)) == 0xFEDCBA98 );
   TEST_ADD( res, lsb(read64(tmp8+8)) == 0x76543210 );
   TEST_ADD( res, msb(read64(tmp8+3)) == 0x543210EF );
   TEST_ADD( res, lsb(read64(tmp8+3)) == 0xCDAB8967 );

   TEST_ADD( res, msb(read64(tmp16+4)) == 0xFEDCBA98 );
   TEST_ADD( res, lsb(read64(tmp16+4)) == 0x76543210 );
   TEST_ADD( res, msb(read64(tmp16+3)) == 0xBA987654 );
   TEST_ADD( res, lsb(read64(tmp16+3)) == 0x3210CDEF );
}

void util_bits_float( Test::Result& res )
{
   float f = 1.0f;
   uint  b = toBits( f );
   TEST_ADD( res, b == 0x3F800000 );
   f = toFloat( b );
   TEST_ADD( res, f == 1.0f );
   f = toFloat( 0x7F800000 ); // +INF
   TEST_ADD( res, f == f );

   f = toFloat( 0x7F800001 ); // sNaN
   b = toBits( f );
   TEST_ADD( res, f != f );
   TEST_ADD( res, b == 0x7F800001 );
   f = toFloat( 0x7FC00001 ); // qNaN
   b = toBits( f );
   TEST_ADD( res, f != f );
   TEST_ADD( res, b == 0x7FC00001 );

   f = +1.0f; //0x3F800000
   b = toMonotonousBits(f);
   TEST_ADD( res, b == 0xBF800000 );
   TEST_ADD( res, f == fromMonotonousBits(b) );
   f = -1.0f; //0xBF800000
   b = toMonotonousBits(f);
   TEST_ADD( res, b == 0x407FFFFF );
   TEST_ADD( res, f == fromMonotonousBits(b) );

   f = toFloat( 0xDEADBEEF );
   uint32_t s, e, m;
   toBits( f, s, e, m );
   TEST_ADD( res, s == 0x1 );
   TEST_ADD( res, e == 0xBD );
   TEST_ADD( res, m == 0x2DBEEF );

   TEST_ADD( res, copySign( 1.0f,  2.0f) ==  1.0f );
   TEST_ADD( res, copySign( 1.0f, -2.0f) == -1.0f );
   TEST_ADD( res, copySign(-1.0f, -2.0f) == -1.0f );
   TEST_ADD( res, copySign(-1.0f,  2.0f) ==  1.0f );

   TEST_ADD( res, copySign( 1.0,  2.0) ==  1.0 );
   TEST_ADD( res, copySign( 1.0, -2.0) == -1.0 );
   TEST_ADD( res, copySign(-1.0, -2.0) == -1.0 );
   TEST_ADD( res, copySign(-1.0,  2.0) ==  1.0 );
}

void util_bits_round( Test::Result& res )
{
   TEST_ADD( res, roundTo(-2.51f, 0) == -3.0f );
   TEST_ADD( res, roundTo(-2.50f, 0) == -2.0f );
   TEST_ADD( res, roundTo(-2.49f, 0) == -2.0f );
   TEST_ADD( res, roundTo(-1.51f, 0) == -2.0f );
   TEST_ADD( res, roundTo(-1.50f, 0) == -2.0f );
   TEST_ADD( res, roundTo(-1.49f, 0) == -1.0f );
   TEST_ADD( res, roundTo(-1.01f, 0) == -1.0f );
   TEST_ADD( res, roundTo(-1.00f, 0) == -1.0f );
   TEST_ADD( res, roundTo(-0.99f, 0) == -1.0f );
   TEST_ADD( res, roundTo(-0.51f, 0) == -1.0f );
   TEST_ADD( res, roundTo(-0.50f, 0) ==  0.0f );
   TEST_ADD( res, roundTo(-0.49f, 0) ==  0.0f );
   TEST_ADD( res, roundTo(-0.25f, 0) ==  0.0f );
   TEST_ADD( res, roundTo( 0.00f, 0) ==  0.0f );
   TEST_ADD( res, roundTo( 0.00f, 0) ==  0.0f );
   TEST_ADD( res, roundTo( 0.25f, 0) ==  0.0f );
   TEST_ADD( res, roundTo( 0.49f, 0) ==  0.0f );
   TEST_ADD( res, roundTo( 0.50f, 0) ==  0.0f );
   TEST_ADD( res, roundTo( 0.51f, 0) ==  1.0f );
   TEST_ADD( res, roundTo( 0.99f, 0) ==  1.0f );
   TEST_ADD( res, roundTo( 1.00f, 0) ==  1.0f );
   TEST_ADD( res, roundTo( 1.01f, 0) ==  1.0f );
   TEST_ADD( res, roundTo( 1.49f, 0) ==  1.0f );
   TEST_ADD( res, roundTo( 1.50f, 0) ==  2.0f );
   TEST_ADD( res, roundTo( 1.51f, 0) ==  2.0f );
   TEST_ADD( res, roundTo( 2.49f, 0) ==  2.0f );
   TEST_ADD( res, roundTo( 2.50f, 0) ==  2.0f );
   TEST_ADD( res, roundTo( 2.51f, 0) ==  3.0f );

   TEST_ADD( res, roundTo( 0.74f,  0) ==  1.00f );
   TEST_ADD( res, roundTo( 0.75f,  0) ==  1.00f );
   TEST_ADD( res, roundTo( 0.76f,  0) ==  1.00f );
   TEST_ADD( res, roundTo( 0.74f, -1) ==  0.50f );
   TEST_ADD( res, roundTo( 0.75f, -1) ==  1.00f );
   TEST_ADD( res, roundTo( 0.76f, -1) ==  1.00f );
   TEST_ADD( res, roundTo( 0.74f, -2) ==  0.75f );
   TEST_ADD( res, roundTo( 0.75f, -2) ==  0.75f );
   TEST_ADD( res, roundTo( 0.76f, -2) ==  0.75f );
   TEST_ADD( res, roundTo( 7.00f,  0) ==  7.00f );
   TEST_ADD( res, roundTo( 7.00f,  1) ==  8.00f );

   /**
   for( float g = -5.0f; g < 5.0f; g += 1.0f )
   {
      float f;
      f = g + 0.0f;
      printf("f=%5.2f --> %5.2f, %5.2f\n", f, roundTo(f, 0), roundTo(f, -1));
      f = g + 0.1f;
      printf("f=%5.2f --> %5.2f, %5.2f\n", f, roundTo(f, 0), roundTo(f, -1));
      f = g + 0.24f;
      printf("f=%5.2f --> %5.2f, %5.2f\n", f, roundTo(f, 0), roundTo(f, -1));
      f = g + 0.25f;
      printf("f=%5.2f --> %5.2f, %5.2f\n", f, roundTo(f, 0), roundTo(f, -1));
      f = g + 0.26f;
      printf("f=%5.2f --> %5.2f, %5.2f\n", f, roundTo(f, 0), roundTo(f, -1));
      f = g + 0.49f;
      printf("f=%5.2f --> %5.2f, %5.2f\n", f, roundTo(f, 0), roundTo(f, -1));
      f = g + 0.50f;
      printf("f=%5.2f --> %5.2f, %5.2f\n", f, roundTo(f, 0), roundTo(f, -1));
      f = g + 0.51f;
      printf("f=%5.2f --> %5.2f, %5.2f\n", f, roundTo(f, 0), roundTo(f, -1));
      f = g + 0.74f;
      printf("f=%5.2f --> %5.2f, %5.2f\n", f, roundTo(f, 0), roundTo(f, -1));
      f = g + 0.75f;
      printf("f=%5.2f --> %5.2f, %5.2f\n", f, roundTo(f, 0), roundTo(f, -1));
      f = g + 0.76f;
      printf("f=%5.2f --> %5.2f, %5.2f\n", f, roundTo(f, 0), roundTo(f, -1));
      f = g + 0.9f;
      printf("f=%5.2f --> %5.2f, %5.2f\n", f, roundTo(f, 0), roundTo(f, -1));
   }
   **/
}

void util_date( Test::Result& res )
{
   Date  date1, date2;

   date1.set( 2000, Date::MONTH_JAN,  1 );  // January 1st 2000.
   //TEST_ADD( res, date1.toStr() == "Sat Jan  1 00:00:00 2000" ); // Not cross-platform.
   TEST_ADD( res, date1.toISO8601() == "2000-01-01T00:00:00" );

   date2.set( 2013, Date::MONTH_DEC, 25 );  // Christmas 2013.
   //TEST_ADD( res, date2.toStr() == "Wed Dec 25 00:00:00 2013" ); // Not cross-platform.
   TEST_ADD( res, date2.toISO8601() == "2013-12-25T00:00:00" );

   double diff = date2 - date1;
   double diffGold = (365*9 + 366*4 + 365-7) * 24 * 60 * 60; // Leap years: 2000, 2004, 2008, 2012.
   TEST_ADD( res, diff == diffGold );

   int y, m, d, wd;
   date2.get( y, m, d, wd );
   TEST_ADD( res, y == 2013 );
   TEST_ADD( res, m == Date::MONTH_DEC );
   TEST_ADD( res, d == 25 );
   TEST_ADD( res, wd == Date::WEEKDAY_WED );

   //// Check date validity.
   //TEST_ADD( res, date1.valid() );
   //StdErr << (uint)(time_t)date1 << nl;
   //date1.set( 2005, Date::MONTH_FEB, 30 );
   //StdErr << (uint)(time_t)date1 << nl;
   //TEST_ADD( res, !date1.valid() );

   // Check time_t type conversion routine.
   time_t t1 = date1;
   time_t t2 = date2;
   TEST_ADD( res, t1 < t2 );

   date1.fromISO8601( "1999-12-31T22:33:44" );
   int hh, mm, ss;
   date1.get( y, m, d, hh, mm, ss );
   TEST_ADD( res, y  == 1999 );
   TEST_ADD( res, m  == Date::MONTH_DEC );
   TEST_ADD( res, d  == 31 );
   TEST_ADD( res, abs(hh - 22) <= 1 );  // DST affects this!?!
   TEST_ADD( res, mm == 33 );
   TEST_ADD( res, ss == 44 );
}

void util_date_show( Test::Result& /*res*/ )
{
   StdOut << '\n';
   Date  date;

   date = Date::now();
   StdOut << "[Current Time] --> " << date.toStr() << " --> " << date.toISO8601() << nl;

   date.set( 2000, Date::MONTH_JAN,  1 );  // January 1st 2000.
   StdOut << "[Jan 1st 2000] --> " << date.toStr() << " --> " << date.toISO8601() << nl;

   date.set( 2013, Date::MONTH_DEC, 25 );  // Christmas 2013.
   StdOut << "[Xmas    2013] --> " << date.toStr() << " --> " << date.toISO8601() << nl;
}

void util_endian_swapper( Test::Result& res )
{
   char data[] = {
      0x01, 0x02, 0x03, 0x04,
      0x05, 0x06, 0x07, 0x08
   };

#define CHK( a, b, c, d, e, f, g, h ) \
   TEST_ADD( res, data[0] == a && data[1] == b && data[2] == c && data[3] == d && \
                  data[4] == e && data[5] == f && data[6] == g && data[7] == h )

   CHK( 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08 );
   EndianSwapper::littleToBig_16b( data );
   CHK( 0x02, 0x01, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08 );
   EndianSwapper::bigToLittle_16b( data );
   CHK( 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08 );
   EndianSwapper::littleToBig_32b( data );
   CHK( 0x04, 0x03, 0x02, 0x01, 0x05, 0x06, 0x07, 0x08 );
   EndianSwapper::bigToLittle_32b( data );
   CHK( 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08 );
   EndianSwapper::littleToBig_64b( data );
   CHK( 0x08, 0x07, 0x06, 0x05, 0x04, 0x03, 0x02, 0x01 );
   EndianSwapper::bigToLittle_64b( data );
   CHK( 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08 );

   TEST_ADD( res, EndianSwapper::get16LE(data) == 0x0201 );
   TEST_ADD( res, EndianSwapper::get32LE(data) == 0x04030201 );
   TEST_ADD( res, EndianSwapper::get64LE(data) == to64(0x08070605, 0x04030201) );

   TEST_ADD( res, EndianSwapper::get16BE(data) == 0x0102 );
   TEST_ADD( res, EndianSwapper::get32BE(data) == 0x01020304 );
   TEST_ADD( res, EndianSwapper::get64BE(data) == to64(0x01020304, 0x05060708) );

   //printf("%02x %02x %02x %02x %02x %02x %02x %02x\n", data[0], data[1], data[2], data[3], data[4], data[5], data[6], data[7]);

#undef CHK

}

void util_formatter( Test::Result& res )
{
   TEST_ADD( res, humanReadableSize(0x00000000).toStr() ==       "0 B"   );
   TEST_ADD( res, humanReadableSize(0x00000003).toStr() ==       "3 B"   );
   TEST_ADD( res, humanReadableSize(0x00000300).toStr() ==     "768 B"   );
   TEST_ADD( res, humanReadableSize(0x000003FF).toStr() ==    "1023 B"   );
   TEST_ADD( res, humanReadableSize(0x00000400).toStr() ==       "1 KiB" );
   TEST_ADD( res, humanReadableSize(0x00000401).toStr() == "1.00098 KiB" );
   TEST_ADD( res, humanReadableSize(0x00000500).toStr() ==    "1.25 KiB" );
   TEST_ADD( res, humanReadableSize(0x000C0000).toStr() ==     "768 KiB" );
   TEST_ADD( res, humanReadableSize(0x00100000).toStr() ==       "1 MiB" );
   TEST_ADD( res, humanReadableSize(0x00100000).value() == 1.0           );
   TEST_ADD( res, humanReadableSize(0x00100001).value()  > 1.0           );
   TEST_ADD( res, humanReadableSize(0x00180000).toStr() ==     "1.5 MiB" );
   TEST_ADD( res, humanReadableSize(0x3FFFFFFF).value()  > 1023          );
   TEST_ADD( res, humanReadableSize(0x40000000).toStr() ==       "1 GiB" );
   TEST_ADD( res, humanReadableSize(0x40000000).value() == 1.0           );
}

void util_half( Test::Result& res )
{
   float f;
   uint  b;
   Half  h;

   // Testing simple values.
   h = 0.0f;
   TEST_ADD( res, h.value() == 0.0f );
   TEST_ADD( res, h.bits() == 0x0000 );
   h = 1.0f;
   TEST_ADD( res, h.value() == 1.0f );
   TEST_ADD( res, h.bits() == 0x3C00 );

   // Testing operators, and some more values.
   h += 3.25f;
   TEST_ADD( res, h.value() == 4.25f );
   TEST_ADD( res, h.bits() == 0x4440 );
   h *= -4.0f;
   TEST_ADD( res, h.value() == -17.0f );
   TEST_ADD( res, h.bits() == 0xCC40 );
   h /= 17.0f;
   TEST_ADD( res, h.value() == -1.0f );
   TEST_ADD( res, h.bits() == 0xBC00 );
   h = 65504.0f;
   TEST_ADD( res, h.value() == 65504.0f );
   TEST_ADD( res, h.bits() == 0x7BFF );
   h = 1e6f; // Clamps to MAX_HALF
   TEST_ADD( res, h.bits() == 0x7BFF );

   // INFs/NaNs.
   h = toFloat( 0xFF800000 ); // -INF
   TEST_ADD( res, h.bits() == 0xFC00 );
   h = toFloat( 0x7F800000 ); // +INF
   TEST_ADD( res, h.bits() == 0x7C00 );

   h = toFloat( 0x7F800001 ); // Special case NaN which could result in INF
   TEST_ADD( res, h.bits() == 0x7C01 );
   b = toBits( h.value() );
   TEST_ADD( res, b == 0x7F802000 );

   // Denorms.
   f = powf( 2.0f, -25.0f ); // Too small.
   h = f;
   TEST_ADD( res, h.bits() == 0x0000 );
   TEST_ADD( res, h.value() != f );
   f = powf( 2.0f, -24.0f ); // Smallest denormalized.
   h = f;
   TEST_ADD( res, h.bits() == 0x0001 );
   TEST_ADD( res, h.value() == f );
   f = 3.0f * powf( 2.0f, -24.0f ); // Some denormalized.
   h = f;
   TEST_ADD( res, h.bits() == 0x0003 );
   TEST_ADD( res, h.value() == f );
   f = powf( 2.0f, -14.0f ) - powf( 2.0f, -26.0f ); // Largest denormalized.
   h = f;
   TEST_ADD( res, h.bits() == 0x03FF );
   TEST_ADD( res, h.value() != f );
   f = powf( 2.0f, -14.0f ); // Smallest normalized.
   h = f;
   TEST_ADD( res, h.bits() == 0x0400 );
   TEST_ADD( res, h.value() == f );
   f = powf( 2.0f, -15.0f ) + powf( 2.0f, -24.0f ); // Stressing denormalized range.
   h = f;
   TEST_ADD( res, h.bits() == 0x0201 );
   TEST_ADD( res, h.value() == f );
}

void util_half_consistency( Test::Result& res )
{
   // Checking consistency of all 16b patterns.
   for( uint b = 0; b < (1<<16); ++b )
   {
      float f = Half::toFloat(b);
      if( b != Half::toBits(f) ) printf("%f 0x%08x vs. 0x%08x\n", f, b, Half::toBits(f));
      TEST_ADD( res, Half::toBits(f) == b );
   }
}

void util_idpool( Test::Result& res )
{
   IDPool  p( 2 );

   TEST_ADD( res, p.next() == 2 );
   TEST_ADD( res, p.next() == 3 );
   TEST_ADD( res, p.next() == 4 );
   p.release( 3 );
   TEST_ADD( res, p.next() == 3 );
   TEST_ADD( res, p.next() == 5 );
   p.release( 3 );
   p.release( 2 );
   TEST_ADD( res, p.next() == 2 );
   TEST_ADD( res, p.next() == 3 );
   TEST_ADD( res, p.next() == 6 );
   p.release( 0 );  // Special corner case.
   TEST_ADD( res, p.next() == 0 );
   TEST_ADD( res, p.next() == 7 );
   p.release( 7 );
   TEST_ADD( res, p.next() == 7 );
   p.release( 7 );
   p.release( 6 );
   p.release( 5 );
   p.release( 4 );
   p.release( 3 );
}

void util_memory( Test::Result& res )
{
   //StdErr << nl;
   uint8_t src[16] = { 0x1, 0x2, 0x3, 0x4, 0x5, 0x6, 0x7, 0x8, 0x9, 0xA, 0xB, 0xC, 0xD, 0xE, 0xF };

   uint8_t dst[16] = { 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0 };

#define CHK_ALL( a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p ) \
   ( dst[ 0]==a && dst[ 1]==b && dst[ 2]==c && dst[ 3]==d && \
     dst[ 4]==e && dst[ 5]==f && dst[ 6]==g && dst[ 7]==h && \
     dst[ 8]==i && dst[ 9]==j && dst[10]==k && dst[11]==l && \
     dst[12]==m && dst[13]==n && dst[14]==o && dst[15]==p )

   //printf( "%1x %1x %1x %1x %1x %1x %1x %1x %1x %1x %1x %1x %1x %1x %1x %1x\n",
   //        dst[ 0], dst[ 1], dst[ 2], dst[ 3], dst[ 4], dst[ 5], dst[ 6], dst[ 7],
   //        dst[ 8], dst[ 9], dst[10], dst[11], dst[12], dst[13], dst[14], dst[15] );

   // Test copyData<>() routine.
   copyData<U32_t>( src+4, dst+10 );
   TEST_ADD( res, CHK_ALL(0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x5,0x6,0x7,0x8,0x0,0x0) );

   copyData<U64_t>( src+7, dst+1 );
   TEST_ADD( res, CHK_ALL(0x0,0x8,0x9,0xA,0xB,0xC,0xD,0xE,0xF,0x0,0x5,0x6,0x7,0x8,0x0,0x0) );

   copyData<U32_t>( src+1, dst+8 );
   TEST_ADD( res, CHK_ALL(0x0,0x8,0x9,0xA,0xB,0xC,0xD,0xE,0x2,0x3,0x4,0x5,0x7,0x8,0x0,0x0) );

   // Test memset_#b() routines.
   memset_8b( dst+1, src+2, 5 );
   TEST_ADD( res, CHK_ALL(0x0,0x3,0x3,0x3,0x3,0x3,0xD,0xE,0x2,0x3,0x4,0x5,0x7,0x8,0x0,0x0) );

   memset_16b( dst+1, src+3, 7 );
   TEST_ADD( res, CHK_ALL(0x0,0x4,0x5,0x4,0x5,0x4,0x5,0x4,0x2,0x3,0x4,0x5,0x7,0x8,0x0,0x0) );

   memset_32b( dst+1, src+4, 9 );
   TEST_ADD( res, CHK_ALL(0x0,0x5,0x6,0x7,0x8,0x5,0x6,0x7,0x8,0x5,0x4,0x5,0x7,0x8,0x0,0x0) );

   memset_64b( dst+0, src+5, 9 ); // Pointer needs to start aligned.
   TEST_ADD( res, CHK_ALL(0x6,0x7,0x8,0x9,0xA,0xB,0xC,0xD,0x6,0x5,0x4,0x5,0x7,0x8,0x0,0x0) );

   //printf( "%1x %1x %1x %1x %1x %1x %1x %1x %1x %1x %1x %1x %1x %1x %1x %1x\n",
   //        dst[ 0], dst[ 1], dst[ 2], dst[ 3], dst[ 4], dst[ 5], dst[ 6], dst[ 7],
   //        dst[ 8], dst[ 9], dst[10], dst[11], dst[12], dst[13], dst[14], dst[15] );

}

void util_packed( Test::Result& res )
{

#define TEST3( var, a, b, c ) \
   TEST_ADD( res, var.x() == a && var.y() == b && var.z() == c )

#define TEST3S( var, a, b, c ) \
   TEST_ADD( res, var.xs() == a && var.ys() == b && var.zs() == c )

   {
   X11Y11Z10 v;
   v.set( 0, 0, 0 );
   TEST3( v, 0, 0, 0 );
   v.x(0xFFFFFF1F);
   TEST3( v, 0x71F, 0, 0 );
   v.y(0xFFFFFF2F);
   TEST3( v, 0x71F, 0x72F, 0 );
   v.z(0xFFFFFF3F);
   TEST3( v, 0x71F, 0x72F, 0x33F );
   v.set( v.x(), 0, v.z() );
   TEST3( v, 0x71F, 0, 0x33F );
   v.set( v.z(), v.x(), 0 );
   TEST3( v, 0x33F, 0x71F, 0 );
   v.set( v.z(), v.x(), v.y() );
   TEST3( v, 0, 0x33F, 0x31F );

   v.set( -1, 2, -3 );
   TEST3S( v, -1, 2, -3 );
   TEST3( v, (1<<11)-1, 2, (1<<10)-3 );
   }

}

void util_sha1( Test::Result& res )
{
   SHA1 sha1;
   const SHA1::Digest& digest = sha1.digest();
   uint32_t gold[5];
   uint32_t temp[5];

   // Example given in A.1 of the FIPS 180.2 document.
   sha1.begin();
   sha1.put( "abc", 3 );
   sha1.end();
   TEST_ADD( res, digest.str() == "a9993e364706816aba3e25717850c26c9cd0d89d" );
   gold[0] = 0xa9993e36;
   gold[1] = 0x4706816a;
   gold[2] = 0xba3e2571;
   gold[3] = 0x7850c26c;
   gold[4] = 0x9cd0d89d;
   //TEST_ADD( res, digest == gold );
   TEST_ADD( res, digest == SHA1::Digest(gold) );
   TEST_ADD( res, digest == SHA1::Digest(gold[0], gold[1], gold[2], gold[3], gold[4]) );

   // Example given in A.2 of the FIPS 180.2 document.
   sha1.begin();
   sha1.put( "abcdbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomnopnopq" );
   sha1.end();
   TEST_ADD( res, digest.str() == "84983e441c3bd26ebaae4aa1f95129e5e54670f1" );

   // Example given in A.3 of the FIPS 180.2 document.
   const size_t n = size_t(1e6);
   char tmp[n];
   for( size_t i = 0; i < n; ++i )
   {
      tmp[i] = 'a';
   }
   sha1.begin();
   sha1.put( tmp, n );
   sha1.end();
   TEST_ADD( res, digest.str() == "34aa973cd4c4daa4f61eeb2bdbad27316534016f" );

   // Testing various put() routines.
   sha1.begin();
   sha1.put( (uint8_t )0x01       ); // 01??????
   sha1.put( (uint16_t)0xF123     ); // 0100F123
   sha1.put( (uint8_t )0x02       ); // 0100F123 02??????
   sha1.put( (uint32_t)0xE1234567 ); // 0100F123 02000000 E1234567
   uint64_t u64 = 0xD1234567;
   u64 <<= 32;
   u64 |= 0x89012345;
   sha1.put( (uint8_t )0x03       ); // 0100F123 02000000 E1234567 03??????
   sha1.put( u64 );                  // 0100F123 02000000 E1234567 03000000 89012345 D1234567
   sha1.end();
   // Is this one cross-platform?
   TEST_ADD( res, digest.str() == "f2c9d372492b6a0138f10770471a6c5f77737141" );

   // Test digest comparison.
   temp[0] = 0x11111111;  gold[0] = 0x11111111;
   temp[1] = 0x22222222;  gold[1] = 0x22222222;
   temp[2] = 0x33333333;  gold[2] = 0x33333333;
   temp[3] = 0x44444444;  gold[3] = 0x44444444;
   temp[4] = 0x55555555;  gold[4] = 0x55555555;
   TEST_ADD( res, !(SHA1::Digest(temp) <  SHA1::Digest(gold)) );
   TEST_ADD( res,  (SHA1::Digest(temp) <= SHA1::Digest(gold)) );
   TEST_ADD( res,  (SHA1::Digest(temp) == SHA1::Digest(gold)) );
   TEST_ADD( res,  (SHA1::Digest(temp) >= SHA1::Digest(gold)) );
   TEST_ADD( res, !(SHA1::Digest(temp) >  SHA1::Digest(gold)) );
   // Test just before, and just after, for every DWORD.
   for( uint i = 0; i < 5; ++i )
   {
      // Just before.
      temp[i] -= 1;
      TEST_ADD( res,  (SHA1::Digest(temp) <  SHA1::Digest(gold)) );
      TEST_ADD( res,  (SHA1::Digest(temp) <= SHA1::Digest(gold)) );
      TEST_ADD( res, !(SHA1::Digest(temp) == SHA1::Digest(gold)) );
      TEST_ADD( res, !(SHA1::Digest(temp) >= SHA1::Digest(gold)) );
      TEST_ADD( res, !(SHA1::Digest(temp) >  SHA1::Digest(gold)) );
      // Just after.
      temp[i] += 2;
      TEST_ADD( res, !(SHA1::Digest(temp) <  SHA1::Digest(gold)) );
      TEST_ADD( res, !(SHA1::Digest(temp) <= SHA1::Digest(gold)) );
      TEST_ADD( res, !(SHA1::Digest(temp) == SHA1::Digest(gold)) );
      TEST_ADD( res,  (SHA1::Digest(temp) >= SHA1::Digest(gold)) );
      TEST_ADD( res,  (SHA1::Digest(temp) >  SHA1::Digest(gold)) );
      // Bring back to original.
      temp[i] -= 1;
   }
   // Sanity.
   TEST_ADD( res, !(SHA1::Digest(temp) <  SHA1::Digest(gold)) );
   TEST_ADD( res,  (SHA1::Digest(temp) <= SHA1::Digest(gold)) );
   TEST_ADD( res,  (SHA1::Digest(temp) == SHA1::Digest(gold)) );
   TEST_ADD( res,  (SHA1::Digest(temp) >= SHA1::Digest(gold)) );
   TEST_ADD( res, !(SHA1::Digest(temp) >  SHA1::Digest(gold)) );
}

void util_sha1_file( Test::Result& /*res*/ )
{
   StdErr << nl;
   SHA1 sha1;
   const char* filename = getenv("TEST_FILE");
   if( !filename )
   {
      StdErr << "TEST_FILE envvar is not set." << nl;
      exit( 1 );
   }

   FILE* file = fopen( filename, "rb" );
   if( !file )
   {
      StdErr << "Could not open file: '" << filename << "'" << nl;
      exit( 1 );
   }

   const size_t numBytes = 512*64;
   char bytes[numBytes];
   size_t s;

   sha1.begin();
   do
   {
      s = fread( bytes, 1, numBytes, file );
      sha1.put( bytes, s );
   } while( s == numBytes );
   StdErr << sha1.end().str() << " - " << filename << nl;

   fclose(file);

   exit( 0 );
}

void util_time( Test::Result& res )
{
   Time  time(1, 30);
   TEST_ADD( res, time.asHours()        == 1.5 );
   TEST_ADD( res, time.asMinutes()      == 90.0 );
   TEST_ADD( res, time.asSeconds()      == 5400.0 );
   TEST_ADD( res, time.asMilliseconds() == 5400000.0 );
   TEST_ADD( res, time.asMicroseconds() == 5400000000.0 );
   //StdErr << time.toStr() << nl;

   int t24[]   = { 0, 1, 11, 12, 13, 23 };
   int t60[]   = { 0, 1, 29, 30, 31, 58, 59 };
   int t1000[] = { 0, 1, 499, 500, 501, 998, 999 };

#define N(table) (sizeof(table)/sizeof(table[0]))

   for( size_t hi = 0; hi < N(t24); ++hi )
   {
      int h = t24[hi];
      for( size_t mi = 0; mi < N(t60); ++mi )
      {
         int m = t60[mi];
         for( size_t si = 0; si < N(t60); ++si )
         {
            int s = t60[si];
            for( size_t msi = 0; msi < N(t1000); ++msi )
            {
               int ms = t1000[msi];
               for( size_t usi = 0; usi < N(t1000); ++usi )
               {
                  int us = t1000[usi];
                  //StdErr << "+++++\n";
                  //StdErr << "loop: " << hi << ":" << mi << ":" << si << ":" << msi << ":" << usi << nl;
                  //StdErr << "loop: " << h << ":" << m << ":" << s << ":" << ms << ":" << us << nl;

                  int h2, m2, s2, ms2, us2;

                  time.set( h, m, s );
                  time.get( h2, m2, s2 );
                  TEST_ADD( res, h2 == h );
                  TEST_ADD( res, m2 == m );
                  TEST_ADD( res, s2 == s );

                  //StdErr << "3: " << h << "h " << m << "m " << s << "s\n";
                  //StdErr << "3: " << h2 << "h " << m2 << "m " << s2 << "s\n";

                  time.set( h, m, s, ms, us );
                  time.get( h2, m2, s2, ms2, us2 );
                  TEST_ADD( res, h2  == h );
                  TEST_ADD( res, m2  == m );
                  TEST_ADD( res, s2  == s );
                  TEST_ADD( res, ms2 == ms );
                  TEST_ADD( res, us2 == us );

                  //StdErr << time.toStr() << nl;
                  //StdErr << "5: " << h << "h " << m << "m " << s << "s " << ms << "ms " << us << "us" << nl;
                  //StdErr << "5: " << h2 << "h " << m2 << "m " << s2 << "s " << ms2 << "ms " << us2 << "us" << nl;
                  //StdErr << time.asSeconds() << " " << time.asMicroseconds() << nl;
                  //if( fail_count != 0 ) getchar();
               }
            }
         }
      }
   }
}

inline bool equal( double a, double b )
{
#if PLAT_ANDROID
   return fabs(a-b) < 1e-2;
#else
   return fabs(a-b) < 1e-3;
#endif
}

#if PLAT_POSIX
inline void  sleep( double t )
{
   double iPart, fPart;
   fPart = modf( t, &iPart );
   timespec ts;
   ts.tv_sec  = int(iPart);
   ts.tv_nsec = int(fPart*1e9);
   nanosleep( &ts, NULL );
}
#elif PLAT_WINDOWS
inline void  sleep( double t )
{
   DWORD tms = DWORD(t * 1000);
   Sleep( tms );
}
#endif

void util_timer( Test::Result& res )
{
#if PLAT_WINDOWS
   double fudge = 0.001;
#else
   double fudge = 0.0;
#endif
   {
      Timer timer;
      TEST_ADD( res,  equal(timer.elapsed(), 0.00) );
      TEST_ADD( res, !equal(timer.elapsed(), 0.01) ); // Sanity check.
      sleep( 0.01 );
      TEST_ADD( res, equal(timer.elapsed(), 0.01) );
      sleep( 0.02-fudge );
      TEST_ADD( res, equal(timer.elapsed(), 0.03) );
      TEST_ADD( res, equal(timer.restart(), 0.03) );
      sleep( 0.04 );
      TEST_ADD( res, equal(timer.elapsed(), 0.04) );
   }

   {
      StopTimer timer;
      TEST_ADD( res,  equal(timer.elapsed(), 0.00) );
      TEST_ADD( res, !equal(timer.elapsed(), 0.01) ); // Sanity check.
      sleep( 0.01 );
      TEST_ADD( res, equal(timer.resume() , 0.01) );
      sleep( 0.02-fudge );
      TEST_ADD( res, equal(timer.elapsed(), 0.03) );
      TEST_ADD( res, equal(timer.restart(), 0.03) );
      sleep( 0.04 );
      TEST_ADD( res, equal(timer.elapsed(), 0.04) );
      sleep( 0.03 );
      TEST_ADD( res, !timer.paused() );
      TEST_ADD( res, equal(timer.pause()  , 0.07) );
      TEST_ADD( res,  timer.paused() );
      sleep( 0.01 );
      TEST_ADD( res, equal(timer.elapsed(), 0.07) );
      sleep( 0.02 );
      TEST_ADD( res,  timer.paused() );
      TEST_ADD( res, equal(timer.resume() , 0.07) );
      TEST_ADD( res, !timer.paused() );
      TEST_ADD( res, equal(timer.elapsed(), 0.07) );
      TEST_ADD( res, !timer.paused() );
      sleep( 0.03 );
      TEST_ADD( res, equal(timer.elapsed(), 0.10) );
      sleep( 0.04 );
      TEST_ADD( res, equal(timer.restart(), 0.14) );
      TEST_ADD( res, !timer.paused() );
      sleep( 0.02 );
      TEST_ADD( res, equal(timer.elapsed(), 0.02) );
      TEST_ADD( res, !timer.paused() );
      TEST_ADD( res, equal(timer.pause()  , 0.02) );
      TEST_ADD( res,  timer.paused() );
      TEST_ADD( res, equal(timer.restart(), 0.02) );
      TEST_ADD( res, !timer.paused() );
      sleep( 0.03 );
      TEST_ADD( res, equal(timer.elapsed(), 0.03) );
   }
}


void util_radixsort( Test::Result& res )
{
   RadixSort rs;
   uint src[16] = {
      0x00000001, 0x80000001, 0x80000002, 0x800FF003,
      0x00FF0000, 0xFF00FF01, 0xFF00FF02, 0xFF00FF03,
      0x000FF000, 0xFF800000, 0x0F0F0F0F, 0xFEFEFEFE,
      0x0000FF00, 0x00000000, 0x00110000, 0xFF00FF01
   };
   rs.sort( 16, src );
   uint sumIndices = rs.sortedIndex(0);
   // Check the order of the result.
   for( uint i = 1; i < 16; ++i )
   {
      const uint prevIdx = rs.sortedIndex(i-1);
      const uint curIdx  = rs.sortedIndex(i);
      TEST_ADD( res, (prevIdx != curIdx) && (src[prevIdx] <= src[curIdx]) );
      sumIndices += curIdx;
   }
   // Check that we hit all of the indices, from 0 to 15 (the equation is (min+max)*n/2).
   TEST_ADD( res, sumIndices == (0+15)*16/2 );

   float srcF[16] = {
      toFloat(0x00000001), toFloat(0x80000001), toFloat(0x80000002), toFloat(0x800FF003),
      toFloat(0x00FF0000), toFloat(0xFF00FF01), toFloat(0xFF00FF02), toFloat(0xFF00FF03),
      toFloat(0x000FF000), toFloat(0xFF800000), toFloat(0x0F0F0F0F), toFloat(0xFEFEFEFE),
      toFloat(0x0000FF00), toFloat(0x00000000), toFloat(0x00110000), toFloat(0xFF00FF01)
   };
   rs.sort( 16, srcF );
   sumIndices = rs.sortedIndex(0);
   // Check the order of the result.
   for( uint i = 1; i < 16; ++i )
   {
      const uint prevIdx = rs.sortedIndex(i-1);
      const uint curIdx  = rs.sortedIndex(i);
      TEST_ADD( res, (prevIdx != curIdx) && (srcF[prevIdx] <= srcF[curIdx]) );
      sumIndices += curIdx;
   }
   // Check that we hit all of the indices, from 0 to 15 (the equation is (min+max)*n/2).
   TEST_ADD( res, sumIndices == (0+15)*16/2 );

   // Michael Herf's bit flipping trick.
   uint32_t* srcU = toMonotonousBits( 16, srcF );
   rs.clear();
   rs.sort( 16, (uint*)srcU );
   fromMonotonousBits( 16, srcU );  // Need to convert back to compare the values properly.
   // Check the order of the result.
   for( uint i = 1; i < 16; ++i )
   {
      const uint prevIdx = rs.sortedIndex(i-1);
      const uint curIdx  = rs.sortedIndex(i);
      TEST_ADD( res, (prevIdx != curIdx) && (srcF[prevIdx] <= srcF[curIdx]) );
      sumIndices += curIdx;
   }

   // Check stability of the sort.
   // Terdiman's trick, unstable, but likely faster.
   srcF[0] = toFloat(0x80000001);
   srcF[1] = toFloat(0x80000001);
   srcF[2] = toFloat(0x80000002);
   TEST_ADD( res, rs.sortedIndices() != NULL );
   rs.clear();
   TEST_ADD( res, rs.sortedIndices() == NULL );
   rs.sort( 3, srcF );
   // If the sort is stable, we should end up with 2, 0, 1
   // but Terdiman's trick yields 2, 1, 0.
   TEST_ADD( res, rs.sortedIndex(0) == 2 );
   TEST_ADD( res, rs.sortedIndex(1) == 1 );
   TEST_ADD( res, rs.sortedIndex(2) == 0 );

   // Herf's trick, stable, but potentially slower.
   srcU = toMonotonousBits( 3, srcF );
   TEST_ADD( res, srcU[0] == 0x7FFFFFFE );
   TEST_ADD( res, srcU[1] == 0x7FFFFFFE );
   TEST_ADD( res, srcU[2] == 0x7FFFFFFD );
   rs.clear();
   rs.sort( 3, (uint*)srcU );
   // If the sort is stable, we should end up with 2, 0, 1
   // but Terdiman's trick yields 2, 1, 0.
   TEST_ADD( res, rs.sortedIndex(0) == 2 );
   TEST_ADD( res, rs.sortedIndex(1) == 0 );
   TEST_ADD( res, rs.sortedIndex(2) == 1 );
   fromMonotonousBits( 3, srcU );
   TEST_ADD( res, srcU[0] == 0x80000001 );
   TEST_ADD( res, srcU[1] == 0x80000001 );
   TEST_ADD( res, srcU[2] == 0x80000002 );
}

void util_unicode( Test::Result& res )
{
   const char utf8_str[] = {
      'A',                                            // U+0041: LATIN CAPITAL LETTER A
      'z',                                            // U+007A: LATIN SMALL LETTER Z
      char(0xC2), char(0xB2),                         // U+00B2: SUPERSCRIPT TWO
      char(0xC3), char(0xA9),                         // U+00E9: LATIN SMALL LETTER E WITH ACUTE
      char(0xC3), char(0xA8),                         // U+00E8: LATIN SMALL LETTER E WITH GRAVE
      char(0xC3), char(0xA0),                         // U+00E0: LATIN SMALL LETTER A WITH GRAVE
      char(0xE2), char(0x80), char(0xA2),             // U+2022: BULLET
      char(0xE3), char(0x8E), char(0x94),             // U+3394: SQUARE THZ
      char(0xF0), char(0x93), char(0x82), char(0x80), // U+13080: EGYPTIAN HIEROGLYPH D010 (an eye)
      char(0xF0), char(0x9F), char(0x86), char(0x89)  // U+1F189: NEGATIVE SQUARED LATIN CAPITAL LETTER Z
   };
   const size_t utf8_len = sizeof(utf8_str)/sizeof(utf8_str[0]);

   const char16_t utf16_str[] = {
      0x0041,         // U+0041: LATIN CAPITAL LETTER A
      0x007A,         // U+007A: LATIN SMALL LETTER Z
      0x00B2,         // U+00B2: SUPERSCRIPT TWO
      0x00E9,         // U+00E9: LATIN SMALL LETTER E WITH ACUTE
      0x00E8,         // U+00E8: LATIN SMALL LETTER E WITH GRAVE
      0x00E0,         // U+00E0: LATIN SMALL LETTER A WITH GRAVE
      0x2022,         // U+2022: BULLET
      0x3394,         // U+3394: SQUARE THZ
      0xD80C, 0xDC80, // U+13080: EGYPTIAN HIEROGLYPH D010 (an eye)
      0xD83C, 0xDD89, // U+1F189: NEGATIVE SQUARED LATIN CAPITAL LETTER Z
   };
   const size_t utf16_len = sizeof(utf16_str)/sizeof(utf16_str[0]);

   //const char32_t utf32_str[] = {
   //   0x0041,  // U+0041: LATIN CAPITAL LETTER A
   //   0x007A,  // U+007A: LATIN SMALL LETTER Z
   //   0x00B2,  // U+00B2: SUPERSCRIPT TWO
   //   0x00E9,  // U+00E9: LATIN SMALL LETTER E WITH ACUTE
   //   0x00E8,  // U+00E8: LATIN SMALL LETTER E WITH GRAVE
   //   0x00E0,  // U+00E0: LATIN SMALL LETTER A WITH GRAVE
   //   0x2022,  // U+2022: BULLET
   //   0x3394,  // U+3394: SQUARE THZ
   //   0x13080, // U+13080: EGYPTIAN HIEROGLYPH D010 (an eye)
   //   0x1F189, // U+1F189: NEGATIVE SQUARED LATIN CAPITAL LETTER Z
   //};
   //const size_t utf32_len = sizeof(utf32_str)/sizeof(utf32_str[0]);

   char32_t code;
   const char*     cur8;
   const char16_t* cur16;
   //const char32_t* cur32;

   //StdErr << nl;
   //StdErr << "_" << String(utf8_str, utf8_len) << "_" << nl;
   //return;

   TEST_ADD( res, invalid( INVALID_CODEPOINT ) );

   // Decoding.
   cur8 = utf8_str;
   TEST_ADD( res, getUTF8Length(cur8, utf8_len) == 10 );
   cur8 = nextUTF8( cur8, code );
   TEST_ADD( res, (cur8 - utf8_str) == 1 );
   TEST_ADD( res, code == 0x0041 );
   cur8 = nextUTF8( cur8, code );
   TEST_ADD( res, (cur8 - utf8_str) == 2 );
   TEST_ADD( res, code == 0x007A );
   cur8 = nextUTF8( cur8, code );
   TEST_ADD( res, (cur8 - utf8_str) == 4 );
   TEST_ADD( res, code == 0x00B2 );
   cur8 = nextUTF8( cur8, code );
   TEST_ADD( res, (cur8 - utf8_str) == 6 );
   TEST_ADD( res, code == 0x00E9 );
   cur8 = nextUTF8( cur8, code );
   TEST_ADD( res, (cur8 - utf8_str) == 8 );
   TEST_ADD( res, code == 0x00E8 );
   cur8 = nextUTF8( cur8, code );
   TEST_ADD( res, (cur8 - utf8_str) == 10 );
   TEST_ADD( res, code == 0x00E0 );
   cur8 = nextUTF8( cur8, code );
   TEST_ADD( res, (cur8 - utf8_str) == 13 );
   TEST_ADD( res, code == 0x2022 );
   cur8 = nextUTF8( cur8, code );
   TEST_ADD( res, (cur8 - utf8_str) == 16 );
   TEST_ADD( res, code == 0x3394 );
   cur8 = nextUTF8( cur8, code );
   TEST_ADD( res, (cur8 - utf8_str) == 20 );
   TEST_ADD( res, code == 0x13080 );
   cur8 = nextUTF8( cur8, code );
   TEST_ADD( res, (cur8 - utf8_str) == 24 );
   TEST_ADD( res, code == 0x1F189 );

   cur16 = utf16_str;
   TEST_ADD( res, getUTF16Length(cur16, utf16_len) == 10 );
   cur16 = nextUTF16( cur16, code );
   TEST_ADD( res, (cur16 - utf16_str) == 1 );
   TEST_ADD( res, code == 0x0041 );
   cur16 = nextUTF16( cur16, code );
   TEST_ADD( res, (cur16 - utf16_str) == 2 );
   TEST_ADD( res, code == 0x007A );
   cur16 = nextUTF16( cur16, code );
   TEST_ADD( res, (cur16 - utf16_str) == 3 );
   TEST_ADD( res, code == 0x00B2 );
   cur16 = nextUTF16( cur16, code );
   TEST_ADD( res, (cur16 - utf16_str) == 4 );
   TEST_ADD( res, code == 0x00E9 );
   cur16 = nextUTF16( cur16, code );
   TEST_ADD( res, (cur16 - utf16_str) == 5 );
   TEST_ADD( res, code == 0x00E8 );
   cur16 = nextUTF16( cur16, code );
   TEST_ADD( res, (cur16 - utf16_str) == 6 );
   TEST_ADD( res, code == 0x00E0 );
   cur16 = nextUTF16( cur16, code );
   TEST_ADD( res, (cur16 - utf16_str) == 7 );
   TEST_ADD( res, code == 0x2022 );
   cur16 = nextUTF16( cur16, code );
   TEST_ADD( res, (cur16 - utf16_str) == 8 );
   TEST_ADD( res, code == 0x3394 );
   cur16 = nextUTF16( cur16, code );
   TEST_ADD( res, (cur16 - utf16_str) == 10 );
   TEST_ADD( res, code == 0x13080 );
   cur16 = nextUTF16( cur16, code );
   TEST_ADD( res, (cur16 - utf16_str) == 12 );
   TEST_ADD( res, code == 0x1F189 );

#if 1
   // Allocation size estimation.
   // Tests all of the patterns, as well as the speed, since we have 2M values.
   size_t oldSize = 0;
   for( char32_t c = 0; c < (1<<21); ++c )
   {
      size_t newSize = getUTF8Size( c );
      switch( c )
      {
         case 0x00000: // First 0-7b value.
            TEST_ADD( res, newSize == 1 );
            break;
         case 0x00080: // First 8-11b value.
            TEST_ADD( res, newSize == 2 );
            break;
         case 0x00800: // First 12-16b value.
            TEST_ADD( res, newSize == 3 );
            break;
         case 0x10000: // First 17-21b value.
            TEST_ADD( res, newSize == 4 );
            break;
         default:
            TEST_ADD( res, newSize == oldSize );
            break;
      }
      oldSize = newSize;
   }
#endif
   TEST_ADD( res, getUTF8Size(char32_t(0x80000000)) == 4 );

#if 1
   // Allocation size estimation.
   // Tests all of the patterns, as well as the speed, since we have 2M values.
   for( char32_t c = 0; c < (1<<16); ++c )
   {
      TEST_ADD( res, getUTF16Size( c ) == 1 );
   }
   for( char32_t c = (1<<16); c < (1<<21); ++c )
   {
      TEST_ADD( res, getUTF16Size( c ) == 2 );
   }
#endif
   TEST_ADD( res, getUTF16Size(char32_t(0x80000000)) == 2 );

   // Decoding.
#if 1
   // UTF-16 surrogate encoding breaks beyond 0x10FFFF (which isn't valid anyways).
   for( char32_t c = 0; c <= 0x10FFFF; ++c )
   {
      if( c == 0xD800 )
      {
         c = 0xDFFF; // Skip surrogate range.
         continue;
      }
      char     tmp32_8[4]  = { 0x1, 0x0, 0x0, 0x0 };
      char     tmp16_8[4]  = { 0x2, 0x0, 0x0, 0x0 };
      char16_t tmp32_16[2] = { 0x3, 0x0 };
      char16_t tmp8_16[2]  = { 0x4, 0x0 };
      char32_t tmp8_32     = c + 1;
      char32_t tmp16_32    = c + 2;

      // UTF-32 to UTF-8, and vice versa.
      toUTF8( c, tmp32_8 );
      tmp8_32 = toUTF32( tmp32_8 );
      TEST_ADD( res, tmp8_32 == c );

      // UTF-32 to UTF-16, and vice versa.
      toUTF16( c, tmp32_16 );
      tmp16_32 = toUTF32( tmp32_16 );
      TEST_ADD( res, tmp16_32 == c );

      // UTF-8 to UTF-16, and vice versa.
      toUTF8( tmp32_16, tmp16_8 );
      TEST_ADD( res, memcmp( tmp16_8, tmp32_8, 4*sizeof(char) ) == 0 );
      toUTF16( tmp32_8, tmp8_16 );
      TEST_ADD( res, memcmp( tmp8_16, tmp32_16, 2*sizeof(char16_t) ) == 0 );
   }
#endif

   // Test UTF-16 BE and LE variants.
   char* bytes = new char[utf16_len*2];
   // 1. UTF-16LE.
   for( size_t i = 0; i < utf16_len; ++i )
   {
      uint16_t u16 = utf16_str[i];
      bytes[2*i  ] = (u16 & 0xFF);
      u16 >>= 8;
      bytes[2*i+1] = (u16 & 0xFF);
   }
   cur8 = bytes;
   TEST_ADD( res, getUTF16LELength(cur8, utf16_len*2) == 10 );
   cur8 = nextUTF16LE( cur8, code );
   TEST_ADD( res, (cur8 - bytes) == 1*2 );
   TEST_ADD( res, code == 0x0041 );
   cur8 = nextUTF16LE( cur8, code );
   TEST_ADD( res, (cur8 - bytes) == 2*2 );
   TEST_ADD( res, code == 0x007A );
   cur8 = nextUTF16LE( cur8, code );
   TEST_ADD( res, (cur8 - bytes) == 3*2 );
   TEST_ADD( res, code == 0x00B2 );
   cur8 = nextUTF16LE( cur8, code );
   TEST_ADD( res, (cur8 - bytes) == 4*2 );
   TEST_ADD( res, code == 0x00E9 );
   cur8 = nextUTF16LE( cur8, code );
   TEST_ADD( res, (cur8 - bytes) == 5*2 );
   TEST_ADD( res, code == 0x00E8 );
   cur8 = nextUTF16LE( cur8, code );
   TEST_ADD( res, (cur8 - bytes) == 6*2 );
   TEST_ADD( res, code == 0x00E0 );
   cur8 = nextUTF16LE( cur8, code );
   TEST_ADD( res, (cur8 - bytes) == 7*2 );
   TEST_ADD( res, code == 0x2022 );
   cur8 = nextUTF16LE( cur8, code );
   TEST_ADD( res, (cur8 - bytes) == 8*2 );
   TEST_ADD( res, code == 0x3394 );
   cur8 = nextUTF16LE( cur8, code );
   TEST_ADD( res, (cur8 - bytes) == 10*2 );
   TEST_ADD( res, code == 0x13080 );
   cur8 = nextUTF16LE( cur8, code );
   TEST_ADD( res, (cur8 - bytes) == 12*2 );
   TEST_ADD( res, code == 0x1F189 );

   // 2. UTF-16BE.
   for( size_t i = 0; i < utf16_len; ++i )
   {
      uint16_t u16 = utf16_str[i];
      bytes[2*i+1] = (u16 & 0xFF);
      u16 >>= 8;
      bytes[2*i  ] = (u16 & 0xFF);
   }
   cur8 = bytes;
   TEST_ADD( res, getUTF16BELength(cur8, utf16_len*2) == 10 );
   cur8 = nextUTF16BE( cur8, code );
   TEST_ADD( res, (cur8 - bytes) == 1*2 );
   TEST_ADD( res, code == 0x0041 );
   cur8 = nextUTF16BE( cur8, code );
   TEST_ADD( res, (cur8 - bytes) == 2*2 );
   TEST_ADD( res, code == 0x007A );
   cur8 = nextUTF16BE( cur8, code );
   TEST_ADD( res, (cur8 - bytes) == 3*2 );
   TEST_ADD( res, code == 0x00B2 );
   cur8 = nextUTF16BE( cur8, code );
   TEST_ADD( res, (cur8 - bytes) == 4*2 );
   TEST_ADD( res, code == 0x00E9 );
   cur8 = nextUTF16BE( cur8, code );
   TEST_ADD( res, (cur8 - bytes) == 5*2 );
   TEST_ADD( res, code == 0x00E8 );
   cur8 = nextUTF16BE( cur8, code );
   TEST_ADD( res, (cur8 - bytes) == 6*2 );
   TEST_ADD( res, code == 0x00E0 );
   cur8 = nextUTF16BE( cur8, code );
   TEST_ADD( res, (cur8 - bytes) == 7*2 );
   TEST_ADD( res, code == 0x2022 );
   cur8 = nextUTF16BE( cur8, code );
   TEST_ADD( res, (cur8 - bytes) == 8*2 );
   TEST_ADD( res, code == 0x3394 );
   cur8 = nextUTF16BE( cur8, code );
   TEST_ADD( res, (cur8 - bytes) == 10*2 );
   TEST_ADD( res, code == 0x13080 );
   cur8 = nextUTF16BE( cur8, code );
   TEST_ADD( res, (cur8 - bytes) == 12*2 );
   TEST_ADD( res, code == 0x1F189 );

   delete [] bytes;

   // BOM detection.
   const char bom[] = {
      char(0xEF), char(0xBB), char(0xBF),        'a',                                                 // UTF-8   : BOM + 'a'
      char(0xFE), char(0xFF), char(0x00),        'b',                                                 // UTF-16BE: BOM + 'b'
      char(0xFF), char(0xFE),        'c', char(0x00),                                                 // UTF-16LE: BOM + 'c'
      char(0x00), char(0x00), char(0xFE), char(0xFF), char(0x00), char(0x00), char(0x00),        'd', // UTF-32BE: BOM + 'd'
      char(0xFF), char(0xFE), char(0x00), char(0x00),        'e', char(0x00), char(0x00), char(0x00), // UTF-32LE: BOM + 'd'
   };
   UTF utf;
   cur8 = readBOM( bom, utf );
   TEST_ADD( res, utf == UTF8 );
   TEST_ADD( res, (cur8 - bom) == 3 );
   cur8 = nextUTF( cur8, utf, code );
   TEST_ADD( res, (cur8 - bom) == 4 );
   TEST_ADD( res, code == 'a' );
   cur8 = readBOM( cur8, utf );
   TEST_ADD( res, utf == UTF16BE );
   TEST_ADD( res, (cur8 - bom) == 6 );
   cur8 = nextUTF( cur8, utf, code );
   TEST_ADD( res, (cur8 - bom) == 8 );
   TEST_ADD( res, code == 'b' );
   cur8 = readBOM( cur8, utf );
   TEST_ADD( res, utf == UTF16LE );
   TEST_ADD( res, (cur8 - bom) == 10 );
   cur8 = nextUTF( cur8, utf, code );
   TEST_ADD( res, (cur8 - bom) == 12 );
   TEST_ADD( res, code == 'c' );
   cur8 = readBOM( cur8, utf );
   TEST_ADD( res, utf == UTF32BE );
   TEST_ADD( res, (cur8 - bom) == 16 );
   cur8 = nextUTF( cur8, utf, code );
   TEST_ADD( res, (cur8 - bom) == 20 );
   TEST_ADD( res, code == 'd' );
   cur8 = readBOM( cur8, utf );
   TEST_ADD( res, utf == UTF32LE );
   TEST_ADD( res, (cur8 - bom) == 24 );
   cur8 = nextUTF( cur8, utf, code );
   TEST_ADD( res, (cur8 - bom) == 28 );
   TEST_ADD( res, code == 'e' );

   // Fast size and skip routines.
   cur8 = utf8_str;
   TEST_ADD( res, getUTF8Size(cur8) == 1 );
   cur8 = nextUTF8( cur8 );
   TEST_ADD( res, (cur8 - utf8_str) == 1 );
   TEST_ADD( res, getUTF8Size(cur8) == 1 );
   cur8 = nextUTF8( cur8 );
   TEST_ADD( res, (cur8 - utf8_str) == 2 );
   TEST_ADD( res, getUTF8Size(cur8) == 2 );
   cur8 = nextUTF8( cur8 );
   TEST_ADD( res, (cur8 - utf8_str) == 4 );
   TEST_ADD( res, getUTF8Size(cur8) == 2 );
   cur8 = nextUTF8( cur8 );
   TEST_ADD( res, (cur8 - utf8_str) == 6 );
   TEST_ADD( res, getUTF8Size(cur8) == 2 );
   cur8 = nextUTF8( cur8 );
   TEST_ADD( res, (cur8 - utf8_str) == 8 );
   TEST_ADD( res, getUTF8Size(cur8) == 2 );
   cur8 = nextUTF8( cur8 );
   TEST_ADD( res, (cur8 - utf8_str) == 10 );
   TEST_ADD( res, getUTF8Size(cur8) == 3 );
   cur8 = nextUTF8( cur8 );
   TEST_ADD( res, (cur8 - utf8_str) == 13 );
   TEST_ADD( res, getUTF8Size(cur8) == 3 );
   cur8 = nextUTF8( cur8 );
   TEST_ADD( res, (cur8 - utf8_str) == 16 );
   TEST_ADD( res, getUTF8Size(cur8) == 4 );
   cur8 = nextUTF8( cur8 );
   TEST_ADD( res, (cur8 - utf8_str) == 20 );
   TEST_ADD( res, getUTF8Size(cur8) == 4 );
   cur8 = nextUTF8( cur8 );
   TEST_ADD( res, (cur8 - utf8_str) == 24 );

   cur8 = skipUTF8( utf8_str, 9 );
   TEST_ADD( res, (cur8 - utf8_str) == 20 );


   // Test iterator.
   UTF8Iterator it8 = UTF8Iterator( utf8_str );
   TEST_ADD( res, *it8 == 0x0041 );
   ++it8;
   TEST_ADD( res, *it8 == 0x007A );
   ++it8;
   TEST_ADD( res, *it8 == 0x00B2 );
   ++it8;
   TEST_ADD( res, *it8 == 0x00E9 );
   ++it8;
   TEST_ADD( res, *it8 == 0x00E8 );
   ++it8;
   TEST_ADD( res, *it8 == 0x00E0 );
   ++it8;
   TEST_ADD( res, *it8 == 0x2022 );
   ++it8;
   TEST_ADD( res, *it8 == 0x3394 );
   ++it8;
   TEST_ADD( res, *it8 == 0x13080 );
   ++it8;
   TEST_ADD( res, *it8 == 0x1F189 );
   --it8;
   TEST_ADD( res, *it8 == 0x13080 );
   it8.cur( utf8_str + utf8_len ); // Past end-of-allocation.
   it8 -= 10;
   TEST_ADD( res, *it8 == 0x0041 );
   it8 += 9;
   TEST_ADD( res, *it8 == 0x1F189 );
   UTF8Iterator it8b = it8 - 9;
   TEST_ADD( res, (it8.cur() - it8b.cur()) == utf8_len-4 );
   TEST_ADD( res,  (it8b <  it8) );
   TEST_ADD( res,  (it8b <= it8) );
   TEST_ADD( res, !(it8b >  it8) );
   TEST_ADD( res, !(it8b >= it8) );
   TEST_ADD( res, !(it8b == it8) );
   TEST_ADD( res,  (it8b != it8) );
   TEST_ADD( res, (it8 - it8b) ==  9 );
   TEST_ADD( res, (it8b - it8) == -9 );
   TEST_ADD( res, (it8.cur() - it8b.cur()) ==  ptrdiff_t(utf8_len-4) );
   TEST_ADD( res, (it8b.cur() - it8.cur()) == -ptrdiff_t(utf8_len-4) );
   it8 += -9;
   TEST_ADD( res, (it8 == it8b) );

   const char* ascii = "abc";
   it8 = ascii;
   TEST_ADD( res, it8() && (*it8) == 'a' );
   ++it8;
   TEST_ADD( res, it8() && (*it8) == 'b' );
   ++it8;
   TEST_ADD( res, it8() && (*it8) == 'c' );
   ++it8;
   TEST_ADD( res, !it8() );
   ++it8;
}

void info_compiler( Test::Result& )
{
   printf("\n");
   printf("COMPILER: ");
   switch( COMPILER )
   {
      case COMPILER_UNKNOWN: printf("(\?\?\?)"); break;
      case COMPILER_GCC    : printf("GCC");  break;
      case COMPILER_MSVC   : printf("MSVC"); break;
      default              : printf("(please update the test)"); break;
   }
   printf("\n");
   printf("\n");
}

void info_cpu( Test::Result& )
{
   const char* archToStr[] = {
      "Unknown",
      "x86",
      "PPC",
      "ARM"
   };
   const char* endiToStr[] = {
      "Unknown",
      "Little (like x86)",
      "Big (like ppc)"
   };

   printf("\n");
   printf("CPU_ARCH = %s\n", archToStr[CPU_ARCH % 3]);
   printf("CPU_ENDI = %s\n", endiToStr[CPU_ENDIANNESS % 3]);
   printf("CPU_SIZE = %d\n", CPU_SIZE);
   printf("\n");
}

void info_sizes( Test::Result& )
{
   printf("\n");
   printf("NUMBER TYPES (signed) (unsigned)\n");
   printf("------------\n");
   printf("%10s: %2d %2d\n", "char"     , (int)sizeof(char     ), (int)sizeof(unsigned char     ));
   printf("(%9s: -- %2d)\n", "uchar"    ,                         (int)sizeof(uchar             ));
   printf("%10s: %2d %2d\n", "short"    , (int)sizeof(short    ), (int)sizeof(unsigned short    ));
   printf("(%9s: -- %2d)\n", "ushort"   ,                         (int)sizeof(ushort            ));
   printf("%10s: %2d %2d\n", "int"      , (int)sizeof(int      ), (int)sizeof(unsigned int      ));
   printf("(%9s: -- %2d)\n", "uint"     ,                         (int)sizeof(uint              ));
   printf("%10s: %2d %2d\n", "long"     , (int)sizeof(long     ), (int)sizeof(unsigned long     ));
   printf("%10s: %2d %2d\n", "long long", (int)sizeof(long long), (int)sizeof(unsigned long long));
   printf("\n");
   printf("STDINT TYPES (signed) (unsigned)\n");
   printf("------------\n");
   printf("%10s: %2d %2d\n", "int8_t" , (int)sizeof(int8_t),  (int)sizeof(uint8_t ));
   printf("%10s: %2d %2d\n", "int16_t", (int)sizeof(int16_t), (int)sizeof(uint16_t));
   printf("%10s: %2d %2d\n", "int32_t", (int)sizeof(int32_t), (int)sizeof(uint32_t));
   printf("%10s: %2d %2d\n", "int64_t", (int)sizeof(int64_t), (int)sizeof(uint64_t));
   printf("\n");
   printf("OTHER TYPES\n");
   printf("------------\n");
   printf("%10s: %2d\n", "void*"    , (int)sizeof(void*    ));
   printf("%10s: %2d\n", "size_t"   , (int)sizeof(size_t   ));
   printf("%10s: %2d\n", "ptrdiff_t", (int)sizeof(ptrdiff_t));
   printf("\n");
   printf("BASE TYPES\n");
   printf("\n");
   printf("Note: All sizes are in bytes (1=8b, 2=16b, 4=32b, 8=64b)\n");
   printf("\n");
}

struct Atomic: public RCObject {};
struct NonAtomic: public RCObjectNA {};

void util_perf_rcp( Test::Result& )
{
   printf("\n");
   Timer timer;

   const uint n = (1 << 25);
   const uint arraySize = 128;
   const uint m = (n / arraySize);
   double tn, ta;  // Total time, non-atomic and atomic.
   double rn, ra;  // Rate, non-atomic and atomic.

   RCP<NonAtomic>  rcp;// = new NonAtomic();
   RCP<Atomic>    arcp;// = new Atomic();

   RCP<NonAtomic> rcps[2];
   rcps[0] = new NonAtomic();
   rcps[1] = new NonAtomic();

   RCP<Atomic> arcps[2];
   arcps[0] = new Atomic();
   arcps[1] = new Atomic();

   RCP<NonAtomic>  rcpArray[arraySize];
   RCP<Atomic>    arcpArray[arraySize];
   for( uint i = 0; i < arraySize; ++i )
   {
      rcpArray[i] = new NonAtomic();
      arcpArray[i] = new Atomic();
   }

   printf("--------------------------------------\n");
   printf("Looping: single n=%d\n", n);
   // Testing non-atomic counters.
   timer.restart();
   for( uint i = 0; i < n; ++i )
   {
      rcp = rcps[i&1];
   }
   tn = timer.elapsed();
   // Testing atomic counters.
   timer.restart();
   for( uint i = 0; i < n; ++i )
   {
      arcp = arcps[i&1];
   }
   ta = timer.elapsed();
   rn = n/tn;
   ra = n/ta;
   printf("Non-atomic: %g/s\n", rn);
   printf("    Atomic: %g/s\n", ra);
   printf("     Ratio: %g (%g)\n", ra/rn, rn/ra);

   printf("--------------------------------------\n");
   printf("Looping: array=%d m=%d = n=%d\n", arraySize, m, n);
   // Testing non-atomic counters.
   timer.restart();
   for( uint j = 0; j < arraySize; ++j )
   {
      RCP<NonAtomic>& rcp = rcpArray[j];
      for( uint i = 0; i < m; ++i )
      {
         rcp = rcps[i&1];
      }
   }
   tn = timer.elapsed();
   // Testing atomic counters.
   timer.restart();
   for( uint j = 0; j < arraySize; ++j )
   {
      RCP<Atomic>& arcp = arcpArray[j];
      for( uint i = 0; i < m; ++i )
      {
         arcp = arcps[i&1];
      }
   }
   ta = timer.elapsed();
   rn = n/tn;
   ra = n/ta;
   printf("Non-atomic: %g/s\n", rn);
   printf("    Atomic: %g/s\n", ra);
   printf("     Ratio: %g (%g)\n", ra/rn, rn/ra);

   printf("--------------------------------------\n");
   printf("Looping: m=%d array=%d = n=%d\n", m, arraySize, n);
   // Testing non-atomic counters.
   timer.restart();
   for( uint i = 0; i < m; ++i )
   {
      for( uint j = 0; j < arraySize; ++j )
      {
         RCP<NonAtomic>& rcp = rcpArray[j];
         rcp = rcps[i&1];
      }
   }
   tn = timer.elapsed();
   // Testing atomic counters.
   timer.restart();
   for( uint i = 0; i < m; ++i )
   {
      for( uint j = 0; j < arraySize; ++j )
      {
         RCP<Atomic>& arcp = arcpArray[j];
         arcp = arcps[i&1];
      }
   }
   ta = timer.elapsed();
   rn = n/tn;
   ra = n/ta;
   printf("Non-atomic: %g/s\n", rn);
   printf("    Atomic: %g/s\n", ra);
   printf("     Ratio: %g (%g)\n", ra/rn, rn/ra);

   printf("\n");
   printf("\n");
}

void init_util()
{
   RCP<Test::Collection> col = new Test::Collection( "util", "Collection for Base/Util" );
   col->add( new Test::Function("args"          , "Tests Arguments class"                    , util_args           ) );
   col->add( new Test::Function("array_adaptor" , "Tests ArrayAdaptor class"                 , util_array_adaptor  ) );
   col->add( new Test::Function("bits"          , "Tests bit manipulation routines"          , util_bits           ) );
   col->add( new Test::Function("bits_float"    , "Tests bit manipulation routines on floats", util_bits_float     ) );
   col->add( new Test::Function("bits_round"    , "Tests rounding routines"                  , util_bits_round     ) );
   col->add( new Test::Function("date"          , "Tests date manipulation routines"         , util_date           ) );
   col->add( new Test::Function("endian_swapper", "Tests endian swapping routines"           , util_endian_swapper ) );
   col->add( new Test::Function("formatter"     , "Tests formatting routines"                , util_formatter      ) );
   col->add( new Test::Function("half"          , "Tests half-precision floating-point class", util_half           ) );
   //col->add( new Test::Function("half_consistency", "Tests that all 2^16 bit patterns are consistent (some compilers have issues with sNaNs)", util_half_consistency) );
   col->add( new Test::Function("idpool"        , "Tests IDPool class"                       , util_idpool         ) );
   col->add( new Test::Function("memory"        , "Tests memory copy routines"               , util_memory         ) );
   col->add( new Test::Function("packed"        , "Tests packed classes"                     , util_packed         ) );
   col->add( new Test::Function("radixsort"     , "Tests radixsort routines"                 , util_radixsort      ) );
   col->add( new Test::Function("sha1"          , "Tests the SHA1 routines"                  , util_sha1           ) );
   col->add( new Test::Function("time"          , "Tests time manipulation routines"         , util_time           ) );
   col->add( new Test::Function("timer"         , "Tests Timer class"                        , util_timer          ) );
   col->add( new Test::Function("unicode"       , "Tests Unicode routines"                   , util_unicode        ) );
   Test::standard().add( col.ptr() );

   RCP<Test::Collection> col_infos = new Test::Collection( "infos", "Collection of all of the info tests" );
   col_infos->add( new Test::Function( "info_cpu"     , "Prints out information about the CPU architecture being compiled for", info_cpu      ) );
   col_infos->add( new Test::Function( "info_compiler", "Prints information about the compiler used"                          , info_compiler ) );
   col_infos->add( new Test::Function( "info_sizes"   , "Prints out the size of various basic types"                          , info_sizes    ) );
   Test::special().add( col_infos.ptr() );

   Test::special().add( new Test::Function("application"     , "Checks the arguments of a fake Application instance"    , util_application) );
   Test::special().add( new Test::Function("date_show"       , "Shows a few dates"                                      , util_date_show) );
   Test::special().add( new Test::Function("half_consistency", "Checks that all 2^16 bit patterns are consistent (some compilers have issues with sNaNs)", util_half_consistency) );
   Test::special().add( new Test::Function("perf_rcp"        , "Compares performance of atomic and non-atomic RCObjects", util_perf_rcp) );
   Test::special().add( new Test::Function("sha1file"        , "Computes the SHA-1 digest of the file pointed by TEST_FILE", util_sha1_file) );
}
