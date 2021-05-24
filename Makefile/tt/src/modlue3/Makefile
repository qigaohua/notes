TOP_DIR = ../..
MODULE_NAME = modlue3

# CFLAGS CPPFLAGS 是内部变量，编译时自动加上，还有个TARGET_ARCH
CFLAGS +=
CPPFLAGS += -I. -I$(INC_PATH)

# LDFALGS 这个是自己定义的，后面要加上$(LDFALGS)
LDFALGS +=


SRC_FILES=$(wildcard *.c)
SRC_OBJ=$(SRC_FILES:.c=.o)
# SRC_BIN=target_bin
APP_LIB=libmodlue3.a

include $(TOP_DIR)/Makefile.rule


