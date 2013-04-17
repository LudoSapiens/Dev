/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#ifndef PLASMA_PROCEDURALCONTEXT
#define PLASMA_PROCEDURALCONTEXT

#include <Plasma/StdDefs.h>

#include <Base/ADT/String.h>

NAMESPACE_BEGIN

class Task;

/*==============================================================================
   CLASS ProceduralContext
==============================================================================*/

class ProceduralContext
{
public:

   enum Type {
      ANIMGRAPH,
      GEOMETRY,
      MATERIAL,
      WORLD,
   };

   /*----- methods -----*/

   ProceduralContext( uint type, Task* t ) : _type( type ), _task(t) {}

   inline uint type() const  { return _type; }
   inline Task* task() const { return _task; }

   inline const String&  curDir() const { return _curDir; }
   inline void  curDir( const String& v ) { _curDir = v; }

protected:

   /*----- members -----*/

   uint    _type;
   Task*   _task;
   String  _curDir;
};

NAMESPACE_END

#endif
