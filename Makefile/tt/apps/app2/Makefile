TOP_DIR = ../..
MODULE_NAME = app2


CFLAGS +=
CPPFLAGS += -I. -I$(INC_PATH)

# LDFLAGS += -L$(BUILD_LIBS_PATH) -lmodlue1 -lmodlue2 -lmodlue3 -lsrc
LDFLAGS += -L$(BUILD_LIBS_PATH)  -lsrc


SRC_FILES=$(wildcard *.c)
SRC_OBJ=$(SRC_FILES:.c=.o)
APP=app2

# app2: $(SRC_OBJ)
# 	$(CC) $@ $^ $(LDFLAGS)
# 	$(MV) $@ $(BIN_PATH)

include $(TOP_DIR)/Makefile.rule


