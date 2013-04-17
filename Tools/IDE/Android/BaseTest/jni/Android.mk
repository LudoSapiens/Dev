MY_LOCAL_PATH := $(call my-dir)

include ../CommonAndroid.mk

# BaseTest
include $(CLEAR_VARS)

$(DEV_LOCAL_DEFAULTS)

LOCAL_PATH      := $(DEV_PATH)/Projects
LOCAL_MODULE    := BaseTest
LOCAL_SRC_FILES := \
	Base/src/test.cpp \
	Base/src/test_adt.cpp \
	Base/src/test_dbg.cpp \
	Base/src/test_io.cpp \
	Base/src/test_msg.cpp \
	Base/src/test_mt.cpp \
	Base/src/test_net.cpp \
	Base/src/test_old.cpp \
	Base/src/test_util.cpp \

LOCAL_STATIC_LIBRARIES := Base
LOCAL_LDLIBS := -lz

include $(BUILD_EXECUTABLE)

$(call import-module,com.ludosapiens/Base)
