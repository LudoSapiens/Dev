/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#ifndef PLASMA_PLASMABAKER_H
#define PLASMA_PLASMABAKER_H

#include <Plasma/StdDefs.h>

#include <Fusion/Core/Animator.h>
#include <Fusion/VM/VM.h>

#include <Gfx/Pass/Pass.h>

#include <Base/ADT/Dequeue.h>
#include <Base/MT/Lock.h>

NAMESPACE_BEGIN

class Image;
class Renderer;
class SurfaceGeometry;
class World;

/*==============================================================================
  CLASS PlasmaBaker
==============================================================================*/

//!

class PLASMA_DLL_API PlasmaBaker
{

public:

   /*----- static methods -----*/

   static void initialize( VMState* );

   static void vertexAO( World* );
   static void vertexAO( SurfaceGeometry* );

};

/*==============================================================================
  CLASS CubemapBaker
==============================================================================*/
class CubemapBaker:
   public Animator
{
public:

   /*----- methods -----*/

   PLASMA_DLL_API CubemapBaker();
   PLASMA_DLL_API virtual ~CubemapBaker();

   PLASMA_DLL_API void add( World* w, const Vec3f& p, const Vec2i& d, const String& s );
   PLASMA_DLL_API void add( World* w, const Vec3f& p, Image* i );

   PLASMA_DLL_API virtual bool exec( double time, double delta );

protected:

   /*----- data types -----*/

   struct Job
   {
      RCP<World>            _world;
      Vec3f                 _pos;
      Vec2i                 _dim;
      String                _tmpl;
      int                   _slice;
      RCP<Image>            _img;
      RCP<Gfx::Framebuffer> _fbo;

      Job(){}
      Job( World* w, const Vec3f& p, const Vec2i& d, const String& s ):
         _world(w), _pos(p), _dim(d), _tmpl(makeTemplate(s)), _slice(0)
      {}
      Job( World* w, const Vec3f& p, Image* i );

      void init();
   };

   /*----- data members -----*/

   DEQueue<Job>   _jobs;
   Lock           _jobsLock;
   RCP<Renderer>  _renderer;

   /*----- methods -----*/

   bool  handle( Job& job );
   static String  makeTemplate( const String& path );

private:
}; //class CubemapBaker

NAMESPACE_END

#endif
