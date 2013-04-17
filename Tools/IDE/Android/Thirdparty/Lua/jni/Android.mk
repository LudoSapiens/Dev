MY_LOCAL_PATH := $(call my-dir)

include ../../CommonAndroid.mk

# Lua
include $(CLEAR_VARS)

$(DEV_LOCAL_DEFAULTS)

LUA_SRC_PATH          := $(DEV_PATH)/Thirdparty/src/lua/src

LOCAL_PATH            := $(LUA_SRC_PATH)
LOCAL_MODULE          := Lua
LOCAL_MODULE_FILENAME := liblua
LOCAL_SRC_FILES := \
	lapi.c \
	lauxlib.c \
	lbaselib.c \
	lcode.c \
	ldblib.c \
	ldebug.c \
	ldo.c \
	ldump.c \
	lfunc.c \
	lgc.c \
	linit.c \
	liolib.c \
	llex.c \
	lmathlib.c \
	lmem.c \
	loadlib.c \
	lobject.c \
	lopcodes.c \
	loslib.c \
	lparser.c \
	lstate.c \
	lstring.c \
	lstrlib.c \
	ltable.c \
	ltablib.c \
	ltm.c \
	lundump.c \
	lvm.c \
	lzio.c \


include $(BUILD_STATIC_LIBRARY)

install: Lua $(DEV_PLATFORM)/include $(DEV_PLATFORM)/lib
	@echo "Installing $<"
	cp -i $(LUA_SRC_PATH)/{lauxlib,lua,luaconf,lualib}.h $(DEV_PLATFORM)/include
	cp -i $(MY_LOCAL_PATH)/../obj/local/armeabi/liblua.a $(DEV_PLATFORM)/lib
	@echo "Done."
