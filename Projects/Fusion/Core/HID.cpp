/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#include <Fusion/Core/HID.h>

#include <Fusion/Core/Core.h>

#include <Base/Dbg/DebugStream.h>
#include <Base/Util/Platform.h>

#ifndef FUSION_HID_XINPUT_SUPPORT
#if PLAT_WINDOWS
#define FUSION_HID_XINPUT_SUPPORT 1
#else
#define FUSION_HID_XINPUT_SUPPORT 0
#endif
#endif //FUSION_HID_XINPUT_SUPPORT

#ifndef FUSION_HID_IOKIT_SUPPORT
#ifdef __APPLE__
#define FUSION_HID_IOKIT_SUPPORT 1
#else
#define FUSION_HID_IOKIT_SUPPORT 0
#endif
#endif //FUSION_HID_IOKIT_SUPPORT

#if FUSION_HID_IOKIT_SUPPORT
#include <CoreFoundation/CoreFoundation.h>
#include <IOKit/IOKitLib.h>
#include <IOKit/IOCFPlugIn.h>
#include <IOKit/IOTypes.h>
#include <IOKit/hid/IOHIDKeys.h>
#include <IOKit/hid/IOHIDLib.h>
#include <IOKit/hid/IOHIDUsageTables.h>
#include <IOKit/usb/USB.h>
#include <ForceFeedback/ForceFeedback.h>
#include <Fusion/Core/Animator.h>
#include <Base/ADT/Map.h>
#endif //FUSION_HID_IOKIT_SUPPORT

#if FUSION_HID_XINPUT_SUPPORT
#include <Base/Util/windows.h>
#include <XInput.h>
#include <Fusion/Core/Animator.h>
#endif //FUSION_HID_XINPUT_SUPPORT

#if !defined(XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE)
#define XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE  7849
#define XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE 8689
#define XINPUT_GAMEPAD_TRIGGER_THRESHOLD    30
#endif


USING_NAMESPACE

/*==============================================================================
  UNNAMED NAMESPACE
==============================================================================*/
UNNAMESPACE_BEGIN

DBG_STREAM( os_hid, "HID" );

//------------------------------------------------------------------------------
//!
HIDManager* _manager = 0;

const VM::EnumReg _enumsDeviceType[] = {
   { "INVALID", HIDevice::DEVICE_TYPE_INVALID },
   { "GAMEPAD", HIDevice::DEVICE_TYPE_GAMEPAD },
   { "WHEEL"  , HIDevice::DEVICE_TYPE_WHEEL   },
   { "OTHER"  , HIDevice::DEVICE_TYPE_OTHER   },
   { 0, 0 }
};

const VM::EnumReg _enumsXInput[] = {
   { "DPAD_UP"       , HID_GAMEPAD_XINPUT_DPAD_UP        },
   { "DPAD_DOWN"     , HID_GAMEPAD_XINPUT_DPAD_DOWN      },
   { "DPAD_LEFT"     , HID_GAMEPAD_XINPUT_DPAD_LEFT      },
   { "DPAD_RIGHT"    , HID_GAMEPAD_XINPUT_DPAD_RIGHT     },
   { "START"         , HID_GAMEPAD_XINPUT_START          },
   { "BACK"          , HID_GAMEPAD_XINPUT_BACK           },
   { "LEFT_THUMB"    , HID_GAMEPAD_XINPUT_LEFT_THUMB     },
   { "RIGHT_THUMB"   , HID_GAMEPAD_XINPUT_RIGHT_THUMB    },
   { "LEFT_SHOULDER" , HID_GAMEPAD_XINPUT_LEFT_SHOULDER  },
   { "RIGHT_SHOULDER", HID_GAMEPAD_XINPUT_RIGHT_SHOULDER },
   { "A"             , HID_GAMEPAD_XINPUT_A              },
   { "B"             , HID_GAMEPAD_XINPUT_B              },
   { "X"             , HID_GAMEPAD_XINPUT_X              },
   { "Y"             , HID_GAMEPAD_XINPUT_Y              },
   { "LEFT_TRIGGER"  , HID_GAMEPAD_XINPUT_LEFT_TRIGGER   },
   { "RIGHT_TRIGGER" , HID_GAMEPAD_XINPUT_RIGHT_TRIGGER  },
   { "THUMB_LX"      , HID_GAMEPAD_XINPUT_THUMB_LX       },
   { "THUMB_LY"      , HID_GAMEPAD_XINPUT_THUMB_LY       },
   { "THUMB_RX"      , HID_GAMEPAD_XINPUT_THUMB_RX       },
   { "THUMB_RY"      , HID_GAMEPAD_XINPUT_THUMB_RY       },
   { 0, 0 }
};

/*==============================================================================
  IOKIT SUPPORT
==============================================================================*/
#if FUSION_HID_IOKIT_SUPPORT

const uint MAX_CONTROLLERS = 4;  // XInput handles up to 4 controllers

struct IOKitControl
{
   IOHIDElementCookie  cookie;
   int                 flipFactor;

   IOKitControl(): cookie(0), flipFactor(1) { }
};

struct IOKitController
{
   int                     deviceTypeID;
   int                     deviceID;
   io_object_t             hidDevice;
   IOHIDDeviceInterface**  deviceInterface;
   Vector<IOKitControl>    controls;
   bool                    connected;
   IOKitController(): deviceTypeID(-1), deviceID(-1), hidDevice(NULL), deviceInterface(NULL), connected(false){}
};

Vector< IOKitController >  _gControllers;
RCP<Animator>              _gIOKitAnimator;

//------------------------------------------------------------------------------
//! Retrieves the current state of the control, and stores it in an HIValue.
//! Returns false on error.
inline bool  getControlHIValue( IOKitController& controller, int controlID, HIValue& dst )
{
   IOKitControl& ioControl = controller.controls[controlID];

   IOHIDEventStruct hidEvent;
   IOReturn ior = (*controller.deviceInterface)->getElementValue(
      controller.deviceInterface,
      ioControl.cookie,
      &hidEvent
   );
   if( ior == kIOReturnSuccess )
   {
      dst.iValue = hidEvent.value * ioControl.flipFactor;
      return true;
   }
   else
   {
      //printf("Error getting element value for controlID=%d (getElementValue returned %d)\n",
      //       controlID, ior);
      //CHECK( false );
      controller.connected = false;
      return false;
   }
}

//------------------------------------------------------------------------------
//! Updates all of the controls for the specified controller.
//! Returns a list of events to send for all of the controls that changed state.
void  updateControllerState( IOKitController& controller, Vector<Event>& events )
{
   HIDevice& dev = _manager->device( controller.deviceID );
   if( controller.connected )
   {
      if( dev.type() == HIDevice::DEVICE_TYPE_INVALID )
      {
         DBG_MSG( os_hid, "Connecting controller" );
         dev.type( HIDevice::DEVICE_TYPE_GAMEPAD );
      }

      HIValue v, h;

#define CHECK_AND_UPDATE_BUTTON( controlID ) \
      do \
      { \
         if( getControlHIValue(controller, controlID, v) ) \
         { \
            HIDControl& control = dev.control(controlID); \
            if( v.iValue != control.cur().iValue ) \
            { \
               control.cur( v ); \
               events.pushBack( \
                  Event::HID( \
                     Core::lastTime(), \
                     dev.type(), \
                     controller.deviceID, \
                     controlID, \
                     dev.control(controlID).getNormalized() \
                  ) \
               ); \
            } \
         } \
         else \
         { \
            return; \
         } \
      } \
      while( false )

#define CHECK_AND_UPDATE_STICK_DEADZONE( hControlID, vControlID, deadzone ) \
      do \
      { \
         if( getControlHIValue(controller, hControlID, h) && \
             getControlHIValue(controller, vControlID, v) ) \
         { \
            if( (h.iValue < deadzone && h.iValue > -deadzone) && \
                (v.iValue < deadzone && v.iValue > -deadzone) ) \
            { \
               h.iValue = 0; \
               v.iValue = 0; \
            } \
            HIDControl& hControl = dev.control(hControlID); \
            if( h.iValue != hControl.cur().iValue ) \
            { \
               hControl.cur( h ); \
               events.pushBack( \
                  Event::HID( \
                     Core::lastTime(), \
                     dev.type(), \
                     controller.deviceID, \
                     hControlID, \
                     dev.control(hControlID).getNormalized() \
                  ) \
               ); \
            } \
            HIDControl& vControl = dev.control(vControlID); \
            if( v.iValue != vControl.cur().iValue ) \
            { \
               vControl.cur( v ); \
               events.pushBack( \
                  Event::HID( \
                     Core::lastTime(), \
                     dev.type(), \
                     controller.deviceID, \
                     vControlID, \
                     dev.control(vControlID).getNormalized() \
                  ) \
               ); \
            } \
         } \
      } \
      while( false )

      CHECK_AND_UPDATE_BUTTON( HID_GAMEPAD_XINPUT_DPAD_UP        );
      CHECK_AND_UPDATE_BUTTON( HID_GAMEPAD_XINPUT_DPAD_DOWN      );
      CHECK_AND_UPDATE_BUTTON( HID_GAMEPAD_XINPUT_DPAD_LEFT      );
      CHECK_AND_UPDATE_BUTTON( HID_GAMEPAD_XINPUT_DPAD_RIGHT     );
      CHECK_AND_UPDATE_BUTTON( HID_GAMEPAD_XINPUT_START          );
      CHECK_AND_UPDATE_BUTTON( HID_GAMEPAD_XINPUT_BACK           );
      CHECK_AND_UPDATE_BUTTON( HID_GAMEPAD_XINPUT_LEFT_THUMB     );
      CHECK_AND_UPDATE_BUTTON( HID_GAMEPAD_XINPUT_RIGHT_THUMB    );
      CHECK_AND_UPDATE_BUTTON( HID_GAMEPAD_XINPUT_LEFT_SHOULDER  );
      CHECK_AND_UPDATE_BUTTON( HID_GAMEPAD_XINPUT_RIGHT_SHOULDER );
      CHECK_AND_UPDATE_BUTTON( HID_GAMEPAD_XINPUT_A              );
      CHECK_AND_UPDATE_BUTTON( HID_GAMEPAD_XINPUT_B              );
      CHECK_AND_UPDATE_BUTTON( HID_GAMEPAD_XINPUT_X              );
      CHECK_AND_UPDATE_BUTTON( HID_GAMEPAD_XINPUT_Y              );

      CHECK_AND_UPDATE_BUTTON( HID_GAMEPAD_XINPUT_LEFT_TRIGGER   );
      CHECK_AND_UPDATE_BUTTON( HID_GAMEPAD_XINPUT_RIGHT_TRIGGER  );

      CHECK_AND_UPDATE_STICK_DEADZONE(
         HID_GAMEPAD_XINPUT_THUMB_LX,
         HID_GAMEPAD_XINPUT_THUMB_LY,
         XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE
      );
      CHECK_AND_UPDATE_STICK_DEADZONE(
         HID_GAMEPAD_XINPUT_THUMB_RX,
         HID_GAMEPAD_XINPUT_THUMB_RY,
         XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE
      );

#undef CHECK_AND_UPDATE_STICK_DEADZONE
#undef CHECK_AND_UPDATE_BUTTON

   }
}

//------------------------------------------------------------------------------
//!
void updateControllersState()
{
   // Poll every control for their current state.
   for( uint c = 0; c < _gControllers.size(); ++c )
   {
      IOKitController& controller = _gControllers[c];
      if( controller.connected )
      {
         Vector<Event>  events;
         updateControllerState( controller, events );

         // Submit events corresponding to actual changes.
         for( Vector<Event>::ConstIterator cur = events.begin();
              cur != events.end();
              ++cur )
         {
            Core::submitEvent( *cur );
         }
      }
   }
}


//------------------------------------------------------------------------------
//!
bool  initializeDevices( bool issueEvents );

//------------------------------------------------------------------------------
//!
void  clearDevices()
{
   for( uint i = 0; i < _gControllers.size(); ++i )
   {
      IOKitController& controller = _gControllers[i];
      _manager->retireDevice( _manager->device(controller.deviceID) );
   }
}

//------------------------------------------------------------------------------
//! Updates connection information on controllers.
void  updateConnectionStates()
{
   initializeDevices( true );
}


/*==============================================================================
  CLASS IOKitAnimator
==============================================================================*/
class IOKitAnimator:
   public Animator
{
public:

   /*----- methods -----*/

   IOKitAnimator(): _reconnectDelta( 1.0 ), _curDelta( 0.0 )
   {}

   virtual bool exec( double, double delta )
   {
      updateControllersState();

      _curDelta += delta;
      if( _curDelta >= _reconnectDelta )
      {
         updateConnectionStates();
         _curDelta = 0.0;
      }

      // Do not retire the animator.
      return false;
   }

protected:

   /*----- methods -----*/

   virtual ~IOKitAnimator()
   {}

   double _reconnectDelta;
   double _curDelta;

}; //class IOKitAnimator


//------------------------------------------------------------------------------
//! Returns a matching dictionary containing devices of the specified usage.
CFMutableDictionaryRef  getMatchingDictionary( UInt32 usagePage, UInt32 usage )
{
   DBG_BLOCK( os_hid, "getMatchingDictionary" );

   // Set up a matching dictionary to search I/O Registry by class name for all HID class devices.
   CFMutableDictionaryRef dict = IOServiceMatching( kIOHIDDeviceKey );
   if( dict != NULL )
   {
      // Add key for device type (joystick, in this case) to refine the matching dictionary.
      CFNumberRef refUsagePage = CFNumberCreate( kCFAllocatorDefault, kCFNumberIntType, &usagePage );
      CFNumberRef refUsage     = CFNumberCreate( kCFAllocatorDefault, kCFNumberIntType, &usage     );
      CFDictionarySetValue( dict, CFSTR(kIOHIDPrimaryUsagePageKey), refUsagePage );
      CFDictionarySetValue( dict, CFSTR(kIOHIDPrimaryUsageKey    ), refUsage     );
   }
   else
   {
      DBG_MSG( os_hid, "ERROR - IOServiceMatching failed." );
   }

   return dict;
}

//------------------------------------------------------------------------------
//! Returns an iterator pointing to the first of all of the devices of the specifed usage.
io_iterator_t  findDevices( const mach_port_t masterPort, UInt32 usagePage, UInt32 usage )
{
   DBG_BLOCK( os_hid, "findDevices(" << (void*)masterPort << ", " << usagePage << ", " << usage << ")" );

   CFMutableDictionaryRef dict = getMatchingDictionary( usagePage, usage );
   if( dict == NULL )
   {
      return NULL;
   }

   io_iterator_t  iter;
   IOReturn ior = IOServiceGetMatchingServices( masterPort, dict, &iter );
   if( ior != kIOReturnSuccess )
   {
      DBG_MSG( os_hid, "ERROR - IOServiceGetMatchingServices failed." );
      return NULL;
   }

   // Since IOServiceGetMatchingServices consumes a reference to the dictionary,
   // we don't need to deallocate it.
   dict = NULL;

   return iter;
}

//------------------------------------------------------------------------------
//! Retrieves the device interface handle.
IOHIDDeviceInterface**  getDeviceInterface( io_object_t hidDevice )
{
   DBG_BLOCK( os_hid, "getDeviceInterface(" << (void*)hidDevice << ")" );

   io_name_t className;
   IOReturn ior = IOObjectGetClass( hidDevice, className );
   if( ior != kIOReturnSuccess )
   {
      DBG_MSG( os_hid, "ERROR - IOObjectGetClass failed." );
      return NULL;
   }

   SInt32 score = 0;
   IOCFPlugInInterface** plugInInterface;
   ior = IOCreatePlugInInterfaceForService(
      hidDevice,
      kIOHIDDeviceUserClientTypeID,
      kIOCFPlugInInterfaceID,
      &plugInInterface,
      &score
   );
   if( ior != kIOReturnSuccess )
   {
      DBG_MSG( os_hid, "ERROR - IOCreatePlugInInterfaceForService failed." );
      return NULL;
   }

   IOHIDDeviceInterface** deviceInterface;
   HRESULT pir = (*plugInInterface)->QueryInterface(
      plugInInterface,
      CFUUIDGetUUIDBytes(kIOHIDDeviceInterfaceID),
      (void**)&deviceInterface
   );
   // No longer need the plug-in interface (make sure it is deallocated before any error below).
   IODestroyPlugInInterface( plugInInterface );
   if( pir != S_OK )
   {
      DBG_MSG( os_hid, "ERROR - QueryInterface failed." );
      return NULL;
   }

   return deviceInterface;
}

//------------------------------------------------------------------------------
//! Opens a device interface.
//! Returns false on error.
bool  openDeviceInterface( IOHIDDeviceInterface** deviceInterface )
{
   DBG_BLOCK( os_hid, "openDeviceInterface(" << (void*)deviceInterface << ")" );
   if( deviceInterface != NULL )
   {
      IOReturn ior = (*deviceInterface)->open( deviceInterface, 0 );
      return ior == kIOReturnSuccess;
   }
   return false;
}

//------------------------------------------------------------------------------
//! Closes a device interface.
//! Returns false on error.
bool  closeDeviceInterface( IOHIDDeviceInterface** deviceInterface )
{
   DBG_BLOCK( os_hid, "closeDeviceInterface(" << (void*)deviceInterface << ")" );
   if( deviceInterface != NULL )
   {
      IOReturn ior = (*deviceInterface)->close( deviceInterface );
      return ior == kIOReturnSuccess;
   }
   return false;
}

//------------------------------------------------------------------------------
//! Converts a dictionary entry into an integer.
//! Returns false on failure (typically meaning that the entry isn't a number).
bool  getValue( CFDictionaryRef dict, const void *key, int& value )
{
   CFTypeRef object = CFDictionaryGetValue( dict, key );
   if( object != NULL || (CFGetTypeID(object) != CFNumberGetTypeID()) )
   {
      return CFNumberGetValue( (CFNumberRef)object, kCFNumberIntType, &value );
   }
   return false;
}

//------------------------------------------------------------------------------
//! Converts a dictionary entry into an IOHIDElementCookie.
//! Returns false on failure (typically meaning that the entry isn't a number).
bool  getValue( CFDictionaryRef dict, const void *key, IOHIDElementCookie& cookie )
{
   CFTypeRef object = CFDictionaryGetValue( dict, key );
   if( object != NULL || (CFGetTypeID(object) != CFNumberGetTypeID()) )
   {
      long long value;
      bool ok = CFNumberGetValue( (CFNumberRef)object, kCFNumberLongLongType, &value );
      cookie = (IOHIDElementCookie)value;
      return ok;
   }
   return false;
}

// Prototypes
bool  registerElementsFromArray( CFArrayRef array, IOKitController& controller );
bool  registerElementsFromDict( CFDictionaryRef dict, IOKitController& controller );

//------------------------------------------------------------------------------
//! Iterates over all of the elements on an array, and calls registerElementsFromDict
//! for all of the dictionaries found in the array.
bool  registerElementsFromArray( CFArrayRef array, IOKitController& controller )
{
   DBG_BLOCK( os_hid, "registerElementsFromArray" );

   bool ok = true;

   // Iterate over all of the elements.
   CFIndex n = CFArrayGetCount( array );
   DBG_MSG( os_hid, "Found " << (uint)n << " elements" );
   for( CFIndex i = 0; i < n; ++i )
   {
      CFTypeRef object = CFArrayGetValueAtIndex( array, i );
      if( CFGetTypeID(object) == CFDictionaryGetTypeID() )
      {
         registerElementsFromDict( (CFDictionaryRef)object, controller );
      }
   }

   return ok;
}

//------------------------------------------------------------------------------
//! Calls registerElementsFromArray if the dictionary has child elements.
//! Otherwise, it means the element is a leaf node, and therefore registers
//! the element (including the cookie).
bool  registerElementsFromDict( CFDictionaryRef dict, IOKitController& controller )
{
   DBG_BLOCK( os_hid, "registerElementsFromDict" );

   // Check if the dictionary contains elements.
   CFTypeRef object = CFDictionaryGetValue( dict, CFSTR(kIOHIDElementKey) );

   // Container object; register elements recursively.
   if( object != NULL )
   {
      // Should be an array.
      CHECK( CFGetTypeID(object) == CFArrayGetTypeID() );
      bool ok = registerElementsFromArray( (CFArrayRef)object, controller );
      return ok;
   }

   // If we got here, it means that the dictionary contains an actual control.
   bool ok = true;

   int usagePage;
   if( !getValue(dict, CFSTR(kIOHIDElementUsagePageKey), usagePage) )
   {
      DBG_MSG( os_hid, "ERROR - Could not retrieve usage page." );
      return false;
   }

   int usage;
   if( !getValue(dict, CFSTR(kIOHIDElementUsageKey), usage) )
   {
      DBG_MSG( os_hid, "ERROR - Could not retrieve usage." );
      return false;
   }

   IOHIDElementCookie cookie;
   if( !getValue(dict, CFSTR(kIOHIDElementCookieKey), cookie) )
   {
      DBG_MSG( os_hid, "ERROR - Could not retrieve cookie." );
      return false;
   }

   int minValue;
   if( !getValue(dict, CFSTR(kIOHIDElementMinKey), minValue) )
   {
      DBG_MSG( os_hid, "ERROR - Could not retrieve minimum value." );
      return false;
   }

   int maxValue;
   if( !getValue(dict, CFSTR(kIOHIDElementMaxKey), maxValue) )
   {
      DBG_MSG( os_hid, "ERROR - Could not retrieve maximum value." );
      return false;
   }

   DBG_MSG( os_hid, "UsagePage=" << usagePage << " Usage=" << usage << " Cookie="
                     << cookie << " Min/Max=" << minValue << "/" << maxValue );

   // Match up items.
   switch( usagePage )
   {
      case kHIDPage_GenericDesktop:
      {
         switch( usage )
         {
         case kHIDUsage_GD_X:  // Left stick X
            DBG_MSG( os_hid, "GD> Cookie: " << cookie << " --> Left stick X" );
            controller.controls[HID_GAMEPAD_XINPUT_THUMB_LX].cookie = cookie;
            break;
         case kHIDUsage_GD_Y:  // Left stick Y
            DBG_MSG( os_hid, "GD> Cookie: " << cookie << " --> Left stick Y" );
            controller.controls[HID_GAMEPAD_XINPUT_THUMB_LY].cookie = cookie;
            controller.controls[HID_GAMEPAD_XINPUT_THUMB_LY].flipFactor = -1;
            break;
         case kHIDUsage_GD_Z:  // Left trigger
            DBG_MSG( os_hid, "GD> Cookie: " << cookie << " --> Left trigger" );
            controller.controls[HID_GAMEPAD_XINPUT_LEFT_TRIGGER].cookie = cookie;
            break;
         case kHIDUsage_GD_Rx:  // Right stick X
            DBG_MSG( os_hid, "GD> Cookie: " << cookie << " --> Right stick X" );
            controller.controls[HID_GAMEPAD_XINPUT_THUMB_RX].cookie = cookie;
            break;
         case kHIDUsage_GD_Ry:  // Right stick Y
            DBG_MSG( os_hid, "GD> Cookie: " << cookie << " --> Right stick Y" );
            controller.controls[HID_GAMEPAD_XINPUT_THUMB_RY].cookie = cookie;
            controller.controls[HID_GAMEPAD_XINPUT_THUMB_RY].flipFactor = -1;
            break;
         case kHIDUsage_GD_Rz:  // Right trigger
            DBG_MSG( os_hid, "GD> Cookie: " << cookie << " --> Right trigger" );
            controller.controls[HID_GAMEPAD_XINPUT_RIGHT_TRIGGER].cookie = cookie;
            break;
#if 0
         // These below aren't properly setup by the MacOSX driver.
         case kHIDUsage_GD_DPadUp:
         case kHIDUsage_GD_DPadDown:
         case kHIDUsage_GD_DPadRight:
         case kHIDUsage_GD_DPadLeft:
#endif
         default:
            break;
         } // switch( usage )
      }  break; // case kHIDPage_GenericDesktop

      case kHIDPage_Game:
      {
         DBG_MSG( os_hid, "GAME> Cookie: " << cookie );
      }  break; // case kHIDPage_Game

      case kHIDPage_Button:
      {
         DBG_MSG( os_hid, "BUTTON> Cookie: " << cookie << " --> Button #" << usage );
         // Remap button IDs to match bit locations in XInput's wButton's bitmask:
         //   http://msdn2.microsoft.com/en-us/library/bb174832.aspx
         // This is required for the controlID to be identical across platforms.
         int button = -1;
         switch( usage )
         {
            case  1:  button = HID_GAMEPAD_XINPUT_A; break;
            case  2:  button = HID_GAMEPAD_XINPUT_B; break;
            case  3:  button = HID_GAMEPAD_XINPUT_X; break;
            case  4:  button = HID_GAMEPAD_XINPUT_Y; break;
            case  5:  button = HID_GAMEPAD_XINPUT_LEFT_SHOULDER; break;
            case  6:  button = HID_GAMEPAD_XINPUT_RIGHT_SHOULDER; break;
            case  7:  button = HID_GAMEPAD_XINPUT_LEFT_THUMB; break;
            case  8:  button = HID_GAMEPAD_XINPUT_RIGHT_THUMB; break;
            case  9:  button = HID_GAMEPAD_XINPUT_START; break;
            case 10:  button = HID_GAMEPAD_XINPUT_BACK; break;
            case 11:  button = HID_GAMEPAD_XINPUT_XBOX; break; //Not supported in Xinput
            case 12:  button = HID_GAMEPAD_XINPUT_DPAD_UP; break;
            case 13:  button = HID_GAMEPAD_XINPUT_DPAD_DOWN; break;
            case 14:  button = HID_GAMEPAD_XINPUT_DPAD_LEFT; break;
            case 15:  button = HID_GAMEPAD_XINPUT_DPAD_RIGHT; break;
         default:
            break;
         } // switch( usage )
         if( button != -1 )
         {
            controller.controls[button].cookie = cookie;
         }
         else
         {
            printf( "Invalid button encountered.\n" );
            CHECK( false );
            ok = false;
         }
      }  break; // case kHIDPage_Button
   } // switch( usagePage )

   return ok;
}

//------------------------------------------------------------------------------
//! Recursively registers all of the elements contained in the controller.
//! Requires the controller's controls table to be pre-allocated.
bool  registerElements( IOKitController& controller )
{
   DBG_BLOCK( os_hid, "registerElements" );

   // Retrieve properties dictionary.
   CFMutableDictionaryRef  properties;
   kern_return_t kr = IORegistryEntryCreateCFProperties(
      controller.hidDevice,
      &properties,
      kCFAllocatorDefault,
      kNilOptions
   );
   if( kr != KERN_SUCCESS )
   {
      DBG_MSG( os_hid, "ERROR - IORegistryEntryCreateCFProperties failed." );
      return false;
   }
   CHECK( properties != NULL );

   bool ok = registerElementsFromDict( (CFDictionaryRef)properties, controller );

   return ok;
}

//------------------------------------------------------------------------------
//!
void  registerDevices()
{
   _gControllers.resize( MAX_CONTROLLERS );

   for( uint i = 0; i < MAX_CONTROLLERS; ++i )
   {
      // IOKitController.
      IOKitController& controller = _gControllers[i];
      controller.deviceTypeID = HIDevice::DEVICE_TYPE_GAMEPAD;

      // Pre-allocate the controls table (required by registerElements).
      controller.controls.resize(HID_GAMEPAD_XINPUT_XBOX+1);

      // HIDevice.
      HIDevice device("Xbox controller", HIDevice::DEVICE_TYPE_GAMEPAD);
      HIDControl control;

      // Buttons
      HIValue v0;  v0.uValue = 0;
      HIValue v1;  v1.uValue = 1;
      control.valueType(HID_VALUE_TYPE_UINT);
      control.min(v0);
      control.max(v1);
      control.def(v0);
      control.cur(v0);
      control.name("DPAD_UP"); device.add(control);
      control.name("DPAD_DOWN"); device.add(control);
      control.name("DPAD_LEFT"); device.add(control);
      control.name("DPAD_RIGHT"); device.add(control);
      control.name("START"); device.add(control);
      control.name("BACK"); device.add(control);
      control.name("LEFT_THUMB"); device.add(control);
      control.name("RIGHT_THUMB"); device.add(control);
      control.name("LEFT_SHOULDER"); device.add(control);
      control.name("RIGHT_SHOULDER"); device.add(control);
      control.name("A"); device.add(control);
      control.name("B"); device.add(control);
      control.name("X"); device.add(control);
      control.name("Y"); device.add(control);

      v1.uValue = 255;
      control.max(v1);
      control.name("LEFT_TRIGGER"); device.add(control);
      control.name("RIGHT_TRIGGER"); device.add(control);

      control.valueType(HID_VALUE_TYPE_INT);
      v0.iValue = -(1 << 15);
      v1.iValue = (1 << 15) - 1;
      control.min(v0);
      control.max(v1);
      control.name("THUMB_LX"); device.add(control);
      control.name("THUMB_LY"); device.add(control);
      control.name("THUMB_RX"); device.add(control);
      control.name("THUMB_RY"); device.add(control);
      device.type( HIDevice::DEVICE_TYPE_INVALID );

      controller.deviceID = _manager->addDevice(device);
   }
}

//------------------------------------------------------------------------------
//!
bool  initializeDevices( bool issueEvents )
{
   clearDevices();

   IOReturn ior;
   io_iterator_t  hidIter = findDevices( kIOMasterPortDefault, kHIDPage_GenericDesktop, kHIDUsage_GD_GamePad );
   if( hidIter == 0 )
   {
      DBG_MSG( os_hid, "ERROR - findDevices failed to return any device." );
      return true;
   }

   uint controllerID = 0;
   // Iterate over all of the devices.
   for( io_object_t hidDevice = IOIteratorNext( hidIter );
        hidDevice;
        hidDevice = IOIteratorNext( hidIter ) )
   {
      // Retrieve name.
      char str[512];
      kern_return_t kr = IORegistryEntryGetPath(
         hidDevice,
         kIOServicePlane,
         str
      );
      if( kr != KERN_SUCCESS )
      {
         DBG_MSG( os_hid, "Error - IORegistryEntryGetPath failed." );
      }
      DBG_MSG( os_hid, "[ " << str << " ]" );

      // Retrieve class.
      ior = IOObjectGetClass(hidDevice, str);
      if( ior != kIOReturnSuccess )
      {
         DBG_MSG( os_hid, "Error - IOObjectGetClass failed." );
         continue;
      }
      DBG_MSG( os_hid, "Device class: " << str );
      if( strncasecmp(str, "Wireless360Controller", 22) == 0 )
      {
         DBG_MSG( os_hid, "CREATING AN XBOX 360 CONTROLLER\n" );

         IOKitController& controller = _gControllers[controllerID];

         controller.hidDevice        = hidDevice;
         controller.deviceInterface  = getDeviceInterface( hidDevice );
         if( !openDeviceInterface(controller.deviceInterface) )
         {
            DBG_MSG( os_hid, "ERROR - Could not open device interface: " << (void*)controller.deviceInterface << "." );
            CHECK( false );
         }
         controller.connected = true;

         // Registers all of the elements.
         if( !registerElements( controller ) )
         {
            DBG_MSG( os_hid, "ERROR - registerElements failed." );
         }

         // Initialize the controls with original state (drop the events).
         Vector<Event>  events;
         updateControllerState( controller, events );
         if( issueEvents )
         {
            // Submit events corresponding to actual changes.
            for( Vector<Event>::ConstIterator cur = events.begin();
                 cur != events.end();
                 ++cur )
            {
               Core::submitEvent( *cur );
            }
         }

         ++controllerID;  // Will use next entry for next controller.
      }
   }

   // Release iterator object.
   IOObjectRelease( hidIter );

   return true;
}

//------------------------------------------------------------------------------
//! Initializes the IOKit layer.
bool  initIOKit()
{
   DBG_BLOCK( os_hid, "initIOKit" );

   registerDevices();
   bool ok = initializeDevices( false );

   _gIOKitAnimator = new IOKitAnimator();
   Core::addAnimator( _gIOKitAnimator );

   return ok;
}

//------------------------------------------------------------------------------
//!
bool  termIOKit()
{
   DBG_BLOCK( os_hid, "termIOKit" );
   bool ok = true;

   Core::removeAnimator( _gIOKitAnimator );
   _gIOKitAnimator = NULL;

   for( uint c = 0; c < _gControllers.size(); ++c )
   {
      IOKitController& controller = _gControllers[c];
      ok &= closeDeviceInterface( controller.deviceInterface );

      IOReturn ior = IOObjectRelease(controller.hidDevice);
      if( ior != kIOReturnSuccess )
      {
         DBG_MSG( os_hid, "ERROR - IOObjectRelease failed for controller #" << c << "." );
         ok = false;
      }
   }

   return ok;
}


#endif //FUSION_HID_IOKIT_SUPPORT


/*==============================================================================
  XINPUT SUPPORT
==============================================================================*/
#if FUSION_HID_XINPUT_SUPPORT

const uint MAX_CONTROLLERS = 4;  // XInput handles up to 4 controllers

#define INPUT_DEADZONE  ( 0.24f * FLOAT(0x7FFF) )  // Default to 24% of the +/- 32767 range.   This is a reasonable default value but can be altered if needed.

struct CONTROLER_STATE
{
    XINPUT_STATE    state;
    bool            bConnected;
};

CONTROLER_STATE g_Controllers[MAX_CONTROLLERS];
Vector<HIDevice>  _gDevicesToRegister;

//------------------------------------------------------------------------------
//!
void  handleDeadZones( XINPUT_GAMEPAD& gamepad )
{
   if( (gamepad.sThumbLX <  XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE &&
        gamepad.sThumbLX > -XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE) &&
       (gamepad.sThumbLY <  XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE &&
        gamepad.sThumbLY > -XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE) )
   {
      gamepad.sThumbLX = 0;
      gamepad.sThumbLY = 0;
   }

   if( (gamepad.sThumbRX <  XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE &&
        gamepad.sThumbRX > -XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE) &&
       (gamepad.sThumbRY <  XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE &&
        gamepad.sThumbRY > -XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE) )
   {
      gamepad.sThumbRX = 0;
      gamepad.sThumbRY = 0;
   }

   // TODO: Add trigger threshold here? (XINPUT_GAMEPAD_TRIGGER_THRESHOLD)

   // TODO: Add rescaling here?
}

//------------------------------------------------------------------------------
//!
void  updateControllerStates( bool pollDisconnected )
{
   DWORD dwResult;
   for( uint i = 0; i < MAX_CONTROLLERS; ++i )
   {
      if( g_Controllers[i].bConnected || pollDisconnected )
      {
         // Simply get the state of the controller from XInput.
         dwResult = XInputGetState( i, &g_Controllers[i].state );

         if( dwResult == ERROR_SUCCESS )
         {
            g_Controllers[i].bConnected = true;

            // Handle dead zones.
            handleDeadZones( g_Controllers[i].state.Gamepad );
         }
         else
         {
            g_Controllers[i].bConnected = false;
         }
      }
   }
}

//------------------------------------------------------------------------------
//!
void  printControllerStates()
{
   for( uint i = 0; i < MAX_CONTROLLERS; ++i )
   {
      if( g_Controllers[i].bConnected )
      {
         WORD wButtons = g_Controllers[i].state.Gamepad.wButtons;
         printf( "Controller %d: Connected\n"
                 "  Buttons: %s%s%s%s%s%s%s%s%s%s%s%s%s%s\n"
                 "  Left Trigger: %d\n"
                 "  Right Trigger: %d\n"
                 "  Left Thumbstick: %d/%d\n"
                 "  Right Thumbstick: %d/%d", i,
                 (wButtons & XINPUT_GAMEPAD_DPAD_UP) ? "DPAD_UP " : "",
                 (wButtons & XINPUT_GAMEPAD_DPAD_DOWN) ? "DPAD_DOWN " : "",
                 (wButtons & XINPUT_GAMEPAD_DPAD_LEFT) ? "DPAD_LEFT " : "",
                 (wButtons & XINPUT_GAMEPAD_DPAD_RIGHT) ? "DPAD_RIGHT " : "",
                 (wButtons & XINPUT_GAMEPAD_START) ? "START " : "",
                 (wButtons & XINPUT_GAMEPAD_BACK) ? "BACK " : "",
                 (wButtons & XINPUT_GAMEPAD_LEFT_THUMB) ? "LEFT_THUMB " : "",
                 (wButtons & XINPUT_GAMEPAD_RIGHT_THUMB) ? "RIGHT_THUMB " : "",
                 (wButtons & XINPUT_GAMEPAD_LEFT_SHOULDER) ?"LEFT_SHOULDER " : "",
                 (wButtons & XINPUT_GAMEPAD_RIGHT_SHOULDER) ? "RIGHT_SHOULDER " : "",
                 (wButtons & XINPUT_GAMEPAD_A) ? "A " : "",
                 (wButtons & XINPUT_GAMEPAD_B) ? "B " : "",
                 (wButtons & XINPUT_GAMEPAD_X) ? "X " : "",
                 (wButtons & XINPUT_GAMEPAD_Y) ? "Y " : "",
                 g_Controllers[i].state.Gamepad.bLeftTrigger,
                 g_Controllers[i].state.Gamepad.bRightTrigger,
                 g_Controllers[i].state.Gamepad.sThumbLX,
                 g_Controllers[i].state.Gamepad.sThumbLY,
                 g_Controllers[i].state.Gamepad.sThumbRX,
                 g_Controllers[i].state.Gamepad.sThumbRY );
      }
      else
      {
         printf( "Controller %d: Not connected", i );
      }
      printf("\n");
   }
}


void  updateHIDevice( const int deviceID, HIDevice& dst, Vector<Event>& events )
{
   const CONTROLER_STATE& src = g_Controllers[deviceID];
   CHECK( dst.name() == "Xbox controller" );
   CHECK( dst.numControls() == 20 );
   if( src.bConnected )
   {
      if( dst.type() == HIDevice::DEVICE_TYPE_INVALID )
      {
         DBG_MSG( os_hid, "Connecting controller" );
         dst.type( HIDevice::DEVICE_TYPE_GAMEPAD );
      }

      WORD wButtons = src.state.Gamepad.wButtons;
      HIValue v;

#define CHECK_AND_UPDATE_BUTTON( xinput_enum, hid_control_id ) \
   v.uValue = (wButtons & xinput_enum) ? 1 : 0; \
   if( v.uValue != dst.control(hid_control_id).cur().uValue ) \
   { \
      dst.control(hid_control_id).cur( v ); \
      events.pushBack( Event::HID(Core::lastTime(), (int)dst.type(), deviceID, hid_control_id, dst.control(hid_control_id).getNormalized()) ); \
   }

      CHECK_AND_UPDATE_BUTTON( XINPUT_GAMEPAD_DPAD_UP, 0 );
      CHECK_AND_UPDATE_BUTTON( XINPUT_GAMEPAD_DPAD_DOWN, 1 );
      CHECK_AND_UPDATE_BUTTON( XINPUT_GAMEPAD_DPAD_LEFT, 2 );
      CHECK_AND_UPDATE_BUTTON( XINPUT_GAMEPAD_DPAD_RIGHT, 3 );
      CHECK_AND_UPDATE_BUTTON( XINPUT_GAMEPAD_START, 4 );
      CHECK_AND_UPDATE_BUTTON( XINPUT_GAMEPAD_BACK, 5 );
      CHECK_AND_UPDATE_BUTTON( XINPUT_GAMEPAD_LEFT_THUMB, 6 );
      CHECK_AND_UPDATE_BUTTON( XINPUT_GAMEPAD_RIGHT_THUMB, 7 );
      CHECK_AND_UPDATE_BUTTON( XINPUT_GAMEPAD_LEFT_SHOULDER, 8 );
      CHECK_AND_UPDATE_BUTTON( XINPUT_GAMEPAD_RIGHT_SHOULDER, 9 );
      CHECK_AND_UPDATE_BUTTON( XINPUT_GAMEPAD_A, 10 );
      CHECK_AND_UPDATE_BUTTON( XINPUT_GAMEPAD_B, 11 );
      CHECK_AND_UPDATE_BUTTON( XINPUT_GAMEPAD_X, 12 );
      CHECK_AND_UPDATE_BUTTON( XINPUT_GAMEPAD_Y, 13 );

#undef CHECK_AND_UPDATE_BUTTON

      v.uValue = src.state.Gamepad.bLeftTrigger;
      if( v.uValue != dst.control(14).cur().uValue )
      {
         dst.control(14).cur( v );
         events.pushBack( Event::HID(Core::lastTime(), (int)dst.type(), deviceID, 14, dst.control(14).getNormalized()) );
      }
      v.uValue = src.state.Gamepad.bRightTrigger;
      if( v.uValue != dst.control(15).cur().uValue )
      {
         dst.control(15).cur( v );
         events.pushBack( Event::HID(Core::lastTime(), (int)dst.type(), deviceID, 15, dst.control(15).getNormalized()) );
      }

      v.iValue = src.state.Gamepad.sThumbLX;
      if( v.iValue != dst.control(16).cur().iValue )
      {
         dst.control(16).cur( v );
         events.pushBack( Event::HID(Core::lastTime(), (int)dst.type(), deviceID, 16, dst.control(16).getNormalized()) );
      }
      v.iValue = src.state.Gamepad.sThumbLY;
      if( v.iValue != dst.control(17).cur().iValue )
      {
         dst.control(17).cur( v );
         events.pushBack( Event::HID(Core::lastTime(), (int)dst.type(), deviceID, 17, dst.control(17).getNormalized()) );
      }
      v.iValue = src.state.Gamepad.sThumbRX;
      if( v.iValue != dst.control(18).cur().iValue )
      {
         dst.control(18).cur( v );
         events.pushBack( Event::HID(Core::lastTime(), (int)dst.type(), deviceID, 18, dst.control(18).getNormalized()) );
      }
      v.iValue = src.state.Gamepad.sThumbRY;
      if( v.iValue != dst.control(19).cur().iValue )
      {
         dst.control(19).cur( v );
         events.pushBack( Event::HID(Core::lastTime(), (int)dst.type(), deviceID, 19, dst.control(19).getNormalized()) );
      }
   }
   else
   {
      if( dst.type() != HIDevice::DEVICE_TYPE_INVALID )
      {
         DBG_MSG( os_hid, "Disconnecting controller" );
         dst.type( HIDevice::DEVICE_TYPE_INVALID );
      }
   }

#if 0
   for( uint i = 0; i < events.size(); ++i )
   {
      printf(" %d", events[i].value());
   }
   if( !events.empty() )  printf("\n");
#endif
}


void  registerXInputControllers()
{
   for( uint i = 0; i < MAX_CONTROLLERS; ++i )
   {
      HIDevice device("Xbox controller", HIDevice::DEVICE_TYPE_GAMEPAD);
      HIDControl control;

      // Buttons
      HIValue v0;  v0.uValue = 0;
      HIValue v1;  v1.uValue = 1;
      control.valueType(HID_VALUE_TYPE_UINT);
      control.min(v0);
      control.max(v1);
      control.def(v0);
      control.cur(v0);
      control.name("DPAD_UP"); device.add(control);
      control.name("DPAD_DOWN"); device.add(control);
      control.name("DPAD_LEFT"); device.add(control);
      control.name("DPAD_RIGHT"); device.add(control);
      control.name("START"); device.add(control);
      control.name("BACK"); device.add(control);
      control.name("LEFT_THUMB"); device.add(control);
      control.name("RIGHT_THUMB"); device.add(control);
      control.name("LEFT_SHOULDER"); device.add(control);
      control.name("RIGHT_SHOULDER"); device.add(control);
      control.name("A"); device.add(control);
      control.name("B"); device.add(control);
      control.name("X"); device.add(control);
      control.name("Y"); device.add(control);

      v1.uValue = 255;
      control.max(v1);
      control.name("LEFT_TRIGGER"); device.add(control);
      control.name("RIGHT_TRIGGER"); device.add(control);

      control.valueType(HID_VALUE_TYPE_INT);
      v0.iValue = -(1 << 15);
      v1.iValue = (1 << 15) - 1;
      control.min(v0);
      control.max(v1);
      control.name("THUMB_LX"); device.add(control);
      control.name("THUMB_LY"); device.add(control);
      control.name("THUMB_RX"); device.add(control);
      control.name("THUMB_RY"); device.add(control);
      if( !g_Controllers[i].bConnected )
      {
         device.type( HIDevice::DEVICE_TYPE_INVALID );
      }
      _manager->addDevice(device);
   }
}

class XInputAnimator:
   public Animator
{
public:

   /*----- methods -----*/

   XInputAnimator(): _disconnectedDelta(2.0), _currentDelta(0.0)
   {}

   virtual bool exec( double, double delta )
   {
      _currentDelta += delta;
      bool pollDisconnected = (_currentDelta >= _disconnectedDelta);
      if( pollDisconnected )
      {
         _currentDelta = 0.0;
      }

      // Read XInput state.
      updateControllerStates( pollDisconnected );
      //printControllerStates();

      // Convert to our internal (cached) representation.
      Vector< Event > events;
      int n = (int)_manager->numDevices();
      for( int i = 0; i < n; ++i )
      {
         updateHIDevice( i, _manager->device(i), events );
      }

      // Submit events corresponding to actual changes.
      for( Vector< Event >::ConstIterator cur = events.begin();
           cur < events.end();
           ++cur )
      {
         Core::submitEvent( *cur );
      }

      // Do not retire the animator.
      return false;
   }

protected:

   /*----- members -----*/
   double _disconnectedDelta;
   double _currentDelta;

}; //class XInputAnimator

RCP<Animator> _sXInputAnimator;

//------------------------------------------------------------------------------
//!
bool  initXInput()
{
   DBG_BLOCK( os_hid, "initXInput" );
   memset( g_Controllers, 0, sizeof(CONTROLER_STATE)*MAX_CONTROLLERS );
   DBG_MSG( os_hid, "Enabling XInput" );
   XInputEnable( true );
   _sXInputAnimator = new XInputAnimator();
   Core::addAnimator( _sXInputAnimator );
   updateControllerStates( true );
   //printControllerStates();
   registerXInputControllers();
   return true;
}

//------------------------------------------------------------------------------
//!
bool  termXInput()
{
   DBG_BLOCK( os_hid, "initXInput" );
   DBG_MSG( os_hid, "Removing animator" );
   Core::removeAnimator( _sXInputAnimator );
   DBG_MSG( os_hid, "Deallocating animator" );
   _sXInputAnimator = NULL;
   DBG_MSG( os_hid, "Disabling XInput" );
   XInputEnable( false );
   return true;
}



#endif //FUSION_HID_XINPUT_SUPPORT


//------------------------------------------------------------------------------
//!
int valueVM( VMState* vm )
{
   int top = VM::getTop( vm );

   if( top == 0 )
   {
      printf("HID::valueVM - Missing arguments.\n");
      VM::printStack( vm );
      return 0;
   }

   int deviceTypeID, deviceID, controlID;

   if( !VM::get( vm, 1, "deviceTypeID", deviceTypeID ) )
   {
      printf("HID::valueVM - Error retrieving deviceTypeID.\n");
      VM::printStack( vm );
      return 0;
   }

   if( !VM::get( vm, 1, "deviceID", deviceID ) )
   {
      printf("HID::valueVM - Error retrieving deviceID.\n");
      VM::printStack( vm );
      return 0;
   }

   if( top == 1 )
   {
      if( !VM::get( vm, 1, "controlID", controlID ) )
      {
         printf("HID::valueVM - Error retrieving controlID.\n");
         VM::printStack( vm );
         return 0;
      }
   }
   else
   {
      controlID = VM::toInt( vm, 2 );
   }

   HIDControl* control = HIDManager::getControl(deviceTypeID, deviceID, controlID);
   CHECK( control != NULL );
   VM::push( vm, control->getNormalized() );

   return 1;
}

//------------------------------------------------------------------------------
//!
const VM::Reg hidFuncs[] = {
   { "value", valueVM },
   { 0, 0 }
};


UNNAMESPACE_END


/*==============================================================================
  CLASS HIDControl
==============================================================================*/

//------------------------------------------------------------------------------
//!
HIDControl::HIDControl()
{
}

//------------------------------------------------------------------------------
//!
HIDControl::~HIDControl()
{
}

//------------------------------------------------------------------------------
//!
float
HIDControl::getNormalized() const
{
   float tmp;
   switch(_type)
   {
      case HID_VALUE_TYPE_INT:
         tmp = (float)(_cur.iValue)/_max.iValue;
         break;
      case HID_VALUE_TYPE_UINT:
         tmp = (float)(_cur.uValue)/_max.uValue;
         break;
      case HID_VALUE_TYPE_FLOAT:
         tmp = _cur.fValue/_max.fValue;
         break;
      case HID_VALUE_TYPE_POINTER:
      default:
         return -CGConstf::infinity();
   }
   tmp = CGM::clamp(tmp, -1.0f, +1.0f);
   return tmp;
}

//------------------------------------------------------------------------------
//!
void
HIDControl::setNormalized( float v )
{
   v = CGM::clamp(v, -1.0f, +1.0f);
   switch(_type)
   {
      case HID_VALUE_TYPE_INT:
         _cur.iValue = (int)((v * _max.iValue) + 0.5f);
         break;
      case HID_VALUE_TYPE_UINT:
         _cur.uValue = (uint)((v * _max.uValue) + 0.5f);
         break;
      case HID_VALUE_TYPE_FLOAT:
         _cur.fValue = (v * _max.fValue);
         break;
      case HID_VALUE_TYPE_POINTER:
      default:
         // Just ignore it
         break;
   }
}


/*==============================================================================
  CLASS HIDFeedback
==============================================================================*/

//------------------------------------------------------------------------------
//!
HIDFeedback::HIDFeedback()
{
}

//------------------------------------------------------------------------------
//!
HIDFeedback::~HIDFeedback()
{
}


/*==============================================================================
  CLASS HIDevice
==============================================================================*/

//------------------------------------------------------------------------------
//!
HIDevice::HIDevice( const String& name, const DeviceType type ):
   _name( name ),
   _type( type )
{
}

//------------------------------------------------------------------------------
//!
HIDevice::~HIDevice()
{
}


/*==============================================================================
  CLASS HIManager
==============================================================================*/

//------------------------------------------------------------------------------
//!
HIDManager::HIDManager()
{
   CHECK( _manager == nullptr );
   _manager = this;
}

//------------------------------------------------------------------------------
//!
HIDManager::~HIDManager()
{
   _manager = nullptr;
}

//------------------------------------------------------------------------------
//!
bool
HIDManager::initialize( VMState* vm, const char* nameSpace )
{
   DBG_BLOCK( os_hid, "HIDManager::initialize" );
   _manager = new HIDManager();

   bool ok = true;

#if FUSION_HID_IOKIT_SUPPORT
   ok &= initIOKit();
   CHECK( ok );
#endif //FUSION_HID_IOKIT_SUPPORT

#if FUSION_HID_XINPUT_SUPPORT
   ok &= initXInput();
   CHECK( ok );
#endif //FUSION_HID_XINPUT_SUPPORT

   // Register functions and enums.
   String tableName;
   tableName.reserve(256);

   VM::registerFunctions( vm, nameSpace, hidFuncs );

   tableName = nameSpace;
   tableName += ".DeviceType";
   VM::registerEnum( vm, tableName.cstr(), _enumsDeviceType );

   tableName = nameSpace;
   tableName += ".Gamepad.XInput";
   VM::registerEnum( vm, tableName.cstr(), _enumsXInput );

   return ok;
}

//------------------------------------------------------------------------------
//!
bool
HIDManager::terminate()
{
   DBG_BLOCK( os_hid, "HIDManager::terminate" );
   bool ok = true;

#if FUSION_HID_IOKIT_SUPPORT
   ok &= termIOKit();
#endif //FUSION_HID_IOKIT_SUPPORT

#if FUSION_HID_XINPUT_SUPPORT
   ok &= termXInput();
#endif //FUSION_HID_XINPUT_SUPPORT

   delete _manager;
   _manager = NULL;

   return ok;
}

//------------------------------------------------------------------------------
//!
const HIDManager::DeviceContainer&
HIDManager::devices()
{
   return _manager->_devices;
}

//------------------------------------------------------------------------------
//!
uint
HIDManager::numDevices()
{
   return (uint)_manager->_devices.size();
}

//------------------------------------------------------------------------------
//!
HIDevice&
HIDManager::device( uint id )
{
   CHECK((size_t)id < _manager->_devices.size());
   return _manager->_devices[id];
}

//------------------------------------------------------------------------------
//!
HIDevice*
HIDManager::find( const String& name )
{
   // Iterate over all of the devices, and return the proper one.
   const uint n = _manager->numDevices();
   for( uint id = 0; id < n; ++id )
   {
      HIDevice& dev = _manager->device(id);
      if( name == _manager->device(id).name() )
      {
         return &dev;
      }
   }
   return NULL;
}

//------------------------------------------------------------------------------
//!
uint
HIDManager::getID( const HIDevice& dev )
{
   // Iterate over all of the devices, and return the proper one.
   const uint n = _manager->numDevices();
   const HIDevice* devPtr = &dev;
   for( uint id = 0; id < n; ++id )
   {
      if( devPtr == &(_manager->device(id)) )
      {
         return id;
      }
   }
   return HID_INVALID_ID;
}

//------------------------------------------------------------------------------
//!
HIDControl*
HIDManager::getControl( int deviceTypeID, int deviceID, int controlID )
{
   switch( deviceTypeID )
   {
   case HIDevice::DEVICE_TYPE_GAMEPAD:
   //case HIDevice::DEVICE_TYPE_WHEEL:
   //case HIDevice::DEVICE_TYPE_OTHER:
      break;
   case HIDevice::DEVICE_TYPE_INVALID:
   default:
      return NULL;
   }

   //DeviceContainer&  devices = _manager->_deviceTypes[deviceTypeID];
   DeviceContainer&  devices = _manager->_devices;

   if( deviceID < (int)devices.size() )
   {
      HIDevice&  device = devices[deviceID];
      if( controlID < (int)device.numControls() )
      {
         return &(device.control(controlID));
      }
      else
      {
         return NULL;
      }
   }

   return NULL;
}

//------------------------------------------------------------------------------
//!
uint
HIDManager::addDevice( HIDevice& device )
{
   uint id = (uint)_devices.size();
   _devices.pushBack(device);
   return id;
}

//------------------------------------------------------------------------------
//!
void
HIDManager::retireDevice( HIDevice& dev )
{
   dev.type( HIDevice::DEVICE_TYPE_INVALID );
}
