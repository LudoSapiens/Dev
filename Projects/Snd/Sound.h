/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#ifndef SND_SOUND_H
#define SND_SOUND_H

#include <Snd/StdDefs.h>
#include <Snd/Types.h>

#include <Base/Util/RCObject.h>

#include <cstddef>

NAMESPACE_BEGIN

namespace Snd
{


/*==============================================================================
  CLASS Sound
==============================================================================*/
class Sound:
   public RCObject
{
public:

   /*----- methods -----*/

   Freq  freq() const { return _freq; }

   SampleType  sampleType() const { return _sampleType; }

   uint  numChannels() const { return _numChans; }

   size_t  sizeInBytes() const { return _sizeInBytes; }

   double  getDuration() const { return _sizeInBytes / (double)(_freq * _numChans * toBytes(_sampleType)); }

protected:

   /*----- data members -----*/
   Freq        _freq;        //!< Frequency (in Hz)
   SampleType  _sampleType;  //!< Bits per sample (8 or 16 bit)
   uint        _numChans;    //!< The number of channels (typically 1)
   size_t      _sizeInBytes; //!< The total size in bytes

   /*----- methods -----*/

   SND_MAKE_MANAGERS_FRIENDS();
   void  freq( const Freq freq ) { _freq = freq; }
   void  sampleType( const SampleType st ) { _sampleType = st; }
   void  numChannels( const uint n ) { _numChans = n; }
   void  sizeInBytes( const size_t size ) { _sizeInBytes = size; }

   // Only managers can create this object
   Sound();
   virtual ~Sound();

private:
}; //class Sound


}  //namespace Snd

NAMESPACE_END


#endif //SND_SOUND_H
