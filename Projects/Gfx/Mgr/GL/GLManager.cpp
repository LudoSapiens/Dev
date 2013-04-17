/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#include <Gfx/Mgr/GL/GLManager.h>

#if GFX_OGL_SUPPORT

//Complete forward declarations
#include <Gfx/FB/RenderState.h>
#include <Gfx/Geom/Buffer.h>
#include <Gfx/Geom/Geometry.h>
#include <Gfx/Mgr/GL/GLContext.h>
#include <Gfx/Prog/Program.h>
#include <Gfx/Tex/Sampler.h>
#include <Gfx/Tex/Texture.h>
#include <Gfx/Tex/TextureState.h>

#include <CGMath/Mat4.h>

#include <Base/ADT/Map.h>
#include <Base/ADT/String.h>
#include <Base/Dbg/DebugStream.h>
#include <Base/Util/Bits.h>

#include <cstdio>

#define BUFFER_OFFSET(i) ((char*)NULL + (i))


//For details, see Apple's technote: http://developer.apple.com/technotes/tn2006/tn2085.html
#define USE_MULTITHREADED_OPENGL  0


/*==============================================================================
   GLOBAL NAMESPACE
==============================================================================*/

//--------------------------------------------
// OpenGL headers and initialization routines
//--------------------------------------------

typedef void(*voidFuncPtr)();

#if defined(__APPLE__)

#include <OpenGL/OpenGL.h>
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>

#elif GFX_OGL_GLX_SUPPORT

#include <GL/gl.h>
#include <GL/glu.h>
extern "C" void (*glXGetProcAddressARB(const GLubyte *procName))( void );
#define glGetProcAddress( str ) glXGetProcAddressARB( (const GLubyte*)str )

#elif GFX_OGL_WGL_SUPPORT

#include <Base/Util/windows.h>
#include <GL/gl.h>
#include <GL/glu.h>
#define glGetProcAddress wglGetProcAddress

#endif

//Need to specify APIENTRY for non-Windows platforms
#if !defined(APIENTRY)
#  if defined(__WIN32__)
#    define APIENTRY __stdcall
#  else
#    define APIENTRY
#  endif
#endif


USING_NAMESPACE
using namespace Gfx;

UNNAMESPACE_BEGIN

DBG_STREAM( os_gl, "GLManager" );

//Keep all of the initialization routines and extension pointers as private to this file only.
//Notes:
// - Prototypes were set using http://oss.sgi.com/projects/ogl-sample/ABI/glext.h from 2006-09-14
// - Used final names (i.e. without "ARB" extensions) where OpenGL <=2.1 supported it
// - Used "gl_" as prefix in order not to clash with official names (potentially included in glext.h)
// - The added typedefs are copied from glext.h (see above); they should not needlessly get redefined on platforms which already have them
// - BTW, glext.h should NOT be included (as many versions across many OSes exist)

//------------------------------------------------------------------------------
//!
uint sInitializeCount = 0;


//Extension-specific defines and typedefs
#ifndef GL_VERSION_1_2
#define GL_CONSTANT_COLOR            0x8001
#define GL_ONE_MINUS_CONSTANT_COLOR  0x8002
#define GL_TEXTURE_3D            0x806F
#define GL_TEXTURE_CUBE_MAP      0x8513
#define GL_TEXTURE_WRAP_R        0x8072
#define GL_CLAMP_TO_EDGE         0x812F
#define GL_TEXTURE_BASE_LEVEL    0x813C
#define GL_TEXTURE_MAX_LEVEL     0x813D
#endif // GL_VERSION_1_2

#ifndef GL_VERSION_1_3
#define GL_TEXTURE0              0x84c0
#define GL_CLAMP_TO_BORDER       0x812D
#define GL_TEXTURE_CUBE_MAP               0x8513
#define GL_TEXTURE_BINDING_CUBE_MAP       0x8514
#define GL_TEXTURE_CUBE_MAP_POSITIVE_X    0x8515
#define GL_TEXTURE_CUBE_MAP_NEGATIVE_X    0x8516
#define GL_TEXTURE_CUBE_MAP_POSITIVE_Y    0x8517
#define GL_TEXTURE_CUBE_MAP_NEGATIVE_Y    0x8518
#define GL_TEXTURE_CUBE_MAP_POSITIVE_Z    0x8519
#define GL_TEXTURE_CUBE_MAP_NEGATIVE_Z    0x851A
#endif // GL_VERSION_1_3

#ifndef GL_VERSION_1_4
#define GL_TEXTURE_LOD_BIAS      0x8501
#endif // GL_VERSION_1_4

#ifndef GL_VERSION_1_5
//#warning "Defining some missing types from 1.5"
typedef ptrdiff_t GLintptr;
typedef ptrdiff_t GLsizeiptr;
#define GL_ARRAY_BUFFER          0x8892
#define GL_ELEMENT_ARRAY_BUFFER  0x8893
#define GL_STREAM_DRAW           0x88E0
#define GL_STREAM_READ           0x88E1
#define GL_STREAM_COPY           0x88E2
#define GL_STATIC_DRAW           0x88E4
#define GL_STATIC_READ           0x88E5
#define GL_STATIC_COPY           0x88E6
#define GL_DYNAMIC_DRAW          0x88E8
#define GL_DYNAMIC_READ          0x88E9
#define GL_DYNAMIC_COPY          0x88EA
#define GL_READ_ONLY             0x88B8
#define GL_WRITE_ONLY            0x88B9
#define GL_READ_WRITE            0x88BA
#define GL_BUFFER_SIZE           0x8764
#define GL_BUFFER_USAGE          0x8765
#define GL_BUFFER_ACCESS         0x88BB
#define GL_BUFFER_MAPPED         0x88BC
#define GL_BUFFER_MAP_POINTER    0x88BD
#endif //GL_VERSION_1_5

#ifndef GL_VERSION_2_0
//#warning "Defining some missing types from 2.0"
typedef char GLchar;			/* native character */
#define GL_VERTEX_PROGRAM_POINT_SIZE      0x8642
#define GL_POINT_SPRITE_ARB               0x8861 // Deprecated enum.
#define GL_FRAGMENT_SHADER                0x8B30
#define GL_VERTEX_SHADER                  0x8B31
#define GL_FLOAT_VEC2                     0x8B50
#define GL_FLOAT_VEC3                     0x8B51
#define GL_FLOAT_VEC4                     0x8B52
#define GL_INT_VEC2                       0x8B53
#define GL_INT_VEC3                       0x8B54
#define GL_INT_VEC4                       0x8B55
#define GL_BOOL                           0x8B56
#define GL_BOOL_VEC2                      0x8B57
#define GL_BOOL_VEC3                      0x8B58
#define GL_BOOL_VEC4                      0x8B59
#define GL_FLOAT_MAT2                     0x8B5A
#define GL_FLOAT_MAT3                     0x8B5B
#define GL_FLOAT_MAT4                     0x8B5C
#define GL_SAMPLER_1D                     0x8B5D
#define GL_SAMPLER_2D                     0x8B5E
#define GL_SAMPLER_3D                     0x8B5F
#define GL_SAMPLER_CUBE                   0x8B60
#define GL_SAMPLER_1D_SHADOW              0x8B61
#define GL_SAMPLER_2D_SHADOW              0x8B62
#define GL_COMPILE_STATUS                 0x8B81
#define GL_LINK_STATUS                    0x8B82
#define GL_ACTIVE_UNIFORMS                0x8B86
#endif //GL_VERSION_2_0

#ifndef GL_VERSION_2_1
#define GL_FLOAT_MAT2x3                   0x8B65
#define GL_FLOAT_MAT2x4                   0x8B66
#define GL_FLOAT_MAT3x2                   0x8B67
#define GL_FLOAT_MAT3x4                   0x8B68
#define GL_FLOAT_MAT4x2                   0x8B69
#define GL_FLOAT_MAT4x3                   0x8B6A
#endif //GL_VERSION_2_1

#ifndef GL_VERSION_3_0
#define GL_TEXTURE_1D_ARRAY               0x8C18
#define GL_PROXY_TEXTURE_1D_ARRAY         0x8C19
#define GL_TEXTURE_2D_ARRAY               0x8C1A
#define GL_PROXY_TEXTURE_2D_ARRAY         0x8C1B
#endif //GL_VERSION_3_0


#ifndef GL_VERSION_3_2

// Geometry shaders were defined in, GL_EXT_geometry_shader4, then GL_ARB_geometry_shader4
// then got into core beginning 3.2.
// We use the non-EXT/ARB names here.
#define GL_GEOMETRY_SHADER           0x8DD9
#define GL_GEOMETRY_VERTICES_OUT     0x8DDA
#define GL_GEOMETRY_INPUT_TYPE       0x8DDB
#define GL_GEOMETRY_OUTPUT_TYPE      0x8DDC
#define GL_LINES_ADJACENCY           0xA
#define GL_LINE_STRIP_ADJACENCY      0xB
#define GL_TRIANGLES_ADJACENCY       0xC
#define GL_TRIANGLE_STRIP_ADJACENCY  0xD

#endif //GL_VERSION_3_2

#ifndef GL_TEXTURE_MAX_ANISOTROPY
#define GL_TEXTURE_MAX_ANISOTROPY         0x84FE
#endif

#ifndef GL_MIRRORED_REPEAT
#define GL_MIRRORED_REPEAT                0x8370
#endif

#ifndef GL_MIRROR_CLAMP
#define GL_MIRROR_CLAMP                   0x8742
#endif

#ifndef GL_MIRROR_CLAMP_TO_EDGE
#define GL_MIRROR_CLAMP_TO_EDGE           0x8743
#endif

#ifndef GL_MIRROR_CLAMP_TO_BORDER
#define GL_MIRROR_CLAMP_TO_BORDER         0x8912
#endif

#ifndef GL_INCR_WRAP
#define GL_INCR_WRAP                      0x8507
#define GL_DECR_WRAP                      0x8508
#endif

#ifndef GL_EXT_packed_depth_stencil
#define GL_DEPTH_STENCIL_EXT              0x84F9
#define GL_UNSIGNED_INT_24_8_EXT          0x84FA
#define GL_DEPTH24_STENCIL8_EXT           0x88F0
#define GL_TEXTURE_STENCIL_SIZE_EXT       0x88F1
#endif

#ifndef GL_ARB_shadow
#define GL_TEXTURE_COMPARE_MODE_ARB       0x884C
#define GL_TEXTURE_COMPARE_FUNC_ARB       0x884D
#define GL_COMPARE_R_TO_TEXTURE_ARB       0x884E
#endif

#ifndef GL_ARB_half_float_pixel
#define GL_HALF_FLOAT_ARB                 0x140B
#endif

#ifndef GL_ARB_color_buffer_float
#define GL_RGBA_FLOAT_MODE_ARB            0x8820
#define GL_CLAMP_VERTEX_COLOR_ARB         0x891A
#define GL_CLAMP_FRAGMENT_COLOR_ARB       0x891B
#define GL_CLAMP_READ_COLOR_ARB           0x891C
#define GL_FIXED_ONLY_ARB                 0x891D
#endif

// A small macro to try to write proc address assignments in a more concise fashion
#define GET_PROC_ADDR( ptr, nameStr, type ) \
   ptr = (type)glGetProcAddress( nameStr ); \
   CHECK(ptr)

//GL_ARB_texture_float
#ifndef GL_ARB_texture_float
#define GL_RGBA32F_ARB                      0x8814
#define GL_RGB32F_ARB                       0x8815
#define GL_LUMINANCE32F_ARB                 0x8818
#define GL_LUMINANCE_ALPHA32F_ARB           0x8819
#define GL_RGBA16F_ARB                      0x881A
#define GL_RGB16F_ARB                       0x881B
#define GL_INTENSITY16F_ARB                 0x881D
#define GL_LUMINANCE_ALPHA16F_ARB           0x881F
#endif

#ifndef GL_ARB_seamless_cube_map
#define GL_TEXTURE_CUBE_MAP_SEAMLESS      0x884F
#endif

/*==============================================================================
   VERTEX BUFFER OBJECTS
==============================================================================*/

#if __APPLE__

#define gl_bindBuffer glBindBuffer
#define gl_genBuffers glGenBuffers
#define gl_deleteBuffers glDeleteBuffers
#define gl_bufferData glBufferData
#define gl_bufferSubData glBufferSubData
#define gl_getBufferSubData glGetBufferSubData

#define gl_enableVertexAttribArray glEnableVertexAttribArray
#define gl_disableVertexAttribArray glDisableVertexAttribArray
#define gl_vertexAttribPointer glVertexAttribPointer

#define gl_clientActiveTexture glClientActiveTexture

#define gl_mapBuffer glMapBuffer
#define gl_unmapBuffer glUnmapBuffer

void initFast() {}

#else

void (APIENTRY* gl_bindBuffer)( GLenum, GLuint );
void (APIENTRY* gl_deleteBuffers)( GLsizei, const GLuint* );
void (APIENTRY* gl_genBuffers)( GLsizei, GLuint* );
void (APIENTRY* gl_bufferData)( GLenum, GLsizeiptr, const GLvoid*, GLenum );
void (APIENTRY* gl_bufferSubData)( GLenum, GLintptr, GLsizeiptr, const GLvoid* );
void (APIENTRY* gl_getBufferSubData)( GLenum, GLintptr, GLsizeiptr, GLvoid* );
void (APIENTRY* gl_enableVertexAttribArray)( GLuint );
void (APIENTRY* gl_disableVertexAttribArray)( GLuint );
void (APIENTRY* gl_vertexAttribPointer)( GLuint, GLint, GLenum, GLboolean, GLsizei, const GLvoid* );
void (APIENTRY* gl_clientActiveTexture)( GLenum );
void*     (APIENTRY* gl_mapBuffer  )( GLenum, GLenum );
GLboolean (APIENTRY* gl_unmapBuffer)( GLenum );

//------------------------------------------------------------------------------
//!
void initFast()
{
   gl_bindBuffer = (void (APIENTRY*)( GLenum, GLuint ))
      glGetProcAddress( "glBindBufferARB" );
   gl_genBuffers = (void (APIENTRY*)( GLsizei, GLuint* ))
      glGetProcAddress( "glGenBuffersARB" );
   gl_deleteBuffers = (void (APIENTRY*)( GLsizei, const GLuint* ))
      glGetProcAddress( "glDeleteBuffersARB" );
   gl_bufferData = (void (APIENTRY*)( GLenum, GLsizeiptr, const GLvoid*, GLenum ))
      glGetProcAddress( "glBufferDataARB" );
   gl_bufferSubData = (void (APIENTRY*)( GLenum, GLintptr, GLsizeiptr, const GLvoid* ))
      glGetProcAddress( "glBufferSubDataARB" );
   gl_getBufferSubData = (void (APIENTRY*)( GLenum, GLintptr, GLsizeiptr, GLvoid* ))
      glGetProcAddress( "glGetBufferSubDataARB" );

   gl_enableVertexAttribArray = (void (APIENTRY*)( GLuint ))
      glGetProcAddress( "glEnableVertexAttribArrayARB" );
   gl_disableVertexAttribArray = (void (APIENTRY*)( GLuint ))
      glGetProcAddress( "glDisableVertexAttribArrayARB" );
   gl_vertexAttribPointer = (void (APIENTRY*)( GLuint, GLint, GLenum, GLboolean, GLsizei, const GLvoid* ))
      glGetProcAddress( "glVertexAttribPointerARB" );

   gl_clientActiveTexture = (void (APIENTRY*)( GLenum ))
      glGetProcAddress( "glClientActiveTexture" );

   gl_mapBuffer = (void* (APIENTRY*)( GLenum, GLenum ))
      glGetProcAddress( "glMapBufferARB" );
   gl_unmapBuffer = (GLboolean (APIENTRY*)( GLenum ))
      glGetProcAddress( "glUnmapBufferARB" );
}
#endif

/*==============================================================================
   FRAMEBUFFER OBJECTS
==============================================================================*/

#if __APPLE__

#define gl_checkFramebufferStatusEXT glCheckFramebufferStatusEXT
#define gl_genFramebuffersEXT glGenFramebuffersEXT
#define gl_deleteFramebuffersEXT glDeleteFramebuffersEXT
#define gl_genRenderbuffersEXT glGenRenderbuffersEXT
#define gl_deleteRenderbuffersEXT glDeleteRenderbuffersEXT
#define gl_bindFramebufferEXT glBindFramebufferEXT
#define gl_framebufferTexture2DEXT glFramebufferTexture2DEXT
#define gl_framebufferRenderbufferEXT glFramebufferRenderbufferEXT
#define gl_generateMipmapEXT glGenerateMipmapEXT

void initFBOs() {}

#else

#define GL_FRAMEBUFFER_COMPLETE_EXT                        0x8CD5
#define GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT_EXT           0x8CD6
#define GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT_EXT   0x8CD7
#define GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS_EXT           0x8CD9
#define GL_FRAMEBUFFER_INCOMPLETE_FORMATS_EXT              0x8CDA
#define GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER_EXT          0x8CDB
#define GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER_EXT          0x8CDC
#define GL_FRAMEBUFFER_UNSUPPORTED_EXT                     0x8CDD
#define GL_COLOR_ATTACHMENT0_EXT           0x8CE0
#define GL_DEPTH_ATTACHMENT_EXT            0x8D00
#define GL_STENCIL_ATTACHMENT_EXT          0x8D20
#define GL_FRAMEBUFFER_EXT                 0x8D40
#define GL_RENDERBUFFER_EXT                0x8D41

GLenum (APIENTRY* gl_checkFramebufferStatusEXT)( GLenum target );
void (APIENTRY* gl_genFramebuffersEXT)( GLsizei n, GLuint *framebuffers );
void (APIENTRY* gl_deleteFramebuffersEXT)( GLsizei n, const GLuint *renderbuffers );
void (APIENTRY* gl_genRenderbuffersEXT)( GLsizei n, GLuint *renderbuffers );
void (APIENTRY* gl_deleteRenderbuffersEXT)( GLsizei n, const GLuint *renderbuffers );
void (APIENTRY* gl_bindFramebufferEXT)( GLenum target, GLuint framebuffer );
void (APIENTRY* gl_framebufferTexture2DEXT)( GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level );
void (APIENTRY* gl_framebufferRenderbufferEXT)( GLenum target, GLenum attachment, GLenum renderbuffertarget, GLuint renderbuffer );
void (APIENTRY* gl_generateMipmapEXT)( GLenum target );
//------------------------------------------------------------------------------
//!
void initFBOs()
{
   GET_PROC_ADDR( gl_checkFramebufferStatusEXT, "glCheckFramebufferStatusEXT",
                  GLenum (APIENTRY*)( GLenum target ) );
   GET_PROC_ADDR( gl_genFramebuffersEXT, "glGenFramebuffersEXT",
                  void (APIENTRY*)( GLsizei n, GLuint *framebuffers ) );
   GET_PROC_ADDR( gl_deleteFramebuffersEXT, "glDeleteFramebuffersEXT",
                  void (APIENTRY*)( GLsizei n, const GLuint *renderbuffers ) );
   GET_PROC_ADDR( gl_genRenderbuffersEXT, "glGenRenderbuffersEXT",
                  void (APIENTRY*)( GLsizei n, GLuint *framebuffers ) );
   GET_PROC_ADDR( gl_deleteRenderbuffersEXT, "glDeleteRenderbuffersEXT",
                  void (APIENTRY*)( GLsizei n, const GLuint *renderbuffers ) );
   GET_PROC_ADDR( gl_bindFramebufferEXT, "glBindFramebufferEXT",
                  void (APIENTRY*)(GLenum target, GLuint framebuffer) );
   GET_PROC_ADDR( gl_framebufferTexture2DEXT, "glFramebufferTexture2DEXT",
                  void (APIENTRY*)( GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level ) );
   GET_PROC_ADDR( gl_framebufferRenderbufferEXT, "glFramebufferRenderbufferEXT",
                  void (APIENTRY*)( GLenum target, GLenum attachment, GLenum renderbuffertarget, GLuint renderbuffer ) );
   GET_PROC_ADDR( gl_generateMipmapEXT, "glGenerateMipmapEXT",
                  void (APIENTRY*)( GLenum target ) );
}
#endif


/*==============================================================================
   VERTEX ARRAY OBJECTS
==============================================================================*/

#if __APPLE__

#define gl_bindVertexArray     glBindVertexArrayAPPLE
#define gl_deleteVertexArrays  glDeleteVertexArraysAPPLE
#define gl_genVertexArrays     glGenVertexArraysAPPLE

void initVAOs() {}

#else

void (APIENTRY* gl_bindVertexArray   )( GLuint array );
void (APIENTRY* gl_deleteVertexArrays)( GLsizei n, const GLuint* arrays );
void (APIENTRY* gl_genVertexArrays   )( GLsizei n, GLuint* arrays );
//GLboolean (APIENTRY* gl_isVertexArray)( GLuint array );

void initVAOs()
{
   GET_PROC_ADDR( gl_bindVertexArray, "glBindVertexArray",
                  void (APIENTRY*)( GLuint ) );
   GET_PROC_ADDR( gl_deleteVertexArrays, "glDeleteVertexArrays",
                  void (APIENTRY*)( GLsizei, const GLuint* ) );
   GET_PROC_ADDR( gl_genVertexArrays, "glGenVertexArrays",
                  void (APIENTRY*)( GLsizei, GLuint* ) );
}

#endif // VAO

/*==============================================================================
   SHADERS
==============================================================================*/

#if __APPLE__

#define gl_createShader glCreateShader
#define gl_deleteShader glDeleteShader
#define gl_compileShader glCompileShader
#define gl_shaderSource glShaderSource
#define gl_getShaderInfoLog glGetShaderInfoLog
#define gl_getProgramInfoLog glGetProgramInfoLog
#define gl_getShaderiv glGetShaderiv

#define gl_createProgram glCreateProgram
#define gl_deleteProgram glDeleteProgram
#define gl_attachShader glAttachShader
#define gl_detachShader glDetachShader
#define gl_linkProgram glLinkProgram
#define gl_useProgram glUseProgram
#define gl_getProgramiv glGetProgramiv
#define gl_programParameteri glProgramParameteriEXT

#define gl_getActiveUniform glGetActiveUniform
#define gl_getUniformLocation glGetUniformLocation
#define gl_uniform1i glUniform1i
#define gl_uniform1fv glUniform1fv
#define gl_uniform2fv glUniform2fv
#define gl_uniform3fv glUniform3fv
#define gl_uniform4fv glUniform4fv
#define gl_uniformMatrix2fv glUniformMatrix2fv
#define gl_uniformMatrix3fv glUniformMatrix3fv
#define gl_uniformMatrix4fv glUniformMatrix4fv

#define gl_getAttribLocation glGetAttribLocation
#define gl_getActiveAttrib glGetActiveAttrib
#define gl_bindAttribLocation glBindAttribLocation
#define gl_vertexAttrib1f glVertexAttrib1f
#define gl_vertexAttrib3fv glVertexAttrib3fv
#define gl_vertexAttrib4fv glVertexAttrib4fv

void initShaders() {}

#else

GLuint (APIENTRY* gl_createShader)( GLenum );
void (APIENTRY* gl_deleteShader)( GLuint );
void (APIENTRY* gl_compileShader)( GLuint );
void (APIENTRY* gl_shaderSource)( GLuint, GLsizei, const GLchar**, const GLint* );
void (APIENTRY* gl_getShaderInfoLog)( GLuint, GLsizei, GLsizei*, GLchar* );
void (APIENTRY* gl_getProgramInfoLog)( GLuint, GLsizei, GLsizei*, GLchar* );
void (APIENTRY* gl_getShaderiv)( GLuint, GLenum, GLint* );

GLuint (APIENTRY* gl_createProgram)( void );
void (APIENTRY* gl_deleteProgram)( GLuint );
void (APIENTRY* gl_attachShader)( GLuint, GLuint );
void (APIENTRY* gl_detachShader)( GLuint, GLuint );
void (APIENTRY* gl_linkProgram)( GLuint );
void (APIENTRY* gl_useProgram)( GLuint );
void (APIENTRY* gl_getProgramiv)( GLuint, GLenum, GLint* );
void (APIENTRY* gl_programParameteri)( GLuint program, GLenum pname, GLint value );

void (APIENTRY* gl_getActiveUniform)( GLuint, GLuint, GLsizei, GLsizei*, GLint*, GLenum*, GLchar* );
int  (APIENTRY* gl_getUniformLocation)( GLuint, const GLchar* );
void (APIENTRY* gl_uniform1i)( GLint, GLint );
void (APIENTRY* gl_uniform1fv)( GLint, GLsizei, const GLfloat* );
void (APIENTRY* gl_uniform2fv)( GLint, GLsizei, const GLfloat* );
void (APIENTRY* gl_uniform3fv)( GLint, GLsizei, const GLfloat* );
void (APIENTRY* gl_uniform4fv)( GLint, GLsizei, const GLfloat* );
void (APIENTRY* gl_uniformMatrix2fv)( GLint, GLsizei, GLboolean, const GLfloat* );
void (APIENTRY* gl_uniformMatrix3fv)( GLint, GLsizei, GLboolean, const GLfloat* );
void (APIENTRY* gl_uniformMatrix4fv)( GLint, GLsizei, GLboolean, const GLfloat* );

int  (APIENTRY* gl_getAttribLocation)( GLuint, const GLchar* );
void (APIENTRY* gl_getActiveAttrib)( GLuint, GLuint, GLsizei, GLsizei*, GLint*, GLenum*, GLchar* );
void (APIENTRY* gl_bindAttribLocation)( GLuint, GLuint, const GLchar* );
void (APIENTRY* gl_vertexAttrib1f)( GLuint, GLfloat );
void (APIENTRY* gl_vertexAttrib3fv)( GLuint, const GLfloat* );
void (APIENTRY* gl_vertexAttrib4fv)( GLuint, const GLfloat* );

//------------------------------------------------------------------------------
//!
void initShaders()
{
   gl_createShader = (GLuint (APIENTRY*)( GLenum ))
      glGetProcAddress( "glCreateShaderObjectARB" );
   gl_deleteShader = (void (APIENTRY*)( GLuint ))
      glGetProcAddress( "glDeleteObjectARB" );
   gl_compileShader = (void (APIENTRY*)( GLuint ))
      glGetProcAddress( "glCompileShaderARB" );
   gl_shaderSource = (void (APIENTRY*)( GLuint, GLsizei, const GLchar**, const GLint* ))
      glGetProcAddress( "glShaderSourceARB" );
   gl_getShaderInfoLog = (void (APIENTRY*)( GLuint, GLsizei, GLsizei*, GLchar* ))
      glGetProcAddress( "glGetShaderInfoLog" );
   gl_getProgramInfoLog = (void (APIENTRY*)( GLuint, GLsizei, GLsizei*, GLchar* ))
      glGetProcAddress( "glGetProramInfoLogARB" );
   gl_getShaderiv = (void (APIENTRY*)( GLuint, GLenum, GLint* ))
      glGetProcAddress( "glGetObjectParameterivARB" );

   gl_createProgram = (GLuint (APIENTRY*)( void ))
      glGetProcAddress( "glCreateProgramObjectARB" );
   gl_deleteProgram = (void (APIENTRY*)( GLuint ))
      glGetProcAddress( "glDeleteObjectARB" );
   gl_attachShader = (void (APIENTRY*)( GLuint, GLuint ))
      glGetProcAddress( "glAttachObjectARB" );
   gl_detachShader = (void (APIENTRY*)( GLuint, GLuint ))
      glGetProcAddress( "glDetachObjectARB" );
   gl_linkProgram = (void (APIENTRY*)( GLuint ))
      glGetProcAddress( "glLinkProgramARB" );
   gl_useProgram = (void (APIENTRY*)( GLuint ))
      glGetProcAddress( "glUseProgramObjectARB" );
   gl_getProgramiv = (void (APIENTRY*)( GLuint, GLenum, GLint* ))
      glGetProcAddress( "glGetObjectParameterivARB" );
   gl_programParameteri = (void (APIENTRY*)( GLuint program, GLenum pname, GLint value ))
      glGetProcAddress( "glProgramParameteriARB" );

   gl_getActiveUniform = (void (APIENTRY*)( GLuint, GLuint, GLsizei, GLsizei*, GLint*, GLenum*, GLchar* ))
      glGetProcAddress( "glGetActiveUniformARB" );
   gl_getUniformLocation = (int (APIENTRY*)( GLuint, const GLchar* ))
      glGetProcAddress("glGetUniformLocationARB" );
   gl_uniform1i = (void (APIENTRY*)( GLint, GLint ))
      glGetProcAddress( "glUniform1iARB" );
   gl_uniform1fv = (void (APIENTRY*)( GLint, GLsizei, const GLfloat* ))
      glGetProcAddress( "glUniform1fvARB" );
   gl_uniform2fv = (void (APIENTRY*)( GLint, GLsizei, const GLfloat* ))
      glGetProcAddress( "glUniform2fvARB" );
   gl_uniform3fv = (void (APIENTRY*)( GLint, GLsizei, const GLfloat* ))
      glGetProcAddress( "glUniform3fvARB" );
   gl_uniform4fv = (void (APIENTRY*)( GLint, GLsizei, const GLfloat* ))
      glGetProcAddress( "glUniform4fvARB" );
   gl_uniformMatrix2fv = (void (APIENTRY*)( GLint, GLsizei, GLboolean, const GLfloat* ))
      glGetProcAddress( "glUniformMatrix2fvARB" );
   gl_uniformMatrix3fv = (void (APIENTRY*)( GLint, GLsizei, GLboolean, const GLfloat* ))
      glGetProcAddress( "glUniformMatrix3fvARB" );
   gl_uniformMatrix4fv = (void (APIENTRY*)( GLint, GLsizei, GLboolean, const GLfloat* ))
      glGetProcAddress( "glUniformMatrix4fvARB" );

   gl_getAttribLocation = (int (APIENTRY*)( GLuint, const GLchar* ))
      glGetProcAddress( "glGetAttribLocationARB" );
   gl_getActiveAttrib = (void (APIENTRY*)( GLuint, GLuint, GLsizei, GLsizei*, GLint*, GLenum*, GLchar* ))
      glGetProcAddress( "glGetActiveAttribARB" );
   gl_bindAttribLocation = (void (APIENTRY*)( GLuint, GLuint, const GLchar* ))
      glGetProcAddress( "glBindAttribLocationARB" );
   gl_vertexAttrib1f = (void (APIENTRY*)( GLuint, GLfloat ))
      glGetProcAddress( "glVertexAttrib1fARB" );
   gl_vertexAttrib3fv = (void (APIENTRY*)( GLuint, const GLfloat* ))
      glGetProcAddress( "glVertexAttrib3fvARB" );
   gl_vertexAttrib4fv = (void (APIENTRY*)( GLuint, const GLfloat* ))
      glGetProcAddress( "glVertexAttrib4fvARB" );
}

#endif

/*==============================================================================
   TEXTURES
==============================================================================*/

#if __APPLE__

#define gl_activeTexture      glActiveTexture
#define gl_texImage3D         glTexImage3D
#define gl_texSubImage3D      glTexSubImage3D
#define gl_copyTexSubImage3D  glCopyTexSubImage3D

void initTextures() {}

#else

void (APIENTRY* gl_activeTexture    )( uint );
void (APIENTRY* gl_texImage3D       )(GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLenum format, GLenum type, const GLvoid *pixels);
void (APIENTRY* gl_texSubImage3D    )(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLenum type, const GLvoid *pixels);
void (APIENTRY* gl_copyTexSubImage3D)(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLint x, GLint y, GLsizei width, GLsizei height);

//------------------------------------------------------------------------------
//!
void initTextures()
{
   GET_PROC_ADDR( gl_activeTexture, "glActiveTextureARB",
                  void (APIENTRY*)( uint ) );
   GET_PROC_ADDR( gl_texImage3D, "glTexImage3D",
                  void (APIENTRY*)( GLenum, GLint, GLint, GLsizei, GLsizei, GLsizei, GLint, GLenum, GLenum, const GLvoid* ) );
   GET_PROC_ADDR( gl_texSubImage3D, "glTexSubImage3D",
                  void (APIENTRY*)( GLenum, GLint, GLint, GLint, GLint, GLsizei, GLsizei, GLsizei, GLenum, GLenum, const GLvoid* ) );
   GET_PROC_ADDR( gl_copyTexSubImage3D, "glCopyTexSubImage3D",
                  void (APIENTRY*)( GLenum, GLint, GLint, GLint, GLint, GLint, GLint, GLsizei, GLsizei ) );
}

#endif

/*==============================================================================
   OTHERS
==============================================================================*/

void (APIENTRY* gl_clampColorARB)(GLenum, GLenum);

//------------------------------------------------------------------------------
//!
void initOthers()
{
#if defined(__APPLE__)
   gl_clampColorARB = NULL;
   glClampColorARB( GL_CLAMP_VERTEX_COLOR_ARB, GL_FALSE );
#else
   gl_clampColorARB = (void (APIENTRY*)( GLenum, GLenum ))glGetProcAddress( "glClampColorARB" );

   if( gl_clampColorARB != NULL )
   {
      gl_clampColorARB( GL_CLAMP_VERTEX_COLOR_ARB, GL_FALSE );
   }
   else
   {
      printf("ERROR - glClampColorARB not found.\n");
   }
#endif
}

extern String vs_str_default;
extern String ps_str_default;

//------------------------------------------------------------------------------
//!
void initDefines( String& vsPreamble, String& psPreamble )
{
   // Prepare some GLSL #defines for some oddball cases.
   String defines;

#if __APPLE__
   defines += "#define DYN_LOOP_SLOW 1\n";
#else
   defines += "#define DYN_LOOP_SLOW 0\n";
#endif

   vsPreamble += defines;
   psPreamble += defines;
}

//------------------------------------------------------------------------------
//!
void initStates()
{
   //
   // Settings of default states.
   //
   glDisable   ( GL_LIGHTING );
   glDisable   ( GL_FOG );
   glEnable    ( GL_BLEND );
   glBlendFunc ( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
   glShadeModel( GL_SMOOTH );
   glEnable    ( GL_SCISSOR_TEST );
   glEnable    ( GL_CULL_FACE );
   //glEnable    ( GL_MULTISAMPLE );  // Doesn't seem necessary.
   glEnable( GL_POINT_SPRITE_ARB );
   glEnable( GL_VERTEX_PROGRAM_POINT_SIZE );
   glEnable( GL_TEXTURE_CUBE_MAP_SEAMLESS );
}

//------------------------------------------------------------------------------
//!
void createTextureFormatTable();

//------------------------------------------------------------------------------
//!
void init()
{
   DBG_BLOCK( os_gl, "GLManager::init" );
   if( sInitializeCount == 0 )
   {
#if defined(__APPLE__)
      //OSStatus err = osxOpenGLInitEntryPoints(); //init bundle
      //CHECK( err == noErr );

#if USE_MULTITHREADED_OPENGL
      CGLError cgl_err = kCGLNoError;
      CGLContextObj ctx = CGLGetCurrentContext();

      // Enable the multi-threading
      cgl_err = CGLEnable( ctx, kCGLCEMPEngine);

      if( cgl_err != kCGLNoError )
      {
         // Multi-threaded execution is possibly not available
         // Insert your code to take appropriate action
         printf("Multi-threaded OpenGL could not be turned on\n");
      }
#endif //USE_MULTITHREAD_OPENGL

#endif //defined(__APPLE__)
   }
   ++sInitializeCount;

   createTextureFormatTable();
   initStates();
   initFast();
   initTextures();
   initShaders();
   initFBOs();
   initVAOs();
   initOthers();
   initDefines( vs_str_default, ps_str_default );
}

//------------------------------------------------------------------------------
//!
void terminateGL()
{
   DBG_BLOCK( os_gl, "GLManager::terminate" );
   if( sInitializeCount == 0 )
   {
      CHECK(0 && "OpenGL is terminated too often");
   }

   --sInitializeCount;
   if( sInitializeCount == 0 )
   {
      //Deallocate some stuff

#if defined(__APPLE_)
      //osxOpenGLDellocEntryPoints(); //release bundle
#endif
   }
}


/*==============================================================================
   Utilities
==============================================================================*/

//------------------------------------------------------------------------------
//!
inline void displayGLError(const char* fmt, GLenum err)
{
   printf(fmt, gluErrorString(err));
}

//------------------------------------------------------------------------------
//!
bool  checkForErrors(const char* fmt)
{
   GLenum err = glGetError();

   if( err == GL_NO_ERROR )
   {
      return false;
   }
   else
   {
      while( err != GL_NO_ERROR )
      {
         displayGLError(fmt, err);
         err = glGetError();
      }
      return true;
   }
}

#if 0
//------------------------------------------------------------------------------
//!
void printGLErrors()
{
   GLenum err = glGetError();

   if( err == GL_NO_ERROR )
   {
      printf("GL_ERROR: none\n");
   }
   else
   {
      printf("GL_ERROR: %d\n", (int)err );
      printf("---------\n");
      while( err != GL_NO_ERROR )
      {
         printf("%s\n", gluErrorString(err));
         err = glGetError();
      }
      printf("---------\n");
   }
}
#endif

#ifdef _DEBUG
//------------------------------------------------------------------------------
//!
bool checkFramebufferStatus()
{
   GLenum status;
   status = (GLenum)gl_checkFramebufferStatusEXT(GL_FRAMEBUFFER_EXT);
   switch(status) {
      case GL_FRAMEBUFFER_COMPLETE_EXT:
         break;
      case GL_FRAMEBUFFER_UNSUPPORTED_EXT:
         printf("Unsupported framebuffer format\n");
         break;
      case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT_EXT:
         printf("Incomplete attachment in framebuffer\n");
         break;
      case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT_EXT:
         printf("Framebuffer incomplete, missing attachment\n");
         break;
      /*
      * Removed in revision 117 of the FBO extension spec
      case GL_FRAMEBUFFER_INCOMPLETE_DUPLICATE_ATTACHMENT_EXT:
         printf("Framebuffer incomplete, duplicate attachment\n");
         break;
      */
      case GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS_EXT:
         printf("Framebuffer incomplete, attached images must have same dimensions\n");
         break;
      case GL_FRAMEBUFFER_INCOMPLETE_FORMATS_EXT:
         printf("Framebuffer incomplete, attached images must have same format\n");
         break;
      case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER_EXT:
         printf("Framebuffer incomplete, missing draw buffer\n");
         break;
      case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER_EXT:
         printf("Framebuffer incomplete, missing read buffer\n");
         break;
      default:
         printf("Unknown glCheckFramebufferStatusEXT error: 0x%04x\n", status);
         break;
   }
   return status == GL_FRAMEBUFFER_COMPLETE_EXT;
}
#endif

//------------------------------------------------------------------------------
//!
inline GLenum
toGL( const PrimitiveType pt )
{
   switch( pt )
   {
      case PRIM_INVALID       : return (GLenum)-1;
      case PRIM_POINTS        : return GL_POINTS;
      case PRIM_LINES         : return GL_LINES;
      case PRIM_LINE_LOOP     : return GL_LINE_LOOP;
      case PRIM_LINE_STRIP    : return GL_LINE_STRIP;
      case PRIM_TRIANGLES     : return GL_TRIANGLES;
      case PRIM_TRIANGLE_STRIP: return GL_TRIANGLE_STRIP;
      case PRIM_TRIANGLE_FAN  : return GL_TRIANGLE_FAN;
      case PRIM_QUADS         : return GL_QUADS;
      case PRIM_QUAD_STRIP    : return GL_QUAD_STRIP;
      case PRIM_POLYGON       : return GL_POLYGON;
      default                 : return (GLenum)-1;
   }
}

//-----------------------------------------------------------------------------
//!
inline GLenum
toGL( const GSInputType t )
{
   switch( t )
   {
      case GS_IN_POINTS             : return GL_POINTS;
      case GS_IN_LINES              : return GL_LINES;
      case GS_IN_LINES_ADJACENCY    : return GL_LINES_ADJACENCY;
      case GS_IN_TRIANGLES          : return GL_TRIANGLES;
      case GS_IN_TRIANGLES_ADJACENCY: return GL_TRIANGLES_ADJACENCY;
      default                       : return (GLenum)-1;
   }
}

//-----------------------------------------------------------------------------
//!
inline GLenum
toGL( const GSOutputType t )
{
   switch( t )
   {
      case GS_OUT_POINTS        : return GL_POINTS;
      case GS_OUT_LINE_STRIP    : return GL_LINE_STRIP;
      case GS_OUT_TRIANGLE_STRIP: return GL_TRIANGLE_STRIP;
      default                   : return (GLenum)-1;
   }
}

//------------------------------------------------------------------------------
//!
inline void
toGL( const AttributeFormat afmt, uint& numChannels, GLenum& type )
{
   numChannels = toNumChannels(afmt);
   switch( afmt )
   {
      case ATTRIB_FMT_INVALID:
         type = (GLenum)-1;
         break;
      case ATTRIB_FMT_8      :
      case ATTRIB_FMT_8_8    :
      case ATTRIB_FMT_8_8_8  :
      case ATTRIB_FMT_8_8_8_8:
         type = GL_UNSIGNED_BYTE;
         break;
      case ATTRIB_FMT_16         :
      case ATTRIB_FMT_16_16      :
      case ATTRIB_FMT_16_16_16   :
      case ATTRIB_FMT_16_16_16_16:
         type = GL_UNSIGNED_SHORT;
         break;
      case ATTRIB_FMT_16F            :
      case ATTRIB_FMT_16F_16F        :
      case ATTRIB_FMT_16F_16F_16F    :
      case ATTRIB_FMT_16F_16F_16F_16F:
         type = GL_HALF_FLOAT_ARB;
         break;
      case ATTRIB_FMT_32         :
      case ATTRIB_FMT_32_32      :
      case ATTRIB_FMT_32_32_32   :
      case ATTRIB_FMT_32_32_32_32:
         type = GL_UNSIGNED_INT;
         break;
      case ATTRIB_FMT_32F            :
      case ATTRIB_FMT_32F_32F        :
      case ATTRIB_FMT_32F_32F_32F    :
      case ATTRIB_FMT_32F_32F_32F_32F:
         type = GL_FLOAT;
         break;
      default:
         type = (GLenum)-1;
         break;
   }
}

//------------------------------------------------------------------------------
//!
inline GLenum
toGL( const IndexFormat afmt )
{
   switch( afmt )
   {
      case INDEX_FMT_INVALID:
         return (GLenum)-1;
      case INDEX_FMT_8:
         return GL_UNSIGNED_BYTE;
      case INDEX_FMT_16:
         return GL_UNSIGNED_SHORT;
      case INDEX_FMT_32:
         return GL_UNSIGNED_INT;
      default:
         return (GLenum)-1;
   }
}

//------------------------------------------------------------------------------
//!
inline GLenum
toGLCubemapTarget( const uint slice )
{
   switch( slice )
   {
      case TEX_SLICE_NEG_X: return GL_TEXTURE_CUBE_MAP_NEGATIVE_X;
      case TEX_SLICE_POS_X: return GL_TEXTURE_CUBE_MAP_POSITIVE_X;
      case TEX_SLICE_NEG_Y: return GL_TEXTURE_CUBE_MAP_NEGATIVE_Y;
      case TEX_SLICE_POS_Y: return GL_TEXTURE_CUBE_MAP_POSITIVE_Y;
      case TEX_SLICE_NEG_Z: return GL_TEXTURE_CUBE_MAP_NEGATIVE_Z;
      case TEX_SLICE_POS_Z: return GL_TEXTURE_CUBE_MAP_POSITIVE_Z;
      default             : return (GLenum)-1;
   }
}

struct TexFmtDataEntry
{
   GLint   internalFormat;
   GLenum  format;
   GLenum  type;
   TexFmtDataEntry( GLint intFmt = 0, GLenum fmt = (GLenum)-1, GLenum typ = 0 ):
      internalFormat(intFmt), format(fmt), type(typ) {}
};

//!< A table with all of the texture format data entries.
const int _gNumTexFormats = TEX_FMT_8_24+1;
TexFmtDataEntry  _gTextureFormatTable[_gNumTexFormats];

//------------------------------------------------------------------------------
//!
void createTextureFormatTable()
{
   //Note: the 32b fixed don't exist yet (but might once integer make it into the core)
   //                   TextureFormat                               internalFormat             format                type
   _gTextureFormatTable[TEX_FMT_INVALID        ] = TexFmtDataEntry( 0                        , (GLenum)-1          , (GLenum)-1               );
   _gTextureFormatTable[TEX_FMT_8              ] = TexFmtDataEntry( GL_LUMINANCE8            , GL_LUMINANCE        , GL_UNSIGNED_BYTE         );
   _gTextureFormatTable[TEX_FMT_8_8            ] = TexFmtDataEntry( GL_LUMINANCE8_ALPHA8     , GL_LUMINANCE_ALPHA  , GL_UNSIGNED_BYTE         );
   //_gTextureFormatTable[TEX_FMT_8_8_8          ] = TexFmtDataEntry( GL_RGB8                  , GL_RGB              , GL_UNSIGNED_BYTE         ); //Probably not supported
   _gTextureFormatTable[TEX_FMT_8_8_8_8        ] = TexFmtDataEntry( GL_RGBA8                 , GL_RGBA             , GL_UNSIGNED_BYTE         );
   _gTextureFormatTable[TEX_FMT_16             ] = TexFmtDataEntry( GL_LUMINANCE16           , GL_LUMINANCE        , GL_UNSIGNED_SHORT        );
   _gTextureFormatTable[TEX_FMT_16_16          ] = TexFmtDataEntry( GL_LUMINANCE16_ALPHA16   , GL_LUMINANCE_ALPHA  , GL_UNSIGNED_SHORT        );
   //_gTextureFormatTable[TEX_FMT_16_16_16       ] = TexFmtDataEntry( GL_RGB16                 , GL_RGB              , GL_UNSIGNED_SHORT        ); //Probably not supported
   _gTextureFormatTable[TEX_FMT_16_16_16_16    ] = TexFmtDataEntry( GL_RGBA16                , GL_RGBA             , GL_UNSIGNED_SHORT        );
   _gTextureFormatTable[TEX_FMT_16F            ] = TexFmtDataEntry( GL_RED                   , GL_RED              , GL_HALF_FLOAT_ARB        );
   _gTextureFormatTable[TEX_FMT_16F_16F        ] = TexFmtDataEntry( GL_LUMINANCE_ALPHA16F_ARB, GL_LUMINANCE_ALPHA  , GL_HALF_FLOAT_ARB        );
   //_gTextureFormatTable[TEX_FMT_16F_16F_16F    ] = TexFmtDataEntry( GL_RGB16F_ARB            , GL_RGB              , GL_HALF_FLOAT_ARB        ); //Probably not supported
   _gTextureFormatTable[TEX_FMT_16F_16F_16F_16F] = TexFmtDataEntry( GL_RGBA16F_ARB           , GL_RGBA             , GL_HALF_FLOAT_ARB        );
   _gTextureFormatTable[TEX_FMT_32             ] = TexFmtDataEntry( GL_LUMINANCE             , GL_LUMINANCE        , GL_UNSIGNED_INT          );
   //_gTextureFormatTable[TEX_FMT_32_32          ] = TexFmtDataEntry( 2                        , GL_LUMINANCE_ALPHA  , GL_UNSIGNED_INT          );
   //_gTextureFormatTable[TEX_FMT_32_32_32       ] = TexFmtDataEntry( 3                        , GL_RGB              , GL_UNSIGNED_INT          );
   //_gTextureFormatTable[TEX_FMT_32_32_32_32    ] = TexFmtDataEntry( 4                        , GL_RGBA             , GL_UNSIGNED_INT          );
   _gTextureFormatTable[TEX_FMT_32F            ] = TexFmtDataEntry( GL_LUMINANCE32F_ARB      , GL_LUMINANCE        , GL_FLOAT                 );
   _gTextureFormatTable[TEX_FMT_32F_32F        ] = TexFmtDataEntry( GL_LUMINANCE_ALPHA32F_ARB, GL_LUMINANCE_ALPHA  , GL_FLOAT                 );
   //_gTextureFormatTable[TEX_FMT_32F_32F_32F    ] = TexFmtDataEntry( GL_RGB32F_ARB            , GL_RGB              , GL_FLOAT                 );
   _gTextureFormatTable[TEX_FMT_32F_32F_32F_32F] = TexFmtDataEntry( GL_RGBA32F_ARB           , GL_RGBA             , GL_FLOAT                 );
   _gTextureFormatTable[TEX_FMT_24_8           ] = TexFmtDataEntry( GL_DEPTH24_STENCIL8_EXT  , GL_DEPTH_STENCIL_EXT, GL_UNSIGNED_INT_24_8_EXT );
   _gTextureFormatTable[TEX_FMT_8_24           ] = TexFmtDataEntry( GL_DEPTH24_STENCIL8_EXT  , GL_DEPTH_STENCIL_EXT, GL_UNSIGNED_INT_24_8_EXT );
}

//------------------------------------------------------------------------------
//!
inline void
toGL( const TextureFormat tfmt, GLint& internalFormat, GLenum& format, GLenum& type )
{
   CHECK( tfmt < _gNumTexFormats );
   const TexFmtDataEntry& entry = _gTextureFormatTable[tfmt];
   internalFormat = entry.internalFormat;
   format         = entry.format;
   type           = entry.type;
}

//------------------------------------------------------------------------------
//!
inline GLint
toGLInternalFormat( const TextureFormat tfmt )
{
   CHECK( tfmt < _gNumTexFormats );
   return _gTextureFormatTable[tfmt].internalFormat;
}

//------------------------------------------------------------------------------
//!
inline GLenum
toGL( const MapMode mode )
{
   switch( mode )
   {
      case MAP_READ      : return GL_READ_ONLY;
      case MAP_WRITE     : return GL_WRITE_ONLY;
      case MAP_READ_WRITE: return GL_READ_WRITE;
      default            : return (GLenum)-1;
   }
}

//------------------------------------------------------------------------------
//!
inline void
toGLFilters(
   const TextureState& state,
   GLenum&             magFilter,
   GLenum&             minFilter,
   GLint&              maxAniso
)
{
   if( state.maxAniso() != 0 )
   {
      magFilter = GL_LINEAR;
      minFilter = GL_LINEAR_MIPMAP_LINEAR;
      maxAniso  = state.maxAniso();
   }
   else
   {
      switch( state.magFilter() )
      {
         case TEX_FILTER_POINT : magFilter = GL_NEAREST; break;
         case TEX_FILTER_LINEAR: magFilter = GL_LINEAR;  break;
         default               : magFilter = GL_NEAREST; break;
      }
      switch( state.minFilter() )
      {
         case TEX_FILTER_POINT:
         {
            switch( state.mipFilter() )
            {
               case TEX_FILTER_NONE:   minFilter = GL_NEAREST;                break;
               case TEX_FILTER_POINT:  minFilter = GL_NEAREST_MIPMAP_NEAREST; break;
               case TEX_FILTER_LINEAR: minFilter = GL_NEAREST_MIPMAP_LINEAR;  break;
               default:                minFilter = GL_NEAREST_MIPMAP_NEAREST; break;
            }
         } break;
         case TEX_FILTER_LINEAR:
         {
            switch( state.mipFilter() )
            {
               case TEX_FILTER_NONE:   minFilter = GL_LINEAR;                 break;
               case TEX_FILTER_POINT:  minFilter = GL_LINEAR_MIPMAP_NEAREST;  break;
               case TEX_FILTER_LINEAR: minFilter = GL_LINEAR_MIPMAP_LINEAR;   break;
               default:                minFilter = GL_LINEAR_MIPMAP_NEAREST;  break;
            }
         } break;
         default:
         {
            switch( state.mipFilter() )
            {
               case TEX_FILTER_NONE:   minFilter = GL_NEAREST;                break;
               case TEX_FILTER_POINT:  minFilter = GL_NEAREST_MIPMAP_NEAREST; break;
               case TEX_FILTER_LINEAR: minFilter = GL_NEAREST_MIPMAP_LINEAR;  break;
               default:                minFilter = GL_NEAREST_MIPMAP_NEAREST; break;
            }
         } break;
      }
      maxAniso = 1;
   }
}

#if 0
//------------------------------------------------------------------------------
//!
inline GLenum
removeMipmap( const GLenum f )
{
   switch( f )
   {
      case GL_NEAREST:
      case GL_NEAREST_MIPMAP_NEAREST:
      case GL_NEAREST_MIPMAP_LINEAR:
         return GL_NEAREST;
      case GL_LINEAR:
      case GL_LINEAR_MIPMAP_NEAREST:
      case GL_LINEAR_MIPMAP_LINEAR:
         return GL_NEAREST;
      default:
         CHECK(0 && "INVALID ENUM");
         return (GLenum)-1;
   }
}
#endif

/**
//------------------------------------------------------------------------------
//!
inline GLenum
toGL( TextureType texType )
{
   switch( texType )
   {
      case TEX_TYPE_UNSPECIFIED: return (GLenum)-1;
      case TEX_TYPE_1D         : return GL_TEXTURE_1D;
      case TEX_TYPE_1D_ARRAY   : return (GLenum)-1;
      case TEX_TYPE_2D         : return GL_TEXTURE_2D;
      case TEX_TYPE_2D_ARRAY   : return (GLenum)-1;
      case TEX_TYPE_3D         : return GL_TEXTURE_3D;
      case TEX_TYPE_CUBEMAP    : return GL_TEXTURE_CUBE_MAP;
      default                  : return (GLenum)-1;
   }
}
**/

//------------------------------------------------------------------------------
//!
inline GLenum
toGL( TexClamp clamp )
{
   switch( clamp )
   {
      case TEX_CLAMP_WRAP             : return GL_REPEAT;
      case TEX_CLAMP_MIRROR           : return GL_MIRRORED_REPEAT;
      case TEX_CLAMP_LAST             : return GL_CLAMP_TO_EDGE;
      case TEX_CLAMP_BORDER           : return GL_CLAMP_TO_BORDER;
      case TEX_CLAMP_MIRRORONCE_LAST  : return GL_MIRROR_CLAMP_TO_EDGE;
      case TEX_CLAMP_MIRRORONCE_BORDER: return GL_MIRROR_CLAMP_TO_BORDER;
      default                         : return GL_CLAMP_TO_EDGE;
   }
}

//------------------------------------------------------------------------------
//!
inline GLenum
toGL( CompareFunc func )
{
   switch( func )
   {
      case COMPARE_FUNC_NEVER       : return GL_NEVER;
      case COMPARE_FUNC_LESS        : return GL_LESS;
      case COMPARE_FUNC_EQUAL       : return GL_EQUAL;
      case COMPARE_FUNC_LESSEQUAL   : return GL_LEQUAL;
      case COMPARE_FUNC_GREATER     : return GL_GREATER;
      case COMPARE_FUNC_NOTEQUAL    : return GL_NOTEQUAL;
      case COMPARE_FUNC_GREATEREQUAL: return GL_GEQUAL;
      case COMPARE_FUNC_ALWAYS      : return GL_ALWAYS;
      default                       : return GL_LEQUAL;
   }
}

//------------------------------------------------------------------------------
//!
inline GLenum
toGL( Gfx::AlphaBlend blend )
{
   switch( blend )
   {
      case ALPHA_BLEND_ZERO                 : return GL_ZERO;
      case ALPHA_BLEND_ONE                  : return GL_ONE;
      case ALPHA_BLEND_SRC_COLOR            : return GL_SRC_COLOR;
      case ALPHA_BLEND_INV_SRC_COLOR        : return GL_ONE_MINUS_SRC_COLOR;
      case ALPHA_BLEND_SRC_ALPHA            : return GL_SRC_ALPHA;
      case ALPHA_BLEND_INV_SRC_ALPHA        : return GL_ONE_MINUS_SRC_ALPHA;
      case ALPHA_BLEND_DST_ALPHA            : return GL_DST_ALPHA;
      case ALPHA_BLEND_INV_DST_ALPHA        : return GL_ONE_MINUS_DST_ALPHA;
      case ALPHA_BLEND_DST_COLOR            : return GL_DST_COLOR;
      case ALPHA_BLEND_INV_DST_COLOR        : return GL_ONE_MINUS_DST_COLOR;
      case ALPHA_BLEND_SRC_ALPHA_SAT        : return GL_SRC_ALPHA_SATURATE;
      case ALPHA_BLEND_BOTH_SRC_ALPHA       : return GL_SRC_ALPHA;           // ???
      case ALPHA_BLEND_BOTH_INV_SRC_ALPHA   : return GL_ONE_MINUS_SRC_ALPHA; // ???
      case ALPHA_BLEND_BLEND_FACTOR         : return GL_CONSTANT_COLOR;
      case ALPHA_BLEND_INV_BLEND_FACTOR     : return GL_ONE_MINUS_CONSTANT_COLOR;
      default                               : return GL_ONE;
   }
}

//------------------------------------------------------------------------------
//!
inline GLenum
toGL( StencilOp op )
{
   switch( op )
   {
      case STENCIL_OP_KEEP     : return GL_KEEP;
      case STENCIL_OP_ZERO     : return GL_ZERO ;
      case STENCIL_OP_REPLACE  : return GL_REPLACE;
      case STENCIL_OP_INCR_SAT : return GL_INCR;
      case STENCIL_OP_DECR_SAT : return GL_DECR;
      case STENCIL_OP_INVERT   : return GL_INVERT;
      case STENCIL_OP_INCR     : return GL_INCR_WRAP;
      case STENCIL_OP_DECR     : return GL_DECR_WRAP;
      default                  : return GL_KEEP;
   }
}

//------------------------------------------------------------------------------
//!
inline void
applyState( const AlphaState& state, AlphaState& curState )
{
   switch( state.compare(curState) )
   {
      case 0:
         // Identical, nothing to do.
         return;
      case 1:
         // Optimized path: only blending state changed.
         if( state.alphaBlending() )
         {
            glEnable( GL_BLEND );
         }
         else
         {
            glDisable( GL_BLEND );
         }
         break;
      case 4:
         // Testing.
         if( state.alphaTesting() )
         {
            glEnable( GL_ALPHA_TEST );
         }
         else
         {
            glDisable( GL_ALPHA_TEST );
         }
         break;
      case 5:
         // Blending.
         if( state.alphaBlending() )
         {
            glEnable( GL_BLEND );
         }
         else
         {
            glDisable( GL_BLEND );
         }
         // Testing.
         if( state.alphaTesting() )
         {
            glEnable( GL_ALPHA_TEST );
         }
         else
         {
            glDisable( GL_ALPHA_TEST );
         }
         break;
      default:
         // Perform full state update.

         // Blending.
         if( state.alphaBlending() )
         {
            glEnable( GL_BLEND );
         }
         else
         {
            glDisable( GL_BLEND );
         }
         glBlendFunc( toGL( state.alphaBlendSrc() ), toGL( state.alphaBlendDst() ) );

         // Testing.
         if( state.alphaTesting() )
         {
            glEnable( GL_ALPHA_TEST );
         }
         else
         {
            glDisable( GL_ALPHA_TEST );
         }
         glAlphaFunc( toGL( state.alphaTestFunc() ), state.alphaTestRef() );
         break;
   }

   // Update current state.
   curState = state;
}

//------------------------------------------------------------------------------
//!
inline void
applyState( const ColorState& state, ColorState& curState )
{
   if( state != curState )
   {
      if( state.colorWriting() )
      {
         glColorMask( GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE );
      }
      else
      {
         glColorMask( GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE );
      }
      curState = state;
   }
}

//------------------------------------------------------------------------------
//!
inline void
applyState( const CullState& state, CullState& curState )
{
   if( state != curState )
   {
      switch( state.side() )
      {
         case FACE_NONE:            glDisable( GL_CULL_FACE );                                  break;
         case FACE_FRONT:           glEnable( GL_CULL_FACE ); glCullFace( GL_FRONT );           break;
         case FACE_BACK:            glEnable( GL_CULL_FACE ); glCullFace( GL_BACK );            break;
         case FACE_FRONT_AND_BACK:  glEnable( GL_CULL_FACE ); glCullFace( GL_FRONT_AND_BACK );  break;
         default:                   glEnable( GL_CULL_FACE ); glCullFace( GL_BACK );            break;
      }
      curState = state;
   }
}

//------------------------------------------------------------------------------
//!
inline void
applyState( const DepthState& state, DepthState& curState )
{
   if( state != curState )
   {
      if( state.depthTesting() )
      {
         glEnable( GL_DEPTH_TEST );
      }
      else
      {
         glDisable( GL_DEPTH_TEST );
      }
      glDepthFunc( toGL( state.depthTestFunc() ) );
      glDepthMask( state.depthWriting() ? GL_TRUE : GL_FALSE );
      curState = state;
   }
}

//------------------------------------------------------------------------------
//!
inline void
applyState( const OffsetState& state, OffsetState& curState )
{
   if( state != curState )
   {
      if( state.factor() != 0.0f || state.constant() != 0.0f )
      {
         glEnable( GL_POLYGON_OFFSET_FILL );
         glEnable( GL_POLYGON_OFFSET_LINE );
         glPolygonOffset( state.factor(), state.constant() );
      }
      else
      {
         glDisable( GL_POLYGON_OFFSET_FILL );
         glDisable( GL_POLYGON_OFFSET_LINE );
      }
      curState = state;
   }
}

//------------------------------------------------------------------------------
//!
inline void
applyState( const StencilState& state, StencilState& curState )
{
   if( state != curState )
   {
      if( state.stencilTesting() )
      {
         glEnable( GL_STENCIL_TEST );
      }
      else
      {
         glDisable( GL_STENCIL_TEST );
      }
      glStencilFunc(
         toGL( state.stencilTestFunc() ),
         state.stencilTestRef(),
         state.stencilTestRefMask()
      );

      glStencilOp(
         toGL( state.stencilFailOp() ),
         toGL( state.stencilPassDepthFailOp() ),
         toGL( state.stencilPassDepthPassOp() )
      );
      glStencilMask( state.stencilTestWriteMask() );

      curState = state;
   }
}

//------------------------------------------------------------------------------
//!
float identityMatrix[16] =
{
   1.0f, 0.0f, 0.0f, 0.0f,
   0.0f, 1.0f, 0.0f, 0.0f,
   0.0f, 0.0f, 1.0f, 0.0f,
   0.0f, 0.0f, 0.0f, 1.0f
};


AlphaState    alphaStateDef;    //!< Default alpha states.
ColorState    colorStateDef;    //!< Default color states.
CullState     cullStateDef;     //!< Default cull states.
DepthState    depthStateDef;    //!< Default depth states.
OffsetState   offsetStateDef;   //!< Default polygon offset states.
StencilState  stencilStateDef;  //!< Default stencil states.


/*==============================================================================
  CLASS BufferMapInfo
==============================================================================*/
class BufferMapInfo
{
public:

   /*----- methods -----*/

   BufferMapInfo( const size_t offsetInBytes, const size_t sizeInBytes );
   ~BufferMapInfo();

   inline size_t  offsetInBytes() const { return _offsetInBytes; }
   inline size_t  sizeInBytes() const { return _sizeInBytes; }

   inline       void*  data()       { return (void*)_data; }
   inline const void*  data() const { return (void*)_data; }

protected:

   /*----- data members -----*/

   size_t  _offsetInBytes;
   size_t  _sizeInBytes;
   uchar*  _data;

}; //class BufferMapInfo

//------------------------------------------------------------------------------
//!
BufferMapInfo::BufferMapInfo( const size_t offsetInBytes, const size_t sizeInBytes ):
   _offsetInBytes( offsetInBytes ), _sizeInBytes( sizeInBytes )
{
   _data = new uchar[sizeInBytes];
}

//------------------------------------------------------------------------------
//!
BufferMapInfo::~BufferMapInfo()
{
   delete [] _data;
}


//OpenGL-specific subclasses of Gfx

/*==============================================================================
   CLASS OpenGLIndexBuffer
==============================================================================*/

class OpenGLIndexBuffer:
   public IndexBuffer
{
public:

   /*----- methods -----*/
   OpenGLIndexBuffer( const IndexFormat format, const BufferFlags flags );

   virtual ~OpenGLIndexBuffer();

   inline GLenum  usage() const
   {
      return isStreamable() ? GL_DYNAMIC_DRAW : GL_STATIC_DRAW;
   }

   /*----- data members -----*/
   GLuint          _id;  //!< A unique identifier
   BufferMapInfo*  _mapInfo;

};  //class OpenGLIndexBuffer

//------------------------------------------------------------------------------
//!
OpenGLIndexBuffer::OpenGLIndexBuffer
( const IndexFormat format, const BufferFlags flags ):
   IndexBuffer(format, flags),
   _mapInfo(NULL)
{
   gl_genBuffers(1, &_id);
}

//------------------------------------------------------------------------------
//!
OpenGLIndexBuffer::~OpenGLIndexBuffer
( void )
{
   CHECK( _mapInfo == NULL );
   gl_deleteBuffers(1, &_id);  //should silently ignore invalid IDs
}


/*==============================================================================
   CLASS OpenGLVertexBuffer
==============================================================================*/

class OpenGLVertexBuffer:
   public VertexBuffer
{
public:

   /*----- static methods -----*/


   static void disable( uint attributes )
   {
      if( attributes & ATTRIB_TYPE_COLOR )
      {
         glDisableClientState( GL_COLOR_ARRAY );
      }
      if( attributes & ATTRIB_TYPE_TEXCOORD0 )
      {
         gl_clientActiveTexture( GL_TEXTURE0 );
         glDisableClientState( GL_TEXTURE_COORD_ARRAY );
      }
      if( attributes & ATTRIB_TYPE_TEXCOORD1 )
      {
         gl_clientActiveTexture( GL_TEXTURE0 + 1 );
         glDisableClientState( GL_TEXTURE_COORD_ARRAY );
      }
      if( attributes & ATTRIB_TYPE_TEXCOORD2 )
      {
         gl_clientActiveTexture( GL_TEXTURE0 + 2 );
         glDisableClientState( GL_TEXTURE_COORD_ARRAY );
      }
      if( attributes & ATTRIB_TYPE_TEXCOORD3 )
      {
         gl_clientActiveTexture( GL_TEXTURE0 + 3 );
         glDisableClientState( GL_TEXTURE_COORD_ARRAY );
      }
      if( attributes & ATTRIB_TYPE_TEXCOORD4 )
      {
         gl_clientActiveTexture( GL_TEXTURE0 + 4 );
         glDisableClientState( GL_TEXTURE_COORD_ARRAY );
      }
      if( attributes & ATTRIB_TYPE_TEXCOORD5 )
      {
         gl_clientActiveTexture( GL_TEXTURE0 + 5 );
         glDisableClientState( GL_TEXTURE_COORD_ARRAY );
      }
      if( attributes & ATTRIB_TYPE_TEXCOORD6 )
      {
         gl_clientActiveTexture( GL_TEXTURE0 + 6 );
         glDisableClientState( GL_TEXTURE_COORD_ARRAY );
      }
      if( attributes & ATTRIB_TYPE_TEXCOORD7 )
      {
         gl_clientActiveTexture( GL_TEXTURE0 + 7 );
         glDisableClientState( GL_TEXTURE_COORD_ARRAY );
      }
      if( attributes & ATTRIB_TYPE_NORMAL )
      {
         glDisableClientState( GL_NORMAL_ARRAY );
      }
      if( attributes & ATTRIB_TYPE_TANGENT )
      {
         gl_disableVertexAttribArray( 1 );
      }
      if( attributes & ATTRIB_TYPE_BINORMAL )
      {
         gl_disableVertexAttribArray( 2 );
      }
   }


   /*----- methods -----*/

   OpenGLVertexBuffer( const BufferFlags flags ):
      VertexBuffer(flags),
      _mapInfo(NULL)
   {
      gl_genBuffers(1, &_id);
   }

   virtual ~OpenGLVertexBuffer()
   {
      CHECK(_mapInfo == NULL);
      gl_deleteBuffers(1, &_id);  //should silently ignore invalid IDs
   }

   uint activate() const
   {
      gl_bindBuffer( GL_ARRAY_BUFFER, _id );

      uint attributes = 0;
      AttributesContainer::ConstIterator curAttrib = _attributes.begin();
      AttributesContainer::ConstIterator endAttrib = _attributes.end();
      for( ; curAttrib != endAttrib; ++curAttrib )
      {
         uint elemCount;
         GLenum elemType;
         toGL( (*curAttrib)->_format, elemCount, elemType );
         attributes |= (*curAttrib)->_type;
         switch( (*curAttrib)->_type )
         {
            case ATTRIB_TYPE_POSITION:
               // FIXME: Should be call last?
               glEnableClientState( GL_VERTEX_ARRAY );
               glVertexPointer( elemCount, elemType, GLsizei(_strideInBytes), BUFFER_OFFSET( (*curAttrib)->_offset ) );
               break;
            case ATTRIB_TYPE_COLOR:
               glEnableClientState( GL_COLOR_ARRAY );
               glColorPointer( elemCount, elemType, GLsizei(_strideInBytes), BUFFER_OFFSET( (*curAttrib)->_offset ) );
               break;
            case ATTRIB_TYPE_TEXCOORD0:
               gl_clientActiveTexture( GL_TEXTURE0 );
               glEnableClientState( GL_TEXTURE_COORD_ARRAY );
               glTexCoordPointer( elemCount, elemType, GLsizei(_strideInBytes), BUFFER_OFFSET( (*curAttrib)->_offset ) );
               break;
            case ATTRIB_TYPE_TEXCOORD1:
               gl_clientActiveTexture( GL_TEXTURE0 + 1 );
               glEnableClientState( GL_TEXTURE_COORD_ARRAY );
               glTexCoordPointer( elemCount, elemType, GLsizei(_strideInBytes), BUFFER_OFFSET( (*curAttrib)->_offset ) );
               break;
            case ATTRIB_TYPE_TEXCOORD2:
               gl_clientActiveTexture( GL_TEXTURE0 + 2 );
               glEnableClientState( GL_TEXTURE_COORD_ARRAY );
               glTexCoordPointer( elemCount, elemType, GLsizei(_strideInBytes), BUFFER_OFFSET( (*curAttrib)->_offset ) );
               break;
            case ATTRIB_TYPE_TEXCOORD3:
               gl_clientActiveTexture( GL_TEXTURE0 + 3 );
               glEnableClientState( GL_TEXTURE_COORD_ARRAY );
               glTexCoordPointer( elemCount, elemType, GLsizei(_strideInBytes), BUFFER_OFFSET( (*curAttrib)->_offset ) );
               break;
            case ATTRIB_TYPE_TEXCOORD4:
               gl_clientActiveTexture( GL_TEXTURE0 + 4 );
               glEnableClientState( GL_TEXTURE_COORD_ARRAY );
               glTexCoordPointer( elemCount, elemType, GLsizei(_strideInBytes), BUFFER_OFFSET( (*curAttrib)->_offset ) );
               break;
            case ATTRIB_TYPE_TEXCOORD5:
               gl_clientActiveTexture( GL_TEXTURE0 + 5 );
               glEnableClientState( GL_TEXTURE_COORD_ARRAY );
               glTexCoordPointer( elemCount, elemType, GLsizei(_strideInBytes), BUFFER_OFFSET( (*curAttrib)->_offset ) );
               break;
            case ATTRIB_TYPE_TEXCOORD6:
               gl_clientActiveTexture( GL_TEXTURE0 + 6 );
               glEnableClientState( GL_TEXTURE_COORD_ARRAY );
               glTexCoordPointer( elemCount, elemType, GLsizei(_strideInBytes), BUFFER_OFFSET( (*curAttrib)->_offset ) );
               break;
            case ATTRIB_TYPE_TEXCOORD7:
               gl_clientActiveTexture( GL_TEXTURE0 + 7 );
               glEnableClientState( GL_TEXTURE_COORD_ARRAY );
               glTexCoordPointer( elemCount, elemType, GLsizei(_strideInBytes), BUFFER_OFFSET( (*curAttrib)->_offset ) );
               break;
            case Gfx::ATTRIB_TYPE_NORMAL:
               glEnableClientState( GL_NORMAL_ARRAY );
               glNormalPointer( elemType, GLsizei(_strideInBytes), BUFFER_OFFSET( (*curAttrib)->_offset ) );
               break;
            case Gfx::ATTRIB_TYPE_TANGENT:
               gl_enableVertexAttribArray( 1 );
               gl_vertexAttribPointer( 1, elemCount, elemType, true, GLsizei(_strideInBytes), BUFFER_OFFSET( (*curAttrib)->_offset ) );
               break;
            case Gfx::ATTRIB_TYPE_BINORMAL:
               gl_enableVertexAttribArray( 2 );
               gl_vertexAttribPointer( 2, elemCount, elemType, true, GLsizei(_strideInBytes), BUFFER_OFFSET( (*curAttrib)->_offset ) );
               break;
            default:;
         }
      }
      return attributes;
   }

   inline GLenum  usage() const
   {
      return isStreamable() ? GL_DYNAMIC_DRAW : GL_STATIC_DRAW;
   }

   /*----- data members -----*/
   GLuint          _id;  //!< A unique identifier
   BufferMapInfo*  _mapInfo;

};  //class OpenGLVertexBuffer


/*==============================================================================
  CLASS OpenGLGeometry
==============================================================================*/
class OpenGLGeometry:
   public Gfx::Geometry
{
public:

   /*----- methods -----*/

   OpenGLGeometry( const PrimitiveType type ):
      Geometry( type )
   {
      gl_genVertexArrays(1, &_id);
   }

   virtual ~OpenGLGeometry()
   {
      gl_deleteVertexArrays(1, &_id);
   }

   uint  getRevision() const
   {
      uint rev = 0;
      for( auto cur = _buffers.begin(); cur != _buffers.end(); ++cur )
      {
         rev += (*cur)->revision();
      }
      return rev;
   }

   void  activate() const
   {
      gl_bindVertexArray( _id );
      uint rev = getRevision();
      if( rev != _revision )
      {
         // Refresh the VAO.
         uint attributes = 0;
         BufferContainer::const_iterator curBuf = buffers().begin();
         BufferContainer::const_iterator endBuf = buffers().end();
         for( ; curBuf != endBuf; ++curBuf )
         {
            const OpenGLVertexBuffer* oglBuffer = (const OpenGLVertexBuffer*)(*curBuf).ptr();
            attributes |= oglBuffer->activate();
         }

         // Disable inactive attributes.
         uint remAttributes = Gfx::ATTRIB_TYPE_ALL & (~attributes);
         OpenGLVertexBuffer::disable( remAttributes );

         // Bind the index buffer to the VAO.
         const OpenGLIndexBuffer* indices = (const OpenGLIndexBuffer*)(indexBuffer().ptr());
         if( indices )  gl_bindBuffer( GL_ELEMENT_ARRAY_BUFFER, indices->_id );
         else           gl_bindBuffer( GL_ELEMENT_ARRAY_BUFFER, 0 );

         // Mark with the computed revision.
         _revision = rev;
      }
   }

protected:

   /*----- data members -----*/

   GLuint  _id;

}; //class OpenGLGeometry


/*==============================================================================
   CLASS OpenGLFramebuffer
==============================================================================*/

class OpenGLFramebuffer:
   public Gfx::Framebuffer
{
public:

   /*----- methods -----*/
   OpenGLFramebuffer()
   {
      gl_genFramebuffersEXT(1, &_id);
   }

   virtual ~OpenGLFramebuffer()
   {
      gl_deleteFramebuffersEXT(1, &_id);
   }

   /*----- data members -----*/
   GLuint  _id;

};  //class OpenGLFramebuffer


/*==============================================================================
   CLASS OpenGLShader
==============================================================================*/

String vs_str_default =
"#version 120\n"
"uniform mat4 gfxProjectionMatrix;\n"
"uniform mat4 gfxViewMatrix;\n"
"uniform mat4 gfxWorldMatrix;\n"
"uniform mat4 gfxWorldViewMatrix;\n"
"uniform mat4 gfxWorldViewProjectionMatrix;\n"
"uniform vec3 gfxCameraPosition;\n"
"attribute vec4 gfxTangent;\n"
"attribute vec4 gfxBinormal;\n";

String gs_str_default =
"#version 120\n"
"#extension GL_EXT_geometry_shader4 : enable\n"
"uniform mat4 gfxProjectionMatrix;\n"
"uniform mat4 gfxViewMatrix;\n"
"uniform mat4 gfxWorldMatrix;\n"
"uniform mat4 gfxWorldViewMatrix;\n"
"uniform mat4 gfxWorldViewProjectionMatrix;\n"
"uniform vec3 gfxCameraPosition;\n";

String ps_str_default =
"#version 120\n"
"uniform vec3 gfxCameraPosition;\n"
"uniform mat4 gfxWorldViewMatrix;\n"
"uniform mat4 gfxWorldMatrix;\n";

class OpenGLShader:
   public Shader
{

public:

   /*----- methods -----*/

   // Only Managers can create this object.
   OpenGLShader( ShaderType type, const String& code ):
      Shader( type )
   {
      // Compile shader with attributes as header.
      GLint size[2]      = {    0, (GLint)code.size() };
      const char* str[2] = { NULL, code.cstr() };

      switch( type )
      {
         case VERTEX_SHADER:
            size[0] = GLint(vs_str_default.size());
            str[0]  = vs_str_default.cstr();
            _id     = gl_createShader( GL_VERTEX_SHADER );
            gl_shaderSource( _id, 2, str, size );
            break;
         case GEOMETRY_SHADER:
            size[0] = GLint(gs_str_default.size());
            str[0]  = gs_str_default.cstr();
            _id     = gl_createShader( GL_GEOMETRY_SHADER );
            gl_shaderSource( _id, 2, str, size );
            break;
         case FRAGMENT_SHADER:
            size[0] = GLint(ps_str_default.size());
            str[0]  = ps_str_default.cstr();
            _id     = gl_createShader( GL_FRAGMENT_SHADER );
            gl_shaderSource( _id, 2, str, size );
            break;
         default:
            break;
      }

      gl_compileShader( _id );

      // Check for errors.
      GLint status;
      gl_getShaderiv( _id, GL_COMPILE_STATUS, &status );

      if( status != 1 )
      {
         GLchar error[2048] = "";
         gl_getShaderInfoLog( _id, sizeof(error)-1, 0, error );
         printf( "error compiling: %s\n", error );
      }
   }

   virtual ~OpenGLShader()
   {
      gl_deleteShader( _id );
   }

   /*----- data members -----*/
   GLuint  _id;  //!< A unique identifier

};


/*==============================================================================
   CLASS OpenGLProgram
==============================================================================*/

class OpenGLProgram:
   public Program
{

public:

   /*----- methods -----*/

   OpenGLProgram() :
      _dirty( true )
   {
      _id = gl_createProgram();
   }

   virtual ~OpenGLProgram()
   {
      gl_deleteProgram( _id );
   }

   virtual void addShader( const RCP<Shader>& shader )
   {
      Program::addShader( shader );
      _dirty = true;
      OpenGLShader* oglShader = (OpenGLShader*)shader.ptr();
      gl_attachShader( _id, oglShader->_id );
   }

   virtual bool removeShader( const RCP<Shader>& shader )
   {
      if( Program::removeShader( shader ) )
      {
         _dirty = true;
         OpenGLShader* oglShader = (OpenGLShader*)shader.ptr();
         gl_detachShader( _id, oglShader->_id );
         return true;
      }
      return false;
   }

   inline void compile() const
   {
      CHECK( !_shaders.empty() );
      if( _dirty )
      {
         _dirty = false;

         // Set default attributes.
         gl_bindAttribLocation( _id, 1, "gfxTangent" );
         gl_bindAttribLocation( _id, 2, "gfxBinormal" );

         if( hasGeometryShader() )
         {
            GSInputType inType;
            GSOutputType outType;
            uint maxOutVerts;
            getGSParameters( inType, outType, maxOutVerts );
            GLint temp = (GLint)maxOutVerts;
            gl_programParameteri( _id, GL_GEOMETRY_INPUT_TYPE, toGL(inType ) );
            gl_programParameteri( _id, GL_GEOMETRY_OUTPUT_TYPE, toGL(outType) );
            gl_programParameteri( _id, GL_GEOMETRY_VERTICES_OUT, temp );
         }

         // Link.
         gl_linkProgram( _id );

         // Check for errors.
         GLint status;
         gl_getProgramiv( _id, GL_LINK_STATUS, &status );
         if( status != 1 )
         {
            GLchar error[2048] = "";
            gl_getProgramInfoLog( _id, sizeof(error)-1, 0, error );
            CHECK( false );
         }

         // Find uniforms location.
         computeUniformLocation();
      }
   }

   void computeUniformLocation() const
   {
      _uniforms.clear();

      // Find uniform matrices location.
      _projLoc   = gl_getUniformLocation( _id, "gfxProjectionMatrix" );
      _viewLoc   = gl_getUniformLocation( _id, "gfxViewMatrix" );
      _worldLoc  = gl_getUniformLocation( _id, "gfxWorldMatrix" );
      _wvLoc     = gl_getUniformLocation( _id, "gfxWorldViewMatrix" );
      _wvpLoc    = gl_getUniformLocation( _id, "gfxWorldViewProjectionMatrix" );
      _camPosLoc = gl_getUniformLocation( _id, "gfxCameraPosition" );

      // Find out how many uniforms there are.
      GLint numUniforms = 0;
      gl_getProgramiv( _id, GL_ACTIVE_UNIFORMS, &numUniforms );

      // Loop over all uniforms, keeping only the relevant ones.
      GLchar uniformName[256];
      GLint arraySize;
      GLenum uniformType;

      for( GLint i = 0; i < numUniforms; ++i )
      {
         // Retrieve the uniform.
         gl_getActiveUniform( _id, i, (GLsizei)256, 0, &arraySize, &uniformType, uniformName );

         // Ignore irrelevant uniforms, and ones starting with "gfx".
         String constName( uniformName );
         if( strncmp( uniformName, "gfx", 3 ) != 0 )
         {
            // OpenGL names uniform arrays 'myVarName[0]'.
            // Keep the name intact (with trailing '[0]'), since Constants::getContant() support it.
            // The ATI driver erroneously reports N uniforms for 'myVar[N]', so only care about the first one.
            String::SizeType s = constName.find('[');
            if( s != String::npos )
            {
               if( constName[s+1] == '0' )
               {
                  // Need to add both with and without the trailing [0].
                  int loc = gl_getUniformLocation( _id, uniformName );
                  _uniforms[constName.cstr()] = loc;
                  _uniforms[constName.sub(0, s).cstr()] = loc;
               }
            }
            else
            {
               // Add the name to the hashmap.
               _uniforms[constName.cstr()] = gl_getUniformLocation( _id, uniformName );
            }
         }
      }
   }

   inline void activate( GLuint& id ) const
   {
      compile(); //only done if dirty
      if( id != _id )
      {
         gl_useProgram( _id );
         id = _id;
      }
   }

   inline int getSampler( const ConstString& name ) const
   {
      Map<ConstString,int>::Iterator it = _uniforms.find( name );
      return it != _uniforms.end() ? (*it).second : -1;
   }

   inline int getConstant( const ConstString& name ) const
   {
      Map<ConstString,int>::Iterator it = _uniforms.find( name );
      return it != _uniforms.end() ? (*it).second : -1;
   }

   /*----- data members -----*/

   GLuint                       _id;  //!< A unique identifier
   mutable GLint                _projLoc;
   mutable GLint                _viewLoc;
   mutable GLint                _worldLoc;
   mutable GLint                _wvLoc;
   mutable GLint                _wvpLoc;
   mutable GLint                _camPosLoc;
   mutable bool                 _dirty;
   mutable Map<ConstString,int> _uniforms;
};


/*==============================================================================
   CLASS OpenGLTexture
==============================================================================*/

class OpenGLTexture:
   public Gfx::Texture
{
public:

   /*----- methods -----*/
   OpenGLTexture( GLenum target );

   virtual ~OpenGLTexture();

   bool  created( const uint level = 0 ) const { return getbits(_levelCreated, level, 1) & 0x01; }
   void  setCreated( const uint level = 0 ) { _levelCreated = setbits(_levelCreated, level, 1, 0x1); }
   void  unsetCreated( const uint level = 0 ) { _levelCreated = setbits(_levelCreated, level, 1, 0x0); }

   /*----- data members -----*/
   GLuint        _id;  //!< A unique identifier
   GLenum        _oglType;
   uint          _levelCreated;  //!< One dirty bit per level to indicate whether or not the level was initialized
   mutable Gfx::TextureState  _texState;

};  //class OpenGLTexture

//------------------------------------------------------------------------------
//!
OpenGLTexture::OpenGLTexture( GLenum target ) :
   _oglType( target ),
   _levelCreated( 0 )
{
   glGenTextures(1, &_id);
   _texState.setInvalidFields();
}

//------------------------------------------------------------------------------
//!
OpenGLTexture::~OpenGLTexture
( void )
{
   glDeleteTextures(1, &_id);  //should silently ignore invalid IDs
}

UNNAMESPACE_END


/*==============================================================================
   CLASS GLManager::Cache
==============================================================================*/

class GLManager::Cache
{
public:

   /*----- methods -----*/

   Cache();
   ~Cache();

   void  invalidate();

   /*----- data members -----*/

   AlphaState    _alphaState;    //!< Current alpha states.
   ColorState    _colorState;    //!< Current color states.
   CullState     _cullState;     //!< Current cull states.
   DepthState    _depthState;    //!< Current depth states.
   OffsetState   _offsetState;   //!< Current offset states.
   StencilState  _stencilState;  //!< Current stencil states.
   GLuint        _prog;          //!< Current program id.
};

//------------------------------------------------------------------------------
//!
GLManager::Cache::Cache()
{
   invalidate();
}

//------------------------------------------------------------------------------
//!
GLManager::Cache::~Cache()
{
}

//------------------------------------------------------------------------------
//!
void
GLManager::Cache::invalidate()
{
   _alphaState.setInvalidFields();
   _colorState.setInvalidFields();
   _cullState.setInvalidFields();
   _depthState.setInvalidFields();
   _offsetState.setInvalidFields();
   _stencilState.setInvalidFields();
   _prog = 0;
}

/*==============================================================================
   CLASS GLManager
==============================================================================*/

//------------------------------------------------------------------------------
//!
GLManager::GLManager( GLContext* context ):
   Manager( context, "OpenGL" ),
   _context( context ), _curTexUnitsUsed(0)
{
   DBG_BLOCK( os_gl, "Creating GLManager" );
   _context->makeCurrent();
   init();

   _cache = new Cache();
   //printInfo();
}

//------------------------------------------------------------------------------
//!
GLManager::~GLManager()
{
   DBG_BLOCK( os_gl, "Destroying GLManager" );
   ::terminateGL();
   delete _cache;
   _oneToOneGeom = NULL;  //required because otherwise, glDeleteBuffersARB will fail (OpenGL scoped out)
}

//------------------------------------------------------------------------------
//!
void
GLManager::printInfo( TextStream& os ) const
{
   Manager::printInfo( os );

   const GLubyte* str = glGetString( GL_VENDOR );
   os << "VENDOR: " << (const char*)str << nl;

   str = glGetString( GL_RENDERER );
   os << "RENDERER: " << (const char*)str << nl;

   str = glGetString( GL_VERSION );
   os << "VERSION: " << (const char*)str << nl;

   str = glGetString( GL_EXTENSIONS );
   os << "EXTENSIONS: " << (const char*)str << nl;
}

//------------------------------------------------------------------------------
//!
void
GLManager::display()
{
   _context->swapBuffers();
}

//------------------------------------------------------------------------------
//!
float
GLManager::oneToOneOffset()
{
   return 0.0f;
}

//------------------------------------------------------------------------------
//! Screen grab.
bool
GLManager::screenGrab( uint x, uint y, uint w, uint h, void* data )
{
   glReadPixels( x, y, w, h, GL_RGBA, GL_UNSIGNED_BYTE, data );
   return true;
}

//------------------------------------------------------------------------------
//!
RCP<IndexBuffer>
GLManager::createBuffer(
   const IndexFormat  format,
   const BufferFlags  flags,
   const size_t       bufferSizeInBytes,
   const void*        data
)
{
   OpenGLIndexBuffer* ogl_buffer = new OpenGLIndexBuffer( format, flags );
   RCP<IndexBuffer> buffer( ogl_buffer );
   setData( buffer, bufferSizeInBytes, data );
   return buffer;
}

//------------------------------------------------------------------------------
//!
RCP<VertexBuffer>
GLManager::createBuffer(
   const BufferFlags  flags,
   const size_t       bufferSizeInBytes,
   const void*        data
)
{
   OpenGLVertexBuffer* ogl_buffer = new OpenGLVertexBuffer( flags );
   RCP<VertexBuffer> buffer( ogl_buffer );
   setData( buffer, bufferSizeInBytes, data );
   return buffer;
}

//------------------------------------------------------------------------------
//!
bool
GLManager::setData(
   const RCP<IndexBuffer>&  buffer,
   const size_t             sizeInBytes,
   const void*              data
)
{
   // Is there any data to copy?
   //if( !data )
   //{
   //   return false;
   //}

   OpenGLIndexBuffer* ogl_buffer = (OpenGLIndexBuffer*)buffer.ptr();
   buffer->_sizeInBytes = sizeInBytes;

   // id was already created, so just bind and send the data.
   gl_bindBuffer( GL_ELEMENT_ARRAY_BUFFER, ogl_buffer->_id ) ;
   gl_bufferData( GL_ELEMENT_ARRAY_BUFFER, buffer->_sizeInBytes, data, ogl_buffer->usage() );
   return true;
}

//------------------------------------------------------------------------------
//!
bool
GLManager::setData(
   const RCP<VertexBuffer>&  buffer,
   const size_t              sizeInBytes,
   const void*               data
)
{
   // Is there any data to copy?
   //if( !data )
   //{
   //   return false;
   //}

   OpenGLVertexBuffer* ogl_buffer = (OpenGLVertexBuffer*)buffer.ptr();
   buffer->_sizeInBytes = sizeInBytes;
   // id was already created, so just bind and send the data.
   gl_bindBuffer( GL_ARRAY_BUFFER, ogl_buffer->_id );
   gl_bufferData( GL_ARRAY_BUFFER, buffer->_sizeInBytes, data, ogl_buffer->usage() );
   return true;
}

//------------------------------------------------------------------------------
//!
void*
GLManager::map( const RCP<IndexBuffer>& buffer, const MapMode mode, const size_t offsetInBytes, const size_t sizeInBytes )
{
   OpenGLIndexBuffer* ogl_buffer = (OpenGLIndexBuffer*)buffer.ptr();
   gl_bindBuffer( GL_ELEMENT_ARRAY_BUFFER, ogl_buffer->_id );
   if( offsetInBytes == 0 && (sizeInBytes == 0 || sizeInBytes == ogl_buffer->sizeInBytes()) )
   {
      // Map the whole buffer.
      // According to http://developer.nvidia.com/object/using_VBOs.html, invalidating
      // the old data can yield better performance.
      gl_bufferData( GL_ELEMENT_ARRAY_BUFFER, ogl_buffer->_sizeInBytes, NULL, ogl_buffer->usage() );
      void* ptr = gl_mapBuffer( GL_ELEMENT_ARRAY_BUFFER, toGL(mode) );
      checkForErrors("GLManager::map(IndexBuffer) - %s\n");
      return ptr;
   }
   else
   {
      // Allocate a temporary buffer, and use gl_bufferSubData on unmap.
      CHECK( ogl_buffer->_mapInfo == NULL );
      ogl_buffer->_mapInfo = new BufferMapInfo( offsetInBytes, sizeInBytes );
      if( (mode & MAP_READ) != 0x0 )
      {
         // Need to copy data from GPU.
         gl_getBufferSubData( GL_ELEMENT_ARRAY_BUFFER, offsetInBytes, sizeInBytes, ogl_buffer->_mapInfo->data() );
      }
      checkForErrors("GLManager::map(IndexBuffer w/mapInfo) - %s\n");
      return ogl_buffer->_mapInfo->data();
   }
}

//------------------------------------------------------------------------------
//!
bool
GLManager::unmap( const RCP<IndexBuffer>& buffer )
{
   OpenGLIndexBuffer* ogl_buffer = (OpenGLIndexBuffer*)buffer.ptr();
   gl_bindBuffer( GL_ELEMENT_ARRAY_BUFFER, ogl_buffer->_id );
   bool ok;
   if( ogl_buffer->_mapInfo == NULL )
   {
      ok = (gl_unmapBuffer( GL_ELEMENT_ARRAY_BUFFER ) == GL_TRUE);
      ok &= !checkForErrors("GLManager::unmap(IndexBuffer) - %s\n");
   }
   else
   {
      gl_bufferSubData( GL_ELEMENT_ARRAY_BUFFER, ogl_buffer->_mapInfo->offsetInBytes(), ogl_buffer->_mapInfo->sizeInBytes(), ogl_buffer->_mapInfo->data() );
      delete ogl_buffer->_mapInfo;
      ogl_buffer->_mapInfo = NULL;
      ok = !checkForErrors("GLManager::unmap(IndexBuffer w/mapInfo) - %s\n");
   }
   return ok;

}

//------------------------------------------------------------------------------
//!
void*
GLManager::map( const RCP<VertexBuffer>& buffer, const MapMode mode, const size_t offsetInBytes, const size_t sizeInBytes )
{
   OpenGLVertexBuffer* ogl_buffer = (OpenGLVertexBuffer*)buffer.ptr();
   gl_bindBuffer( GL_ARRAY_BUFFER, ogl_buffer->_id );
   if( offsetInBytes == 0 && (sizeInBytes == 0 || sizeInBytes == ogl_buffer->sizeInBytes()) )
   {
      // Map the whole buffer.
      // According to http://developer.nvidia.com/object/using_VBOs.html, invalidating
      // the old data can yield better performance.
      gl_bufferData( GL_ARRAY_BUFFER, ogl_buffer->_sizeInBytes, NULL, ogl_buffer->usage() );
      void* ptr = gl_mapBuffer( GL_ARRAY_BUFFER, toGL(mode) );
      checkForErrors("GLManager::map(VertexBuffer) - %s\n");
      return ptr;
   }
   else
   {
      // Allocate a temporary buffer, and use gl_bufferSubData on unmap.
      CHECK( ogl_buffer->_mapInfo == NULL );
      ogl_buffer->_mapInfo = new BufferMapInfo( offsetInBytes, sizeInBytes );
      if( (mode & MAP_READ) != 0x0 )
      {
         // Need to copy data from GPU.
         gl_getBufferSubData( GL_ARRAY_BUFFER, offsetInBytes, sizeInBytes, ogl_buffer->_mapInfo->data() );
      }
      checkForErrors("GLManager::map(VertexBuffer w/mapInfo) - %s\n");
      return ogl_buffer->_mapInfo->data();
   }
}

//------------------------------------------------------------------------------
//!
bool
GLManager::unmap( const RCP<VertexBuffer>& buffer )
{
   OpenGLVertexBuffer* ogl_buffer = (OpenGLVertexBuffer*)buffer.ptr();
   gl_bindBuffer( GL_ARRAY_BUFFER, ogl_buffer->_id );
   bool ok;
   if( ogl_buffer->_mapInfo == NULL )
   {
      ok = (gl_unmapBuffer( GL_ARRAY_BUFFER ) == GL_TRUE);
      ok &= !checkForErrors("GLManager::unmap(VertexBuffer) - %s\n");
   }
   else
   {
      gl_bufferSubData( GL_ARRAY_BUFFER, ogl_buffer->_mapInfo->offsetInBytes(), ogl_buffer->_mapInfo->sizeInBytes(), ogl_buffer->_mapInfo->data() );
      delete ogl_buffer->_mapInfo;
      ogl_buffer->_mapInfo = NULL;
      ok = !checkForErrors("GLManager::unmap(VertexBuffer w/mapInfo) - %s\n");
   }
   return ok;
}


//------------------------------------------------------------------------------
//!
RCP<Framebuffer>
GLManager::createFramebuffer()
{
   OpenGLFramebuffer* ogl_fb = new OpenGLFramebuffer();
   return ogl_fb;
}

//------------------------------------------------------------------------------
//!
RCP<Geometry>
GLManager::createGeometry( const PrimitiveType pt )
{
   return RCP<Geometry>( new OpenGLGeometry(pt) );
}

//------------------------------------------------------------------------------
//!
RCP<Shader>
GLManager::createShader( ShaderType type, const String& code )
{
   return RCP<Shader>( new OpenGLShader( type, code ) );
}

//------------------------------------------------------------------------------
//!
RCP<Program>
GLManager::createProgram()
{
   return RCP<Program>( new OpenGLProgram() );
}

//------------------------------------------------------------------------------
//!
size_t
GLManager::getConstants(
   const RCP<Program>&          program,
   ConstantBuffer::Container&   constants,
   const Vector< Set<String> >* ignoreGroups,
   uint32_t*                    usedIgnoreGroup
)
{
   DBG_BLOCK( os_gl, "GLManager::getConstants" );

   const OpenGLProgram* oglProg = (const OpenGLProgram*)program.ptr();

   // Make sure the program is compiled before trying to access the uniform locations.
   oglProg->compile();

   // Find out how many uniforms there are.
   GLint numUniforms = 0;
   gl_getProgramiv( oglProg->_id, GL_ACTIVE_UNIFORMS, &numUniforms );
   DBG_MSG( os_gl, "OpenGLProgram #" << oglProg->_id << " has " << (int)numUniforms << " uniforms" );

   // Clear ignore groups.
   if( usedIgnoreGroup )
   {
      CHECK( ignoreGroups != NULL );
      // Align to chunks of 32 (if uint*) or 8 (if uchar*).
      uint32_t nBits = alignTo( (uint32_t)ignoreGroups->size(), (uint32_t)8*sizeof(*usedIgnoreGroup) );
      setbits( usedIgnoreGroup, 0, nBits, 0 );
   }

   // Loop over all uniforms, keeping only the relevant ones.
   GLchar uniformName[256];
   GLint arraySize;
   GLenum uniformType;

   const ConstantType invalid = (Gfx::ConstantType)-1;
   ConstantType constType;
   size_t currentOffset = 0;

   for( GLint i = 0; i < numUniforms; ++i )
   {
      // Retrieve the uniform.
      gl_getActiveUniform( oglProg->_id, i, (GLsizei)256, 0, &arraySize, &uniformType, uniformName );
      DBG_MSG( os_gl, "[" << (int)i << "] " << uniformName );

      // Set constType only on the relevant ones.
      switch( uniformType )
      {
         case GL_FLOAT     : constType = Gfx::CONST_FLOAT;  break;
         case GL_FLOAT_VEC2: constType = Gfx::CONST_FLOAT2; break;
         case GL_FLOAT_VEC3: constType = Gfx::CONST_FLOAT3; break;
         case GL_FLOAT_VEC4: constType = Gfx::CONST_FLOAT4; break;
         case GL_FLOAT_MAT2: constType = Gfx::CONST_MAT2;   break;
         case GL_FLOAT_MAT3: constType = Gfx::CONST_MAT3;   break;
         case GL_FLOAT_MAT4: constType = Gfx::CONST_MAT4;   break;

         case GL_INT:
         case GL_INT_VEC2:
         case GL_INT_VEC3:
         case GL_INT_VEC4:
         case GL_BOOL:
         case GL_BOOL_VEC2:
         case GL_BOOL_VEC3:
         case GL_BOOL_VEC4:
         case GL_FLOAT_MAT2x3:
         case GL_FLOAT_MAT2x4:
         case GL_FLOAT_MAT3x2:
         case GL_FLOAT_MAT3x4:
         case GL_FLOAT_MAT4x2:
         case GL_FLOAT_MAT4x3:
            printf( "Unsupported uniform type for '%s'\n", uniformName );
            constType = invalid;
            break;

         case GL_SAMPLER_1D:
         case GL_SAMPLER_2D:
         case GL_SAMPLER_3D:
         case GL_SAMPLER_CUBE:
         case GL_SAMPLER_1D_SHADOW:
         case GL_SAMPLER_2D_SHADOW:
            // Silently ignore samplers.
            DBG_MSG( os_gl, "Ignoring '" << uniformName << "'" );
            constType = invalid;
            break;

         default:
            printf( "Unknown uniform type for '%s'\n", uniformName );
            constType = invalid;
            break;
      } //switch( uniformType )

      // Ignore irrelevant uniforms, and ones starting with "gfx"
      String constName( uniformName );

      if( strncmp( uniformName, "gfx", 3 ) == 0 )
      {
         constType = invalid;
      }
      else if( ignoreGroups )
      {
         // OpenGL can name uniform arrays 'myVarName[0]', so compare without the trailing brackets.
         String constNameClean( constName );
         String::SizeType s = constNameClean.find('[');
         if( s != String::npos )
         {
            constNameClean.erase( s, constName.size()-s );
         }
         for( uint32_t g = 0; g < ignoreGroups->size(); ++g )
         {
            if( (*ignoreGroups)[g].has( constNameClean ) )
            {
               constType = invalid;
               if( usedIgnoreGroup )
               {
                  setbits( usedIgnoreGroup, g, 1, 1 );
               }
            }
         }
      }

      if( constType != invalid )
      {
         // OpenGL names uniform arrays 'myVarName[0]'.
         String::SizeType s = constName.find('[');
         if( s != String::npos )
         {
            // Keep the name intact (with trailing '[0]'), since Constants::getContant() support it.
            //constName.erase( s, constName.size()-s );

            // The ATI driver erroneously reports N uniforms for 'myVar[N]', so only care about the first one.
            if( constName[s+1] != '0' ) continue;
         }
         else
         {
            // Force to 0 to indicate we store data, not pointer-to-data.
            if( arraySize == 1 ) arraySize = 0;
         }
         size_t constOffset = (arraySize == 0 ? Gfx::toBytes(constType) : sizeof(void*));

         constants.pushBack(
            Gfx::ConstantBuffer::Constant(
               constName.cstr(),
               constType,
               currentOffset,
               arraySize
            )
         );
         currentOffset += constOffset;
      }
   }

   CHECK( currentOffset == Gfx::ConstantBuffer::getSize(constants) );

   return currentOffset;
}

//------------------------------------------------------------------------------
//!
RCP<ConstantBuffer>
GLManager::createConstants( const size_t size )
{
   return RCP<ConstantBuffer>( new ConstantBuffer( size ) );
}

//------------------------------------------------------------------------------
//!
RCP<ConstantBuffer>
GLManager::createConstants( const ConstantBuffer::Container& constants, size_t size )
{
   return RCP<ConstantBuffer>( new ConstantBuffer( constants, size ) );
}

//------------------------------------------------------------------------------
//!
RCP<ConstantBuffer>
GLManager::createConstants(
   const RCP<Program>&          program,
   const Vector< Set<String> >* ignoreGroups,
   uint32_t*                    usedIgnoreGroup
)
{
   ConstantBuffer::Container constants;
   size_t size = getConstants( program, constants, ignoreGroups, usedIgnoreGroup );
   if( !constants.empty() )
   {
      return RCP<ConstantBuffer>( new ConstantBuffer( constants, size ) );
   }
   else
   {
      return RCP<ConstantBuffer>( new ConstantBuffer(0) );
   }
}

//------------------------------------------------------------------------------
//!
RCP<Texture>
GLManager::create1DTexture(
   const uint            width,
   const TextureFormat   tfmt,
   const TextureChannels chOrder,
   const TextureFlags    flags
)
{
   OpenGLTexture* ogl_texture = new OpenGLTexture( GL_TEXTURE_1D );
   ogl_texture->set1D(width);
   ogl_texture->format(tfmt);
   ogl_texture->channelOrder(chOrder);
   ogl_texture->flags(flags);
   RCP<Texture> texture(ogl_texture);
   return texture;
}

//------------------------------------------------------------------------------
//!
RCP<Texture>
GLManager::create2DTexture(
   const uint            width,
   const uint            height,
   const TextureFormat   tfmt,
   const TextureChannels chOrder,
   const TextureFlags    flags
)
{
   OpenGLTexture* ogl_texture = new OpenGLTexture( GL_TEXTURE_2D );
   ogl_texture->set2D(width, height);
   ogl_texture->format(tfmt);
   ogl_texture->channelOrder(chOrder);
   if( width*height > 1 )  ogl_texture->flags(flags);
   else                    ogl_texture->flags( TextureFlags(flags & ~Gfx::TEX_FLAGS_MIPMAPPED) ); // Kill mipmapping on 1x1 textures (ATI idiosyncracy).

   GLint  internalFormat;
   GLenum format;
   GLenum type;
   toGL(tfmt, internalFormat, format, type);
   if( internalFormat == 0 || format == (GLenum)-1 || type == (GLenum)-1 )
   {
      DBG_MSG( os_gl, "Invalid format: " << (int)internalFormat << ", " << (int)format << ", " << (int)type );
      //Invalid format, OpenGL will choke
      return NULL;
   }
   if( internalFormat == GL_RGBA16F_ARB && isRenderable(flags) )
   {
      // The ATI OpenGL driver errors out when binding a GL_HALF_FLOAT_ARB texture, even with a NULL data pointer.
      type = GL_UNSIGNED_BYTE;
   }

   // Call glTexImage2D in order to bind the size+format for the driver
   glBindTexture(GL_TEXTURE_2D, ogl_texture->_id);

   // The NVIDIA driver requires to call glGenerateMipmapEXT to allocate storage *BEFORE* binding the FBO
   // while OSX won't generate the mipmap chain if min filter is GL_NEAREST
   glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, ogl_texture->isMipmapped()?GL_NEAREST_MIPMAP_NEAREST:GL_NEAREST );

   // OSX won't bind an NP2 texture if the wrap policy won't allow it
   glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
   glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );

   // Depth texture.
   if( chOrder == TEX_CHANS_Z )
   {
      internalFormat = GL_DEPTH_COMPONENT;
      format         = GL_DEPTH_COMPONENT;
      glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE_ARB, GL_COMPARE_R_TO_TEXTURE_ARB );
   }

   // Set depth comparaison.
   if( tfmt == TEX_FMT_24_8 )
   {
      glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE_ARB, GL_COMPARE_R_TO_TEXTURE_ARB );
   }


   // Create the texture (without any data)
   glTexImage2D(GL_TEXTURE_2D,
                0,  //level
                internalFormat,
                width,
                height,
                0, //border
                format,
                type,
                NULL);

   DBG_MSG( os_gl, "Created OGL texture id=" << ogl_texture->_id );

   if( ogl_texture->isMipmapped() )
   {
      DBG_MSG( os_gl, "Generating storage for mipmap chain" );
      gl_generateMipmapEXT( ogl_texture->_oglType );
   }

   RCP<Texture> texture(ogl_texture);
   return texture;
}

//------------------------------------------------------------------------------
//!
RCP<Texture>
GLManager::create3DTexture(
   const uint            width,
   const uint            height,
   const uint            depth,
   const TextureFormat   tfmt,
   const TextureChannels chOrder,
   const TextureFlags    flags
)
{
   OpenGLTexture* ogl_texture = new OpenGLTexture( GL_TEXTURE_3D );
   ogl_texture->set3D(width, height, depth);
   ogl_texture->format(tfmt);
   ogl_texture->channelOrder(chOrder);
   ogl_texture->flags(flags);
   RCP<Texture> texture(ogl_texture);
   return texture;
}

//------------------------------------------------------------------------------
//!
RCP<Texture>
GLManager::createCubeTexture(
   const uint            edgeLength,
   const TextureFormat   tfmt,
   const TextureChannels chOrder,
   const TextureFlags    flags
)
{
   OpenGLTexture* ogl_texture = new OpenGLTexture( GL_TEXTURE_CUBE_MAP );
   ogl_texture->setCubemap(edgeLength);
   ogl_texture->format(tfmt);
   ogl_texture->channelOrder(chOrder);
   ogl_texture->flags(flags);
   RCP<Texture> texture(ogl_texture);
   return texture;
}

//------------------------------------------------------------------------------
//!
bool
GLManager::setData(
   const RCP<Texture>& texture,
   const uint          level,
   const void*         data,
   const bool          skipDefinedRegionUpdate
)
{
   OpenGLTexture* ogl_texture = (OpenGLTexture*)texture.ptr();

   GLint  internalFormat;
   GLenum format;
   GLenum type;
   toGL(ogl_texture->format(), internalFormat, format, type );
   if( internalFormat == 0 || format == (GLenum)-1 || type == (GLenum)-1 )
   {
      //Invalid format, OpenGL will choke
      return false;
   }

   switch(ogl_texture->type())
   {
      case TEX_TYPE_2D:
      {
         glBindTexture(GL_TEXTURE_2D, ogl_texture->_id);
         glTexImage2D(GL_TEXTURE_2D,
                      level,
                      internalFormat,
                      ogl_texture->levelWidth(level),
                      ogl_texture->levelHeight(level),
                      0, //border
                      format,
                      type,
                      data);
         if( level == 0 && !skipDefinedRegionUpdate )
         {
            ogl_texture->definedRegionX().setRange(0, ogl_texture->width());
            ogl_texture->definedRegionY().setRange(0, ogl_texture->height());
         }
         /**
         printf("Done setting %dx%d texture ID=%d\n", ogl_texture->levelWidth(level), ogl_texture->levelHeight(level), ogl_texture->_id);
         printf("Defined range is +{%d,%d}-{%dx%d}\n",
                ogl_texture->definedOffsetX(), ogl_texture->definedOffsetY(),
                ogl_texture->definedWidth(), ogl_texture->definedHeight());
         **/
      } break;
      default:
      {
         printf("Sorry, unsupported texture type\n");
         return false;
      }
   }
   return true;
}

//------------------------------------------------------------------------------
//!
bool
GLManager::setData(
   const RCP<Texture>& texture,
   const uint          level,
   const uint          offset_x,
   const uint          width,
   const void*         data,
   const bool          skipDefinedRegionUpdate
)
{
   OpenGLTexture* ogl_texture = (OpenGLTexture*)texture.ptr();

   GLint  internalFormat;
   GLenum format;
   GLenum type;
   toGL(ogl_texture->format(), internalFormat, format, type );
   if( internalFormat == 0 || format == (GLenum)-1 || type == (GLenum)-1 )
   {
      //Invalid format, OpenGL will choke
      return false;
   }

   switch(ogl_texture->type())
   {
      case TEX_TYPE_1D:
      {
         glBindTexture(GL_TEXTURE_1D, ogl_texture->_id);
         if( !ogl_texture->created(level) )
         {
            uint level_width = ogl_texture->levelWidth(level);
            if( offset_x != 0 ||
                width != level_width )
            {
               //Need to fill the whole texture, which is done by the driver (hence, the NULL data pointer)
               glTexImage1D(GL_TEXTURE_1D,
                            level,
                            internalFormat,
                            level_width,
                            0, //border
                            format,
                            type,
                            NULL);
               //Copy the data to the GPU
               glTexSubImage1D(GL_TEXTURE_1D,
                               level,
                               offset_x,
                               width,
                               format,
                               type,
                               data);
            }
            else
            {
               //Copy the data to the GPU
               glTexImage1D(GL_TEXTURE_1D,
                            level,
                            internalFormat,
                            width,
                            0, //border
                            format,
                            type,
                            data);
            }
            ogl_texture->setCreated(level);
         }
         else
         {
            //Copy the data to the GPU
            glTexSubImage1D(GL_TEXTURE_1D,
                            level,
                            offset_x,
                            width,
                            format,
                            type,
                            data);
         }

         if( level == 0 && !skipDefinedRegionUpdate )
         {
            ogl_texture->definedRegionX().update(offset_x);
            ogl_texture->definedRegionX().update(width);
         }
         /**
         printf("Done setting region {%d}-{%d} in a %d texture ID=%d\n",
                ogl_texture->definedOffsetX(),
                ogl_texture->definedWidth(),
                ogl_texture->levelWidth(level),
                ogl_texture->_id);
         printf("Defined range is +{%d}-{%d}\n",
                ogl_texture->definedOffsetX(),
                ogl_texture->definedWidth());
         **/
      } break;
      default:
      {
         printf("Sorry, invalid texture type (should be 1D)\n");
         return false;
      }
   }
   return true;
}

//------------------------------------------------------------------------------
//!
bool
GLManager::setData(
   const RCP<Texture>& texture,
   const uint          level,
   const uint          offset_x,
   const uint          offset_y,
   const uint          width,
   const uint          height,
   const void*         data,
   const bool          skipDefinedRegionUpdate
)
{
   OpenGLTexture* ogl_texture = (OpenGLTexture*)texture.ptr();

   GLint  internalFormat;
   GLenum format;
   GLenum type;
   toGL(ogl_texture->format(), internalFormat, format, type );
   if( internalFormat == 0 || format == (GLenum)-1 || type == (GLenum)-1 )
   {
      //Invalid format, OpenGL will choke
      return false;
   }

   switch(ogl_texture->type())
   {
      case TEX_TYPE_2D:
      {
         glBindTexture(GL_TEXTURE_2D, ogl_texture->_id);
         if( !ogl_texture->created(level) )
         {
            uint level_width  = ogl_texture->levelWidth(level);
            uint level_height = ogl_texture->levelHeight(level);
            if( offset_x != 0 || offset_y != 0 ||
                width != level_width || height != level_height )
            {
               //Need to fill the whole texture, which is done by the driver (hence, the NULL data pointer)
               glTexImage2D(GL_TEXTURE_2D,
                            level,
                            internalFormat,
                            level_width,
                            level_height,
                            0, //border
                            format,
                            type,
                            NULL);
               //Copy the data to the GPU
               glTexSubImage2D(GL_TEXTURE_2D,
                               level,
                               offset_x, offset_y,
                               width, height,
                               format,
                               type,
                               data);
            }
            else
            {
               //Copy the data to the GPU
               glTexImage2D(GL_TEXTURE_2D,
                            level,
                            internalFormat,
                            width, height,
                            0, //border
                            format,
                            type,
                            data);
            }
            ogl_texture->setCreated(level);
         }
         else
         {
            //Copy the data to the GPU
            glTexSubImage2D(GL_TEXTURE_2D,
                            level,
                            offset_x, offset_y,
                            width, height,
                            format,
                            type,
                            data);
         }

         if( level == 0 && !skipDefinedRegionUpdate )
         {
            ogl_texture->definedRegionX().update(offset_x);
            ogl_texture->definedRegionX().update(width);
            ogl_texture->definedRegionY().update(offset_y);
            ogl_texture->definedRegionY().update(height);
         }
         /**
         printf("Done setting region {%d,%d}-{%dx%d} in a %dx%d texture ID=%d\n",
                ogl_texture->definedOffsetX(), ogl_texture->definedOffsetY(),
                ogl_texture->definedWidth(), ogl_texture->definedHeight(),
                ogl_texture->levelWidth(level), ogl_texture->levelHeight(level),
                ogl_texture->_id);
         printf("Defined range is +{%d,%d}-{%dx%d}\n",
                ogl_texture->definedOffsetX(), ogl_texture->definedOffsetY(),
                ogl_texture->definedWidth(), ogl_texture->definedHeight());
         **/
      } break;
      default:
      {
         printf("Sorry, invalid texture type (should be 2D)\n");
         return false;
      }
   }
   return true;
}

//------------------------------------------------------------------------------
//!
bool
GLManager::setData(
   const RCP<Texture>& texture,
   const uint          level,
   const uint          slice,
   const void*         data,
   const bool          skipDefinedRegionUpdate
)
{
   OpenGLTexture* ogl_texture = (OpenGLTexture*)texture.ptr();

   GLint  internalFormat;
   GLenum format;
   GLenum type;
   toGL(ogl_texture->format(), internalFormat, format, type );
   if( internalFormat == 0 || format == (GLenum)-1 || type == (GLenum)-1 )
   {
      //Invalid format, OpenGL will choke
      return false;
   }

   switch(ogl_texture->type())
   {
      case TEX_TYPE_2D:
      {
         glBindTexture(GL_TEXTURE_2D, ogl_texture->_id);
         glTexImage2D(GL_TEXTURE_2D,
                      level,
                      internalFormat,
                      ogl_texture->levelWidth(level),
                      ogl_texture->levelHeight(level),
                      0, //border
                      format,
                      type,
                      data);
         if( level == 0 && !skipDefinedRegionUpdate )
         {
            ogl_texture->definedRegionX().setRange(0, ogl_texture->width());
            ogl_texture->definedRegionY().setRange(0, ogl_texture->height());
         }
         /**
         printf("Done setting %dx%d texture ID=%d\n", ogl_texture->levelWidth(level), ogl_texture->levelHeight(level), ogl_texture->_id);
         printf("Defined range is +{%d,%d}-{%dx%d}\n",
                ogl_texture->definedOffsetX(), ogl_texture->definedOffsetY(),
                ogl_texture->definedWidth(), ogl_texture->definedHeight());
         **/
      } break;
      case TEX_TYPE_2D_ARRAY:
      {
         glBindTexture(GL_TEXTURE_2D_ARRAY, ogl_texture->_id);
         gl_texImage3D(GL_TEXTURE_2D_ARRAY,
                      level,
                      internalFormat,
                      ogl_texture->levelWidth(level),
                      ogl_texture->levelHeight(level),
                      slice,
                      0, //border
                      format,
                      type,
                      data);
         if( level == 0 && !skipDefinedRegionUpdate )
         {
            ogl_texture->definedRegionX().setRange(0, ogl_texture->width());
            ogl_texture->definedRegionY().setRange(0, ogl_texture->height());
         }
         /**
         printf("Done setting %dx%d array texture ID=%d\n", ogl_texture->levelWidth(level), ogl_texture->levelHeight(level), ogl_texture->_id);
         printf("Defined range is +{%d,%d}-{%dx%d}\n",
                ogl_texture->definedOffsetX(), ogl_texture->definedOffsetY(),
                ogl_texture->definedWidth(), ogl_texture->definedHeight());
         **/
      } break;
      case TEX_TYPE_CUBEMAP:
      {
         glBindTexture(GL_TEXTURE_CUBE_MAP, ogl_texture->_id);
         GLenum target = toGLCubemapTarget( slice );
         glTexImage2D(target,
                      level,
                      internalFormat,
                      ogl_texture->levelWidth(level),
                      ogl_texture->levelHeight(level),
                      0, //border
                      format,
                      type,
                      data);
         if( level == 0 && !skipDefinedRegionUpdate )
         {
            ogl_texture->definedRegionX().setRange(0, ogl_texture->width());
            ogl_texture->definedRegionY().setRange(0, ogl_texture->height());
         }
         /**
         printf("Done setting %dx%d cube texture ID=%d\n", ogl_texture->levelWidth(level), ogl_texture->levelHeight(level), ogl_texture->_id);
         printf("Defined range is +{%d,%d}-{%dx%d}\n",
                ogl_texture->definedOffsetX(), ogl_texture->definedOffsetY(),
                ogl_texture->definedWidth(), ogl_texture->definedHeight());
         **/
      } break;
      default:
      {
         printf("Sorry, unsupported texture type\n");
         return false;
      }
   }
   return true;
}

//------------------------------------------------------------------------------
//!
bool
GLManager::setData(
   const RCP<Texture>& texture,
   const uint          level,
   const uint          slice,
   const uint          offset_x,
   const uint          offset_y,
   const uint          width,
   const uint          height,
   const void*         data,
   const bool          skipDefinedRegionUpdate
)
{
   OpenGLTexture* ogl_texture = (OpenGLTexture*)texture.ptr();

   GLint  internalFormat;
   GLenum format;
   GLenum type;
   toGL(ogl_texture->format(), internalFormat, format, type );
   if( internalFormat == 0 || format == (GLenum)-1 || type == (GLenum)-1 )
   {
      //Invalid format, OpenGL will choke
      return false;
   }

   switch(ogl_texture->type())
   {
      case TEX_TYPE_2D:
      {
         glBindTexture(GL_TEXTURE_2D, ogl_texture->_id);
         if( !ogl_texture->created(level) )
         {
            uint level_width  = ogl_texture->levelWidth(level);
            uint level_height = ogl_texture->levelHeight(level);
            if( offset_x != 0 || offset_y != 0 ||
                width != level_width || height != level_height )
            {
               //Need to fill the whole texture, which is done by the driver (hence, the NULL data pointer)
               glTexImage2D(GL_TEXTURE_2D,
                            level,
                            internalFormat,
                            level_width,
                            level_height,
                            0, //border
                            format,
                            type,
                            NULL);
               //Copy the data to the GPU
               glTexSubImage2D(GL_TEXTURE_2D,
                               level,
                               offset_x, offset_y,
                               width, height,
                               format,
                               type,
                               data);
            }
            else
            {
               //Copy the data to the GPU
               glTexImage2D(GL_TEXTURE_2D,
                            level,
                            internalFormat,
                            width, height,
                            0, //border
                            format,
                            type,
                            data);
            }
            ogl_texture->setCreated(level);
         }
         else
         {
            //Copy the data to the GPU
            glTexSubImage2D(GL_TEXTURE_2D,
                            level,
                            offset_x, offset_y,
                            width, height,
                            format,
                            type,
                            data);
         }

         if( level == 0 && !skipDefinedRegionUpdate )
         {
            ogl_texture->definedRegionX().update(offset_x);
            ogl_texture->definedRegionX().update(width);
            ogl_texture->definedRegionY().update(offset_y);
            ogl_texture->definedRegionY().update(height);
         }
         /**
         printf("Done setting region {%d,%d}-{%dx%d} in a %dx%d texture ID=%d\n",
                ogl_texture->definedOffsetX(), ogl_texture->definedOffsetY(),
                ogl_texture->definedWidth(), ogl_texture->definedHeight(),
                ogl_texture->levelWidth(level), ogl_texture->levelHeight(level),
                ogl_texture->_id);
         printf("Defined range is +{%d,%d}-{%dx%d}\n",
                ogl_texture->definedOffsetX(), ogl_texture->definedOffsetY(),
                ogl_texture->definedWidth(), ogl_texture->definedHeight());
         **/
      } break;
      case TEX_TYPE_2D_ARRAY:
      {
         glBindTexture(GL_TEXTURE_2D_ARRAY, ogl_texture->_id);
         if( !ogl_texture->created(level) )
         {
            uint level_width  = ogl_texture->levelWidth(level);
            uint level_height = ogl_texture->levelHeight(level);
            uint depth        = ogl_texture->depth();
            if( offset_x != 0 || offset_y != 0 ||
                width != level_width || height != level_height )
            {
               //Need to fill the whole texture, which is done by the driver (hence, the NULL data pointer)
               gl_texImage3D(GL_TEXTURE_2D_ARRAY,
                             level,
                             internalFormat,
                             level_width,
                             level_height,
                             depth,
                             0, //border
                             format,
                             type,
                             NULL);
               //Copy the data to the GPU
               gl_texSubImage3D(GL_TEXTURE_2D_ARRAY,
                                level,
                                offset_x, offset_y, slice,
                                width, height, 1,
                                format,
                                type,
                                data);
            }
            else
            {
               //Copy the data to the GPU
               gl_texSubImage3D(GL_TEXTURE_2D_ARRAY,
                                level,
                                offset_x, offset_y, slice,
                                width, height, 1,
                                format,
                                type,
                                data);
            }
            ogl_texture->setCreated(level);
         }
         else
         {
            //Copy the data to the GPU
            gl_texSubImage3D(GL_TEXTURE_2D_ARRAY,
                             level,
                             offset_x, offset_y, slice,
                             width, height, 1,
                             format,
                             type,
                             data);
         }

         if( level == 0 && !skipDefinedRegionUpdate )
         {
            ogl_texture->definedRegionX().update(offset_x);
            ogl_texture->definedRegionX().update(width);
            ogl_texture->definedRegionY().update(offset_y);
            ogl_texture->definedRegionY().update(height);
         }
         /**
         printf("Done setting region {%d,%d}-{%dx%d} in a %dx%d array texture ID=%d\n",
                ogl_texture->definedOffsetX(), ogl_texture->definedOffsetY(),
                ogl_texture->definedWidth(), ogl_texture->definedHeight(),
                ogl_texture->levelWidth(level), ogl_texture->levelHeight(level),
                ogl_texture->_id);
         printf("Defined range is +{%d,%d}-{%dx%d}\n",
                ogl_texture->definedOffsetX(), ogl_texture->definedOffsetY(),
                ogl_texture->definedWidth(), ogl_texture->definedHeight());
         **/
      } break;
      case TEX_TYPE_CUBEMAP:
      {
         GLuint target = toGLCubemapTarget( slice );
         glBindTexture(GL_TEXTURE_CUBE_MAP, ogl_texture->_id);
         if( !ogl_texture->created(level) )
         {
            uint level_width  = ogl_texture->levelWidth(level);
            uint level_height = ogl_texture->levelHeight(level);
            if( offset_x != 0 || offset_y != 0 ||
                width != level_width || height != level_height )
            {
               //Need to fill the whole texture, which is done by the driver (hence, the NULL data pointer)
               glTexImage2D(target,
                            level,
                            internalFormat,
                            level_width,
                            level_height,
                            0, //border
                            format,
                            type,
                            NULL);
               //Copy the data to the GPU
               glTexSubImage2D(target,
                               level,
                               offset_x, offset_y,
                               width, height,
                               format,
                               type,
                               data);
            }
            else
            {
               //Copy the data to the GPU
               glTexImage2D(target,
                            level,
                            internalFormat,
                            width, height,
                            0, //border
                            format,
                            type,
                            data);
            }
            ogl_texture->setCreated(level);
         }
         else
         {
            //Copy the data to the GPU
            glTexSubImage2D(target,
                            level,
                            offset_x, offset_y,
                            width, height,
                            format,
                            type,
                            data);
         }

         if( level == 0 && !skipDefinedRegionUpdate )
         {
            ogl_texture->definedRegionX().update(offset_x);
            ogl_texture->definedRegionX().update(width);
            ogl_texture->definedRegionY().update(offset_y);
            ogl_texture->definedRegionY().update(height);
         }
         /**
         printf("Done setting region {%d,%d}-{%dx%d} in a %dx%d cube texture ID=%d\n",
                ogl_texture->definedOffsetX(), ogl_texture->definedOffsetY(),
                ogl_texture->definedWidth(), ogl_texture->definedHeight(),
                ogl_texture->levelWidth(level), ogl_texture->levelHeight(level),
                ogl_texture->_id);
         printf("Defined range is +{%d,%d}-{%dx%d}\n",
                ogl_texture->definedOffsetX(), ogl_texture->definedOffsetY(),
                ogl_texture->definedWidth(), ogl_texture->definedHeight());
         **/
      } break;
      default:
      {
         printf("Sorry, invalid texture type (should be 2D/2Da/Cube)\n");
         return false;
      }
   }
   return true;
}

//------------------------------------------------------------------------------
//!
bool
GLManager::setData(
   const RCP<Texture>& /*texture*/,
   const uint          /*level*/,
   const uint          /*offset_x*/,
   const uint          /*offset_y*/,
   const uint          /*offset_z*/,
   const uint          /*width*/,
   const uint          /*height*/,
   const uint          /*depth*/,
   const void*         /*data*/,
   const bool          /*skipDefinedRegionUpdate*/
)
{
#if 0
   OpenGLTexture* ogl_texture = (OpenGLTexture*)texture.ptr();

   GLint  internalFormat;
   GLenum format;
   GLenum type;
   toGL(ogl_texture->format(), internalFormat, format, type );
   if( internalFormat == 0 || format == (GLenum)-1 || type == (GLenum)-1 )
   {
      //Invalid format, OpenGL will choke
      return false;
   }

   switch(ogl_texture->type())
   {
      case TEX_TYPE_3D:
      {
         glBindTexture(GL_TEXTURE_3D, ogl_texture->_id);
         if( !ogl_texture->created(level) )
         {
            uint level_width  = ogl_texture->levelWidth(level);
            uint level_height = ogl_texture->levelHeight(level);
            uint level_depth  = ogl_texture->levelDepth(level);
            if( offset_x != 0 || offset_y != 0 || offset_z != 0 ||
                width != level_width || height != level_height || depth != level_depth )
            {
               //Need to fill the whole texture, which is done by the driver (hence, the NULL data pointer)
               glTexImage3D(GL_TEXTURE_3D,
                            level,
                            internalFormat,
                            level_width,
                            level_height,
                            level_depth,
                            0, //border
                            format,
                            type,
                            NULL);
               //Copy the data to the GPU
               glTexSubImage3D(GL_TEXTURE_3D,
                               level,
                               offset_x, offset_y, offset_z,
                               width, height, depth,
                               format,
                               type,
                               data);
            }
            else
            {
               //Copy the data to the GPU
               glTexImage3D(GL_TEXTURE_3D,
                            level,
                            internalFormat,
                            width,
                            height,
                            depth,
                            0, //border
                            format,
                            type,
                            NULL);
            }
            ogl_texture->setCreated(level);
         }
         else
         {
            //Copy the data to the GPU
            glTexSubImage3D(GL_TEXTURE_3D,
                            level,
                            offset_x, offset_y, offset_z,
                            width, height, depth,
                            format,
                            type,
                            data);
         }

         if( level == 0 && !skipDefinedRegionUpdate )
         {
            ogl_texture->definedRegionX().update(offset_x);
            ogl_texture->definedRegionX().update(width);
            ogl_texture->definedRegionY().update(offset_y);
            ogl_texture->definedRegionY().update(height);
            ogl_texture->definedRegionZ().update(offset_z);
            ogl_texture->definedRegionZ().update(depth);
         }
         /**
         printf("Done setting region {%d,%d,%d}-{%dx%dx%d} in a %dx%dx%d texture ID=%d\n",
                ogl_texture->definedOffsetX(), ogl_texture->definedOffsetY(), ogl_texture->definedOffsetZ(),
                ogl_texture->definedWidth(), ogl_texture->definedHeight(), ogl_texture->gdefinedDepth(),
                ogl_texture->levelWidth(level), ogl_texture->levelHeight(level), ogl_texture->levelDepth(level),
                ogl_texture->_id);
         printf("Defined range is +{%d,%d,%d}-{%dx%dx%d}\n",
                ogl_texture->definedOffsetX(), ogl_texture->definedOffsetY(), ogl_texture->definedOffsetZ(),
                ogl_texture->definedWidth(), ogl_texture->definedHeight(), ogl_texture->definedDepth());
         **/
      } break;
      default:
      {
         printf("Sorry, invalid texture type (should be 3D)\n");
         return false;
      }
   }
   return true;
#else
   CHECK(false);
   return false;
#endif
}

//------------------------------------------------------------------------------
//! Generate the mipmap chain using the base map
bool
GLManager::generateMipmaps( const RCP<Texture>& texture )
{
   DBG_BLOCK( os_gl, "GLManager::generateMipmaps()" );
   if( texture->isMipmapped() )
   {
      DBG_BLOCK( os_gl, "Generating for " << toString(texture->type()) );
      OpenGLTexture* ogl_texture = (OpenGLTexture*)texture.ptr();
      glBindTexture( ogl_texture->_oglType, ogl_texture->_id );
      gl_generateMipmapEXT( ogl_texture->_oglType );
      return !checkForErrors("GLManager::generateMipmaps - %s\n");
   }
   else
   {
      DBG_MSG( os_gl, "ERROR - Trying to generate mipmaps on non-mipmapped texture" );
      return false;
   }
}

//------------------------------------------------------------------------------
//!
bool
GLManager::setFramebuffer( const Framebuffer* fb )
{
   DBG_BLOCK( os_gl, "GLManager::setFramebuffer" );
   const OpenGLFramebuffer* ogl_fb = (const OpenGLFramebuffer*)fb;

   if( ogl_fb )
   {
      // Use specified buffers
      DBG_MSG( os_gl, "Binding FBO #" << ogl_fb->_id );
      gl_bindFramebufferEXT(GL_FRAMEBUFFER_EXT, ogl_fb->_id);

      // Depth buffer.
      if( ogl_fb->_depthBuffer.isValid() )
      {
         const OpenGLTexture* ogl_depth = (const OpenGLTexture*)ogl_fb->_depthBuffer.ptr();
         uint level = ogl_fb->_depthBufferLevel;

         if( ogl_fb->isDirty( Framebuffer::FB_BUFFER_BIT_DEPTH ) )
         {
            gl_framebufferTexture2DEXT(
               GL_FRAMEBUFFER_EXT,
               GL_DEPTH_ATTACHMENT_EXT,
               GL_TEXTURE_2D, ogl_depth->_id, level
            );

            ogl_fb->unsetDirty( Framebuffer::FB_BUFFER_BIT_DEPTH );
            DBG_MSG( os_gl, "Setting depth buffer from texture #" << ogl_depth->_id << " l=" << level );
         }

         _curWidth  = ogl_depth->levelWidth( level );
         _curHeight = ogl_depth->levelHeight( level );
      }

      // Stencil buffer.
      if( ogl_fb->_stencilBuffer.isValid() && ogl_fb->isDirty( Framebuffer::FB_BUFFER_BIT_STENCIL ) )
      {
         const OpenGLTexture* ogl_stencil = (const OpenGLTexture*)ogl_fb->_stencilBuffer.ptr();
         uint level = ogl_fb->_stencilBufferLevel;
         gl_framebufferTexture2DEXT(
            GL_FRAMEBUFFER_EXT,
            GL_STENCIL_ATTACHMENT_EXT,
            GL_TEXTURE_2D, ogl_stencil->_id, level
         );
         ogl_fb->unsetDirty( Framebuffer::FB_BUFFER_BIT_STENCIL );
         DBG_MSG( os_gl, "Setting stencil buffer from texture #" << ogl_stencil->_id << " l=" << level );
      }

      // Color buffer.
      if( ogl_fb->_colorBuffer.isValid() )
      {
         const OpenGLTexture* ogl_color = (const OpenGLTexture*)ogl_fb->_colorBuffer.ptr();
         uint level = ogl_fb->_colorBufferLevel;

         if( ogl_fb->isDirty( Framebuffer::FB_BUFFER_BIT_COLOR ) )
         {
            gl_framebufferTexture2DEXT(
               GL_FRAMEBUFFER_EXT,
               GL_COLOR_ATTACHMENT0_EXT,
               GL_TEXTURE_2D, ogl_color->_id, level
            );
            ogl_fb->unsetDirty(Framebuffer::FB_BUFFER_BIT_COLOR);
            DBG_MSG( os_gl, "Setting color buffer from texture #" << ogl_color->_id << " l=" << level );

            glDrawBuffer( GL_COLOR_ATTACHMENT0_EXT );
         }
         _curWidth  = ogl_color->levelWidth( level );
         _curHeight = ogl_color->levelHeight( level );
      }
      else
      {
         glDrawBuffer( GL_NONE );
         glReadBuffer( GL_NONE );
      }

#ifdef _DEBUG
      if( !checkFramebufferStatus() )
      {
         return false;
      }
#endif

      _doingRTT = true;
   }
   else
   {
      DBG_MSG( os_gl, "Unbinding FBO" );
      // Use backbuffer as render target
      gl_bindFramebufferEXT( GL_FRAMEBUFFER_EXT, 0 );

      _curWidth  = _width;
      _curHeight = _height;

      _doingRTT = false;
   }

#ifdef _DEBUG
   if( checkForErrors("GLManager::setFramebuffer - %s\n") )
   {
      if( ogl_fb )
      {
         StdErr << "Color format: " << (ogl_fb->_colorBuffer.isValid() ? String(ogl_fb->_colorBuffer->format()) : String("<null>")) << nl;
         StdErr << "Depth format: " << (ogl_fb->_depthBuffer.isValid() ? String(ogl_fb->_depthBuffer->format()) : String("<null>")) << nl;
      }
      else
      {
         StdErr << "<null> framebuffer" << nl;
      }
   }
#endif

   return true;
}

//------------------------------------------------------------------------------
//!
bool
GLManager::executeGeometry(
   const Geometry*     geom,
   const Program*      prog,
   const ConstantList* constants,
   const SamplerList*  samp,
   const float*        matrices[],
   const float*        camPosition,
   const uint*         range
)
{
   DBG_BLOCK( os_gl, "GLManager::executeGeometry" );

   if( geom == NULL )
   {
      DBG_MSG( os_gl, "Empty geometry...  Aborting." );
      return false;
   }
   if( prog == NULL )
   {
      DBG_MSG( os_gl, "Empty program...  Aborting." );
      return false;
   }

   // Shader.
   const OpenGLProgram* oglProg = (const OpenGLProgram*)prog;
   oglProg->activate( _cache->_prog );

   // Matrices.
   if( oglProg->_projLoc != -1 )
   {
      gl_uniformMatrix4fv( oglProg->_projLoc, 1, GL_FALSE, matrices[0] );
   }
   if( oglProg->_viewLoc != -1 )
   {
      gl_uniformMatrix4fv( oglProg->_viewLoc, 1, GL_FALSE, matrices[1] );
   }
   if( oglProg->_worldLoc != -1 )
   {
      gl_uniformMatrix4fv( oglProg->_worldLoc, 1, GL_FALSE, matrices[2] );
   }
   if( oglProg->_wvLoc != -1 )
   {
      Mat4f wv = (*(const Mat4f*)matrices[1]) * (*(const Mat4f*)matrices[2]);
      gl_uniformMatrix4fv( oglProg->_wvLoc, 1, GL_FALSE, wv.ptr() );
   }
   if( oglProg->_wvpLoc != -1 )
   {
      Mat4f wvp = (*(const Mat4f*)matrices[3]) * (*(const Mat4f*)matrices[2]);
      gl_uniformMatrix4fv( oglProg->_wvpLoc, 1, GL_FALSE, wvp.ptr() );
   }

   // Camera.
   if( oglProg->_camPosLoc != -1 )
   {
      if( camPosition != NULL )
      {
         gl_uniform3fv( oglProg->_camPosLoc, 1, camPosition );
      }
      else
      {
         printf("ERROR - Camera position required by shader, but not defined in the pass\n");
      }
   }

   // Constants.
   if( constants )
   {
      DBG_MSG( os_gl, constants->size() << " constants" );
      for( uint i = 0; i < constants->size(); ++i )
      {
         const uchar* cbuffer                       = (*constants)[i]->_buffer;
         const ConstantBuffer::Container& constDesc = (*constants)[i]->_constants;

         for( uint c = 0; c < constDesc.size(); ++c )
         {
            DBG_MSG( os_gl, "Handling constant: " << constDesc[c].name().cstr() );
            GLint loc = oglProg->getConstant( constDesc[c].name() );
            DBG_MSG( os_gl, "const[" << c << "]: '" << constDesc[c].name().cstr() << "' loc=" << (int)loc );
            if( loc == -1 )  continue;
            GLint count = (GLint)constDesc[c].count();

            const float* data = constDesc[c].count() == 0 ?
               (const float*)(cbuffer + constDesc[c].offset()) :
              *(const float**)(cbuffer + constDesc[c].offset()); // Special pointer case.

            // OpenGL needs a count of 1.
            if( count == 0 )  count = 1;

            switch( constDesc[c].type() )
            {
               case CONST_FLOAT : gl_uniform1fv( loc, count, data ); break;
               case CONST_FLOAT2: gl_uniform2fv( loc, count, data ); break;
               case CONST_FLOAT3: gl_uniform3fv( loc, count, data ); break;
               case CONST_FLOAT4: gl_uniform4fv( loc, count, data ); break;
               case CONST_MAT2  : gl_uniformMatrix2fv( loc, count, GL_FALSE, data ); break;
               case CONST_MAT3  : gl_uniformMatrix3fv( loc, count, GL_FALSE, data ); break;
               case CONST_MAT4  : gl_uniformMatrix4fv( loc, count, GL_FALSE, data ); break;
            }
         }
      }
   }

   // Textures.
   uint curTexUnit = 0;
   if( samp )
   {
      for( uint i = 0; i < samp->size(); ++i )
      {
         const Sampler* s            = (*samp)[i].ptr();
         int samplerID               = oglProg->getSampler( s->name() );
         const OpenGLTexture* oglTex = (const OpenGLTexture*)s->texture().ptr();

         if( samplerID == -1 )
         {
            DBG_MSG( os_gl, "Sampler '" << s->name().cstr() << "' is not in the program... Skipped." );
            continue;
         }

         gl_activeTexture( GL_TEXTURE0 + curTexUnit );
         gl_uniform1i( samplerID, curTexUnit );
         glBindTexture( oglTex->_oglType, oglTex->_id );

         DBG_MSG( os_gl, "Using texture #" << oglTex->_id << " in unit #" << curTexUnit );
         ++curTexUnit;

         // Set texture parameters.
         const TextureState& ts = s->state();
         uint tsDiff            = ts.compare( oglTex->_texState );
         if( oglTex->_texState != ts )
         {
            GLint maxA;
            GLenum magF, minF;
            toGLFilters( ts, magF, minF, maxA );
            switch( tsDiff )
            {
               case 1:
                  glTexParameteri( oglTex->_oglType, GL_TEXTURE_MAG_FILTER, magF );
                  glTexParameteri( oglTex->_oglType, GL_TEXTURE_MIN_FILTER, minF );
                  break;
               case 2:
                  glTexParameteri( oglTex->_oglType, GL_TEXTURE_WRAP_S, toGL( ts.clampX() ) );
                  glTexParameteri( oglTex->_oglType, GL_TEXTURE_WRAP_T, toGL( ts.clampY() ) );
                  glTexParameteri( oglTex->_oglType, GL_TEXTURE_WRAP_R, toGL( ts.clampZ() ) );
                  break;
               case 3:
                  glTexParameteri( oglTex->_oglType, GL_TEXTURE_MAG_FILTER, magF );
                  glTexParameteri( oglTex->_oglType, GL_TEXTURE_MIN_FILTER, minF );
                  glTexParameteri( oglTex->_oglType, GL_TEXTURE_WRAP_S, toGL( ts.clampX() ) );
                  glTexParameteri( oglTex->_oglType, GL_TEXTURE_WRAP_T, toGL( ts.clampY() ) );
                  glTexParameteri( oglTex->_oglType, GL_TEXTURE_WRAP_R, toGL( ts.clampZ() ) );
                  break;
               default:
                  glTexParameteri( oglTex->_oglType, GL_TEXTURE_MAG_FILTER, magF );
                  glTexParameteri( oglTex->_oglType, GL_TEXTURE_MIN_FILTER, minF );
                  glTexParameteri( oglTex->_oglType, GL_TEXTURE_MAX_ANISOTROPY, maxA );
                  glTexParameteri( oglTex->_oglType, GL_TEXTURE_WRAP_S, toGL( ts.clampX() ) );
                  glTexParameteri( oglTex->_oglType, GL_TEXTURE_WRAP_T, toGL( ts.clampY() ) );
                  glTexParameteri( oglTex->_oglType, GL_TEXTURE_WRAP_R, toGL( ts.clampZ() ) );
                  glTexParameteri( oglTex->_oglType, GL_TEXTURE_BASE_LEVEL, ts.baseLevel() );
                  glTexParameteri( oglTex->_oglType, GL_TEXTURE_MAX_LEVEL, ts.lastLevel() );
                  glTexParameterf( oglTex->_oglType, GL_TEXTURE_LOD_BIAS, ts.LODBias() );
            }
            oglTex->_texState = ts;
         }
      }
   }
   // Deactive extraneous texture units.
   for( uint i = curTexUnit; i < _curTexUnitsUsed; ++i )
   {
      gl_activeTexture( GL_TEXTURE0 + i );
      glBindTexture( GL_TEXTURE_2D, 0 );
   }
   // Update current number of active texture units.
   _curTexUnitsUsed = curTexUnit;

   // Geometry.
   const OpenGLGeometry* oglGeom = (const OpenGLGeometry*)geom;
   oglGeom->activate();

   // Draw primitives.
   const OpenGLIndexBuffer* indexBuffer = (const OpenGLIndexBuffer*)(geom->indexBuffer().ptr());
   if( indexBuffer )
   {
      // With an index buffer.
      if( range )
      {
         glDrawElements(
            toGL( geom->primitiveType() ),
            range[1],
            toGL( indexBuffer->format() ),
            (const void*)(toBytes( indexBuffer->format() ) * range[0])
         );
      }
      else
      {
         glDrawElements(
            toGL( geom->primitiveType() ),
            GLsizei(indexBuffer->numIndices()),
            toGL( indexBuffer->format() ),
            0
         );
      }
   }
   else
   {
      // With incrementing indices.
      if( range )
      {
         glDrawArrays(
            toGL( geom->primitiveType() ),
            GLint(range[0]),
            GLsizei(range[1])
         );
      }
      else
      {
         glDrawArrays(
            toGL( geom->primitiveType() ),
            GLint(0),
            GLsizei(oglGeom->buffer(0)->numVertices())
         );
      }
   }

   return true;
}

//------------------------------------------------------------------------------
//!
bool
GLManager::render( const Pass& pass )
{
   DBG_BLOCK( os_gl, "GLManager::render" );

   bool ok = true;
   bool wire = false;

   ok &= setFramebuffer( pass._framebuffer.ptr() );

   // Set viewport and scissor.
   glViewport( 0, 0, _curWidth, _curHeight );
   glScissor( 0, 0, _curWidth, _curHeight );

#if 0
   printf("Custom OpenGL rendering %dx%d\n", _curWidth, _curHeight);
   glDisable(GL_DEPTH_TEST);
   glMatrixMode(GL_MODELVIEW);
   glLoadIdentity();
   glMatrixMode(GL_PROJECTION);
   glLoadIdentity();
   glClearColor(1, 0, 0, 1);
   glClear( GL_COLOR_BUFFER_BIT );
   glColor3f(0, 1, 0);
   glBegin(GL_TRIANGLES);
      glVertex2f(0, 0);
      glVertex2f(1, 0);
      glVertex2f(1, 1);
   glEnd();
   return true;
#endif

   // Set to default render states.
   applyState( alphaStateDef, _cache->_alphaState );
   applyState( colorStateDef, _cache->_colorState );
   applyState( cullStateDef, _cache->_cullState );
   applyState( depthStateDef, _cache->_depthState );
   applyState( offsetStateDef, _cache->_offsetState );
   applyState( stencilStateDef, _cache->_stencilState );

   // Clear buffers.
   if( pass._clearBuffers )
   {
      glClearColor( pass._color[0], pass._color[1], pass._color[2], pass._color[3] );
      glClearDepth( pass._z );
      glClearStencil( pass._stencil );
      switch( pass._clearBuffers )
      {
         case CLEAR_COLOR  : glClear( GL_COLOR_BUFFER_BIT ); break;
         case CLEAR_DEPTH  : glClear( GL_DEPTH_BUFFER_BIT ); break;
         case CLEAR_STENCIL: glClear( GL_STENCIL_BUFFER_BIT ); break;
         case CLEAR_COLOR | CLEAR_DEPTH   : glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT ); break;
         case CLEAR_COLOR | CLEAR_STENCIL : glClear( GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT ); break;
         case CLEAR_DEPTH | CLEAR_STENCIL : glClear( GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT ); break;
         case CLEAR_ALL: glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT ); break;
         default: DBG_MSG( os_gl, "Invalid _clearBuffers" ); break;
      }
   }

   // Keep current information in the pass.
   Mat4f viewProj( Mat4f::identity() );

   const Geometry*     curGeom  = 0;
   const Program*      curProg  = 0;
   const SamplerList*  curSamp  = 0;
   const ConstantList* curConst = 0;
   const float* curMatrices[4]  = { identityMatrix, identityMatrix, identityMatrix, viewProj.ptr() };
   const float* curCamPosition  = 0;
   const uint*           range  = 0;

   // Used only for PASS_CMD_RENDER.
   const int*       curScissor  = 0;
   const int*      curViewport  = 0;

   // Execute all command in the pass.
   Pass::CommandContainer::ConstIterator curCmd =  pass._commands.begin();
   Pass::CommandContainer::ConstIterator endCmd =  pass._commands.end();
   while( curCmd != endCmd )
   {
      switch( (*curCmd)._id )
      {
         case Pass::PASS_CMD_NOOP:
         {
         } break;
         case Pass::PASS_CMD_MESSAGE:
         {
            DBG( const char* msg = (*curCmd)._cPtr );
            DBG_MSG( os_gl, msg );
         } break;
         case Pass::PASS_CMD_RENDER:
         {
            // Save state (we reuse many variables existing in this scope).
            const Cache savedCache = *_cache;

            // Render node.
            const RCP<RenderNode> rn = (RenderNode*)(*curCmd)._data.ptr(); // Sneaky const_cast at the same time.
            Manager::render( rn );

            // Restore various states.
            // - Framebuffer.
            ok &= setFramebuffer( pass._framebuffer.ptr() );
            // - Viewport
            if( curViewport != 0 )
            {
               glViewport( curViewport[0], curViewport[1], curViewport[2], curViewport[3] );
            }
            else
            {
               glViewport( 0, 0, _curWidth, _curHeight );
            }
            // - Scissor.
            if( curScissor != 0 )
            {
               glScissor( curScissor[0], curScissor[1], curScissor[2], curScissor[3] );
            }
            else
            {
               glScissor( 0, 0, _curWidth, _curHeight );
            }
            // - State.
            applyState( savedCache._alphaState,   _cache->_alphaState   );
            applyState( savedCache._colorState,   _cache->_colorState   );
            applyState( savedCache._cullState,    _cache->_cullState   );
            applyState( savedCache._depthState,   _cache->_depthState   );
            applyState( savedCache._offsetState,  _cache->_offsetState   );
            applyState( savedCache._stencilState, _cache->_stencilState );
            // (no need to set clear color, as we have no CLEAR commands)
            // - Wireframe.
            if( wire )
            {
               glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
            }
            else
            {
               glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
            }
         } break;
         case Pass::PASS_CMD_SET_ALPHA_STATE:
         {
            applyState( *(const AlphaState*)(*curCmd)._data.ptr(), _cache->_alphaState );
         } break;
         case Pass::PASS_CMD_SET_COLOR_STATE:
         {
            applyState( *(const ColorState*)(*curCmd)._data.ptr(), _cache->_colorState );
         } break;
         case Pass::PASS_CMD_SET_CULL_STATE:
         {
            applyState( *(const CullState*)(*curCmd)._data.ptr(), _cache->_cullState );
         } break;
         case Pass::PASS_CMD_SET_DEPTH_STATE:
         {
            applyState( *(const DepthState*)(*curCmd)._data.ptr(), _cache->_depthState );
         } break;
         case Pass::PASS_CMD_SET_OFFSET_STATE:
         {
            applyState( *(const OffsetState*)(*curCmd)._data.ptr(), _cache->_offsetState );
         } break;
         case Pass::PASS_CMD_SET_STENCIL_STATE:
         {
            applyState( *(const StencilState*)(*curCmd)._data.ptr(), _cache->_stencilState );
         } break;
         case Pass::PASS_CMD_SET_WIREFRAME:
         {
            glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
            wire = true;
         } break;
         case Pass::PASS_CMD_SET_FILL:
         {
            glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
            wire = false;
         } break;
         case Pass::PASS_CMD_SET_PROGRAM:
         {
            curProg = (const Program*)(*curCmd)._data.ptr();
         } break;
         case Pass::PASS_CMD_EXEC:
         {
            ok &= executeGeometry( curGeom, curProg, curConst, curSamp, curMatrices, curCamPosition, range );
         } break;
         case Pass::PASS_CMD_SET_GEOMETRY:
         {
            //Keep it for the exec clause.
            range   = 0;
            curGeom = (const Geometry*)(*curCmd)._data.ptr();
         } break;
         case Pass::PASS_CMD_EXEC_GEOMETRY:
         {
            range   = 0;
            curGeom = (const Geometry*)(*curCmd)._data.ptr();
            ok &= executeGeometry( curGeom, curProg, curConst, curSamp, curMatrices, curCamPosition, range );
         } break;
         case Pass::PASS_CMD_SET_RANGEGEOMETRY:
         {
            //Keep it for the exec clause.
            range   = (*curCmd)._uPtr;
            curGeom = (const Geometry*)(*curCmd)._data.ptr();
         } break;
         case Pass::PASS_CMD_EXEC_RANGEGEOMETRY:
         {
            range   = (*curCmd)._uPtr;
            curGeom = (const Geometry*)(*curCmd)._data.ptr();
            ok &= executeGeometry( curGeom, curProg, curConst, curSamp, curMatrices, curCamPosition, range );
         } break;
         case Pass::PASS_CMD_SET_CONSTANTS:
         {
            curConst = (const ConstantList*)(*curCmd)._data.ptr();
         } break;
         case Pass::PASS_CMD_SET_CAMERA_POSITION:
         {
            curCamPosition = (const float*)(*curCmd)._fPtr;
         } break;
         case Pass::PASS_CMD_SET_SAMPLERS:
         {
            curSamp = (const SamplerList*)(*curCmd)._data.ptr();
         } break;
         case Pass::PASS_CMD_COPY_COLOR:
         {
            const OpenGLTexture* dst = (const OpenGLTexture*)(*curCmd)._data.ptr();
            CHECK( dst->_oglType == GL_TEXTURE_2D );
            glFlush();
            glBindTexture( dst->_oglType, dst->_id );
            glCopyTexImage2D( dst->_oglType, 0, toGLInternalFormat(dst->format()), 0, 0, _curWidth, _curHeight, 0 );
            glBindTexture( dst->_oglType, 0 );
         } break;
         case Pass::PASS_CMD_COPY_DEPTH:
         {
            const OpenGLTexture* dst = (const OpenGLTexture*)(*curCmd)._data.ptr();
            CHECK( dst->_oglType == GL_TEXTURE_2D );
            glBindTexture( dst->_oglType, dst->_id );
            glCopyTexImage2D( dst->_oglType, 0, toGLInternalFormat(dst->format()), 0, 0, _curWidth, _curHeight, 0 );
            glBindTexture( dst->_oglType, 0 );
         } break;
         case Pass::PASS_CMD_COPY_DEPTH_STENCIL:
         {
            const OpenGLTexture* dst = (const OpenGLTexture*)(*curCmd)._data.ptr();
            CHECK( dst->_oglType == GL_TEXTURE_2D );
            glBindTexture( dst->_oglType, dst->_id );
            glCopyTexImage2D( dst->_oglType, 0, toGLInternalFormat(dst->format()), 0, 0, _curWidth, _curHeight, 0 );
            glBindTexture( dst->_oglType, 0 );
         } break;
         case Pass::PASS_CMD_COPY_STENCIL:
         {
            const OpenGLTexture* dst = (const OpenGLTexture*)(*curCmd)._data.ptr();
            CHECK( dst->_oglType == GL_TEXTURE_2D );
            glBindTexture( dst->_oglType, dst->_id );
            glCopyTexImage2D( dst->_oglType, 0, toGLInternalFormat(dst->format()), 0, 0, _curWidth, _curHeight, 0 );
            glBindTexture( dst->_oglType, 0 );
         } break;
         case Pass::PASS_CMD_SET_SCISSOR:
         {
            curScissor = (*curCmd)._iPtr;
            glScissor( curScissor[0], curScissor[1], curScissor[2], curScissor[3] );
         } break;
         case Pass::PASS_CMD_SET_VIEWPORT:
         {
            curViewport = (*curCmd)._iPtr;
            glViewport( curViewport[0], curViewport[1], curViewport[2], curViewport[3] );
         } break;
         case Pass::PASS_CMD_SET_PROJECTION:
         {
            curMatrices[0] = (*curCmd)._fPtr;
            // Update view projection matrix.
            viewProj = (*(const Mat4f*)curMatrices[0]) * (*(const Mat4f*)curMatrices[1]);
         } break;
         case Pass::PASS_CMD_SET_VIEW:
         {
            curMatrices[1] = (*curCmd)._fPtr;
            // Update view projection matrix.
            viewProj = (*(const Mat4f*)curMatrices[0]) * (*(const Mat4f*)curMatrices[1]);
         } break;
         case Pass::PASS_CMD_SET_WORLD:
         {
            curMatrices[2] = (*curCmd)._fPtr;
         } break;
         default:
         {
            printf("UNKOWN PASS COMMAND TYPE: %d\n", (*curCmd)._id);
            ok = false;
         } break;
      }
      ++curCmd;
   }

   if( wire )
   {
      glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
   }

   glFlush();

   if( _doingRTT )
   {
      Framebuffer* fb = const_cast<Gfx::Framebuffer*>(pass._framebuffer.ptr());
      if( pass._fbGenerateMipmaps )
      {
         if( fb->_colorBuffer.isValid() && fb->_colorBuffer->isMipmapped() )
         {
            generateMipmaps(fb->_colorBuffer);
         }
         if( fb->_depthBuffer.isValid() && fb->_depthBuffer->isMipmapped() )
         {
            generateMipmaps(fb->_depthBuffer);
         }
         if( fb->_stencilBuffer.isValid() && fb->_stencilBuffer->isMipmapped() )
         {
            generateMipmaps(fb->_stencilBuffer);
         }
      }
   }

   // Deactive extraneous texture units.
   for( uint i = 0; i < _curTexUnitsUsed; ++i )
   {
      gl_activeTexture( GL_TEXTURE0 + i );
      glBindTexture( GL_TEXTURE_2D, 0 );
   }
   // Update current number of active texture units.
   _curTexUnitsUsed = 0;

   // Unbind current VAO.
   gl_bindVertexArray( 0 );

   return ok;
}

#endif //GFX_OGL_SUPPORT
