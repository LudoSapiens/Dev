MY_LOCAL_PATH := $(call my-dir)

include ../CommonAndroid.mk

# Base
include $(CLEAR_VARS)

$(DEV_LOCAL_DEFAULTS)

LOCAL_PATH := $(DEV_PATH)/Projects

LOCAL_MODULE := Base

LOCAL_SRC_FILES := \
	Base/ADT/ConstString.cpp \
	Base/ADT/String.cpp \
	Base/ADT/StringMap.cpp \
	Base/Dbg/DebugStream.cpp \
	Base/Dbg/ErrorManager.cpp \
	Base/Dbg/UnitTest.cpp \
	Base/IO/BinaryStream.cpp \
	Base/IO/FileDevice.cpp \
	Base/IO/FileSystem.cpp \
	Base/IO/GZippedFileDevice.cpp \
	Base/IO/IODevice.cpp \
	Base/IO/LockedMemoryDevice.cpp \
	Base/IO/MemoryDevice.cpp \
	Base/IO/MultiDevice.cpp \
	Base/IO/NullDevice.cpp \
	Base/IO/Path.cpp \
	Base/IO/StringDevice.cpp \
	Base/IO/TextStream.cpp \
	Base/Msg/Observer.cpp \
	Base/Msg/Subject.cpp \
	Base/MT/Lock.cpp \
	Base/MT/RWLock.cpp \
	Base/MT/Semaphore.cpp \
	Base/MT/Task.cpp \
	Base/MT/TaskQueue.cpp \
	Base/MT/Thread.cpp \
	Base/MT/Trigger.cpp \
	Base/MT/ValueTrigger.cpp \
	Base/Net/_SocketImpl.cpp \
	Base/Net/Socket.cpp \
	Base/Net/SocketDevice.cpp \
	Base/Util/Application.cpp \
	Base/Util/Arguments.cpp \
	Base/Util/Date.cpp \
	Base/Util/Memory.cpp \
	Base/Util/RadixSort.cpp \
	Base/Util/RCObject.cpp \
	Base/Util/RCObjectNA.cpp \
	Base/Util/SHA.cpp \
	Base/Util/Time.cpp \
	Base/Util/Validator.cpp \

include $(BUILD_STATIC_LIBRARY)
