/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#ifndef BASE_IODEVICE_H
#define BASE_IODEVICE_H

#include <Base/StdDefs.h>

#include <Base/ADT/String.h>
#include <Base/Util/Enum.h>
#include <Base/Util/RCObject.h>

#include <cstddef>

NAMESPACE_BEGIN

/*==============================================================================
  CLASS IODevice
==============================================================================*/
class IODevice:
   public RCObject
{
public:

   /*----- types -----*/
   enum Mode
   {
      MODE_UNKNOWN     = 0x0000,
      MODE_READ        = 0x0001,
      MODE_WRITE       = 0x0002,
      MODE_READ_WRITE  = (MODE_READ|MODE_WRITE),
      MODE_NEWFILE     = 0x0004,
      MODE_MOVE_TO_END = 0x0008,
      MODE_STRICT      = 0x0010, // Strict streams cannot seek.
      MODE_MASK        = 0x001F
   };

   enum State
   {
      STATE_BAD  = 0x0000,
      STATE_OK   = 0x0001,
      STATE_EOF  = 0x0002,
      STATE_MASK = 0x0003
   };

   static BASE_DLL_API const size_t INVALID_SIZE = (size_t)-1;

   /*----- methods -----*/

   BASE_DLL_API IODevice( Mode mode = MODE_UNKNOWN );
   BASE_DLL_API virtual ~IODevice();

   // State.
   //BASE_DLL_API virtual bool dataReady();
   //BASE_DLL_API virtual bool dataDone();
   inline State  state() const { return (State)_state; }
   inline bool bad() const { return (_state            ) == STATE_BAD; }
   inline bool ok()  const { return (_state & STATE_OK)  == STATE_OK ; }
   inline bool eof() const { return (_state & STATE_EOF) == STATE_EOF; }
   inline bool dataLeft() const { return (_state & (STATE_OK|STATE_EOF)) == STATE_OK; }

   // Mode.
   inline Mode  mode() const { return (Mode)_mode; }
   inline bool  isReadable() const { return (_mode & MODE_READ) != 0; }
   inline bool  isWritable() const { return (_mode & MODE_WRITE) != 0; }
   inline bool  isReadWrite() const { return (_mode & MODE_READ_WRITE) == MODE_READ_WRITE; }
   inline bool  isStrict() const { return (_mode & MODE_STRICT) == MODE_STRICT; }

   // Opening and Closing.
   //BASE_DLL_API virtual bool open( Mode mode );
   //BASE_DLL_API virtual void close();

   // Utility.
   BASE_DLL_API size_t  getSize();

   inline       bool    seek( size_t pos ) { return doSeek(pos); }
   inline       size_t  pos() const        { return doPos();     }

   // Input related routines.
   BASE_DLL_API size_t  read( char* data, size_t n );
   BASE_DLL_API bool    readAll( String& dst );
   inline       size_t  peek( char* data, size_t n ) { return doPeek(data, n); }

   // Output related routines.
   BASE_DLL_API size_t  write( const char* data, size_t n );
   inline       bool    flush() { return doFlush(); }

protected:

   /*----- methods -----*/

   inline void  setMode( Mode m ) { _mode = m; }
   inline void  addMode( Mode m ) { _mode |= m; }
   inline void  removeMode( Mode m ) { _mode &= ~m; }

   inline void  setState( State s ) { _state = s; }
   inline void  addState( State s ) { _state |= s; }
   inline void  removeState( State s ) { _state &= ~s; }

   // Virtual interface.

   BASE_DLL_API virtual bool    doSeek( size_t pos );
   BASE_DLL_API virtual size_t  doPos() const;

   BASE_DLL_API virtual size_t  doRead( char* data, size_t n ) = 0;
   BASE_DLL_API virtual size_t  doPeek( char* data, size_t n );

   BASE_DLL_API virtual size_t  doWrite( const char* data, size_t n ) = 0;
   BASE_DLL_API virtual bool    doFlush();

private:
   /*----- data members -----*/

   int     _mode;  //!< The current mode (read, write, or read+write, append, etc.).
   int     _state; //!< The current state.

}; //class IODevice


GEN_ENUM_BITWISE_OPS( IODevice::Mode ) //;

//class NullDevice:
//   public IODevice
//
//class FileDevice:
//   public IODevice
//
//class MemDevice:
//   public IODevice
//
//class StringDevice:
//   public MemDevice
//
//class SocketDevice:
//   public IODevice
//
//class UDPDevice:
//   public SocketDevice
//
//class TCPDevice:
//   public SocketDevice


NAMESPACE_END

#endif //BASE_IODEVICE_H
