##
# A Makefile to help build our third-party libraries on the command-line.
##

#############
# Variables #
#######################################################################

# Tools
CP         := cp
MAKE       := make
MKDIR      := mkdir
NOTHING    := echo -n
XCODEBUILD := xcodebuild

# Settings
SDK_IPHONE_DEVICE    := iphoneos
SDK_IPHONE_SIMULATOR := iphonesimulator
SDK_MACOSX           := macosx

CURRENT_MAKEFILE := $(CURDIR)/$(lastword $(MAKEFILE_LIST))
CURRENT_DIR      := $(dir $(CURRENT_MAKEFILE))
DEV_ROOT         := $(abspath $(CURRENT_DIR)/../../../..)
DEV_THIRDPARTY   := $(abspath $(DEV_ROOT)/Thirdparty)
BULLET_SRC       := $(DEV_THIRDPARTY)/src/bullet/src
BULLET_HEADERS   := $(subst $(BULLET_SRC)/,,$(shell find $(BULLET_SRC) -not \( -path "*/MiniCL/*" -or -path "*/vectormath/*" \) -name "*.h"))
#BULLET_HEADERS   := btBulletCollisionCommon.h btBulletDynamicsCommon.h Bullet-C-Api.h BulletCollision/BroadphaseCollision/btAxisSweep3.h
BULLET_LIBRARIES := libBulletCollision.a libBulletDynamics.a libBulletMultiThreaded.a libBulletSoftBody.a libLinearMath.a
BUILD_DIR        := $(CURRENT_DIR)/build

PLAT_IOS_DEVICE    := $(DEV_THIRDPARTY)/platform/iphone-device
PLAT_IOS_SIMULATOR := $(DEV_THIRDPARTY)/platform/iphone-simulator
PLAT_OSX           := $(DEV_THIRDPARTY)/platform/macosx

ALL := bullet lua oggvorbis

# Header search paths.
vpath %.h $(shell find $(BULLET_SRC) -type d)
# Library search paths.
vpath %.a $(wildcard $(BUILD_DIR)/Release*)

# These environment variables don't play well with Xcode.
unexport CC CXX

default: all

# Fake phony rule to allow extension rules to be phony.
# That allows, for example, things like 'make file.view' even if
# a file of that name exists.
.PHONY: always BULLET_INCLUDES
always:
	@$(NOTHING)

.SUFFIXES:

.PHONY: bullet.common.install bullet.ios.device.install bullet.ios.simulator.install

all: $(ALL)

build: $(ALL:%=%.build)

install: $(ALL:%=%.install)

osx: $(ALL:%=%.osx)

ios: $(ALL:%=%.ios)

# Header-copying rules.
$(PLAT_IOS_DEVICE)/include/Bullet/%: %
	@$(MKDIR) -p $(dir $@)
	$(CP) $^ $@

$(PLAT_IOS_SIMULATOR)/include/Bullet/%: %
	@$(MKDIR) -p $(dir $@)
	$(CP) $^ $@

$(PLAT_OSX)/include/Bullet/%: %
	@$(MKDIR) -p $(dir $@)
	$(CP) $^ $@

# Library-copying rules.
$(PLAT_IOS_DEVICE)/lib/%: $(BUILD_DIR)/Release-iphoneos/%
	@$(MKDIR) -p $(dir $@)
	$(CP) $^ $@

$(PLAT_IOS_SIMULATOR)/lib/%: $(BUILD_DIR)/Release-iphonesimulator/%
	@$(MKDIR) -p $(dir $@)
	$(CP) $^ $@

$(PLAT_OSX)/lib/%: $(BUILD_DIR)/Release/%
	@$(MKDIR) -p $(dir $@)
	$(CP) $^ $@


##########
# Bullet #
#######################################################################
bullet: bullet.osx bullet.ios

bullet.build: bullet.osx.build bullet.ios.build

bullet.install: bullet.osx.install bullet.ios.install

bullet.ios: bullet.ios.device bullet.ios.simulator

bullet.ios.build: bullet.ios.device.build bullet.ios.simulator.build

bullet.ios.install: bullet.ios.device.install bullet.ios.simulator.install

bullet.ios.device: bullet.ios.device.build bullet.ios.device.install

bullet.ios.device.build: always
	$(XCODEBUILD) -sdk "$(SDK_IPHONE_DEVICE)" -project "Bullet.xcodeproj" -target "All (iOS)"

bullet.ios.device.install: $(BULLET_HEADERS:%=$(PLAT_IOS_DEVICE)/include/Bullet/%) $(BULLET_LIBRARIES:%=$(PLAT_IOS_DEVICE)/lib/%)

bullet.ios.simulator: bullet.ios.simulator.build bullet.ios.simulator.install

bullet.ios.simulator.build: always
	$(XCODEBUILD) -sdk "$(SDK_IPHONE_SIMULATOR)" -project "Bullet.xcodeproj" -target "All (iOS)"

bullet.ios.simulator.install: bullet.ios.simulator.build $(BULLET_HEADERS:%=$(PLAT_IOS_SIMULATOR)/include/Bullet/%) $(BULLET_LIBRARIES:%=$(PLAT_IOS_SIMULATOR)/lib/%)

bullet.osx: bullet.osx.build bullet.osx.install

bullet.osx.build: always
	$(XCODEBUILD) -sdk "$(SDK_MACOSX)" -project "Bullet.xcodeproj" -target "All (OSX)"

bullet.osx.install: bullet.osx.build $(BULLET_HEADERS:%=$(PLAT_OSX)/include/Bullet/%) $(BULLET_LIBRARIES:%=$(PLAT_OSX)/lib/%)

# I wish I knew how to generate dependencies with a loop over $(BULLET_LIBRARIES).
$(BUILD_DIR)/Release-iphoneos/libBulletCollision.a: bullet.ios.device.build
$(BUILD_DIR)/Release-iphoneos/libBulletDynamics.a: bullet.ios.device.build
$(BUILD_DIR)/Release-iphoneos/libBulletMultiThreaded.a: bullet.ios.device.build
$(BUILD_DIR)/Release-iphoneos/libBulletSoftBody.a: bullet.ios.device.build
$(BUILD_DIR)/Release-iphoneos/libLinearMath.a: bullet.ios.device.build

$(BUILD_DIR)/Release-iphonesimulator/libBulletCollision.a: bullet.ios.simulator.build
$(BUILD_DIR)/Release-iphonesimulator/libBulletDynamics.a: bullet.ios.simulator.build
$(BUILD_DIR)/Release-iphonesimulator/libBulletMultiThreaded.a: bullet.ios.simulator.build
$(BUILD_DIR)/Release-iphonesimulator/libBulletSoftBody.a: bullet.ios.simulator.build
$(BUILD_DIR)/Release-iphonesimulator/libLinearMath.a: bullet.ios.simulator.build

$(BUILD_DIR)/Release/libBulletCollision.a: bullet.osx.build
$(BUILD_DIR)/Release/libBulletDynamics.a: bullet.osx.build
$(BUILD_DIR)/Release/libBulletMultiThreaded.a: bullet.osx.build
$(BUILD_DIR)/Release/libBulletSoftBody.a: bullet.osx.build
$(BUILD_DIR)/Release/libLinearMath.a: bullet.osx.build

##################
# Ogg and Vorbis #
#######################################################################
oggvorbis: oggvorbis.osx oggvorbis.ios

oggvorbis.build: oggvorbis

oggvorbis.install: oggvorbis

oggvorbis.ios: oggvorbis.ios.device oggvorbis.ios.simulator

oggvorbis.ios.device: always
	$(XCODEBUILD) -sdk "$(SDK_IPHONE_DEVICE)" -project "OggVorbis.xcodeproj" -target "All (iOS)"

oggvorbis.ios.simulator: always
	$(XCODEBUILD) -sdk "$(SDK_IPHONE_SIMULATOR)" -project "OggVorbis.xcodeproj" -target "All (iOS)"

oggvorbis.osx:
	$(XCODEBUILD) -sdk "$(SDK_MACOSX)" -project "OggVorbis.xcodeproj" -target "All (OSX)"


#######
# Lua #
#######################################################################
lua: lua.osx lua.ios

lua.build: lua

lua.install: lua

lua.ios: lua.ios.device lua.ios.simulator

lua.ios.device: always
	$(XCODEBUILD) -sdk "$(SDK_IPHONE_DEVICE)" -project "Lua.xcodeproj" -target "Lua (iOS)"

lua.ios.simulator: always
	$(XCODEBUILD) -sdk "$(SDK_IPHONE_SIMULATOR)" -project "Lua.xcodeproj" -target "Lua (iOS)"

lua.osx:
	$(XCODEBUILD) -sdk "$(SDK_MACOSX)" -project "Lua.xcodeproj" -target "Lua (OSX)" -target "Lua CLI (OSX)"


#########
# Utils #
#######################################################################

sdks:
	$(XCODEBUILD) -showsdks

dbg:
	@echo CURRENT_MAKEFILE=$(CURRENT_MAKEFILE)=
	@echo CURRENT_DIR=$(CURRENT_DIR)=
	@echo DEV_ROOT=$(DEV_ROOT)=
	@echo DEV_THIRDPARTY=$(DEV_THIRDPARTY)=
