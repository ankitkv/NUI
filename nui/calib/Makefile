include ../build/CommonDefs.mak

BIN_DIR = ../bin

INC_DIRS = \
	$(OPENNI2_INCLUDE) \
	../libnui

LIB_DIRS = \
	$(OPENNI2_REDIST)

SRC_FILES = *.cpp

CFLAGS += -DUNIX -DGLX_GLXEXT_LEGACY
USED_LIBS += glut GL

USED_LIBS += OpenNI2 nui

EXE_NAME = calib

CFLAGS += -Wall

LDFLAGS += -lX11

include ../build/CommonCppMakefile
