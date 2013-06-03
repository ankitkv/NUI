include ../build/CommonDefs.mak

BIN_DIR = ../bin

INC_DIRS = \
	$(OPENNI2_INCLUDE)

LIB_DIRS = \
	$(OPENNI2_REDIST)

SRC_FILES = *.cpp

USED_LIBS += OpenNI2

LIB_NAME = nui

CFLAGS += -Wall

LDFLAGS += -lopencv_calib3d -lopencv_core -lopencv_imgproc

include ../build/CommonCppMakefile
