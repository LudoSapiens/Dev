MY_LOCAL_PATH := $(call my-dir)

include ../CommonAndroid.mk

# CGMath
include $(CLEAR_VARS)

$(DEV_LOCAL_DEFAULTS)

LOCAL_PATH := $(DEV_PATH)/Projects

LOCAL_MODULE := CGMath

LOCAL_SRC_FILES := \
	CGMath/AABBTree.cpp \
	CGMath/BIH.cpp \
	CGMath/Dist.cpp \
	CGMath/HGrid.cpp \
	CGMath/Noise.cpp \
	CGMath/Octree.cpp \
	CGMath/Random.cpp \
	CGMath/Variant.cpp \

include $(BUILD_STATIC_LIBRARY)
