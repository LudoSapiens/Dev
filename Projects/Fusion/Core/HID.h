/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#ifndef FUSION_HID_H
#define FUSION_HID_H

#include <Fusion/StdDefs.h>

#include <Fusion/VM/VM.h>

#include <Base/ADT/String.h>
#include <Base/Dbg/Defs.h>
#include <Base/ADT/Vector.h>


NAMESPACE_BEGIN


//------------------------------------------------------------------------------
//! A variadic value type.
typedef union
{
   int    iValue;
   uint   uValue;
   float  fValue;
   void*  pValue;
} HIValue;

enum HIDValueType
{
   HID_VALUE_TYPE_INT,
   HID_VALUE_TYPE_UINT,
   HID_VALUE_TYPE_FLOAT,
   HID_VALUE_TYPE_POINTER
};

enum
{
   HID_INVALID_ID = (uint)-1
};


/*==============================================================================
  CLASS HIDControl
==============================================================================*/
//! Anything that the user can modify on the device which can trigger an event.
class HIDControl
{
public:

   /*----- methods -----*/

   FUSION_DLL_API HIDControl();
   FUSION_DLL_API virtual ~HIDControl();

   const String&  name() const { return _name; }
   void  name( const String& name ) { _name = name; }

   HIDValueType  valueType() const { return _type; }
   void  valueType( const HIDValueType vt ) { _type = vt; }

   HIValue  min() const { return _min; }
   HIValue  max() const { return _max; }
   HIValue  def() const { return _def; }
   HIValue  cur() const { return _cur; }

   void  min( const HIValue v ) { _min = v; }
   void  max( const HIValue v ) { _max = v; }
   void  def( const HIValue v ) { _def = v; }
   void  cur( const HIValue v ) { _cur = v; }

   // Button interface.
   bool  on()  const { return _cur.uValue != 0; }
   bool  off() const { return _cur.uValue == 0; }
   void  set( const bool v ) { _cur.uValue = (v?1:0); }

   // Slider interface.
   int asInt() const { return _cur.iValue; }
   uint asUInt() const { return _cur.uValue; }
   float asFloat() const { return _cur.fValue; }
   void* asPointer() const { return _cur.pValue; }

   void toInt( const int v ) { _cur.iValue = v; }
   void toUInt( const uint v ) { _cur.uValue = v; }
   void toFloat( const float v ) { _cur.fValue = v; }
   void toPointer( void* v ) { _cur.pValue = v; }

   FUSION_DLL_API float  getNormalized() const;
   FUSION_DLL_API void   setNormalized( float v );

protected:

   /*----- data members -----*/

   String        _name;  //!< A descriptive name to distinguish from other controls.
   HIDValueType  _type;  //!< The type of value used in this HIDControl.
   HIValue       _min;   //!< The minimum value possible.
   HIValue       _max;   //!< The maximum value.
   HIValue       _def;   //!< The default value (akin to the zero).
   HIValue       _cur;   //!< The current value.

   /*----- methods -----*/

   /* methods... */

private:
}; //class HIDControl


/*==============================================================================
  CLASS HIDFeedback
==============================================================================*/
//! Anything that the HIDevice can use to inform the user:
//! - Rumble motors
//! - LED indicators
//! - Speakers
class HIDFeedback
{
public:
   /*----- types -----*/
   enum
   {
      HID_FEEDBACK_TYPE_RUMBLE,
      HID_FEEDBACK_TYPE_LIGHT,
      HID_FEEDBACK_TYPE_SOUND
   } Type;

   /*----- methods -----*/

   FUSION_DLL_API HIDFeedback();
   FUSION_DLL_API virtual ~HIDFeedback();


protected:

   /*----- data members -----*/

   /* members... */

   /*----- methods -----*/

   /* methods... */

private:
}; //class HIDFeedback


/*==============================================================================
  CLASS HIDevice
==============================================================================*/
//! A class used to represent all of the Human Interface Devices.
class HIDevice
{
public:

   /*----- types -----*/
   enum DeviceType
   {
      DEVICE_TYPE_INVALID,
      DEVICE_TYPE_GAMEPAD,
      DEVICE_TYPE_WHEEL,
      DEVICE_TYPE_OTHER
   };

   typedef Vector< HIDControl >  ControlContainer;

   /*----- methods -----*/

   FUSION_DLL_API HIDevice( const String& name, const DeviceType type );
   FUSION_DLL_API virtual ~HIDevice();

   const String&  name() const { return _name; }
   void  name( const String& name ) { _name = name; }

   DeviceType  type() const { return _type; }
   void  type( const DeviceType type ) { _type = type; }

         ControlContainer&  controls()       { return _controls; }
   const ControlContainer&  controls() const { return _controls; }

   uint  numControls() const { return (uint)_controls.size(); }

         HIDControl& control( uint cid )       { CHECK((size_t)cid < _controls.size()); return _controls[cid]; }
   const HIDControl& control( uint cid ) const { CHECK((size_t)cid < _controls.size()); return _controls[cid]; }

   uint  add( const HIDControl& control ) { uint id = (uint)_controls.size(); _controls.pushBack(control); return id; }

protected:

   /*----- data members -----*/

   String            _name;      //!< A string representing the device.
   DeviceType        _type;      //!< An enumerated type.
   ControlContainer  _controls;  //!< All of the controls this device has.

private:
}; //class HIDevice


/*==============================================================================
  CLASS HIDManager
==============================================================================*/
//!< The HIDManager is responsible for keeping all of the HIDDevices.
//!< It allows to access all of them.
class HIDManager
{
public:

   /*----- types -----*/
   typedef Vector< HIDevice >  DeviceContainer;

   /*----- methods -----*/

   static bool  initialize( VMState* vm, const char* nameSpace );
   static bool  terminate();

   static FUSION_DLL_API const DeviceContainer&  devices();

   static FUSION_DLL_API uint  numDevices();

   static FUSION_DLL_API HIDevice&  device( uint id );

   static FUSION_DLL_API HIDevice*  find( const String& name );

   static FUSION_DLL_API uint  getID( const HIDevice& dev );

   static FUSION_DLL_API HIDControl*  getControl( int deviceTypeID, int deviceID, int controlID );

   // These should be protected...
   uint  addDevice( HIDevice& device );
   void  retireDevice( HIDevice& device );

protected:

   /*----- data members -----*/

   DeviceContainer  _devices;

   /*----- methods -----*/

   HIDManager();
   virtual ~HIDManager();

private:
}; //class HIDManager


//------------------------------------------------------------------------------
//! Control IDs for the XBox 360 controller.
enum
{
   HID_GAMEPAD_XINPUT_DPAD_UP,
   HID_GAMEPAD_XINPUT_DPAD_DOWN,
   HID_GAMEPAD_XINPUT_DPAD_LEFT,
   HID_GAMEPAD_XINPUT_DPAD_RIGHT,
   HID_GAMEPAD_XINPUT_START,
   HID_GAMEPAD_XINPUT_BACK,
   HID_GAMEPAD_XINPUT_LEFT_THUMB,
   HID_GAMEPAD_XINPUT_RIGHT_THUMB,
   HID_GAMEPAD_XINPUT_LEFT_SHOULDER,
   HID_GAMEPAD_XINPUT_RIGHT_SHOULDER,
   HID_GAMEPAD_XINPUT_A,
   HID_GAMEPAD_XINPUT_B,
   HID_GAMEPAD_XINPUT_X,
   HID_GAMEPAD_XINPUT_Y,
   HID_GAMEPAD_XINPUT_LEFT_TRIGGER,
   HID_GAMEPAD_XINPUT_RIGHT_TRIGGER,
   HID_GAMEPAD_XINPUT_THUMB_LX,
   HID_GAMEPAD_XINPUT_THUMB_LY,
   HID_GAMEPAD_XINPUT_THUMB_RX,
   HID_GAMEPAD_XINPUT_THUMB_RY,
   HID_GAMEPAD_XINPUT_XBOX //Unsupported
};


NAMESPACE_END


#endif //FUSION_HID_H
