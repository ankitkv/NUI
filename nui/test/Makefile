include ../build/CommonDefs.mak

BIN_DIR = ../bin

INC_DIRS = \
	$(OPENNI2_INCLUDE) \
	../libnui

LIB_DIRS = \
	$(OPENNI2_REDIST)

SRC_FILES = *.cpp

USED_LIBS += OpenNI2 nui

EXE_NAME = test

CFLAGS += -Wall

LDFLAGS = -lX11

include ../build/CommonCppMakefile
