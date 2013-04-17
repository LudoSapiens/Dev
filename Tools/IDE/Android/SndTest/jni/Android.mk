MY_LOCAL_PATH := $(call my-dir)

include ../CommonAndroid.mk

include $(CLEAR_VARS)

$(DEV_LOCAL_DEFAULTS)

LOCAL_PATH      := $(DEV_PATH)/Projects
LOCAL_MODULE    := SndTest
LOCAL_SRC_FILES := Snd/src/test.cpp

LOCAL_STATIC_LIBRARIES := Snd CGMath Base
LOCAL_LDLIBS := -lz

include $(BUILD_EXECUTABLE)

$(call import-module,com.ludosapiens/Base)
$(call import-module,com.ludosapiens/CGMath)
$(call import-module,com.ludosapiens/Snd)
