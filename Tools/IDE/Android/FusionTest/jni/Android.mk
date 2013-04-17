MY_LOCAL_PATH := $(call my-dir)

include ../CommonAndroid.mk

# FusionTest
include $(CLEAR_VARS)

$(DEV_LOCAL_DEFAULTS)

LOCAL_PATH      := $(DEV_PATH)/Projects
LOCAL_MODULE    := FusionTest
LOCAL_SRC_FILES := Fusion/src/test.cpp

LOCAL_STATIC_LIBRARIES := Snd Gfx CGMath Base
LOCAL_LDLIBS := -lz

include $(BUILD_EXECUTABLE)

$(call import-module,com.ludosapiens/Base)
$(call import-module,com.ludosapiens/CGMath)
$(call import-module,com.ludosapiens/Gfx)
$(call import-module,com.ludosapiens/Snd)
