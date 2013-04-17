/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#ifndef SND_TYPES_H
#define SND_TYPES_H

#include <Snd/StdDefs.h>

#include <CGMath/Vec3.h>

#include <cstddef>

NAMESPACE_BEGIN

namespace Snd
{


enum
{
   INVALID_ID = (uint)-1
};

/*==============================================================================
  SampleType
==============================================================================*/
typedef enum
{
   SND_SAMPLE_8,
   SND_SAMPLE_16,
   SND_SAMPLE_24,
   SND_SAMPLE_24F,
   SND_SAMPLE_32,
   SND_SAMPLE_32F
} SampleType;

//------------------------------------------------------------------------------
//!
inline size_t toBytes( const SampleType st )
{
   switch(st)
   {
      case SND_SAMPLE_8:
         return 1;
      case SND_SAMPLE_16:
         return 2;
      case SND_SAMPLE_24:
      case SND_SAMPLE_24F:
         return 3;
      case SND_SAMPLE_32:
      case SND_SAMPLE_32F:
         return 4;
      default:
         return 0;
   }
}


/*==============================================================================
  Freq
==============================================================================*/
typedef uint  Freq;


/*==============================================================================
  CLASS Cone
==============================================================================*/
class Cone
{
public:

   /*----- data members -----*/
   Vec3f  direction;
   float  innerAngle;
   float  outerAngle;

   /*----- methods -----*/
   Cone( const Vec3f& direction = Vec3f::zero(), const float innerAngle = 360.0f, const float outerAngle = 360.0f ):
      direction(direction), innerAngle(innerAngle), outerAngle(outerAngle)
   { }

protected:
private:
}; //class Cone


}  //namespace Snd

NAMESPACE_END


#endif //SND_TYPES_H
