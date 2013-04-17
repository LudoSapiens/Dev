# Requires having the following environment variables:
#   OPENNI_DIR: The root directory for the OpenNI project.

OSTYPE := $(shell uname -s)

ifeq ("$(OSTYPE)","Darwin")
OPENNI_PLAT := Linux-x86
endif

BIN_DIR = Bin

INC_DIRS = $(OPENNI_DIR)/Include ../../Projects

SRC_FILES = \
	Source/main.cpp \
	Source/SceneDrawer.cpp	

EXE_NAME = LudoTracker

ifneq "$(GLES)" "1"
ifeq ("$(OSTYPE)","Darwin")
	LDFLAGS += -framework OpenGL -framework GLUT
else
	USED_LIBS += glut
endif
else
	DEFINES += USE_GLES
	USED_LIBS += GLES_CM IMGegl srv_um
	SRC_FILES += Source/opengles.cpp
endif

USED_LIBS += OpenNI

include $(OPENNI_DIR)/Platform/$(OPENNI_PLAT)/Build/Common/CommonCppMakefile

dbg:
	echo $(OPENNI_DIR)
