MY_LOCAL_PATH := $(call my-dir)

include ../CommonAndroid.mk

# MotionBullet
include $(CLEAR_VARS)

$(DEV_LOCAL_DEFAULTS)

LOCAL_PATH := $(DEV_PATH)/Projects

LOCAL_MODULE := MotionBullet

LOCAL_SRC_FILES := \
	MotionBullet/Attractor/Attractor.cpp \
	MotionBullet/Attractor/BasicAttractors.cpp \
	MotionBullet/Attractor/GravitationalAttractor.cpp \
	MotionBullet/Collision/BasicShapes.cpp \
	MotionBullet/Collision/CollisionGroup.cpp \
	MotionBullet/Collision/CollisionInfo.cpp \
	MotionBullet/Collision/CollisionShape.cpp \
	MotionBullet/Constraint/CharacterConstraint.cpp \
	MotionBullet/Constraint/Constraint.cpp \
	MotionBullet/Constraint/Joints.cpp \
	MotionBullet/Constraint/Springs.cpp \
	MotionBullet/World/MotionWorld.cpp \
	MotionBullet/World/RigidBody.cpp \

# Need to add Bullet subdirectory (doesn't work; put in CommonAndroid.mk).
#DEV_LOCAL_CFLAGS := $(DEV_LOCAL_CFLAGS) -I$(DEV_PLATFORM)/include/Bullet

include $(BUILD_STATIC_LIBRARY)
