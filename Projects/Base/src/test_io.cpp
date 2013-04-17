/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#include <Base/Dbg/UnitTest.h>

#include <Base/IO/BinaryStream.h>
#include <Base/IO/FileDevice.h>
#include <Base/IO/FileSystem.h>
#include <Base/IO/GZippedFileDevice.h>
#include <Base/IO/LockedMemoryDevice.h>
#include <Base/IO/MemoryDevice.h>
#include <Base/IO/MMapDevice.h>
#include <Base/IO/MultiDevice.h>
#include <Base/IO/NullDevice.h>
#include <Base/IO/Path.h>
#include <Base/IO/StreamIndent.h>
#include <Base/IO/StringDevice.h>
#include <Base/IO/TextStream.h>

#include <Base/ADT/Set.h>
#include <Base/ADT/String.h>
#include <Base/Dbg/DebugStream.h>
#include <Base/Util/Platform.h>

USING_NAMESPACE

void io_binary_stream( Test::Result& res )
{
   String str;
   RCP<StringDevice> idev = new StringDevice( str, IODevice::MODE_READ_WRITE );
   RCP<StringDevice> odev = new StringDevice( str, IODevice::MODE_READ_WRITE );
   //od = new FileDevice( "src/fs/dirB/fileB", IODevice::MODE_READ );
   //od = new FileDevice( "file.bin", IODevice::MODE_WRITE );
   BinaryStream bs( odev.ptr() );

   // Output some data.
   int8_t   o8   = 0x01;
   int16_t  o16  = 0x0302;
   int32_t  o32  = 0x07060504;
   int64_t  o64  = 0x15141312;
            o64 <<= 32;
            o64 |= 0x11100908;
   String   oStr("abc");
   bs << o8 << o16 << o32 << o64;
   bs << oStr;
   TEST_ADD( res, str.size() == 22 );
   TEST_ADD( res, odev->pos() == 22 );
   TEST_ADD( res, idev->pos() ==  0 );
   // Input same data, check consistency.
   bs.device( idev.ptr() );
   int8_t   i8;
   int16_t  i16;
   int32_t  i32;
   int64_t  i64;
   String   iStr;
   bs >> i8 >> i16 >> i32 >> i64;
   bs >> iStr;
   TEST_ADD( res, i8 == o8 );
   TEST_ADD( res, i16 == o16 );
   TEST_ADD( res, i32 == o32 );
   TEST_ADD( res, i64 == o64 );
   TEST_ADD( res, iStr == oStr );
}

void io_binary_stream_endian( Test::Result& res )
{
   String str;
   RCP<StringDevice> odev = new StringDevice( str, IODevice::MODE_READ_WRITE );
   RCP<StringDevice> idev = new StringDevice( str, IODevice::MODE_READ_WRITE );

   // Output data.
   int8_t   o8   = 0x01;
   int16_t  o16  = 0x0302;
   int32_t  o32  = 0x07060504;
   int64_t  o64  = 0x15141312;
            o64 <<= 32;
            o64 |= 0x11100908;
   String   oStr("abc");
   // Input variables.
   int8_t   i8;
   int16_t  i16;
   int32_t  i32;
   int64_t  i64;
   String   iStr;

   //////////////////////
   // Little endian order
   // Output some data.
   BinaryStream os_little( odev.ptr(), BinaryStream::ENDIAN_LITTLE );
   os_little << o8 << o16 << o32 << o64 << oStr;
   TEST_ADD( res, str.size() == 22 );
   TEST_ADD( res, str[ 0] == 0x01 );
   TEST_ADD( res, str[ 1] == 0x02 && str[ 2] == 0x03 );
   TEST_ADD( res, str[ 3] == 0x04 && str[ 4] == 0x05 && str[ 5] == 0x06 && str[ 6] == 0x07 );
   TEST_ADD( res, str[ 7] == 0x08 && str[ 8] == 0x09 && str[ 9] == 0x10 && str[10] == 0x11 &&
                  str[11] == 0x12 && str[12] == 0x13 && str[13] == 0x14 && str[14] == 0x15 );

   // Input same data, check consistency.
   BinaryStream is_little( idev.ptr(), BinaryStream::ENDIAN_LITTLE );
   is_little >> i8 >> i16 >> i32 >> i64 >> iStr;
   TEST_ADD( res, i8 == o8 );
   TEST_ADD( res, i16 == o16 );
   TEST_ADD( res, i32 == o32 );
   TEST_ADD( res, i64 == o64 );
   TEST_ADD( res, iStr == oStr );

   odev->reset();
   TEST_ADD( res, str.empty() );
   ///////////////////
   // Bit endian order
   // Output some data.
   BinaryStream os_big( odev.ptr(), BinaryStream::ENDIAN_BIG );
   os_big << o8 << o16 << o32 << o64 << oStr;
   TEST_ADD( res, str.size() == 22 );
   TEST_ADD( res, str[ 0] == 0x01 );
   TEST_ADD( res, str[ 1] == 0x03 && str[ 2] == 0x02 );
   TEST_ADD( res, str[ 3] == 0x07 && str[ 4] == 0x06 && str[ 5] == 0x05 && str[ 6] == 0x04 );
   TEST_ADD( res, str[ 7] == 0x15 && str[ 8] == 0x14 && str[ 9] == 0x13 && str[10] == 0x12 &&
                  str[11] == 0x11 && str[12] == 0x10 && str[13] == 0x09 && str[14] == 0x08 );
   // Input same data, check consistency.
   BinaryStream is_big( idev.ptr(), BinaryStream::ENDIAN_BIG );
   idev->seek( 0 );
   is_big >> i8 >> i16 >> i32 >> i64 >> iStr;
   TEST_ADD( res, i8 == o8 );
   TEST_ADD( res, i16 == o16 );
   TEST_ADD( res, i32 == o32 );
   TEST_ADD( res, i64 == o64 );
   TEST_ADD( res, iStr == oStr );

}

void io_file_input( Test::Result& res )
{
   RCP<FileDevice>  fd;
   String           str;
   char             tmp[100];

   // Testing read().
   fd = new FileDevice( "src/fs/fileC", IODevice::MODE_READ );
   TEST_ADD( res, fd->pos() == 0 );
   fd->read( tmp, 1 );
   TEST_ADD( res, fd->pos() == 1 );
   TEST_ADD( res, tmp[0] == 'a' );
   fd->read( tmp, 1 );
   TEST_ADD( res, fd->pos() == 2 );
   TEST_ADD( res, tmp[0] == 'b' );
   fd->read( tmp, 1 );
   TEST_ADD( res, fd->pos() == 3 );
   TEST_ADD( res, tmp[0] == 'c' );

   // Testing file re-opening.
   fd = new FileDevice( "src/fs/fileC", IODevice::MODE_READ );
   TEST_ADD( res, fd->pos() == 0 );

   // Testing readAll().
   str.clear();
   fd->readAll( str );
   TEST_ADD( res, str == "abc" );

   // Testing readSeek().
   fd = new FileDevice( "src/fs/dirB/fileB", IODevice::MODE_READ );
   fd->read( tmp, 10 ); str.clear(); str.append( tmp, 10 );
   TEST_ADD( res, str == "0123456789" );

   fd->seek( 1 );
   fd->read( tmp, 9 ); str.clear(); str.append( tmp, 9 );
   TEST_ADD( res, str == "123456789" );

   fd->seek( 2 );
   fd->read( tmp, 8 ); str.clear(); str.append( tmp, 8 );
   TEST_ADD( res, str == "23456789" );

   fd->seek( 1 );
   fd->read( tmp, 9 ); str.clear(); str.append( tmp, 9 );
   TEST_ADD( res, str == "123456789" );

   fd->seek( 0 );
   fd->read( tmp, 10 ); str.clear(); str.append( tmp, 10 );
   TEST_ADD( res, str == "0123456789" );

   // Testing peek().
   fd->seek( 0x10 );
   TEST_ADD( res, fd->pos() == 0x10 );
   fd->peek( tmp, 3 ); str.clear(); str.append( tmp, 3 );
   TEST_ADD( res, str == "abc" );
   TEST_ADD( res, fd->pos() == 0x10 );
   fd->peek( tmp+1, 1 ); str.clear(); str.append( tmp, 3 );
   TEST_ADD( res, str == "aac" );
   TEST_ADD( res, fd->pos() == 0x10 );
}

void io_file_input_gz( Test::Result& res )
{
   RCP<GZippedFileDevice>  fd;
   String                  str;
   char                    tmp[100];

   {
      // Uncompressed file (should be a passthrough).
      // Testing read().
      fd = new GZippedFileDevice( "src/fs/fileC", IODevice::MODE_READ );
      TEST_ADD( res, fd->pos() == 0 );
      fd->read( tmp, 1 );
      TEST_ADD( res, fd->pos() == 1 );
      TEST_ADD( res, tmp[0] == 'a' );
      fd->read( tmp, 1 );
      TEST_ADD( res, fd->pos() == 2 );
      TEST_ADD( res, tmp[0] == 'b' );
      fd->read( tmp, 1 );
      TEST_ADD( res, fd->pos() == 3 );
      TEST_ADD( res, tmp[0] == 'c' );

      // Testing file re-opening.
      fd = new GZippedFileDevice( "src/fs/fileC", IODevice::MODE_READ );
      TEST_ADD( res, fd->pos() == 0 );

      // Testing readAll().
      str.clear();
      fd->readAll( str );
      TEST_ADD( res, str == "abc" );

      // Testing readSeek().
      fd = new GZippedFileDevice( "src/fs/dirB/fileB", IODevice::MODE_READ );
      fd->read( tmp, 10 ); str.clear(); str.append( tmp, 10 );
      TEST_ADD( res, str == "0123456789" );

      fd->seek( 1 );
      fd->read( tmp, 9 ); str.clear(); str.append( tmp, 9 );
      TEST_ADD( res, str == "123456789" );

      fd->seek( 2 );
      fd->read( tmp, 8 ); str.clear(); str.append( tmp, 8 );
      TEST_ADD( res, str == "23456789" );

      fd->seek( 1 );
      fd->read( tmp, 9 ); str.clear(); str.append( tmp, 9 );
      TEST_ADD( res, str == "123456789" );

      fd->seek( 0 );
      fd->read( tmp, 10 ); str.clear(); str.append( tmp, 10 );
      TEST_ADD( res, str == "0123456789" );

      // Testing peek().
      fd->seek( 0x10 );
      TEST_ADD( res, fd->pos() == 0x10 );
      fd->peek( tmp, 3 ); str.clear(); str.append( tmp, 3 );
      TEST_ADD( res, str == "abc" );
      TEST_ADD( res, fd->pos() == 0x10 );
      fd->peek( tmp+1, 1 ); str.clear(); str.append( tmp, 3 );
      TEST_ADD( res, str == "aac" );
      TEST_ADD( res, fd->pos() == 0x10 );
   }

   {
      // Compressed file (should be a passthrough).
      // Testing read().
      fd = new GZippedFileDevice( "src/fs/fileC.gz", IODevice::MODE_READ );
      TEST_ADD( res, fd->pos() == 0 );
      fd->read( tmp, 1 );
      TEST_ADD( res, fd->pos() == 1 );
      TEST_ADD( res, tmp[0] == 'a' );
      fd->read( tmp, 1 );
      TEST_ADD( res, fd->pos() == 2 );
      TEST_ADD( res, tmp[0] == 'b' );
      fd->read( tmp, 1 );
      TEST_ADD( res, fd->pos() == 3 );
      TEST_ADD( res, tmp[0] == 'c' );

      // Testing file re-opening.
      fd = new GZippedFileDevice( "src/fs/fileC.gz", IODevice::MODE_READ );
      TEST_ADD( res, fd->pos() == 0 );

      // Testing readAll().
      str.clear();
      fd->readAll( str );
      TEST_ADD( res, str == "abc" );

      // Testing readSeek().
      fd = new GZippedFileDevice( "src/fs/dirB/fileB.gz", IODevice::MODE_READ );
      fd->read( tmp, 10 ); str.clear(); str.append( tmp, 10 );
      TEST_ADD( res, str == "0123456789" );

      fd->seek( 1 );
      fd->read( tmp, 9 ); str.clear(); str.append( tmp, 9 );
      TEST_ADD( res, str == "123456789" );

      fd->seek( 2 );
      fd->read( tmp, 8 ); str.clear(); str.append( tmp, 8 );
      TEST_ADD( res, str == "23456789" );

      fd->seek( 1 );
      fd->read( tmp, 9 ); str.clear(); str.append( tmp, 9 );
      TEST_ADD( res, str == "123456789" );

      fd->seek( 0 );
      fd->read( tmp, 10 ); str.clear(); str.append( tmp, 10 );
      TEST_ADD( res, str == "0123456789" );

      // Testing peek().
      fd->seek( 0x10 );
      TEST_ADD( res, fd->pos() == 0x10 );
      fd->peek( tmp, 3 ); str.clear(); str.append( tmp, 3 );
      TEST_ADD( res, str == "abc" );
      TEST_ADD( res, fd->pos() == 0x10 );
      fd->peek( tmp+1, 1 ); str.clear(); str.append( tmp, 3 );
      TEST_ADD( res, str == "aac" );
      TEST_ADD( res, fd->pos() == 0x10 );
   }
}

void io_file_output( Test::Result& res )
{
   for( uint i = 0; i < 512; ++i )
   {
      const char* path = "file_output.txt";
      FS::Entry entry = FS::Entry( path );
      entry.remove();
      entry.refresh();
      TEST_ADD( res, !entry.exists() );

      RCP<FileDevice> fd = new FileDevice( path, IODevice::MODE_WRITE );
      const char str[] = "0123456789\n"
                         "abcdefghijklmnopqrstuvwxyz\n"
                         "ABCDEFGHIJKLMNOPQRSTUVWXYZ\n";
      TEST_ADD( res, fd->isOpen() );
      if( fd->isOpen() )
      {
         for( uint i = 0; i < 10; ++i )
         {
            fd->write( str, sizeof(str)-1 );
         }
         fd->close();
      }

      entry.refresh();
      TEST_ADD( res, entry.exists() );
      TEST_ADD( res, entry.size() == (10+26+26+3)*10 );
      entry.remove();
   }
}

void io_file_output_gz( Test::Result& res )
{
   for( uint i = 0; i < 512; ++i )
   {
      const char* path = "file_output.txt.gz";
      FS::Entry entry = FS::Entry( path );
      entry.remove();
      entry.refresh();
      TEST_ADD( res, !entry.exists() );

      RCP<GZippedFileDevice> fd = new GZippedFileDevice( path, IODevice::MODE_WRITE );
      const char str[] = "0123456789\n"
                         "abcdefghijklmnopqrstuvwxyz\n"
                         "ABCDEFGHIJKLMNOPQRSTUVWXYZ\n";
      for( uint i = 0; i < 10; ++i )
      {
         fd->write( str, sizeof(str)-1 );
      }
      fd->close();

      entry.refresh();
      TEST_ADD( res, entry.exists() );
      TEST_ADD( res, entry.size() == 93 );
      entry.remove();
   }
}

void io_file_input_output( Test::Result& res )
{
   Path  path( Path::getCurrentDirectory() );
   path /= "tmp_io_file_input_output";

   RCP<FileDevice> fd = new FileDevice( path, IODevice::MODE_READ_WRITE|IODevice::MODE_NEWFILE );
   TEST_ADD( res, fd->ok() );
   TEST_ADD( res, fd->isReadable() );
   TEST_ADD( res, fd->isWritable() );
   TEST_ADD( res, fd->isReadWrite() );
   TEST_ADD( res, fd->pos() == 0 );

   TextStream ios( fd.ptr() );
   char ch = '_';
   String str;

   ios << "abcdef";
   TEST_ADD( res, fd->pos() == 6 );
   ios.device()->seek( 2 );
   ios << "CD";
#if _MSC_VER
   fd->flush(); // Sounds like a bug in VS2010; doesn't like swapping read and write.
#endif
   TEST_ADD( res, fd->pos() == 4 );
   ios >> ch;
   TEST_ADD( res, fd->pos() == 5 );
   TEST_ADD( res, ch == 'e' );
   ios.device()->seek( 0 );
   TEST_ADD( res, fd->pos() == 0 );
   ios >> str;
   TEST_ADD( res, str == "abCDef" );

   fd->close();

   FS::remove( path );
}

void io_fs( Test::Result& res )
{
   Path  basePath( Path::getCurrentDirectory() );
   basePath /= "src";
   basePath /= "fs";

   Path  path( basePath );
   FS::Entry  entry( path );
   TEST_ADD( res, entry.exists() );
   TEST_ADD( res, entry.type() == FS::TYPE_DIRECTORY );
   TEST_ADD( res, !entry.needsRedirect() );

   path /= "dirA";
   entry.refresh( path );
   TEST_ADD( res, entry.exists() );
   TEST_ADD( res, entry.type() == FS::TYPE_DIRECTORY );
   TEST_ADD( res, !entry.needsRedirect() );

   path /= "fileA";
   entry.refresh( path );
   TEST_ADD( res, entry.exists() );
   TEST_ADD( res, entry.type() == FS::TYPE_FILE );
   TEST_ADD( res, !entry.needsRedirect() );
   TEST_ADD( res, entry.size() == 0 );

   path /= "bogus";
   entry.refresh( path );
   TEST_ADD( res, !entry.exists() );
   TEST_ADD( res, entry.type() == FS::TYPE_UNKNOWN );

   path = basePath;
   path /= "fileC";
   entry.refresh( path );
   TEST_ADD( res, entry.exists() );
   TEST_ADD( res, entry.size() == 3 );

   // Directory iteration.
   // Since the alphabetical order is not guaranteed, we store in a set of strings.
   FS::DirIterator iter( basePath );
   Set<String> subdirs;

   TEST_ADD( res, iter() );
   subdirs.add( *iter );
   ++iter;
   TEST_ADD( res, iter() );
   subdirs.add( *iter );
   ++iter;
   TEST_ADD( res, iter() );
   subdirs.add( *iter );
   ++iter;
   TEST_ADD( res, iter() );
   subdirs.add( *iter );
   ++iter;
   TEST_ADD( res, iter() );
   subdirs.add( *iter );
   ++iter;
   TEST_ADD( res, iter() );
   subdirs.add( *iter );
   ++iter;
   TEST_ADD( res, !iter() );
   ++iter;
   TEST_ADD( res, !iter() );

   TEST_ADD( res, subdirs.has("."    ) );
   TEST_ADD( res, subdirs.has(".."   ) );
   TEST_ADD( res, subdirs.has("dirA" ) );
   TEST_ADD( res, subdirs.has("dirB" ) );
   TEST_ADD( res, subdirs.has("fileC") );

   path /= "bogus";
   iter.reset( path );
   TEST_ADD( res, !iter() );
   ++iter;
   TEST_ADD( res, !iter() );
}

void io_indent( Test::Result& res )
{
   printf("\n");
   StreamIndent indent( ">>>", 3 );
   StdErr << indent << "1." << nl;
   ++indent;
   StdErr << indent << "1.1" << nl;
   ++indent;
   StdErr << indent << "1.1.1" << nl;
   --indent;
   StdErr << indent << "1.2" << nl;
   ++indent;
   StdErr << indent << "1.2.1" << nl;
   --indent;
   StdErr << indent << "1.3" << nl;
   --indent;
   StdErr << indent << "2" << nl;

   StdErr << indent << "A" << nl;
   indent += 2;
   StdErr << indent << "AAA" << nl;
   indent -= 1;
   StdErr << indent << "AA" << nl;
   indent -= 1;

   StdErr << indent << "0 space" << nl;
   indent.increase(1);
   StdErr << indent << "1 space" << nl;
   indent.increase(1);
   StdErr << indent << "2 spaces" << nl;
   indent.increase(2);
   StdErr << indent << "4 spaces" << nl;
   --indent;
   StdErr << indent << "1 space" << nl;
   indent.decrease(1);
   StdErr << indent << "0 space" << nl;

   TEST_ADD( res, false );
}

void io_locked_memory_device( Test::Result& res )
{
   RCP<LockedMemoryDevice>  dev;
   const char*  istr = "This is my string";
   char         ostr[32];
   size_t       s;

   // Test input.
   dev = new LockedMemoryDevice( istr, strlen(istr) );
   s = dev->read( ostr, 4 );
   TEST_ADD( res, s == 4 );
   TEST_ADD( res, strncmp(ostr, "This", 4) == 0 );
   s = dev->read( ostr, 5 );
   TEST_ADD( res, s == 5 );
   TEST_ADD( res, strncmp(ostr, " is m", 5) == 0 );

   // Test input and output.
   dev = new LockedMemoryDevice( ostr, 4, IODevice::MODE_READ_WRITE );
   s = dev->write( "ab", 2 );
   TEST_ADD( res, s == 2 );
   TEST_ADD( res, ostr[0] == 'a' && ostr[1] == 'b' );
   s = dev->write( "c", 1 );
   TEST_ADD( res, s == 1 );
   TEST_ADD( res, ostr[0] == 'a' && ostr[1] == 'b' && ostr[2] == 'c' );
   TEST_ADD( res, dev->pos() == 3 );
   dev->seek(0);
   TEST_ADD( res, dev->pos() == 0 );
   s = dev->read( ostr+3, 3 );
   TEST_ADD( res, s == 3 );
   TEST_ADD( res, ostr[0] == 'a' && ostr[1] == 'b' && ostr[2] == 'c' &&
                  ostr[3] == 'a' && ostr[4] == 'b' && ostr[5] == 'c' );
   s = dev->write( "DEF", 3 );
   TEST_ADD( res, s == 1 );
   dev->seek( dev->pos() - s );
   s = dev->read( ostr, 3 );
   TEST_ADD( res, s == 1 );
   TEST_ADD( res, ostr[0] == 'D' );
}

void io_memory_device( Test::Result& res )
{
   RCP<MemoryDevice> device( new MemoryDevice( IODevice::MODE_READ_WRITE ) );

   const char*  istr = "This is my string";
   char         ostr[32];
   size_t       s;

   TEST_ADD( res, device->size() == 0 );
   s = device->write( istr, 4 );
   TEST_ADD( res, s == 4 );
   TEST_ADD( res, device->size() == 4 );
   TEST_ADD( res, device->pos()  == 4 );
   device->seek( 0 );
   TEST_ADD( res, device->pos()  == 0 );
   s = device->read( ostr, 5 );
   TEST_ADD( res, s == 4 );
   TEST_ADD( res, strncmp(ostr, istr, 4) == 0 );
   s = device->read( ostr, 5 );
   TEST_ADD( res, s == 0 );

   s  = device->write( istr+4, 1 ); // ' '
   s += device->write( istr+5, 2 ); // 'is'
   TEST_ADD( res, s == 3 );
   device->seek( device->pos() - s );
   s = device->read( ostr, 3 );
   TEST_ADD( res, s == 3 );
   TEST_ADD( res, strncmp(ostr, " is", 3) == 0 );
}

void io_mmap_device( Test::Result& res )
{
   FS::Entry e = FS::Entry( "src/fs/fileC" );
   RCP<MMapDevice> mmap = new MMapDevice( e, IODevice::MODE_READ );
   TEST_ADD( res, mmap->ok() );
   const char* str = mmap->bytes();
   TEST_ADD( res, str != NULL && strncmp(str, "abc", 3) == 0 );
}

void io_multi_device( Test::Result& res )
{
   RCP<MultiDevice>  md = new MultiDevice();

   TextStream os( md.ptr() );

   String str1, str2;
   RCP<StringDevice> sd1 = new StringDevice( str1 );
   RCP<StringDevice> sd2 = new StringDevice( str2 );

   // Only write into sd1.
   md->add( sd1.ptr() );
   TEST_ADD( res, str1.empty() );
   TEST_ADD( res, str2.empty() );
   os << "Test1" << nl;
   TEST_ADD( res, !str1.empty() );
   TEST_ADD( res, str1 == "Test1\n" );
   TEST_ADD( res, str2.empty() );

   // Reset sd1 and sd2.
   sd1->reset();
   sd2->reset();

   // Write into sd1 and sd2.
   md->add( sd2.ptr() );
   TEST_ADD( res, str1.empty() );
   TEST_ADD( res, str2.empty() );
   os << "Test2" << nl;
   TEST_ADD( res, !str1.empty() );
   TEST_ADD( res, str1 == "Test2\n" );
   TEST_ADD( res, !str2.empty() );
   TEST_ADD( res, str2 == "Test2\n" );
}

void io_path( Test::Result& res )
{
   Path path;
   path = "/";
   TEST_ADD( res, path.getLongExt() == "" );
   path = "/file.ext";
   TEST_ADD( res, path.getLongExt() == "ext" );
   path = "/file.ext1.ext2";
   TEST_ADD( res, path.getLongExt() == "ext1.ext2" );
   path = "/dir/file.tar.gz";
   TEST_ADD( res, path.getLongExt() == "tar.gz" );
   path = "some/dir/file.ext1.ext2.ext3";
   String ext = path.getLongExt();
   TEST_ADD( res, ext == "ext1.ext2.ext3" );
   ext = Path::nextExt( ext );
   TEST_ADD( res, ext == "ext2.ext3" );
   ext = Path::nextExt( ext );
   TEST_ADD( res, ext == "ext3" );
   ext = Path::nextExt( ext );
   TEST_ADD( res, ext == "" );

   TEST_ADD( res, Path("abc/def").basename() == "def" );
   TEST_ADD( res, Path("abc/def/").basename() == "def" );
}

void io_string_device( Test::Result& res )
{
   String str;
   RCP<StringDevice> sd( new StringDevice( str ) );
   TextStream os( sd.ptr() );

   TEST_ADD( res, str.empty() );
   os << "Test" << nl;
   TEST_ADD( res, str == "Test\n" );
}

void io_string_device_redirect( Test::Result& res )
{
   String str;
   RCP<StringDevice> sd( new StringDevice( str ) );
   DebugStream os1( "test" );
   DebugStream os2( "test" );

   os1.activate();
   os1.device( sd.ptr() ); // Redirect stream into the specified device.

   TEST_ADD( res, str.empty() );
   os1 << "Test1" << nl;
   TEST_ADD( res, str == "Test1\n" );

   sd->reset();
   // Check that os2 is redirected as well.
   os2 << "Test2" << nl;
   TEST_ADD( res, str == "Test2\n" );
}

void io_line_iterator( Test::Result& res )
{
   TextStream os( NULL );
   TextStream::LineIterator iter;
   String str;

//#if 1
   // An empty file has 1 empty line.
   str = "";
   os.device( new LockedMemoryDevice( str.cstr(), str.size() ) );
   iter = os.lines();
   TEST_ADD( res, iter.isValid() );
   TEST_ADD( res, (*iter).empty() );
   ++iter;
   TEST_ADD( res, !iter.isValid() );
   TEST_ADD( res, (*iter).empty() );

   // Testing that this has 2 empty lines.
   str = "\n";
   os.device( new LockedMemoryDevice( str.cstr(), str.size() ) );
   iter = os.lines();
   TEST_ADD( res, iter.isValid() );
   TEST_ADD( res, iter->empty() );
   ++iter;
   TEST_ADD( res, iter.isValid() );
   TEST_ADD( res, (*iter).empty() );
   ++iter;
   TEST_ADD( res, !iter.isValid() );
   TEST_ADD( res, (*iter).empty() );

   str.clear();
   const char* lines[] = {
      "First line",
      "Second line",
      "Non standard line\r",
      "A very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very long line",
      "",
      "Last line"
   };
   for( int i = 0; i < 6; ++i )
   {
      str += lines[i];
      if( i != 5 ) str += '\n';
   }
   os.device( new LockedMemoryDevice(str.cstr(), str.size()) );
   int i = 0;
   for( TextStream::LineIterator iter = os.lines(); iter.isValid(); ++iter, ++i )
   {
      const String& line = *iter;
      TEST_ADD( res, line == lines[i] );
   }

   str = "vertex/m\nfragment/colorTex";
   os.device( new LockedMemoryDevice( str.cstr(), str.size() ) );
   iter = os.lines();
   TEST_ADD( res, iter.isValid() );
   TEST_ADD( res, (*iter) == "vertex/m" );
   ++iter;
   TEST_ADD( res, iter.isValid() );
   TEST_ADD( res, (*iter) == "fragment/colorTex" );
   ++iter;
   TEST_ADD( res, !iter.isValid() );

}

void io_sync( Test::Result& /*res*/ )
{
   fprintf(stderr, "\n");
   fprintf(stderr, " =%d\n", (int)ftell(stderr));
   StdErr << "Something" << nl;
   fprintf(stderr, " =%d\n", (int)ftell(stderr));
}

void io_text_stream( Test::Result& res )
{
   printf("\n");
   TextStream os( new NullDevice() );
   os << "Test" << nl;

   os.device( new FileDevice(stderr, IODevice::MODE_WRITE) );
   os << "Test2" << nl;

   StdErr << "Test of StdErr" << nl;
   StdErr.device( new FileDevice( Path("base.log"), IODevice::MODE_WRITE|IODevice::MODE_MOVE_TO_END ) );
   StdErr << "Test of StdErr in base.log" << nl;

   String str;
   TextStream os2( str );
   TEST_ADD( res, str.empty() );
   os2 << "test";
   TEST_ADD( res, str == "test" );

   str.clear();
   os2 << (size_t)11 << (ptrdiff_t)-22;
   TEST_ADD( res, str == "11-22" );
}

void init_io()
{
   RCP<Test::Collection> col = new Test::Collection( "io", "Collection for Base/IO" );
   col->add( new Test::Function("binary_stream"         , "Tests binary stream"                                  , io_binary_stream         ) );
   col->add( new Test::Function("binary_stream_endian"  , "Tests binary stream endianness"                       , io_binary_stream_endian  ) );
   col->add( new Test::Function("file_input"            , "Tests FileDevice input capability"                    , io_file_input            ) );
   col->add( new Test::Function("file_input_gz"         , "Tests GZippedFileDevice input capability"             , io_file_input_gz         ) );
   col->add( new Test::Function("file_output"           , "Tests FileDevice output capability"                   , io_file_output           ) );
   col->add( new Test::Function("file_output_gz"        , "Tests GZippedFileDevice output capability"            , io_file_output_gz        ) );
   col->add( new Test::Function("file_input_output"     , "Tests FileDevice input+output capability"             , io_file_input_output     ) );
   col->add( new Test::Function("fs"                    , "Tests routines present in the FileSystem class"       , io_fs                    ) );
   col->add( new Test::Function("locked_memory_device"  , "Tests LockedMemoryDevice input and output capability" , io_locked_memory_device  ) );
   col->add( new Test::Function("memory_device"         , "Tests MemoryDevice input and output capability"       , io_memory_device         ) );
   col->add( new Test::Function("mmap_device"           , "Tests MMapDevice input and output capability"         , io_mmap_device           ) );
   col->add( new Test::Function("multi_device"          , "Tests forking a stream's output through a MultiDevice", io_multi_device          ) );
   col->add( new Test::Function("path"                  , "Tests the Path class"                                 , io_path                  ) );
   col->add( new Test::Function("string_device"         , "Tests dumping a stream into a String"                 , io_string_device         ) );
   col->add( new Test::Function("string_device_redirect", "Tests redirecting a DebugStream into a String"        , io_string_device_redirect) );
   col->add( new Test::Function("line_iter"             , "Tests LineIterator"                                   , io_line_iterator         ) );
   Test::standard().add( col.ptr() );

   Test::special().add( new Test::Function("indent"     , "Prints out indentation variations"    , io_indent     ) );
   Test::special().add( new Test::Function("text_stream", "Prints out data using a TextStream"   , io_text_stream) );
   Test::special().add( new Test::Function("io_sync"    , "Tries to get a FileDevice out-of-sync", io_sync       ) );
}
