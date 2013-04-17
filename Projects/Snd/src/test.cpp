/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#include <Base/Dbg/UnitTest.h>

#include <Base/ADT/Vector.h>
#include <Base/IO/FileSystem.h>
#include <Base/IO/Path.h>
#include <Base/MT/Thread.h>

#include <CGMath/CGMath.h>

#include <Snd/Manager.h>
#include <Snd/Listener.h>
#include <Snd/Source.h>

#include <cassert>

USING_NAMESPACE

Path  sSoundsDir;

/*==============================================================================
   GLOBAL VARIABLES
==============================================================================*/
RCP<Snd::Manager>  _mgr;

/*==============================================================================
   GLOBAL FUNCTIONS
==============================================================================*/
//------------------------------------------------------------------------------
//!
void printPositionLine(
   const float l, const float s,
   const char centerChar = 'L', const char movingChar = 'S',
   const char firstChar = '[', const char lastChar =']',
   const char sepChar = '='
)
{
   String pre = String(sepChar) * 80;
   pre[0] = firstChar;
   pre[pre.size() - 1] = lastChar;
   pre[(uint)(l*pre.size())] = centerChar;
   pre[(uint)(s*pre.size())] = movingChar;
   printf("%s\r", pre.cstr());
   fflush(NULL);
}

//------------------------------------------------------------------------------
//!
RCP<Snd::Sound>
makeSound_Ramp8(
   const RCP<Snd::Manager>& mgr,
   uint nSecs      = 3,
   Snd::Freq freq  = 44100,
   Snd::Freq sFreq =     0,  //start frequency
   Snd::Freq eFreq =   500   //end frequency
)
{
   uint nSamples = freq * nSecs;
   uchar* samples = new uchar[nSamples];
   double t;
   double T;
   double f1;
   double f2;
   double fi;
   double v;
   for( uint i = 0; i < nSamples; ++i )
   {
      t = (double)i/freq;
      T = t / nSecs;
      f1 = 2.0 * CGConstd::pi() * sFreq;
      f2 = 2.0 * CGConstd::pi() * eFreq;
      fi = (1-T)*f1 + T*f2;
      v = 0.25 * CGM::sin( fi * t );
      samples[i] = (uchar)(((v + 1.0) / 2.0) * 255.0);
   }

   RCP<Snd::Sound> sound = mgr->createSound();
   mgr->setData(sound.ptr(), nSamples, samples, Snd::SND_SAMPLE_8, 1, freq);
   delete [] samples;  //deallocate memory
   return sound;
}

//------------------------------------------------------------------------------
//!
RCP<Snd::Sound>
makeSound_Ramp16(
   const RCP<Snd::Manager>& mgr,
   uint nSecs      = 3,
   Snd::Freq freq  = 44100,
   Snd::Freq sFreq =     0,  //start frequency
   Snd::Freq eFreq =   500   //end frequency
)
{
   uint nSamples = freq * nSecs;
   ushort* samples = new ushort[nSamples];
   double t;
   double T;
   double f1;
   double f2;
   double fi;
   double v;
   for( uint i = 0; i < nSamples; ++i )
   {
      t = (double)i/freq;
      T = t / nSecs;
      f1 = 2.0 * CGConstd::pi() * sFreq;
      f2 = 2.0 * CGConstd::pi() * eFreq;
      fi = (1-T)*f1 + T*f2;
      v = 0.25 * CGM::sin( fi * t );
      samples[i] = 0x8000 ^ (ushort)(((v + 1.0) / 2.0) * 65535.0);
   }

   RCP<Snd::Sound> sound = mgr->createSound();
   mgr->setData(sound.ptr(), nSamples*sizeof(ushort), samples, Snd::SND_SAMPLE_16, 1, freq);
   delete [] samples;  //deallocate memory
   return sound;
}

//------------------------------------------------------------------------------
//!
RCP<Snd::Sound> makeSound_Woop(
   const RCP<Snd::Manager>& mgr,
   double    nSecs =  0.25,
   Snd::Freq freq  = 44100,
   Snd::Freq sFreq =     0,
   Snd::Freq eFreq =   300
)
{
   uint nSamples = (uint)(freq * nSecs);
   ushort* samples = new ushort[nSamples];
   double t;
   double T;
   double f1;
   double f2;
   double fi;
   double v;
   for( uint i = 0; i < nSamples; ++i )
   {
      t = (double)i/freq;
      T = t / nSecs;
      f1 = 2.0 * CGConstd::pi() * sFreq;
      f2 = 2.0 * CGConstd::pi() * eFreq;
      fi = (1-T)*f1 + T*f2;
      v = 0.25 * CGM::sin( fi * t );
      samples[i] = 0x8000 ^ (ushort)(((v + 1.0) / 2.0) * 65535.0);
   }

   RCP<Snd::Sound> sound = mgr->createSound();
   mgr->setData(sound.ptr(), nSamples*sizeof(ushort), samples, Snd::SND_SAMPLE_16, 1, freq);
   delete [] samples;  //deallocate memory
   return sound;
}


/*==============================================================================
   Test routines
==============================================================================*/

//------------------------------------------------------------------------------
//!
void simple_8( Test::Result& )
{
   printf("\n");
   printf("Manager reports %s as API name\n", _mgr->API().cstr());

   double nSecs = 3;
   RCP<Snd::Sound> sound = makeSound_Ramp8(_mgr, (uint)nSecs);

   RCP<Snd::Source> source = _mgr->createSource();
   source->bind(sound.ptr());

   source->play();
   printf( "Playing? %c\n", (source->playing()?'y':'n') );
   Thread::sleep(nSecs + 0.1);
   printf( "Done playing? %c\n", (source->playing()?'n':'y') );
}

//------------------------------------------------------------------------------
//!
void simple_16( Test::Result& )
{
   printf("\n");
   printf("Manager reports %s as API name\n", _mgr->API().cstr());

   uint nSecs = 3;
   RCP<Snd::Sound> sound = makeSound_Ramp16(_mgr, nSecs);

   RCP<Snd::Source> source = _mgr->createSource();
   source->bind(sound.ptr());

   source->play();
   printf( "Playing? %c\n", (source->playing()?'y':'n') );
   Thread::sleep(nSecs + 0.1);
   printf( "Done playing? %c\n", (source->playing()?'n':'y') );
}

//------------------------------------------------------------------------------
//!
void simple_ogg( Test::Result& )
{
   printf("\n");

   Path filename = sSoundsDir;
   filename /= "test.ogg";

#if 0
   Vector<uchar>    data;
   Snd::SampleType  sample;
   uint             nChans;
   Snd::Freq        freq;
   _mgr->readSoundFile( filename.string(), data, sample, nChans, freq );
   double nSecs = data.size();
   nSecs /= freq;
   nSecs /= toBytes(sample);
   printf("Read %d bytes  sample=%d, nChans=%d freq=%d  %g secs\n",
          (uint)data.size(), sample, nChans, freq, nSecs);

   RCP<Snd::Sound> sound = _mgr->createSound();
   _mgr->setData(sound.ptr(), data.size()*sizeof(data[0]), data.data(), sample, nChans, freq);
#else
   RCP<Snd::Sound> sound = _mgr->makeSoundFromFile( filename.string() );
   double nSecs = sound->getDuration();
   printf("Read %d bytes  sample=%d, nChans=%d freq=%d  %g secs\n",
          (uint)sound->sizeInBytes(), sound->sampleType(), sound->numChannels(), sound->freq(), nSecs);
#endif

   RCP<Snd::Source> source = _mgr->createSource();
   source->bind(sound.ptr());
   source->update();

   source->play();
   Thread::sleep(nSecs + 0.1);
}

//------------------------------------------------------------------------------
//!
void pan_source( Test::Result& )
{
   printf("\n");
   Vector<uchar>    data;
   Snd::SampleType  sample;
   uint             nChans;
   Snd::Freq        freq;

   Path filename = sSoundsDir;
   filename /= "test.ogg";
   _mgr->readSoundFile( filename.string(), data, sample, nChans, freq );
   float nSecs = (float)data.size();
   nSecs /= freq;
   nSecs /= toBytes(sample);
   printf("Read %d bytes  sample=%d, nChans=%d freq=%d  %g secs\n",
          (uint)data.size(), sample, nChans, freq, nSecs);

   RCP<Snd::Sound> sound = _mgr->createSound();
   _mgr->setData(sound.ptr(), data.size()*sizeof(data[0]), data.data(), sample, nChans, freq);

   RCP<Snd::Listener> listener = _mgr->createListener();

   RCP<Snd::Source> source = _mgr->createSource();
   source->bind(sound.ptr());

   float timeDelta = nSecs / 100.0f;
   listener->position( Vec3f(0.0f, 0.0f, 0.0f) );
   listener->update();
   source->position( Vec3f(-5.0f, 0.0f, 0.5f) );
   source->velocity( Vec3f(10.0f/nSecs, 0.0f, 0.0f) );
   source->play();
   for( float x = -5.0f; x < 5.0f; x += 0.1f )
   {
      source->position().x = x;
      source->update();
      Thread::sleep(timeDelta);
      printPositionLine(0.5f, (x + 5.0f)/10.0f, 'L', 'S');
   }
   Thread::sleep(0.1);
   printf("\n");
   printf("Did you hear it pan from left to right?\n");
}

//------------------------------------------------------------------------------
//!
void pan_listener( Test::Result& )
{
   printf("\n");
   Vector<uchar>    data;
   Snd::SampleType  sample;
   uint             nChans;
   Snd::Freq        freq;

   Path filename = sSoundsDir;
   filename /= "test.ogg";
   _mgr->readSoundFile( filename.string(), data, sample, nChans, freq );
   float nSecs = (float)data.size();
   nSecs /= freq;
   nSecs /= toBytes(sample);
   printf("Read %d bytes  sample=%d, nChans=%d freq=%d  %g secs\n",
          (uint)data.size(), sample, nChans, freq, nSecs);

   RCP<Snd::Sound> sound = _mgr->createSound();
   _mgr->setData(sound.ptr(), data.size()*sizeof(data[0]), data.data(), sample, nChans, freq);

   RCP<Snd::Listener> listener = _mgr->createListener();

   RCP<Snd::Source> source = _mgr->createSource();
   source->bind(sound.ptr());

   float timeDelta = nSecs / 100.0f;
   source->position( Vec3f(0.0f, 0.0f, 0.0f) );
   source->update();
   listener->position( Vec3f(-5.0f, 0.0f, 0.5f) );
   listener->velocity( Vec3f(10.0f/nSecs, 0.0f, 0.0f) );
   source->play();
   for( float x = -5.0f; x < 5.0f; x += 0.1f )
   {
      listener->position().x = x;
      listener->update();
      Thread::sleep(timeDelta);
      printPositionLine(0.5f, (x + 5.0f)/10.0f, 'S', 'L');
   }
   Thread::sleep(0.1);
   printf("\n");
   printf("Did you hear it pan from right to left?\n");
}

//------------------------------------------------------------------------------
//!
void simple_loop( Test::Result& )
{
   printf("\n");
   double nSecs = 0.25;
   RCP<Snd::Sound> sound = makeSound_Woop( _mgr, nSecs );

   RCP<Snd::Source> source = _mgr->createSource();
   source->bind(sound.ptr());

   source->looping(true);
   source->update();
   source->play();
   printf( "Playing? %c\n", (source->playing()?'y':'n') );
   for( uint i = 0; i < 10; ++i )
   {
      Thread::sleep(nSecs);
      printf( "Done playing? %c\n", (source->playing()?'n':'y') );
   }
   source->stop();
   printf( "Done playing? %c\n", (source->playing()?'n':'y') );
}


//------------------------------------------------------------------------------
//!
void simple_bgm( Test::Result& )
{
   printf("\n");
   // Prepare listener.
   RCP<Snd::Listener> listener = _mgr->createListener();

   // Prepare BGM.
   double nSecs = 0.5;
   RCP<Snd::Sound> sound = makeSound_Woop(_mgr, nSecs);
   RCP<Snd::Source> bgm = _mgr->createSource();
   bgm->bind(sound.ptr());
   sound = NULL;
   bgm->setBGM();
   bgm->looping(true);
   bgm->update();


   // Prepare other sound.
   Vector<uchar>    data;
   Snd::SampleType  sample;
   uint             nChans;
   Snd::Freq        freq;

   Path filename = sSoundsDir;
   filename /= "test.ogg";
   _mgr->readSoundFile( filename.string(), data, sample, nChans, freq );
   nSecs = data.size();
   nSecs /= freq;
   nSecs /= toBytes(sample);
   printf("Read %d bytes  sample=%d, nChans=%d freq=%d  %g secs\n",
          (uint)data.size(), sample, nChans, freq, nSecs);

   sound = _mgr->createSound();
   _mgr->setData(sound.ptr(), data.size()*sizeof(data[0]), data.data(), sample, nChans, freq);

   RCP<Snd::Source> source = _mgr->createSource();
   source->bind(sound.ptr());
   sound = NULL;


   // Start making some sound.
   source->position( Vec3f(0.0f, 0.0f, 0.0f) );
   source->update();

   // Move listener to make sure it doesn't affect the BGM.
   listener->position( Vec3f(-5.0f, 0.0f, 0.5f) );
   listener->velocity( Vec3f(10.0f/(float)nSecs, 0.0f, 0.0f) );

   bgm->play();
   source->play();

   float timeDelta = (float)nSecs / 100.0f;
   for( float x = -5.0f; x < 5.0f; x += 0.1f )
   {
      listener->position().x = x;
      listener->update();
      Thread::sleep(timeDelta);
      printPositionLine(0.5f, (x + 5.0f)/10.0f, 'S', 'L');
   }

   bgm->stop();
   source->stop();

   printf("\n");
   printf("Did you hear it pan from right to left, yet the BGM stayed constant level?\n");
}

//------------------------------------------------------------------------------
//!
void dual_listener( Test::Result& )
{
   printf("\n");
   double nSecs = 0.1;
   RCP<Snd::Sound> sound = makeSound_Woop(_mgr, nSecs);
   RCP<Snd::Source> source = _mgr->createSource();
   source->bind(sound.ptr());
   source->looping( true );
   // Keep position at origin.
   source->update();
   sound = NULL;

#define USE_A 1
#define USE_B 1

#if USE_A
   RCP<Snd::Listener> listenerA = _mgr->createListener();
   listenerA->position( Vec3f(-1.0f, 0.0f, 0.0f) );
   listenerA->pointOfInterest() += listenerA->position();
   listenerA->gain(0.0f);
   listenerA->update();
#endif

#if USE_B
   RCP<Snd::Listener> listenerB = _mgr->createListener();
   listenerB->position( Vec3f( 1.0f, 0.0f, 0.0f) );
   listenerB->pointOfInterest() += listenerB->position();
   listenerB->gain(0.0f);
   listenerB->update();
#endif

   source->update();
   source->play();

#if USE_A
   printf("Bringing listener A up on the left (sound in right ear fading in).\n");
   for( float x = 0.0f; x <= 1.0f; x += 1.0f/16.0f )
   {
      listenerA->gain(x);
      listenerA->update();
      Thread::sleep(nSecs);
   }

   Thread::sleep( 0.5 );
#endif

#if USE_B
   printf("Bringing listener B up on the right (sound in left ear fading in).\n");
   for( float x = 0.0f; x <= 1.0f; x += 1.0f/16.0f )
   {
      listenerB->gain(x);
      listenerB->update();
      Thread::sleep(nSecs);
   }

   Thread::sleep( 0.5 );
#endif

#if USE_A
   printf("Moving listener A to onto B on the right (sound only in left ear).\n");
   //listenerA->velocity( Vec3f((1.0 - -1.0f)/nSecs, 0.0f, 0.0f) );
   for( float x = listenerA->position().x; x <= 1.0f; x += 1.0f/16.0f )
   {
      listenerA->position().x = x;
      listenerA->pointOfInterest().x = x;
      listenerA->update();
      Thread::sleep(nSecs);
   }
   listenerA->velocity( Vec3f(0.0f, 0.0f, 0.0f) );
   listenerA->update();

   Thread::sleep( 0.5 );
#endif

#if USE_B
   printf("Moving listener B to the left (sound in both ears).\n");
   //listenerB->velocity( Vec3f((-1.0 - 1.0f)/nSecs, 0.0f, 0.0f) );
   for( float x = listenerB->position().x; x >= -1.0f; x -= 1.0f/16.0f )
   {
      listenerB->position().x = x;
      listenerB->pointOfInterest().x = x;
      listenerB->update();
      Thread::sleep(nSecs);
   }
   listenerB->velocity( Vec3f(0.0f, 0.0f, 0.0f) );

   Thread::sleep( 0.5 );
#endif

#if USE_A
   printf("Bringing listener A down (sound in left ear fading out).\n");
   for( float x = 1.0f; x >= 0.0f; x -= 1.0f/16.0f )
   {
      listenerA->gain(x);
      listenerA->update();
      Thread::sleep(nSecs);
   }

   Thread::sleep( 0.5 );
#endif

#if USE_B
   printf("Bringing listener B up (sound in right ear fading out).\n");
   for( float x = 1.0f; x >= 0.0f; x -= 1.0f/16.0f )
   {
      listenerB->gain(x);
      listenerB->update();
      Thread::sleep(nSecs);
   }
#endif

   printf("Done!\n");

   source->stop();
}

//------------------------------------------------------------------------------
//!
void auto_delete( Test::Result& )
{
   printf("\n");
   Vector<uchar>    data;
   Snd::SampleType  sample;
   uint             nChans;
   Snd::Freq        freq;

   Path filename = sSoundsDir;
   filename /= "test.ogg";
   _mgr->readSoundFile( filename.string(), data, sample, nChans, freq );
   double nSecs = data.size();
   nSecs /= freq;
   nSecs /= toBytes(sample);
   printf("Read %d bytes  sample=%d, nChans=%d freq=%d  %g secs\n",
          (uint)data.size(), sample, nChans, freq, nSecs);

   RCP<Snd::Sound> sound = _mgr->createSound();
   _mgr->setData(sound.ptr(), data.size()*sizeof(data[0]), data.data(), sample, nChans, freq);

   RCP<Snd::Source> source;

#if 1
   printf("Playing without warning message at the end.\n");
   source = _mgr->createSource();
   source->bind(sound.ptr());
   source->autoDelete(true);
   source->update();
   source->play();
   source = NULL; // this disables the warning
   Thread::sleep(nSecs + 0.1);
#if !SND_USE_PROCESS_THREAD
   _mgr->process();
#endif
#endif

#if 1
   printf("Playing with warning message at the end.\n");
   source = _mgr->createSource();
   source->bind(sound.ptr());
   source->autoDelete(true);
   source->update();
   source->play();
   Thread::sleep(nSecs + 0.1);
#if !SND_USE_PROCESS_THREAD
   _mgr->process();
#endif
   source = NULL;
#endif
}

//------------------------------------------------------------------------------
//!
void simple_gain( Test::Result& )
{
   printf("\n");
   // Create 2 sources (one left, then right), and a listener.
   double nSecs = 0.1;
   RCP<Snd::Sound> sound = makeSound_Woop(_mgr, nSecs);
   RCP<Snd::Source> sourceA = _mgr->createSource();
   sourceA->bind(sound.ptr());
   sourceA->looping( true );
   sourceA->position( Vec3f(-1.0f, 0.0f, 0.0f) );
   sourceA->update();

   nSecs = 0.5;
   sound = makeSound_Woop(_mgr, nSecs);
   RCP<Snd::Source> sourceB = _mgr->createSource();
   sourceB->bind(sound.ptr());
   sourceB->looping( true );
   sourceB->position( Vec3f(+1.0f, 0.0f, 0.0f) );
   sourceB->update();

   RCP<Snd::Listener> listener = _mgr->createListener();
   listener->gain(1.0f);
   listener->update();

   sourceA->play();
   sourceB->play();

   float timeDelta = 0.1f;
   Thread::sleep(0.5);
   printf("Playing with left source gain.\n");
   for( float g = 1.0f; g >= 0.0f; g -= 0.0625f )
   {
      sourceA->gain( g );
      sourceA->update();
      Thread::sleep(timeDelta);
   }
   Thread::sleep(0.2);
   for( float g = 0.0f; g <= 1.0f; g += 0.0625f )
   {
      sourceA->gain( g );
      sourceA->update();
      Thread::sleep(timeDelta);
   }
   Thread::sleep(0.5);
   printf("Playing with right source gain.\n");
   for( float g = 1.0f; g >= 0.0f; g -= 0.0625f )
   {
      sourceB->gain( g );
      sourceB->update();
      Thread::sleep(timeDelta);
   }
   Thread::sleep(0.2);
   for( float g = 0.0f; g <= 1.0f; g += 0.0625f )
   {
      sourceB->gain( g );
      sourceB->update();
      Thread::sleep(timeDelta);
   }
   Thread::sleep(0.5);
   printf("Playing with listener gain.\n");
   for( float g = 1.0f; g >= 0.0f; g -= 0.0625f )
   {
      listener->gain( g );
      listener->update();
      Thread::sleep(timeDelta);
   }
   Thread::sleep(0.2);
   for( float g = 0.0f; g <= 1.0f; g += 0.0625f )
   {
      listener->gain( g );
      listener->update();
      Thread::sleep(timeDelta);
   }
   Thread::sleep(0.5);
   printf("Playing with master gain.\n");
   for( float g = 1.0f; g >= 0.0f; g -= 0.0625f )
   {
      _mgr->masterGain( g );
      Thread::sleep(timeDelta);
   }
   Thread::sleep(0.2);
   for( float g = 0.0f; g <= 1.0f; g += 0.0625f )
   {
      _mgr->masterGain( g );
      Thread::sleep(timeDelta);
   }
   sourceA->stop();
   sourceB->stop();
   Thread::sleep(0.1);
   printf("Done.\n");
   printf("\n");
}

//------------------------------------------------------------------------------
//!
void pitch_master( Test::Result& )
{
   printf("\n");
   Vector<uchar>    data;
   Snd::SampleType  sample;
   uint             nChans;
   Snd::Freq        freq;

   Path filename = sSoundsDir;
   filename /= "test.ogg";
   _mgr->readSoundFile( filename.string(), data, sample, nChans, freq );
   double nSecs = data.size();
   nSecs /= freq;
   nSecs /= toBytes(sample);
   printf("Read %d bytes  sample=%d, nChans=%d freq=%d  %g secs\n",
          (uint)data.size(), sample, nChans, freq, nSecs);

   RCP<Snd::Sound> sound = _mgr->createSound();
   _mgr->setData(sound.ptr(), data.size()*sizeof(data[0]), data.data(), sample, nChans, freq);

   RCP<Snd::Source> source = _mgr->createSource();
   source->bind(sound.ptr());
   source->update();
   source->play();
   printf("Playing with master pitch.\n");
   for( float p = 0.5f; p <= 2.0f; p += 0.05f )
   {
      _mgr->masterPitch( p );
      Thread::sleep(0.1);
   }
   source->stop();
}

//------------------------------------------------------------------------------
//!
void pitch_source( Test::Result& )
{
   printf("\n");
   Vector<uchar>    data;
   Snd::SampleType  sample;
   uint             nChans;
   Snd::Freq        freq;

   Path filename = sSoundsDir;
   filename /= "test.ogg";
   _mgr->readSoundFile( filename.string(), data, sample, nChans, freq );
   double nSecs = data.size();
   nSecs /= freq;
   nSecs /= toBytes(sample);
   printf("Read %d bytes  sample=%d, nChans=%d freq=%d  %g secs\n",
          (uint)data.size(), sample, nChans, freq, nSecs);

   RCP<Snd::Sound> sound = _mgr->createSound();
   _mgr->setData(sound.ptr(), data.size()*sizeof(data[0]), data.data(), sample, nChans, freq);

   RCP<Snd::Source> source = _mgr->createSource();
   source->bind(sound.ptr());
   source->update();
   source->play();
   printf("Playing with source pitch.\n");
   for( float p = 0.5f; p <= 2.0f; p += 0.05f )
   {
      source->pitch( p );
      source->update();
      Thread::sleep(0.1);
   }
   source->stop();
}

/*
//------------------------------------------------------------------------------
//!
void pitch_listener( Test::Result& )
{
   printf("\n");
   Vector<uchar>    data;
   Snd::SampleType  sample;
   uint             nChans;
   Snd::Freq        freq;

   Path filename = sSoundsDir;
   filename /= "test.ogg";
   _mgr->readSoundFile( filename.string(), data, sample, nChans, freq );
   double nSecs = data.size();
   nSecs /= freq;
   nSecs /= toBytes(sample);
   printf("Read %d bytes  sample=%d, nChans=%d freq=%d  %g secs\n",
          (uint)data.size(), sample, nChans, freq, nSecs);

   RCP<Snd::Sound> sound = _mgr->createSound();
   _mgr->setData(sound.ptr(), data.size()*sizeof(data[0]), data.data(), sample, nChans, freq);

   RCP<Snd::Source> source = _mgr->createSource();
   source->bind(sound.ptr());
   source->update();
   source->play();
   RCP<Snd::Listener> listener = mgr->createListener();
   printf("Playing with listener pitch.\n");
   for( float p = 0.5f; p <= 2.0f; p += 0.05f )
   {
      listener->pitch( p );
      listener->update();
      Thread::sleep(0.1);
   }
   source->stop();
}
*/

/*==============================================================================
   UTILITY
==============================================================================*/

void  init_tests()
{
   Test::Collection& spc = Test::special();
   spc.add( new Test::Function( "simple_8"      , "Simple 8-bit sound"            , simple_8       ) );
   spc.add( new Test::Function( "simple_16"     , "Simple 16-bit sound"           , simple_16      ) );
   spc.add( new Test::Function( "simple_ogg"    , "Simple Ogg file"               , simple_ogg     ) );
   spc.add( new Test::Function( "pan_source"    , "Panning the source"            , pan_source     ) );
   spc.add( new Test::Function( "pan_listener"  , "Panning the listener"          , pan_listener   ) );
   spc.add( new Test::Function( "simple_loop"   , "Basic looping"                 , simple_loop    ) );
   spc.add( new Test::Function( "simple_bgm"    , "Simple background music"       , simple_bgm     ) );
   spc.add( new Test::Function( "dual_listener" , "Multi-listener test"           , dual_listener  ) );
   spc.add( new Test::Function( "auto_delete"   , "Automatic deletion of resource", auto_delete    ) );
   spc.add( new Test::Function( "simple_gain"   , "Simple gain (volume) test"     , simple_gain    ) );
   spc.add( new Test::Function( "pitch_master"  , "Tweaking of the master pitch"  , pitch_master   ) );
   spc.add( new Test::Function( "pitch_source"  , "Tweaking the source pitch"     , pitch_source   ) );
   //spc.add( new Test::Function( "pitch_listener", "Tweaking the listener pitch"   , pitch_listener ) );
}

//------------------------------------------------------------------------------
//!
void  setSoundsDir( const char* app )
{
   const char* data = getenv("ARCHETYPE_DATA");
   if( data != NULL )
   {
      sSoundsDir = String(data) + String("/sound/");
      printf("Using ARCHETYPE_DATA sound directory: %s\n", sSoundsDir.cstr());
      return;
   }

   Path path;

   if( app[0] == '/' )
   {
      path = Path( app );
   }
   else
   {
      path = Path::getCurrentDirectory();
      path /= app;
   }

   path /= "Data/sound/test.ogg";
   while( !FS::Entry( path ).exists() )
   {
      path.goUp(4); //Remove '<dir>/Data/sound/test.ogg'
      if( path.empty() )
      {
         printf("ERROR - Reached root directory without finding any Data directory.\n");
         break;
      }
      path /= "Data/sound/test.ogg";
   }
   path.goUp(); //Remove 'test.ogg'

   sSoundsDir = path;
   printf("Using auto-determined sound directory: %s\n", sSoundsDir.cstr());
}

/*==============================================================================
   MAIN
==============================================================================*/
int
main
( int argc, char** argv )
{
   init_tests();
   setSoundsDir( argv[0] );
   _mgr =  Snd::Manager::createManager();
   return Test::main( argc, argv );
}
