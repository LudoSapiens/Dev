MY_LOCAL_PATH := $(call my-dir)

include ../CommonAndroid.mk

# Snd
include $(CLEAR_VARS)

$(DEV_LOCAL_DEFAULTS)

LOCAL_PATH := $(DEV_PATH)/Projects

LOCAL_MODULE := Snd

LOCAL_SRC_FILES := \
	Snd/Listener.cpp \
	Snd/Manager.cpp \
	Snd/NullManager.cpp \
	Snd/Sound.cpp \
	Snd/Source.cpp \

#	Snd/OpenALManager.cpp \

include $(BUILD_STATIC_LIBRARY)
