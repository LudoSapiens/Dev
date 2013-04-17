MY_LOCAL_PATH := $(call my-dir)

include ../CommonAndroid.mk

# Polycubic
include $(CLEAR_VARS)

$(DEV_LOCAL_DEFAULTS)

LOCAL_PATH := $(DEV_PATH)/Projects

LOCAL_MODULE := Polycubic

LOCAL_SRC_FILES := \
	Game/Polycubic/GameApp.cpp \
	Game/Polycubic/GameSave.cpp \
	Game/Polycubic/Manipulator/GameManipulator.cpp \
	Game/Polycubic/Physics/Attractors.cpp \

include $(BUILD_STATIC_LIBRARY)
