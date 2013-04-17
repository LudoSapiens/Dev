MY_LOCAL_PATH := $(call my-dir)

include ../CommonAndroid.mk

# Gfx
include $(CLEAR_VARS)

$(DEV_LOCAL_DEFAULTS)

LOCAL_PATH := $(DEV_PATH)/Projects

LOCAL_MODULE := Gfx

LOCAL_SRC_FILES := \
	Gfx/FB/Framebuffer.cpp \
	Gfx/FB/RenderState.cpp \
	Gfx/Geom/Buffer.cpp \
	Gfx/Geom/Geometry.cpp \
	Gfx/Mgr/Context.cpp \
	Gfx/Mgr/GLES/GLES1Manager.cpp \
	Gfx/Mgr/GLES/GLES2Manager.cpp \
	Gfx/Mgr/GLES/GLESContext.cpp \
	Gfx/Mgr/GLES/GLESContext_Android.cpp \
	Gfx/Mgr/Manager.cpp \
	Gfx/Mgr/Null/NullContext.cpp \
	Gfx/Mgr/Null/NullManager.cpp \
	Gfx/Pass/Pass.cpp \
	Gfx/Pass/RenderNode.cpp \
	Gfx/Prog/Constants.cpp \
	Gfx/Prog/Program.cpp \
	Gfx/Tex/Sampler.cpp \
	Gfx/Tex/Texture.cpp \
	Gfx/Tex/TextureState.cpp \

include $(BUILD_STATIC_LIBRARY)
