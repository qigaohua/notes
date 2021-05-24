TOP_DIR = ../..
MODULE_NAME = app1



CFLAGS +=
CPPFLAGS += -I. -I$(INC_PATH)

LDFLAGS += -L$(BUILD_LIBS_PATH) -lmodlue1 -lmodlue2 -lmodlue3 -lsrc


SRC_FILES=$(wildcard *.c)
SRC_OBJ=$(SRC_FILES:.c=.o)
APP=app1


include $(TOP_DIR)/Makefile.rule


