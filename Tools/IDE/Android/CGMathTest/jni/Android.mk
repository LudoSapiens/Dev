MY_LOCAL_PATH := $(call my-dir)

include ../CommonAndroid.mk

# CGMathTest
include $(CLEAR_VARS)

$(DEV_LOCAL_DEFAULTS)

LOCAL_PATH      := $(DEV_PATH)/Projects
LOCAL_MODULE    := CGMathTest
LOCAL_SRC_FILES := \
	CGMath/src/test.cpp \
	CGMath/src/test_boxes.cpp \
	CGMath/src/test_interpolation.cpp \
	CGMath/src/test_intersection.cpp \
	CGMath/src/test_misc.cpp \
	CGMath/src/test_random.cpp \
	CGMath/src/test_surfaces.cpp \
	CGMath/src/test_vmq.cpp \


LOCAL_STATIC_LIBRARIES := CGMath Base
LOCAL_LDLIBS := -lz

include $(BUILD_EXECUTABLE)

$(call import-module,com.ludosapiens/Base)
$(call import-module,com.ludosapiens/CGMath)
