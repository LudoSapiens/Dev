ifndef COMMON_ANDROID_INCLUDED
COMMON_ANDROID_INCLUDED := 1

# Path for .../Tools/IDE/Android
DEV_ANDROID := $(abspath $(call my-dir))
#$(warning DEV_ANDROID='$(DEV_ANDROID)')

# Path for all of Dev (where Projects/Tools/Data/etc lie).
DEV_PATH := $(abspath $(DEV_ANDROID)/../../..)
#$(warning DEV_PATH='$(DEV_PATH)')

NDK_MODULE_PATH := $(DEV_ANDROID)
#$(warning NDK_MODULE_PATH='$(NDK_MODULE_PATH)')
# Apparently, setting the variable here is too late, so we explicitly register
# the path the same way it is done in $(NDK)/build/core/setup-imports.mk.
$(call import-add-path,$(DEV_ANDROID))

DEV_PLATFORM := $(DEV_PATH)/Thirdparty/platform/android
DEV_LOCAL_CFLAGS   := -I$(DEV_PATH)/Projects -I$(DEV_PLATFORM)/include -I$(DEV_PLATFORM)/include/Bullet
#DEV_LOCAL_CPPFLAGS := $(DEV_LOCAL_CFLAGS)
DEV_LOCAL_LDLIBS   := -L$(DEV_PLATFORM)/lib
#$(warning "defining DEV_LOCAL_CFLAGS=$(DEV_LOCAL_CFLAGS)")

# Disable ARM Thumb instructions.
DEV_LOCAL_ARM_MODE := arm
#$(warning DEV_LOCAL_ARM_MODE='$(DEV_LOCAL_ARM_MODE)')

# Disable Arm's Neon SIMD instructions.
LOCAL_ARM_NEON := false

# Below, simple assign default LOCAL_* variables you want assigned when calling $(DEV_LOCAL_DEFAULTS).
# Every LOCAL_* variable should have a DEV_LOCAL_* equivalent above (because "include $(CLEAR_VARS))" wipes them every time).
# In order to prevent creating problematic Makefile targets, it seems this has to be
# a single line, hence the backslashes (expect for the last one).
# Also, spacing makes it fragile, i.e. $(DEV_LOCAL_CFLAGS  ) doesn't work.
define DEV_LOCAL_DEFAULTS
$(eval LOCAL_ARM_MODE := $(DEV_LOCAL_ARM_MODE)) \
$(eval LOCAL_ARM_NEON := $(DEV_LOCAL_ARM_NEON)) \
$(eval LOCAL_CFLAGS   := $(DEV_LOCAL_CFLAGS))   \
$(eval LOCAL_CPPFLAGS := $(DEV_LOCAL_CPPFLAGS)) \
$(eval LOCAL_LDLIBS   := $(DEV_LOCAL_LDLIBS))
endef

# Some utility targets.

$(DEV_PLATFORM)/include:
	mkdir -p $@

$(DEV_PLATFORM)/lib:
	mkdir -p $@

endif
