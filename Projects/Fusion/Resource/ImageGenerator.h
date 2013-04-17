/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#ifndef FUSION_IMAGE_GENERATOR_H
#define FUSION_IMAGE_GENERATOR_H

#include <Fusion/StdDefs.h>

#include <Fusion/Resource/Image.h>

#include <CGMath/Variant.h>

#include <Base/MT/Task.h>

NAMESPACE_BEGIN

/*==============================================================================
  CLASS ImageGenerator
==============================================================================*/
class ImageGenerator:
   public Task
{
public:

   /*----- static methods -----*/

   static void initialize();

   /*----- methods -----*/

   ImageGenerator(
      const           String&  path,
      const            Table*  args,
      const            Vec2i&  size,
      const Bitmap::PixelType  type        = Bitmap::BYTE,
      const               int  numChannels = 4 );
   //Add ctor with arguments Table.
   virtual ~ImageGenerator();

   virtual void execute();

   inline Image*  image() const { return _result.ptr(); }

protected:

   /*----- methods -----*/


   /*----- data members -----*/

   String             _path;
   RCP<const Table>   _args;

   Vec2i              _size;
   Bitmap::PixelType  _type;
   int                _chan;

   RCP<Image>         _result;

private:
}; //class ImageGenerator


NAMESPACE_END

#endif //FUSION_IMAGE_GENERATOR_H
