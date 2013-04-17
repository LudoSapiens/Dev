MY_LOCAL_PATH := $(call my-dir)

include ../../CommonAndroid.mk

BULLET_SRC_PATH := $(DEV_PATH)/Thirdparty/src/bullet/src

###################
# BulletCollision #
###############################################################################
include $(CLEAR_VARS)

$(DEV_LOCAL_DEFAULTS)

BULLET_COLLISION_CPP := $(shell cd $(BULLET_SRC_PATH) && find BulletCollision -not -wholename "*Gimpact*" -name "*.cpp")
BULLET_COLLISION_H   := $(shell cd $(BULLET_SRC_PATH) && find BulletCollision -not -wholename "*Gimpact*" -name "*.h")

LOCAL_PATH      := $(BULLET_SRC_PATH)
LOCAL_MODULE    := BulletCollision
LOCAL_SRC_FILES := $(BULLET_COLLISION_CPP)

include $(BUILD_STATIC_LIBRARY)


##################
# BulletDynamics #
###############################################################################
include $(CLEAR_VARS)

$(DEV_LOCAL_DEFAULTS)

BULLET_DYNAMICS_CPP := $(shell cd $(BULLET_SRC_PATH) && find BulletDynamics -name "*.cpp")
BULLET_DYNAMICS_H   := $(shell cd $(BULLET_SRC_PATH) && find BulletDynamics -name "*.h")

LOCAL_PATH      := $(BULLET_SRC_PATH)
LOCAL_MODULE    := BulletDynamics
LOCAL_SRC_FILES := $(BULLET_DYNAMICS_CPP)

include $(BUILD_STATIC_LIBRARY)


### #######################
### # BulletMultiThreaded #
### ###############################################################################
### include $(CLEAR_VARS)
### 
### $(DEV_LOCAL_DEFAULTS)
### 
### BULLET_MULTI_THREADED_CPP := $(shell cd $(BULLET_SRC_PATH) && find BulletMultiThreaded -name "*.cpp")
### BULLET_MULTI_THREADED_H   := $(shell cd $(BULLET_SRC_PATH) && find BulletMultiThreaded -name "*.h")
### 
### LOCAL_PATH      := $(BULLET_SRC_PATH)
### LOCAL_MODULE    := BulletMultiThreaded
### LOCAL_SRC_FILES := $(BULLET_MULTI_THREADED_CPP)
### 
### include $(BUILD_STATIC_LIBRARY)

### ##################
### # BulletSoftBody #
### ###############################################################################
### include $(CLEAR_VARS)
### 
### $(DEV_LOCAL_DEFAULTS)
### 
### BULLET_SOFT_BODY_CPP := $(shell cd $(BULLET_SRC_PATH) && find BulletSoftBody -name "*.cpp")
### BULLET_SOFT_BODY_H   := $(shell cd $(BULLET_SRC_PATH) && find BulletSoftBody -name "*.h")
### 
### LOCAL_PATH      := $(BULLET_SRC_PATH)
### LOCAL_MODULE    := BulletSoftBody
### LOCAL_SRC_FILES := $(BULLET_SOFT_BODY_CPP)
### 
### include $(BUILD_STATIC_LIBRARY)

##############
# LinearMath #
###############################################################################
include $(CLEAR_VARS)

$(DEV_LOCAL_DEFAULTS)

LINEAR_MATH_CPP := $(shell cd $(BULLET_SRC_PATH) && find LinearMath -name "*.cpp")
LINEAR_MATH_H   := $(shell cd $(BULLET_SRC_PATH) && find LinearMath -name "*.h")

LOCAL_PATH      := $(BULLET_SRC_PATH)
LOCAL_MODULE    := LinearMath
LOCAL_SRC_FILES := $(LINEAR_MATH_CPP)

include $(BUILD_STATIC_LIBRARY)



###################
# Utility targets #
###################

ALL_TARGETS := BulletCollision BulletDynamics LinearMath
ALL_HEADERS := Bullet-C-Api.h btBulletCollisionCommon.h btBulletDynamicsCommon.h $(BULLET_COLLISION_H) $(BULLET_DYNAMICS_H) $(LINEAR_MATH_H)
ALL_LIBS    := $(ALL_TARGETS:%=$(MY_LOCAL_PATH)/../obj/local/armeabi/lib%.a)

.SUFFIXES: .copy_header

%.copy_header: SRC_FILE = $(BULLET_SRC_PATH)/$*
%.copy_header: DST_FILE = $(DEV_PLATFORM)/include/Bullet/$*
%.copy_header: DST_DIR  = $(dir $(DST_FILE))
%.copy_header:
#	@echo $(DST_FILE)
	@mkdir -p $(DST_DIR)
	cp $(SRC_FILE) $(DST_FILE)

install: install_libraries install_headers

install_headers: $(ALL_HEADERS:%=%.copy_header)
	@echo "Done installing headers."
	@echo ""

install_libraries: $(ALL_TARGETS)
	@echo "Installing libraries ($(ALL_TARGETS)):"
	cp $(ALL_LIBS) $(DEV_PLATFORM)/lib
	@echo "Done installing libraries."
	@echo ""

dbg:
	@echo "BULLET_COLLISION_CPP=$(BULLET_COLLISION_CPP)"
	@echo "BULLET_COLLISION_H=$(BULLET_COLLISION_H)"
