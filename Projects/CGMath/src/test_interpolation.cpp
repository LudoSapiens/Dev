/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#include <Base/Dbg/UnitTest.h>

#include <CGMath/AABBox.h>
#include <CGMath/AARect.h>
#include <CGMath/BIH.h>
#include <CGMath/Frustum.h>

USING_NAMESPACE

//DBG_STREAM( os_test, "Test" );

const char* sAnswerBezier3[] = {
   "                              1                                                 ",
   "                                                                                ",
   "                                                                                ",
   "                                                                                ",
   "                                                                                ",
   "                                                                                ",
   "                                                                                ",
   "                                                                                ",
   "                                                                                ",
   "                                   *                                            ",
   "                       **************************                               ",
   "                  *****                         *******                         ",
   "               ***                                    *****                     ",
   "           ****                                           *****                 ",
   "         ***                                                   ****             ",
   "       **                                                         ****          ",
   "     **                                                              ***        ",
   "   **                                                                   ***     ",
   " **                                                                        ***  ",
   "0                                                                             *2"
};

const char* sAnswerBezier4[] = {
   "                              1                                                2",
   "                                                                                ",
   "                                                                                ",
   "                                                                                ",
   "                                                                                ",
   "                                        **********************                  ",
   "                                 *******                     ******             ",
   "                            *****                                 ****          ",
   "                         ***                                         ***        ",
   "                      ***                                              ***      ",
   "                  ****                                                   **     ",
   "               ****                                                        **   ",
   "             **                                                             **  ",
   "          ***                                                                *  ",
   "         **                                                                   * ",
   "       **                                                                     * ",
   "    ***                                                                       **",
   "   **                                                                          *",
   " **                                                                            *",
   "0*                                                                             3"
};

const char* sAnswerCatmullRom[] = {
   "                                                                               3",
   "                                                                                ",
   "                                                                                ",
   "                                                                                ",
   "                    1***                                                        ",
   "                        ****                                                    ",
   "                           ***                                                  ",
   "                             ***                                                ",
   "                                **                                              ",
   "                                  **                                            ",
   "                                    ***                                         ",
   "                                      **                                        ",
   "                                        **                                      ",
   "                                          **                                    ",
   "                                            **                                  ",
   "                                              **                                ",
   "                                                ***                             ",
   "                                                  ***                           ",
   "                                                     ****                       ",
   "0                                                       ****2                   "
};

const char* sAnswerHermite[] = {
   "                                                                                ",
   "                                                                                ",
   "                                                                                ",
   "                                                                                ",
   "                                                                                ",
   "                                        **********************                  ",
   "                                 *******                     ******             ",
   "                            *****                                 ****          ",
   "                         ***                                         ***        ",
   "                      ***                                              ***      ",
   "                  ****                                                   **     ",
   "               ****                                                        **   ",
   "             **                                                             **  ",
   "          ***                                                                *  ",
   "         **                                                                   * ",
   "       **                                                                     * ",
   "    ***                                                                       **",
   "   **                                                                          *",
   " **                                                                            *",
   "0*                                                                             1"
};

//------------------------------------------------------------------------------
//!
void cgmath_interpolation( Test::Result& res )
{
   Vec4f v1( 1,  2,  3,  4);
   Vec4f v2( 5, 10, 19, 36); //v1 + (4, 8, 16, 32)
   Vec4f dv = v2 - v1;
   Vec4f v;

   // Linear interpolation.
   v = CGM::linear( v1, dv, 0.0f/4.0f );
   TEST_ADD( res, v.equal( v1 ) );

   v = CGM::linear( v1, dv, 1.0f/4.0f );
   TEST_ADD( res, v.equal( Vec4f( 2, 4, 7, 12 ) ) );

   v = CGM::linear( v1, dv, 2.0f/4.0f );
   TEST_ADD( res, v.equal( Vec4f( 3, 6, 11, 20 ) ) );

   v = CGM::linear( v1, dv, 3.0f/4.0f );
   TEST_ADD( res, v.equal( Vec4f( 4, 8, 15, 28 ) ) );

   v = CGM::linear( v1, dv, 4.0f/4.0f );
   TEST_ADD( res, v.equal( v2 ) );
}

//------------------------------------------------------------------------------
//!
void cgmath_smoothstep( Test::Result& res )
{
   Vec4f v1( 1,  2,  3,  4);
   Vec4f v2( 5, 10, 19, 36); // v1+(4, 8, 16, 32)
   Vec4f v;

   // C1 (classic).
   v = CGM::smoothStep( v1, v2, Vec4f(0.0f, 0.0f, 0.0f, 0.0f) );
   TEST_ADD( res, v.equal( Vec4f(0.0f) ) );

   v = CGM::smoothStep( v1, v2, Vec4f(1.0f, 2.0f, 3.0f, 4.0f) );
   TEST_ADD( res, v.equal( Vec4f(0.0f) ) );

   v = CGM::smoothStep( v1, v2, Vec4f(2.0f, 4.0f, 7.0f, 12.0f) );
   TEST_ADD( res, v.equal( Vec4f(0.15625f) ) );

   v = CGM::smoothStep( v1, v2, Vec4f(3.0f, 6.0f, 11.0f, 20.0f) );
   TEST_ADD( res, v.equal( Vec4f(0.5f) ) );

   v = CGM::smoothStep( v1, v2, Vec4f(4.0f, 8.0f, 15.0f, 28.0f) );
   TEST_ADD( res, v.equal( Vec4f(0.84375f) ) );

   v = CGM::smoothStep( v1, v2, Vec4f(5.0f, 10.0f, 19.0f, 36.0f) );
   TEST_ADD( res, v.equal( Vec4f(1.0f) ) );

   v = CGM::smoothStep( v1, v2, Vec4f(100.0f, 100.0f, 100.0f, 100.0f) );
   TEST_ADD( res, v.equal( Vec4f(1.0f) ) );

   // C2 (classic).
   v = CGM::smoothStepC2( v1, v2, Vec4f(0.0f, 0.0f, 0.0f, 0.0f) );
   TEST_ADD( res, v.equal( Vec4f(0.0f) ) );

   v = CGM::smoothStepC2( v1, v2, Vec4f(1.0f, 2.0f, 3.0f, 4.0f) );
   TEST_ADD( res, v.equal( Vec4f(0.0f) ) );

   v = CGM::smoothStepC2( v1, v2, Vec4f(2.0f, 4.0f, 7.0f, 12.0f) );
   TEST_ADD( res, v.equal( Vec4f(0.103516f) ) );

   v = CGM::smoothStepC2( v1, v2, Vec4f(3.0f, 6.0f, 11.0f, 20.0f) );
   TEST_ADD( res, v.equal( Vec4f(0.5f) ) );

   v = CGM::smoothStepC2( v1, v2, Vec4f(4.0f, 8.0f, 15.0f, 28.0f) );
   TEST_ADD( res, v.equal( Vec4f(0.896484f) ) );

   v = CGM::smoothStepC2( v1, v2, Vec4f(5.0f, 10.0f, 19.0f, 36.0f) );
   TEST_ADD( res, v.equal( Vec4f(1.0f) ) );

   v = CGM::smoothStepC2( v1, v2, Vec4f(100.0f, 100.0f, 100.0f, 100.0f) );
   TEST_ADD( res, v.equal( Vec4f(1.0f) ) );
}

#define CLEAR_PAD() \
   for( uint y = 0; y < h; ++y ) \
   { \
      for( uint x = 0; x < w; ++x ) \
      { \
         pad[y][x] = ' '; \
      } \
      pad[y][w] = '\0'; \
   }

#define SET(p_expr, c) \
   do \
   { \
      Vec2f _p = p_expr; \
      if( 0 <= _p.x && _p.x < (float)w && 0 <= _p.y && _p.y < (float)h ) \
      { \
         uint x = (uint)(_p.x + 0.5f); \
         uint y = (uint)(_p.y + 0.5f); \
         pad[y][x] = c; \
      } \
   } while( 0 )

#define PRINT_PAD() \
   for( uint y = 0; y < h; ++y ) \
   { \
      StdErr << pad[h-y-1] << Endl; \
   }

inline bool checkAnswer( const char candidate[][81], const char* answer[], const uint nLines )
{
   bool ok = true;
   for( uint i = 0; i < nLines; ++i )
   {
      ok &= (strcmp(candidate[i], answer[nLines - i - 1]) == 0);
   }
   return ok;
}

//------------------------------------------------------------------------------
//!
void cgmath_bezier( Test::Result& res )
{
   const uint w = 80;
   const uint h = 20;
   char pad[h][w+1]; // Ascii drawing result.
   Vec2f p0, p1, p2, p3;


   CLEAR_PAD();
   p0 = Vec2f( 0.0f,  0.0f);
   p1 = Vec2f(30.0f, 19.0f);
   p2 = Vec2f(79.0f,  0.0f);
   for( float t = 0.0f; t < 1.0f; t += 1.0f/128.0f )
   {
      Vec2f p = CGM::evalBezier(p0, p1, p2, t);
      SET( p, '*' );
   }
   SET( p0, '0' );
   SET( p1, '1' );
   SET( p2, '2' );

   TEST_ADD( res, checkAnswer(pad, sAnswerBezier3, h) );

   //StdErr << "RESULT" << Endl;
   //PRINT_PAD();


   CLEAR_PAD();
   p0 = Vec2f( 0.0f,  0.0f);
   p1 = Vec2f(30.0f, 19.0f);
   p2 = Vec2f(79.0f, 19.0f);
   p3 = Vec2f(79.0f,  0.0f);
   for( float t = 0.0f; t < 1.0f; t += 1.0f/128.0f )
   {
      Vec2f p = CGM::evalBezier(p0, p1, p2, p3, t);
      SET( p, '*' );
   }
   SET( p0, '0' );
   SET( p1, '1' );
   SET( p2, '2' );
   SET( p3, '3' );

   TEST_ADD( res, checkAnswer(pad, sAnswerBezier4, h) );

   //StdErr << "RESULT" << Endl;
   //PRINT_PAD();
}

//------------------------------------------------------------------------------
//!
void cgmath_catmull_rom( Test::Result& res )
{
   const uint w = 80;
   const uint h = 20;
   char pad[h][w+1]; // Ascii drawing result.
   Vec2f p0, p1, p2, p3;

   CLEAR_PAD();
   p0 = Vec2f( 0.0f,  0.0f);
   p1 = Vec2f(20.0f, 15.0f);
   p2 = Vec2f(60.0f,  0.0f);
   p3 = Vec2f(79.0f, 19.0f);
   for( float t = 0.0f; t < 1.0f; t += 1.0f/128.0f )
   {
      Vec2f p = CGM::evalCatmullRom(p0, p1, p2, p3, t);
      SET( p, '*' );
   }
   SET( p0, '0' );
   SET( p1, '1' );
   SET( p2, '2' );
   SET( p3, '3' );

   TEST_ADD( res, checkAnswer(pad, sAnswerCatmullRom, h) );

   //StdErr << "RESULT" << Endl;
   //PRINT_PAD();
}

//------------------------------------------------------------------------------
//!
void cgmath_hermite( Test::Result& res )
{
   const uint w = 80;
   const uint h = 20;
   char pad[h][w+1]; // Ascii drawing result.
   Vec2f p0, m0, p1, m1;

   CLEAR_PAD();
   p0 = Vec2f( 0.0f,  0.0f); // bezier's p0
   m0 = (Vec2f(30.0f, 19.0f) - p0) * 3.0f;
   p1 = Vec2f(79.0f,  0.0f); // bezier's p3
   m1 = (Vec2f(79.0f, 19.0f) - p1) * -3.0f;
   for( float t = 0.0f; t < 1.0f; t += 1.0f/128.0f )
   {
      Vec2f p = CGM::evalHermite(p0, m0, p1, m1, t);
      SET( p, '*' );
   }
   SET( p0,    '0' );
   SET( p0+m0, 'a' );
   SET( p1,    '1' );
   SET( p1+m1, 'b' );

   TEST_ADD( res, checkAnswer(pad, sAnswerHermite, h) );

   //StdErr << "RESULT" << Endl;
   //PRINT_PAD();
}

#undef SET
#undef CLEAR_PAD

//------------------------------------------------------------------------------
//!
void  init_interpolation()
{
   Test::Collection& std = Test::standard();
   std.add( new Test::Function( "interpolation", "Tests interpolation routines (linear, bilinear)", cgmath_interpolation ) );
   std.add( new Test::Function( "smoothstep",    "Tests smoothstep routines",                       cgmath_smoothstep    ) );
   std.add( new Test::Function( "bezier",        "Tests Bezier evaluation code",                    cgmath_bezier        ) );
   std.add( new Test::Function( "catmull_rom",   "Tests Catmull-Rom evaluation code",               cgmath_catmull_rom   ) );
   std.add( new Test::Function( "hermite",       "Tests Hermite evaluation code",                   cgmath_hermite       ) );
}
