##
# A Makefile to help build our third-party libraries on the command-line.
##
# Author: Jocelyn Houle <jocelyn.houle@ludosapiens.com>
#
# Some notes:
#
# The Makefile conforms to the following format:
#   $(t).$(c).$(p)
# where:
#   $(t) is the target (bullet, lua, zlib, etc.).
#   $(c) is the command (uncompress, build, install, etc.).
#   $(p) is the platform (ios, osx, vs2011, etc.).
#
# The $(eval ...) calls can be intimidating at first, but they are used
# to automatically generate targets, dependencies, and rules by iterating
# across variables.
# It's tricky to first get your mind around it, but it pays off.
#
# Some targets (notably, the header-copying ones) need to run inside
# a submake call sometimes, because finding the header files can only
# be done once the package is uncompressed, and then, it's too late
# to defined dependencies, so we launch another make for those.
# We also make sure we don't loop infinitely in such cases (when
# there is no header found, for instance).
#
# I limited the help strings to <80 cols.
##

#############
# Variables #
#######################################################################

# Tools.
CP         := cp
DOWNLOAD   := curl -O
#DOWNLOAD   := wget
GREP       := grep
#MAKE       := make
MKDIR      := mkdir
MSBUILD    := c:/WINDOWS/Microsoft.NET/Framework/v4.0.30319/MSBuild.exe
MV         := mv
NOTHING    := echo -n
TAR        := tar
TOUCH      := touch
UNZIP      := unzip
XCODEBUILD := xcodebuild

ifndef PLATFORM
   PLATFORM := $(shell uname)
endif

# Functions.
findheaders = $(patsubst $(1)/%,%,$(shell test -e $(1) && find $(1) -path "*.h"))

# Copies $(1) into $(2), making sure the destination directory exists.
define COPYFILE
$(2): $(1)
	@$(MKDIR) -p "$$(dir $$@)"
	$(CP) "$$^" "$$@"
endef

# Defines target $(1) with dependencies $(2) which simply calls targets $(3)
# in a submake command.
# This is sometimes necessary to populate some variables (i.e. when decompressing
# a directory, and setting targets based on decompressed files).
define SUBMAKE
$(1): $(2)
	$(MAKE) $(3)
endef

# Defines target $(1) with dependencies $(2).
define GENDEPS
$(1): $(2)
endef


# Expands the plat list in $(1) and lib list in $(2) into all library variants of VisualStudio libraries inside $(3).
# E.g. $(call VSLIBS,32 64,libA.lib libB.lib,platform/vs2011) would yield:
#  platform/vs2011/lib/x86/32/Debug/libA.lib
#  platform/vs2011/lib/x86/32/Debug/libB.lib
#  platform/vs2011/lib/x86/32/Release/libA.lib
#  platform/vs2011/lib/x86/32/Release/libB.lib
#  platform/vs2011/lib/x86/64/Debug/libA.lib
#  platform/vs2011/lib/x86/64/Debug/libB.lib
#  platform/vs2011/lib/x86/64/Release/libA.lib
#  platform/vs2011/lib/x86/64/Release/libB.lib
VSLIBS = $(foreach plat,$(1),$(foreach conf,Debug Release,$(foreach lib,$(2),$(3)/lib/x86/$(plat)/$(conf)/$(lib))))
#EXPAND3 = $(foreach a,$(1),$(foreach b,$(2),$(foreach c,$(3),$(a)$(b)$(c))))


# Settings.
SDK_IOS_DEV      := iphoneos
SDK_IOS_SIM      := iphonesimulator
SDK_OSX          := macosx

CURRENT_MAKEFILE := $(CURDIR)/$(lastword $(MAKEFILE_LIST))
CURRENT_DIR      := $(dir $(CURRENT_MAKEFILE))
DEV_ROOT         := $(abspath $(CURRENT_DIR)/..)
DEV_THIRDPARTY   := $(abspath $(DEV_ROOT)/Thirdparty)

#BULLET_PACKAGE   := $(lastword $(sort $(notdir $(wildcard src/bullet-*.zip))))
#BULLET_VERSION   := $(BULLET_PACKAGE:bullet-%.zip=%)
BULLET_VERSION   := 2.80-rev2531
BULLET_PACKAGE   := bullet-$(BULLET_VERSION).zip
BULLET_URL       := http://bullet.googlecode.com/files/$(BULLET_PACKAGE)
BULLET_DIR       := src/bullet
BULLET_SRC_DIR   := $(BULLET_DIR)/src
BULLET_HEADERS   := $(subst $(BULLET_SRC_DIR)/,,$(shell test -e $(BULLET_SRC_DIR) && find $(BULLET_SRC_DIR) -not \( -path "*/MiniCL/*" -or -path "*/vectormath/*" \) -name "*.h"))
BULLET_LIBS_IOS  := libBulletCollision.a libBulletDynamics.a libBulletMultiThreaded.a libBulletSoftBody.a libLinearMath.a
BULLET_LIBS_OSX  := libBulletCollision.a libBulletDynamics.a libBulletMultiThreaded.a libBulletSoftBody.a libLinearMath.a
BULLET_LIBS_VS   := BulletCollision.lib BulletDynamics.lib LinearMath.lib

#LIBPNG_PACKAGE   := $(lastword $(sort $(notdir $(wildcard src/libpng-*.tar.bz2))))
#LIBPNG_VERSION   := $(OGG_PACKAGE:libpng-%.tar.bz2=%)
LIBPNG_VERSION   := 1.2.18
LIBPNG_PACKAGE   := libpng-$(LIBPNG_VERSION).tar.bz2
LIBPNG_URL       := http://prdownloads.sourceforge.net/libpng/$(LIBPNG_PACKAGE)?download
LIBPNG_DIR       := src/libpng

#LUA_PACKAGE      := $(lastword $(sort $(notdir $(wildcard src/lua-*.tar.gz))))
#LUA_VERSION      := $(LUA_PACKAGE:lua-%.tar.gz=%)
LUA_VERSION      := 5.1.5
LUA_PACKAGE      := lua-$(LUA_VERSION).tar.gz
LUA_URL          := http://www.lua.org/ftp/$(LUA_PACKAGE)
LUA_DIR          := src/lua
LUA_SRC_DIR      := $(LUA_DIR)/src
LUA_HEADERS      := lua.h luaconf.h lualib.h lauxlib.h
#LUA_HEADERS      := $(call findheaders,$(LUA_SRC_DIR))
LUA_LIBS_VS      := Lua.lib

#OGG_PACKAGE      := $(lastword $(sort $(notdir $(wildcard src/libogg-*.tar.gz))))
#OGG_VERSION      := $(OGG_PACKAGE:libogg-%.tar.gz=%)
#OGG_VERSION      := 1.2.2
OGG_VERSION      := 1.3.0
OGG_PACKAGE      := libogg-$(OGG_VERSION).tar.gz
OGG_URL          := http://downloads.xiph.org/releases/ogg/$(OGG_PACKAGE)
OGG_DIR          := src/libogg
OGG_SRC_DIR      := $(OGG_DIR)
OGG_HEADERS      := $(call findheaders,$(OGG_SRC_DIR)/include)

#VORBIS_PACKAGE   := $(lastword $(sort $(notdir $(wildcard src/libvorbis-*.tar.bz2))))
#VORBIS_VERSION   := $(VORBIS_PACKAGE:libvorbis-%.tar.bz2=%)
VORBIS_VERSION   := 1.3.3
VORBIS_PACKAGE   := libvorbis-$(VORBIS_VERSION).tar.gz
VORBIS_URL       := http://downloads.xiph.org/releases/vorbis/$(VORBIS_PACKAGE)
VORBIS_DIR       := src/libvorbis
VORBIS_SRC_DIR   := $(VORBIS_DIR)
VORBIS_HEADERS   := $(call findheaders,$(VORBIS_SRC_DIR)/include)

OGGVORBIS_HEADERS := $(strip $(OGG_HEADERS) $(VORBIS_HEADERS))
OGGVORBIS_LIBS_VS := Ogg.lib Vorbis.lib

#ZLIB_PACKAGE     := $(lastword $(sort $(notdir $(wildcard src/zlib-*.tar.gz))))
#ZLIB_VERSION     := $(ZLIB_PACKAGE:zlib-%.tar.gz=%)
ZLIB_VERSION     := 1.2.7
ZLIB_PACKAGE     := zlib-$(ZLIB_VERSION).tar.bz2
ZLIB_URL         := http://zlib.net/$(ZLIB_PACKAGE)
ZLIB_DIR         := src/zlib
ZLIB_SRC_DIR     := $(ZLIB_DIR)
ZLIB_HEADERS     := zconf.h zlib.h
ZLIB_LIBS_VS     := ZLib.lib


XCODE_DIR         := $(DEV_ROOT)/Tools/IDE/Xcode/Thirdparty
IOS_DEV_BUILD_DIR := $(XCODE_DIR)/build/Release-iphoneos
IOS_SIM_BUILD_DIR := $(XCODE_DIR)/build/Release-iphonesimulator
OSX_BUILD_DIR     := $(XCODE_DIR)/build/Release

VS2010_DIR       := $(DEV_ROOT)/Tools/IDE/Visual\ Studio/2010/Thirdparty
VS2010_FILE      := Thirdparty.sln
VS2010_BUILD_DIR := $(VS2010_DIR)/build
VS2011_DIR       := $(DEV_ROOT)/Tools/IDE/Visual\ Studio/2011/Thirdparty
VS2011_FILE      := Thirdparty.sln
VS2011_BUILD_DIR := $(VS2011_DIR)/build

#PLAT_ROOT        := $(DEV_THIRDPARTY)/platform
PLAT_ROOT        := platform
PLAT_IOS_DEV     := $(PLAT_ROOT)/iphone-device
PLAT_IOS_SIM     := $(PLAT_ROOT)/iphone-simulator
PLAT_OSX         := $(PLAT_ROOT)/macosx
PLAT_VS2010      := $(PLAT_ROOT)/vs2010
PLAT_VS2011      := $(PLAT_ROOT)/vs2011
ALL_PLAT_DIRS    := $(PLAT_IOS_DEV) $(PLAT_IOS_SIM) $(PLAT_OSX) $(PLAT_VS2010) $(PLAT_VS2011)
IOS_SUB          := dev sim

ALL_PLATS        := ios.dev ios.sim osx vs2010 vs2011
ALL_TARGETS := bullet lua oggvorbis zlib

ifndef PLATS
	ifeq ($(PLATFORM), Darwin)
		PLATS   := osx ios
		TARGETS := bullet lua oggvorbis
	endif
	ifneq (,$(findstring CYGWIN,$(PLATFORM)))
		PLATS   := vs2011
		TARGETS := bullet lua oggvorbis zlib
	endif
endif


ALL_COMMANDS := uncompress build install install.binaries install.headers

# Commands.

default: help

# Fake phony rule to allow extension rules to be phony.
# That allows, for example, things like 'make file.view' even if
# a file of that name exists.
.PHONY: always
always:
	@$(NOTHING)

.SUFFIXES: %.build

.PHONY: $(foreach t,$(ALL_TARGETS),$(foreach c,$(ALL_COMMANDS),$(foreach p,$(ALL_PLATS),$(t).$(c).$(p))))

## A Makefile use to build our thirdparty libraries.
##
## Usage:

##   > make help           Print this current usage information.
help: always
	@$(GREP) "^## " Makefile | sed s/"^## "//g

##   > make all            Build and install everything for the current platform.
all: always $(TARGETS)

##   > make download       Downloads all of the packages required.
download: always src/ $(TARGETS:%=%.download)

src: src/.d

##   > make uncompress     Extract all of the packages required.
uncompress: always $(TARGETS:%=%.uncompress)

##   > make build          Build everything for the current platform.
build: always uncompress $(TARGETS:%=%.build)

##   > make install        Install everything for the current platform.
install: always build $(TARGETS:%=%.install)

install.headers: always build $(TARGETS:%=%.install.headers)

install.binaries: always build $(TARGETS:%=%.install.binaries)


##   > make clean          Remove all installed binaries and headers.
clean: always
	$(RM) -rf $(PLAT_ROOT)

clean.osx: always
	$(RM) -rf $(PLAT_OSX)
clean.ios.dev: always
	$(RM) -rf $(PLAT_IOS_DEV)
clean.ios.sim: always
	$(RM) -rf $(PLAT_IOS_SIM)
clean.vs2010: always
	$(RM) -rf $(PLAT_VS2010)
clean.vs2011: always
	$(RM) -rf $(PLAT_VS2011)

##   > make empty          Remove binaries, headers, and uncompressed packages.
empty: clean
	$(RM) -rf $(BULLET_DIR) $(LIBPNG_DIR) $(LUA_DIR) $(OGG_DIR) $(VORBIS_DIR) $(ZLIB_DIR)


##   > make $(p)           Build and install for the platform $(p) (see below).
ios: always
	$(MAKE) PLATS="ios.dev ios.sim" all
ios.dev: always
	$(MAKE) PLATS=$@ all
ios.sim: always
	$(MAKE) PLATS=$@ all
osx: always
	$(MAKE) PLATS=$@ all
vs2010: always
	$(MAKE) PLATS=$@ all
vs2011: always
	$(MAKE) PLATS=$@ all


# Some other specific targets.
#install.binaries: $(TARGETS:%=%.install.binaries)
#install.headers: $(TARGETS:%=%.install.headers)

# Automatic targets.
%.ios.dev: always
	@exit 13

%.ios.sim: always
	@exit 13

%.ios: always %.ios.dev %.ios.sim
	@$(NOTHING)
%.build: $(foreach p,$(PLATS),%.build.$(p))
	@$(NOTHING)
%.install: always %.install.binaries %.install.headers
	@$(NOTHING)
%.install.binaries: always $(foreach p,$(PLATS),%.install.binaries.$(p))
	@$(NOTHING)
%.install.headers: always $(foreach p,$(PLATS),%.install.headers.$(p))
	@$(NOTHING)

# Generate all of the install.$(p): $(t).install.$(p)
$(foreach p,$(ALL_PLATS),$(eval $(call GENDEPS,install.$(p),$(TARGETS:%=%.install.$(p)))))

# Generate all of the $(t).build: $(t).build.$(p)
$(foreach t,$(ALL_TARGETS),$(eval $(call GENDEPS,$(t).build,$(PLATS:%=$(t).build.%))))
# Generate all of the $(t).install.$(p): $(t).install.binaries.$(p) $(t).install.headers.$(p)
#$(foreach t,$(ALL_TARGETS),$(foreach p,$(ALL_PLATS),$(eval $(call GENDEPS,$(t).install.$(p),$(t).install.headers.$(p) $(t).install.binaries.$(p)))))
# Generate all of the $(t).install.binaries: $(t).install.binaries.$(p)
$(foreach t,$(ALL_TARGETS),$(eval $(call GENDEPS,$(t).install.binaries,$(PLATS:%=$(t).install.binaries.%))))
# Generate all of the $(t).install.headers: $(t).install.headers.$(p)
$(foreach t,$(ALL_TARGETS),$(eval $(call GENDEPS,$(t).install.headers,$(PLATS:%=$(t).install.headers.%))))


# Special VS2010 targets to build everything.
build.vs2010: $(foreach p,32,$(foreach c,dbg rel,build.vs2010.$(p).$(c)))
build.vs2010.32.dbg: always
	cd $(VS2010_DIR) && $(MSBUILD) /m $(VS2010_FILE) /p:Platform=32-bit,Configuration=Debug
build.vs2010.32.rel: always
	cd $(VS2010_DIR) && $(MSBUILD) /m $(VS2010_FILE) /p:Platform=32-bit,Configuration=Release
build.vs2010.64.dbg: always
	cd $(VS2010_DIR) && $(MSBUILD) /m $(VS2010_FILE) /p:Platform=64-bit,Configuration=Debug
build.vs2010.64.rel: always
	cd $(VS2010_DIR) && $(MSBUILD) /m $(VS2010_FILE) /p:Platform=64-bit,Configuration=Release

# Special VS2011 targets to build everything.
build.vs2011: $(foreach p,32,$(foreach c,dbg rel,build.vs2011.$(p).$(c)))
build.vs2011.32.dbg: always
	cd $(VS2011_DIR) && $(MSBUILD) /m $(VS2011_FILE) /p:Platform=32-bit,Configuration=Debug
build.vs2011.32.rel: always
	cd $(VS2011_DIR) && $(MSBUILD) /m $(VS2011_FILE) /p:Platform=32-bit,Configuration=Release
build.vs2011.64.dbg: always
	cd $(VS2011_DIR) && $(MSBUILD) /m $(VS2011_FILE) /p:Platform=64-bit,Configuration=Debug
build.vs2011.64.rel: always
	cd $(VS2011_DIR) && $(MSBUILD) /m $(VS2011_FILE) /p:Platform=64-bit,Configuration=Release


##########
# Bullet #
#######################################################################
##   > make bullet         Build and install the Bullet library (required).
bullet: bullet.uncompress bullet.build bullet.install

bullet.download: always src/$(BULLET_PACKAGE)

src/$(BULLET_PACKAGE): src/.d
	cd src/ && $(DOWNLOAD) $(BULLET_URL)

bullet.uncompress: $(BULLET_DIR)

$(BULLET_DIR): src/$(BULLET_PACKAGE)
	$(MAKE) src/bullet-$(BULLET_VERSION)/
	$(MV)   src/bullet-$(BULLET_VERSION)/ $@
	$(TOUCH) $@


bullet.build.ios.dev: always
	cd $(XCODE_DIR) && $(XCODEBUILD) -sdk "$(SDK_IOS_DEV)" -project "Bullet.xcodeproj" -target "All (iOS)"
bullet.build.ios.sim: always
	cd $(XCODE_DIR) && $(XCODEBUILD) -sdk "$(SDK_IOS_SIM)" -project "Bullet.xcodeproj" -target "All (iOS)"
bullet.build.osx: always
	cd $(XCODE_DIR) && $(XCODEBUILD) -sdk "$(SDK_OSX)" -project "Bullet.xcodeproj" -target "All (OSX)"
bullet.build.vs2010: $(foreach p,32,$(foreach c,dbg rel,bullet.build.vs2010.$(p).$(c)))
bullet.build.vs2010.32.dbg: always
	cd $(VS2010_DIR) && $(MSBUILD) /m $(VS2010_FILE) /t:BulletCollision,BulletDynamics,BulletLinearMath /p:Platform=32-bit,Configuration=Debug
bullet.build.vs2010.32.rel: always
	cd $(VS2010_DIR) && $(MSBUILD) /m $(VS2010_FILE) /t:BulletCollision,BulletDynamics,BulletLinearMath /p:Platform=32-bit,Configuration=Release
bullet.build.vs2010.64.dbg: always
	cd $(VS2010_DIR) && $(MSBUILD) /m $(VS2010_FILE) /t:BulletCollision,BulletDynamics,BulletLinearMath /p:Platform=64-bit,Configuration=Debug
bullet.build.vs2010.64.rel: always
	cd $(VS2010_DIR) && $(MSBUILD) /m $(VS2010_FILE) /t:BulletCollision,BulletDynamics,BulletLinearMath /p:Platform=64-bit,Configuration=Release
bullet.build.vs2011: $(foreach p,32 64,$(foreach c,dbg rel,bullet.build.vs2011.$(p).$(c)))
bullet.build.vs2011.32.dbg: always
	cd $(VS2011_DIR) && $(MSBUILD) /m $(VS2011_FILE) /t:BulletCollision,BulletDynamics,BulletLinearMath /p:Platform=32-bit,Configuration=Debug
bullet.build.vs2011.32.rel: always
	cd $(VS2011_DIR) && $(MSBUILD) /m $(VS2011_FILE) /t:BulletCollision,BulletDynamics,BulletLinearMath /p:Platform=32-bit,Configuration=Release
bullet.build.vs2011.64.dbg: always
	cd $(VS2011_DIR) && $(MSBUILD) /m $(VS2011_FILE) /t:BulletCollision,BulletDynamics,BulletLinearMath /p:Platform=64-bit,Configuration=Debug
bullet.build.vs2011.64.rel: always
	cd $(VS2011_DIR) && $(MSBUILD) /m $(VS2011_FILE) /t:BulletCollision,BulletDynamics,BulletLinearMath /p:Platform=64-bit,Configuration=Release


bullet.install.binaries.ios.dev: $(BULLET_LIBS_IOS:%=$(PLAT_IOS_DEV)/lib/%)
bullet.install.binaries.ios.sim: $(BULLET_LIBS_IOS:%=$(PLAT_IOS_SIM)/lib/%)
bullet.install.binaries.osx: $(BULLET_LIBS_OSX:%=$(PLAT_OSX)/lib/%)
bullet.install.binaries.vs2010: $(call VSLIBS,32,$(BULLET_LIBS_VS),$(PLAT_VS2010))
bullet.install.binaries.vs2011: $(call VSLIBS,32 64,$(BULLET_LIBS_VS),$(PLAT_VS2011))

bullet.install.headers.ios.dev: $(BULLET_HEADERS:%=$(PLAT_IOS_DEV)/include/Bullet/%)
bullet.install.headers.ios.sim: $(BULLET_HEADERS:%=$(PLAT_IOS_SIM)/include/Bullet/%)
bullet.install.headers.osx: $(BULLET_HEADERS:%=$(PLAT_OSX)/include/Bullet/%)
bullet.install.headers.vs2010: $(BULLET_HEADERS:%=$(PLAT_VS2010)/include/Bullet/%)
bullet.install.headers.vs2011: $(BULLET_HEADERS:%=$(PLAT_VS2011)/include/Bullet/%)

bullet.install.headers: bullet.uncompress
ifeq (,$(BULLET_HEADERS))  # Headers not found this run.
	[ $(MAKELEVEL) -le 1 ] && $(MAKE) bullet.install.headers  # Prevent infinite loop.
endif

# Generate all of the bullet.install.$(p) targets.
#$(foreach p,$(ALL_PLATS),$(eval $(call SUBMAKE,bullet.install.$(p),bullet.uncompress,bullet.install.headers.$(p))))

# Generate Bullet header copy targets for all of the platforms.
$(foreach d,$(ALL_PLAT_DIRS),$(foreach f,$(BULLET_HEADERS),$(eval $(call COPYFILE,$(BULLET_SRC_DIR)/$(f),$(d)/include/Bullet/$(f)))))

# Generate Bullet binary copy targets for IOS device.
$(foreach lib,$(BULLET_LIBS_IOS),$(eval $(call COPYFILE,$(IOS_DEV_BUILD_DIR)/$(lib),$(PLAT_IOS_DEV)/lib/$(lib))))
# Generate Bullet binary copy targets for IOS simulator.
$(foreach lib,$(BULLET_LIBS_IOS),$(eval $(call COPYFILE,$(IOS_SIM_BUILD_DIR)/$(lib),$(PLAT_IOS_SIM)/lib/$(lib))))
# Generate Bullet binary copy targets for OSX.
$(foreach lib,$(BULLET_LIBS_OSX),$(eval $(call COPYFILE,$(OSX_BUILD_DIR)/$(lib),$(PLAT_OSX)/lib/$(lib))))
# Generate Bullet binary copy targets for VS2010.
$(foreach plat,32,$(foreach conf,Debug Release,$(foreach lib,$(BULLET_LIBS_VS),$(eval $(call COPYFILE,$(VS2010_BUILD_DIR)/x86/$(plat)/$(conf)/bin/$(lib),$(PLAT_VS2010)/lib/x86/$(plat)/$(conf)/$(lib))))))
# Generate Bullet binary copy targets for VS2011.
$(foreach plat,32 64,$(foreach conf,Debug Release,$(foreach lib,$(BULLET_LIBS_VS),$(eval $(call COPYFILE,$(VS2011_BUILD_DIR)/x86/$(plat)/$(conf)/bin/$(lib),$(PLAT_VS2011)/lib/x86/$(plat)/$(conf)/$(lib))))))

# Generate dependencies to have Xcode build the libraries.
$(foreach lib,$(BULLET_LIBS_IOS),$(eval $(call GENDEPS,$(IOS_DEV_BUILD_DIR)/$(lib),bullet.build.ios.dev)))
$(foreach lib,$(BULLET_LIBS_IOS),$(eval $(call GENDEPS,$(IOS_SIM_BUILD_DIR)/$(lib),bullet.build.ios.dev)))
$(foreach lib,$(BULLET_LIBS_OSX),$(eval $(call GENDEPS,$(OSX_BUILD_DIR)/$(lib),bullet.build.osx)))


##########
# LibPNG #
#######################################################################
##   > make libpng         Build and install the LibPNG library (optional).
libpng: libpng.uncompress libpng.build libpng.install

libpng.download: always src/$(LIBPNG_PACKAGE)

src/$(LIBPNG_PACKAGE): src/.d
	cd src/ && $(DOWNLOAD) $(LIBPNG_URL)

libpng.uncompress: $(LIBPNG_DIR)

$(LIBPNG_DIR): src/$(LIBPNG_PACKAGE)
	$(MAKE) src/libpng-$(LIBPNG_VERSION)/
	$(MV)   src/libpng-$(LIBPNG_VERSION)/ $@
	$(TOUCH) $@


#######
# Lua #
#######################################################################
##   > make lua            Build and install the LibPNG library (required).
lua: lua.uncompress lua.build lua.install

lua.download: always src/$(LUA_PACKAGE)

src/$(LUA_PACKAGE): src/.d
	cd src/ && $(DOWNLOAD) $(LUA_URL)

lua.uncompress: $(LUA_DIR)

$(LUA_DIR): src/$(LUA_PACKAGE)
	$(MAKE) src/lua-$(LUA_VERSION)/
	$(MV)   src/lua-$(LUA_VERSION)/ $@
	$(TOUCH) $@


lua.build.ios.dev: always
	cd $(XCODE_DIR) && $(XCODEBUILD) -sdk "$(SDK_IOS_DEV)" -project "Lua.xcodeproj" -target "Lua (iOS)"
lua.build.ios.sim: always
	cd $(XCODE_DIR) && $(XCODEBUILD) -sdk "$(SDK_IOS_SIM)" -project "Lua.xcodeproj" -target "Lua (iOS)"
lua.build.osx:
	cd $(XCODE_DIR) && $(XCODEBUILD) -sdk "$(SDK_OSX)" -project "Lua.xcodeproj" -target "Lua (OSX)" -target "Lua CLI (OSX)"
lua.build.vs2010: $(foreach p,32,$(foreach c,dbg rel,lua.build.vs2010.$(p).$(c)))
lua.build.vs2010.32.dbg: always
	cd $(VS2010_DIR) && $(MSBUILD) /m $(VS2010_FILE) /t:Lua /p:Platform=32-bit,Configuration=Debug
lua.build.vs2010.32.rel: always
	cd $(VS2010_DIR) && $(MSBUILD) /m $(VS2010_FILE) /t:Lua /p:Platform=32-bit,Configuration=Release
lua.build.vs2010.64.dbg: always
	cd $(VS2010_DIR) && $(MSBUILD) /m $(VS2010_FILE) /t:Lua /p:Platform=64-bit,Configuration=Debug
lua.build.vs2010.64.rel: always
	cd $(VS2010_DIR) && $(MSBUILD) /m $(VS2010_FILE) /t:Lua /p:Platform=64-bit,Configuration=Release
lua.build.vs2011: $(foreach p,32 64,$(foreach c,dbg rel,lua.build.vs2011.$(p).$(c)))
lua.build.vs2011.32.dbg: always
	cd $(VS2011_DIR) && $(MSBUILD) /m $(VS2011_FILE) /t:Lua /p:Platform=32-bit,Configuration=Debug
lua.build.vs2011.32.rel: always
	cd $(VS2011_DIR) && $(MSBUILD) /m $(VS2011_FILE) /t:Lua /p:Platform=32-bit,Configuration=Release
lua.build.vs2011.64.dbg: always
	cd $(VS2011_DIR) && $(MSBUILD) /m $(VS2011_FILE) /t:Lua /p:Platform=64-bit,Configuration=Debug
lua.build.vs2011.64.rel: always
	cd $(VS2011_DIR) && $(MSBUILD) /m $(VS2011_FILE) /t:Lua /p:Platform=64-bit,Configuration=Release


lua.install.binaries.ios.dev: lua.build.ios.dev
lua.install.binaries.ios.sim: lua.build.ios.sim
lua.install.binaries.osx: lua.build.osx
lua.install.binaries.vs2010: $(call VSLIBS,32,$(LUA_LIBS_VS),$(PLAT_VS2010))
lua.install.binaries.vs2011: $(call VSLIBS,32 64,$(LUA_LIBS_VS),$(PLAT_VS2011))

lua.install.headers.ios.dev: $(LUA_HEADERS:%=$(PLAT_IOS_DEV)/include/%)
lua.install.headers.ios.sim: $(LUA_HEADERS:%=$(PLAT_IOS_SIM)/include/%)
lua.install.headers.osx: $(LUA_HEADERS:%=$(PLAT_OSX)/include/%)
lua.install.headers.vs2010: $(LUA_HEADERS:%=$(PLAT_VS2010)/include/%)
lua.install.headers.vs2011: $(LUA_HEADERS:%=$(PLAT_VS2011)/include/%)

# Generate all of the lua.install.$(p) targets.
$(foreach p,$(ALL_PLATS),$(eval $(call SUBMAKE,lua.install.$(p),lua.uncompress,lua.install.headers.$(p))))

# Generate Lua header copy targets for all of the platforms.
$(foreach d,$(ALL_PLAT_DIRS),$(foreach f,$(LUA_HEADERS),$(eval $(call COPYFILE,$(LUA_SRC_DIR)/$(f),$(d)/include/$(f)))))

# Generate Lua binary copy targets for VS2010.
$(foreach plat,32,$(foreach conf,Debug Release,$(foreach lib,$(LUA_LIBS_VS),$(eval $(call COPYFILE,$(VS2010_BUILD_DIR)/x86/$(plat)/$(conf)/bin/$(lib),$(PLAT_VS2010)/lib/x86/$(plat)/$(conf)/$(lib))))))

# Generate Lua binary copy targets for VS2011.
$(foreach plat,32 64,$(foreach conf,Debug Release,$(foreach lib,$(LUA_LIBS_VS),$(eval $(call COPYFILE,$(VS2011_BUILD_DIR)/x86/$(plat)/$(conf)/bin/$(lib),$(PLAT_VS2011)/lib/x86/$(plat)/$(conf)/$(lib))))))


##################
# Ogg and Vorbis #
#######################################################################
##   > make oggvorbis      Build and install the Ogg and Vorbis libraries (opt?).
oggvorbis: oggvorbis.uncompress oggvorbis.build oggvorbis.install

oggvorbis.download: always src/$(OGG_PACKAGE) src/$(VORBIS_PACKAGE)

src/$(OGG_PACKAGE): src/.d
	cd src/ && $(DOWNLOAD) $(OGG_URL)

src/$(VORBIS_PACKAGE): src/.d
	cd src/ && $(DOWNLOAD) $(VORBIS_URL)

oggvorbis.uncompress: $(OGG_DIR) $(VORBIS_DIR)

$(OGG_DIR): src/$(OGG_PACKAGE)
	$(MAKE) src/libogg-$(OGG_VERSION)/
	$(MV)   src/libogg-$(OGG_VERSION)/ $@
	$(TOUCH) $@

$(VORBIS_DIR): src/$(VORBIS_PACKAGE)
	$(MAKE) src/libvorbis-$(VORBIS_VERSION)/
	$(MV)   src/libvorbis-$(VORBIS_VERSION)/ $@
	$(TOUCH) $@


oggvorbis.build.ios.dev: always
	cd $(XCODE_DIR) && $(XCODEBUILD) -sdk "$(SDK_IOS_DEV)" -project "OggVorbis.xcodeproj" -target "All (iOS)"
oggvorbis.build.ios.sim: always
	cd $(XCODE_DIR) && $(XCODEBUILD) -sdk "$(SDK_IOS_SIM)" -project "OggVorbis.xcodeproj" -target "All (iOS)"
oggvorbis.build.osx:
	cd $(XCODE_DIR) && $(XCODEBUILD) -sdk "$(SDK_OSX)" -project "OggVorbis.xcodeproj" -target "All (OSX)"
oggvorbis.build.vs2010:
	@echo TODO: $@
oggvorbis.build.vs2011: $(foreach l,ogg vorbis,$(l).build.vs2011)
ogg.build.vs2011: $(foreach p,32 64,$(foreach c,dbg rel,ogg.build.vs2011.$(p).$(c)))
ogg.build.vs2011.32.dbg: always
	cd $(VS2011_DIR) && $(MSBUILD) /m $(VS2011_FILE) /t:Ogg /p:Platform=32-bit,Configuration=Debug
ogg.build.vs2011.32.rel: always
	cd $(VS2011_DIR) && $(MSBUILD) /m $(VS2011_FILE) /t:Ogg /p:Platform=32-bit,Configuration=Release
ogg.build.vs2011.64.dbg: always
	cd $(VS2011_DIR) && $(MSBUILD) /m $(VS2011_FILE) /t:Ogg /p:Platform=64-bit,Configuration=Debug
ogg.build.vs2011.64.rel: always
	cd $(VS2011_DIR) && $(MSBUILD) /m $(VS2011_FILE) /t:Ogg /p:Platform=64-bit,Configuration=Release
vorbis.build.vs2011: $(foreach p,32 64,$(foreach c,dbg rel,vorbis.build.vs2011.$(p).$(c)))
vorbis.build.vs2011.32.dbg: always
	cd $(VS2011_DIR) && $(MSBUILD) /m $(VS2011_FILE) /t:Vorbis /p:Platform=32-bit,Configuration=Debug
vorbis.build.vs2011.32.rel: always
	cd $(VS2011_DIR) && $(MSBUILD) /m $(VS2011_FILE) /t:Vorbis /p:Platform=32-bit,Configuration=Release
vorbis.build.vs2011.64.dbg: always
	cd $(VS2011_DIR) && $(MSBUILD) /m $(VS2011_FILE) /t:Vorbis /p:Platform=64-bit,Configuration=Debug
vorbis.build.vs2011.64.rel: always
	cd $(VS2011_DIR) && $(MSBUILD) /m $(VS2011_FILE) /t:Vorbis /p:Platform=64-bit,Configuration=Release


oggvorbis.install.binaries.ios.dev: oggvorbis.build.ios.dev
oggvorbis.install.binaries.ios.sim: oggvorbis.build.ios.sim
oggvorbis.install.binaries.osx: oggvorbis.build.osx
oggvorbis.install.binaries.vs2010:
	@echo TODO: $@
oggvorbis.install.binaries.vs2011:
	@echo TODO: $@
oggvorbis.install.binaries.vs2011: $(call VSLIBS,32 64,$(OGGVORBIS_LIBS_VS),$(PLAT_VS2011))

oggvorbis.install.headers.ios.dev: $(OGGVORBIS_HEADERS:%=$(PLAT_IOS_DEV)/include/%)
oggvorbis.install.headers.ios.sim: $(OGGVORBIS_HEADERS:%=$(PLAT_IOS_SIM)/include/%)
oggvorbis.install.headers.osx: $(OGGVORBIS_HEADERS:%=$(PLAT_OSX)/include/%)
oggvorbis.install.headers.vs2010: $(OGGVORBIS_HEADERS:%=$(PLAT_VS2010)/include/%)
oggvorbis.install.headers.vs2011: $(OGGVORBIS_HEADERS:%=$(PLAT_VS2011)/include/%)

oggvorbis.install.headers: oggvorbis.uncompress
ifeq (,$(OGGVORBIS_HEADERS))  # Headers not found this run.
	[ $(MAKELEVEL) -le 1 ] && $(MAKE) oggvorbis.install.headers  # Prevent infinite loop.
endif

# Generate OggVorbis header copy targets for all of the platforms.
$(foreach d,$(ALL_PLAT_DIRS),$(foreach f,$(OGG_HEADERS),$(eval $(call COPYFILE,$(OGG_SRC_DIR)/include/$(f),$(d)/include/$(f)))))
$(foreach d,$(ALL_PLAT_DIRS),$(foreach f,$(VORBIS_HEADERS),$(eval $(call COPYFILE,$(VORBIS_SRC_DIR)/include/$(f),$(d)/include/$(f)))))

# Generate OggVobis binary copy targets for VS2011.
$(foreach plat,32 64,$(foreach conf,Debug Release,$(foreach lib,$(OGGVORBIS_LIBS_VS),$(eval $(call COPYFILE,$(VS2011_BUILD_DIR)/x86/$(plat)/$(conf)/bin/$(lib),$(PLAT_VS2011)/lib/x86/$(plat)/$(conf)/$(lib))))))


########
# ZLib #
#######################################################################
##   > make zlib           Build and install ZLib library (optional?).
zlib: zlib.uncompress zlib.build zlib.install

zlib.download: always src/$(ZLIB_PACKAGE)

src/$(ZLIB_PACKAGE): src/.d
	cd src/ && $(DOWNLOAD) $(ZLIB_URL)

zlib.uncompress: $(ZLIB_DIR)

$(ZLIB_DIR): src/$(ZLIB_PACKAGE)
	$(MAKE) src/zlib-$(ZLIB_VERSION)/
	$(MV)   src/zlib-$(ZLIB_VERSION)/ $@
	$(TOUCH) $@

zlib.build.ios.dev: always
	@$(NOTHING)
zlib.build.ios.sim: always
	@$(NOTHING)
zlib.build.osx:
	@$(NOTHING)
zlib.build.vs2010: $(foreach p,32,$(foreach c,dbg rel,zlib.build.vs2010.$(p).$(c)))
zlib.build.vs2010.32.dbg: always
	cd $(VS2010_DIR) && $(MSBUILD) /m $(VS2010_FILE) /t:ZLib /p:Platform=32-bit,Configuration=Debug
zlib.build.vs2010.32.rel: always
	cd $(VS2010_DIR) && $(MSBUILD) /m $(VS2010_FILE) /t:ZLib /p:Platform=32-bit,Configuration=Release
zlib.build.vs2010.64.dbg: always
	cd $(VS2010_DIR) && $(MSBUILD) /m $(VS2010_FILE) /t:ZLib /p:Platform=64-bit,Configuration=Debug
zlib.build.vs2010.64.rel: always
	cd $(VS2010_DIR) && $(MSBUILD) /m $(VS2010_FILE) /t:ZLib /p:Platform=64-bit,Configuration=Release
zlib.build.vs2011: $(foreach p,32 64,$(foreach c,dbg rel,zlib.build.vs2011.$(p).$(c)))
zlib.build.vs2011.32.dbg: always
	cd $(VS2011_DIR) && $(MSBUILD) /m $(VS2011_FILE) /t:ZLib /p:Platform=32-bit,Configuration=Debug
zlib.build.vs2011.32.rel: always
	cd $(VS2011_DIR) && $(MSBUILD) /m $(VS2011_FILE) /t:ZLib /p:Platform=32-bit,Configuration=Release
zlib.build.vs2011.64.dbg: always
	cd $(VS2011_DIR) && $(MSBUILD) /m $(VS2011_FILE) /t:ZLib /p:Platform=64-bit,Configuration=Debug
zlib.build.vs2011.64.rel: always
	cd $(VS2011_DIR) && $(MSBUILD) /m $(VS2011_FILE) /t:ZLib /p:Platform=64-bit,Configuration=Release


zlib.install.binaries.ios.dev: lua.build.ios.dev
zlib.install.binaries.ios.sim: lua.build.ios.sim
zlib.install.binaries.osx: lua.build.osx
zlib.install.binaries.vs2010: $(call VSLIBS,32,$(ZLIB_LIBS_VS),$(PLAT_VS2010))
zlib.install.binaries.vs2011: $(call VSLIBS,32 64,$(ZLIB_LIBS_VS),$(PLAT_VS2011))

zlib.install.headers.ios.dev: $(ZLIB_HEADERS:%=$(PLAT_IOS_DEV)/include/%)
zlib.install.headers.ios.sim: $(ZLIB_HEADERS:%=$(PLAT_IOS_SIM)/include/%)
zlib.install.headers.osx: $(ZLIB_HEADERS:%=$(PLAT_OSX)/include/%)
zlib.install.headers.vs2010: $(ZLIB_HEADERS:%=$(PLAT_VS2010)/include/%)
zlib.install.headers.vs2011: $(ZLIB_HEADERS:%=$(PLAT_VS2011)/include/%)

# Generate all of the zlib.install.$(p) targets.
$(foreach p,$(ALL_PLATS),$(eval $(call SUBMAKE,zlib.install.$(p),zlib.uncompress,zlib.install.headers.$(p))))

# Generate ZLib header copy targets for all of the platforms.
$(foreach d,$(ALL_PLAT_DIRS),$(foreach f,$(ZLIB_HEADERS),$(eval $(call COPYFILE,$(ZLIB_SRC_DIR)/$(f),$(d)/include/$(f)))))

# Generate ZLib binary copy targets for VS2010.
$(foreach plat,32,$(foreach conf,Debug Release,$(foreach lib,$(ZLIB_LIBS_VS),$(eval $(call COPYFILE,$(VS2010_BUILD_DIR)/x86/$(plat)/$(conf)/bin/$(lib),$(PLAT_VS2010)/lib/x86/$(plat)/$(conf)/$(lib))))))

# Generate ZLib binary copy targets for VS2011.
$(foreach plat,32 64,$(foreach conf,Debug Release,$(foreach lib,$(ZLIB_LIBS_VS),$(eval $(call COPYFILE,$(VS2011_BUILD_DIR)/x86/$(plat)/$(conf)/bin/$(lib),$(PLAT_VS2011)/lib/x86/$(plat)/$(conf)/$(lib))))))


#########
# Utils #
#######################################################################

# Directory-creating rules.
%/.d:
	$(MKDIR) -p $(dir $@)
	$(TOUCH) $@

# Tar.gz extraction rule.
%/: %.tar.gz
	cd $(dir $^) && $(TAR) xf $(notdir $^)

# Tar.bz2 extraction rule.
%/: %.tar.bz2
	cd $(dir $^) && $(TAR) xf $(notdir $^)

# Zip extraction rule.
%/: %.zip
	cd $(dir $^) && $(UNZIP) -q $(notdir $^)

##   > make showsdks       Display the SDKs available in Xcode.
showsdks:
	$(XCODEBUILD) -showsdks

##   > make dbg            Print various debug information.
dbg:
	@echo PLATFORM=$(PLATFORM)=
	@echo TARGETS=$(TARGETS)=
	@echo PLATS=$(PLATS)=
	@echo CURRENT_MAKEFILE=$(CURRENT_MAKEFILE)=
	@echo CURRENT_DIR=$(CURRENT_DIR)=
	@echo DEV_ROOT=$(DEV_ROOT)=
	@echo DEV_THIRDPARTY=$(DEV_THIRDPARTY)=
	@echo ""
	@echo "BULLET_VERSION >$(BULLET_VERSION)<"
	@echo "   LUA_VERSION >$(LUA_VERSION)<"
	@echo "   OGG_VERSION >$(OGG_VERSION)<"
	@echo "VORBIS_VERSION >$(VORBIS_VERSION)<"
	@echo "  ZLIB_VERSION >$(ZLIB_VERSION)<"
	@echo ""
	@echo "  ALL_PLAT_DIRS >$(ALL_PLAT_DIRS)"
	@echo "    ALL_TARGETS >$(ALL_TARGETS)"
#	@echo " BULLET_HEADERS >$(BULLET_HEADERS)<"
#	@echo "    LUA_HEADERS >$(LUA_HEADERS)<"
#	@echo "OGGVORBIS_HEADERS >$(OGGVORBIS_HEADERS)<"

## Where $(p) can be one of the following platforms:
##   osx     - Mac OS X.
##   ios     - Shorthand for both 'ios.dev' and 'ios.sim'.
##   ios.dev - iOS devices (iPod Touch/iPhone/iPad).
##   ios.sim - iOS simulator (included with Xcode).
##   vs2010  - Windows using Visual Studio 2010.
##   vs2011  - Windows using Visual Studio 2011.
## ===============================================================================
## Notes:
##  * Most commands have the above targets variants:
##      > make bullet.vs2011
##  * The install commands have install.headers and install.binaries variants.
##  * You can override the targets compiled for with the TARGETS variable:
##      > make TARGETS="osx ios.dev" lua.install.headers
##    would be equivalent to calling:
##      > make lua.install.headers.osx lua.install.headers.ios.dev
