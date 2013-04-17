MY_LOCAL_PATH := $(call my-dir)

include ../CommonAndroid.mk

# NativeApp
include $(CLEAR_VARS)

$(DEV_LOCAL_DEFAULTS)

LOCAL_PATH      := $(MY_LOCAL_PATH)
LOCAL_MODULE    := NativeApp
LOCAL_SRC_FILES := NativeApp.cpp

LOCAL_STATIC_LIBRARIES := Polycubic Plasma MotionBullet Fusion Snd Gfx CGMath Base
LOCAL_LDLIBS := $(DEV_LOCAL_LDLIBS) -lBulletDynamics -lBulletCollision -lLinearMath -llua -lz -lGLESv1_CM -lGLESv2 -llog

include $(BUILD_SHARED_LIBRARY)

$(call import-module,com.ludosapiens/Base)
$(call import-module,com.ludosapiens/CGMath)
$(call import-module,com.ludosapiens/Fusion)
$(call import-module,com.ludosapiens/Gfx)
$(call import-module,com.ludosapiens/MotionBullet)
$(call import-module,com.ludosapiens/Plasma)
$(call import-module,com.ludosapiens/Polycubic)
$(call import-module,com.ludosapiens/Snd)
