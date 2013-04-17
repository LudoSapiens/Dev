/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#include <Snd/Manager.h>

#include <Base/Util/CPU.h>
#include <Base/Dbg/DebugStream.h>

#include <cassert>

//#define SND_VORBIS_SUPPORT 0
//#define SND_STB_VORBIS_SUPPORT 1

#if (SND_VORBIS_SUPPORT > 0) && (SND_STB_VORBIS_SUPPORT > 0)
#  warning Compiling with both Ogg/Vorbis and stb_vorbis
#  warning Disabling stb_vorbis
#  undef SND_STB_VORBIS_SUPPORT
#  define SND_STB_VORBIS_SUPPORT 0
#endif

//----------------------
// OpenAL configuration
//----------------------
#if SND_OPENAL_SUPPORT
#include <Snd/OpenALManager.h>
#endif //SND_OPENAL_SUPPORT
//------------------------

//----------------------
// Null manager
//----------------------
#if SND_NULL_SUPPORT
#include <Snd/NullManager.h>
#endif //SND_NULL_SUPPORT
//------------------------

//-------------------
// Ogg configuration
//-------------------
#if SND_VORBIS_SUPPORT
#define OV_EXCLUDE_STATIC_CALLBACKS 1
#include <vorbis/vorbisfile.h>
#endif //SND_VORBIS_SUPPORT

//--------------------------
// stb_vorbis configuration
//--------------------------
#if SND_STB_VORBIS_SUPPORT
#include <Snd/stb_vorbis.c>
#endif //SND_STB_VORBIS_SUPPORT

//------------------------

#if SND_USE_PROCESS_THREAD
#include <Base/MT/Thread.h>
#endif //SND_USE_PROCESS_THREAD

USING_NAMESPACE

using namespace Snd;


UNNAMESPACE_BEGIN

DBG_STREAM( os_snd, "Sound" );

#if SND_VORBIS_SUPPORT

inline int toSign( SampleType s )
{
   switch( s )
   {
      case SND_SAMPLE_8:  return 0;
      case SND_SAMPLE_16: return 1;
      default:            return 0;
   }
}

#endif // SND_VORBIS_SUPPORT

#if SND_STB_VORBIS_SUPPORT
extern "C"
{
}
#endif //SND_STB_VORBIS_SUPPORT


#if SND_USE_PROCESS_THREAD
//------------------------------------------------------------------------------
//!
class SoundProcessTask:
   public Task
{
public:

   /*----- methods -----*/

   SoundProcessTask( double delay = 0, Manager* manager ):
      _delay( delay ),
      _manager( manager )
   {
   }

   virtual void execute()
   {
      DBG_MSG( os_snd, "Starting sound process thread" );
      while( true )
      {
         Thread::sleep( _delay );
         _manager->process();
      }
   }

   /*----- data members -----*/
   double    _delay;
   Manager*  _manager;

}; //class SoundProcessTask

// A single thread instance (only one Manager should ever get instantiated).
Thread*  _sThread = NULL;

#endif //SND_USE_PROCESS_THREAD


#if SND_VORBIS_SUPPORT
//------------------------------------------------------------------------------
//!
bool  readOggSoundFile
( const String& filename,
  NAMESPACE::Vector<uchar>& dstData,
  SampleType& sample,
  uint& numChannels,
  Freq& freq )
{
   FILE* f = fopen(filename.cstr(), "rb");
   if( f == NULL )
   {
      printf("Error opening file: %s\n", filename.cstr());
      return false;
   }

   if( ferror(f) )
   {
      perror("Error in file");
      return false;
   }

   // Set Vorbis file handle
   OggVorbis_File oggFile;
   if( ov_open(f, &oggFile, NULL, 0) != 0 )
   {
      printf("Error opening vorbis file\n");
      return false;
   }

   // Get some information about the OGG file
   vorbis_info* oggInfo = ov_info(&oggFile, -1);
   if( oggInfo == NULL )
   {
      printf("Error retrieving Ogg info\n");
      return false;
   }
   sample = SND_SAMPLE_16; //always 16?
   numChannels = oggInfo->channels;
   freq = oggInfo->rate;

   NAMESPACE::Vector<char>  temp_buffer;
   temp_buffer.resize( 1 << 15 );  //32KB buffers
   int bigendian = (CPU_ENDIANNESS == CPU_ENDIAN_BIG) ? 1 : 0;  //from CPU.h
   int word = toBytes(sample);
   int sign = toSign(sample);
   size_t curSize = dstData.size();
   int nBytes;
   int bitStream;
   do
   {
      // Read up to a buffer's worth of decoded sound data
      nBytes = ov_read(&oggFile, temp_buffer.data(), temp_buffer.size(), bigendian, word, sign, &bitStream);

      // Append to end of buffer
      dstData.resize( dstData.size() + nBytes );
      memcpy( dstData.data() + curSize, temp_buffer.data(), nBytes );
      curSize = dstData.size();
   } while (nBytes > 0);

   if( ov_clear(&oggFile) != 0 )
   {
      printf("Error clearing vorbis file\n");
      return false;
   }

   /*
   if( fclose(f) != 0 )
   {
      printf("Error closing file: %s\n", filename.cstr());
      return false;
   }
   */

   return true;
}
#endif //SND_VORBIS_SUPPORT

#if SND_STB_VORBIS_SUPPORT
//------------------------------------------------------------------------------
//!
bool  stbVorbisReadOggSoundFile
( const String& filename,
  NAMESPACE::Vector<uchar>& dstData,
  SampleType& sample,
  uint& numChannels,
  Freq& freq )
{
   short* decoded;
   int channels, len;
   len = stb_vorbis_decode_filename( (char*)filename.cstr(), &channels, &decoded );

   if( len == 0 )
   {
      printf( "Error decoding vorbis file: %s.\n", filename.cstr() );
      return false;
   }

   size_t totalSize = len*channels*2;
   dstData.resize( totalSize );
   memcpy( dstData.data(), decoded, len*channels*2 );
   sample      = SND_SAMPLE_16;
   numChannels = channels;
   freq        = 44100;

   return true;
}
#endif //SND_STB_VORBIS_SUPPORT


UNNAMESPACE_END

//------------------------------------------------------------------------------
//!
void Manager::printInfo( TextStream& os )
{
   os << "Snd:";

#if SND_NULL_SUPPORT
   os << " null";
#endif

#if SND_OPENAL_SUPPORT
   os << " OpenAL";
#endif

#if SND_VORBIS_SUPPORT
   os << " Vorbis";
#endif

#if SND_STB_VORBIS_SUPPORT
   os << " STB_Vorbis";
#endif

   os << nl;
}

//------------------------------------------------------------------------------
//!
Manager::Manager
( void ):
   _masterGain( 1.0f ),
   _masterPitch( 1.0f )
{
   DBG_BLOCK( os_snd, "Starting up Snd::Manager" );
#if SND_USE_PROCESS_THREAD
   CHECK( _sThread == NULL );
   _sThread = new Thread( new SoundProcessTask(1.0f/30.0f, this) );
#endif //SND_USE_PROCESS_THREAD
}

//------------------------------------------------------------------------------
//!
Manager::~Manager
( void )
{
   DBG_BLOCK( os_snd, "Shutting down Snd::Manager" );
#if SND_USE_PROCESS_THREAD
   CHECK( _sThread != NULL );
   delete _sThread;
   _sThread = NULL;
#endif //SND_USE_PROCESS_THREAD
}

//------------------------------------------------------------------------------
//!
void
Manager::masterGain( float g )
{
   _masterGain = g;
}

//------------------------------------------------------------------------------
//!
void
Manager::masterPitch( float p )
{
   _masterPitch = p;
}

//------------------------------------------------------------------------------
//!
void
Manager::masterStop()
{
}

//------------------------------------------------------------------------------
//!
void
Manager::masterResume()
{
}

//------------------------------------------------------------------------------
//!
RCP<Manager>
Manager::createManager
( const String& api_name )
{
   String api_name_lower;

   const char* mgr_str = getenv("SND_MGR");
   if( mgr_str == NULL )
   {
      api_name_lower = api_name.lower();
   }
   else
   {
      api_name_lower = mgr_str;
      api_name_lower = api_name_lower.lower();
   }

   //printf("Creating sound manager using: '%s'.\n", api_name_lower.cstr());

#if SND_OPENAL_SUPPORT
   if( api_name_lower.empty() ||
       api_name_lower == "openal" ||
       api_name_lower == "al" )
   {
      OpenALManager* mgr = new OpenALManager();
      return RCP<Manager>( mgr );
   }
   else
#endif //SND_OPENAL_SUPPORT
#if SND_NULL_SUPPORT
   if( api_name_lower.empty() ||
       api_name_lower == "null" )
   {
      NullManager* mgr = new NullManager();
      return RCP<Manager>( mgr );
   }
   else
#endif //SND_NULL_SUPPORT
   {
      assert(0 && "Unknown api name");
      return NULL;
   }
}

// File reading
bool
Manager::readSoundFile
( const String& filename,
  NAMESPACE::Vector<uchar>& dstData,
  SampleType& sample,
  uint& numChannels,
  Freq& freq )
{
   unused(filename);
   unused(dstData);
   unused(sample);
   unused(numChannels);
   unused(freq);
   String ext = filename.getExt().lower();

#if SND_VORBIS_SUPPORT
   if( ext == "ogg" )
   {
      return readOggSoundFile(filename, dstData, sample, numChannels, freq);
   }
   else
#endif
#if SND_STB_VORBIS_SUPPORT
   if( ext == "ogg" )
   {
      return stbVorbisReadOggSoundFile(filename, dstData, sample, numChannels, freq);
   }
   else
#endif
   {
      printf("Unknown file format: %s\n", ext.cstr());
      return false;
   }
}

RCP<Sound>
Manager::makeSoundFromFile( const String& filename )
{
   NAMESPACE::Vector<uchar> data;
   SampleType sample;
   uint numChannels;
   Freq freq;
   if( readSoundFile( filename, data, sample, numChannels, freq ) )
   {
      RCP<Sound> snd = createSound();
      if( snd.isValid() )
      {
         if( setData( snd.ptr(), data.size(), data.data(), sample, numChannels, freq ) )
         {
            return snd;
         }
         else
         {
            printf( "Snd::Manager::makeSoundFromFile() - Error setting data.\n" );
            return NULL;
         }
      }
      else
      {
         printf( "Snd::Manager::makeSoundFromFile() - Error creating Sound instance.\n" );
         return NULL;
      }
   }
   else
   {
      printf( "Snd::Manager::makeSoundFromFile() - Error reading sound file '%s'.\n", filename.cstr() );
      return NULL;
   }
}

//------------------------------------------------------------------------------
//!
void
Manager::release( Listener* /*listener*/ )
{
   
}

//------------------------------------------------------------------------------
//!
void
Manager::release( Source* /*listener*/ )
{
   
}
